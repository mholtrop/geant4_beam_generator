#include "SteppingAction.hh"

#include "DetectorConstruction.hh"
#include "EventAction.hh"

#include "G4GenericMessenger.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

SteppingAction::SteppingAction(const DetectorConstruction* detector, EventAction* eventAction)
  : fDetector(detector), fEventAction(eventAction)
{
  fMessenger = new G4GenericMessenger(this, "/tgt/exit/", "Target-exit handling");
  fMessenger->DeclareProperty("kill", fKillOnExit,
                              "Kill tracks after they leave the target (default true)");
}

SteppingAction::~SteppingAction()
{
  delete fMessenger;
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const G4StepPoint* pre = step->GetPreStepPoint();
  if (pre->GetPhysicalVolume() != fDetector->GetTargetPV()) return;

  fEventAction->AddEdep(step->GetTotalEnergyDeposit());

  const G4StepPoint* post = step->GetPostStepPoint();
  if (post->GetStepStatus() != fGeomBoundary) return;  // still inside the target

  G4Track* track = step->GetTrack();
  fEventAction->Exit().Add(track, fEventAction->GetAncestor(track->GetTrackID()),
                           post->GetKineticEnergy(), post->GetMomentum(),
                           post->GetPosition(), track->GetVertexPosition());

  if (fKillOnExit) track->SetTrackStatus(fStopAndKill);
}
