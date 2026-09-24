#include "EventAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"

#include <cmath>

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdep = 0.;
  fNExit = 0;
  fNCreated = 0;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  const auto* vtx = event->GetPrimaryVertex(0);
  const auto* prim = vtx->GetPrimary(0);
  const G4ThreeVector dir = prim->GetMomentumDirection();

  auto* am = G4AnalysisManager::Instance();
  am->FillNtupleIColumn(0, 0, event->GetEventID());
  am->FillNtupleDColumn(0, 1, prim->GetKineticEnergy() / MeV);
  am->FillNtupleDColumn(0, 2, std::atan2(dir.x(), dir.z()));
  am->FillNtupleDColumn(0, 3, std::atan2(dir.y(), dir.z()));
  am->FillNtupleDColumn(0, 4, vtx->GetX0() / mm);
  am->FillNtupleDColumn(0, 5, vtx->GetY0() / mm);
  am->FillNtupleDColumn(0, 6, vtx->GetZ0() / mm);
  am->FillNtupleDColumn(0, 7, fEdep / MeV);
  am->FillNtupleIColumn(0, 8, fNExit);
  am->FillNtupleIColumn(0, 9, fNCreated);
  am->AddNtupleRow(0);
}
