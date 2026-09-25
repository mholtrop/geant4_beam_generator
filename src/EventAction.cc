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
  fNCreated = 0;
  fNExitAll = 0;
  fAncestor.clear();
  fPrimE.clear();
  fPrimThetaX.clear();
  fPrimThetaY.clear();
  fPrimX.clear();
  fPrimY.clear();
  fPrimZ.clear();
  fExit.Clear();
  fCreated.Clear();
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  // The generator makes one vertex with one particle per beam electron.
  const G4int nVtx = event->GetNumberOfPrimaryVertex();
  for (G4int i = 0; i < nVtx; ++i) {
    const auto* vtx = event->GetPrimaryVertex(i);
    for (const auto* prim = vtx->GetPrimary(); prim != nullptr; prim = prim->GetNext()) {
      const G4ThreeVector dir = prim->GetMomentumDirection();
      fPrimE.push_back(prim->GetKineticEnergy() / MeV);
      fPrimThetaX.push_back(std::atan2(dir.x(), dir.z()));
      fPrimThetaY.push_back(std::atan2(dir.y(), dir.z()));
      fPrimX.push_back(vtx->GetX0() / mm);
      fPrimY.push_back(vtx->GetY0() / mm);
      fPrimZ.push_back(vtx->GetZ0() / mm);
    }
  }

  if (!fFilter.KeepEmptyEvents() && fExit.Size() == 0) return;

  auto* am = G4AnalysisManager::Instance();
  am->FillNtupleIColumn(0, 0, event->GetEventID());
  am->FillNtupleIColumn(0, 1, static_cast<G4int>(fPrimE.size()));
  am->FillNtupleDColumn(0, 2, fEdep / MeV);
  am->FillNtupleIColumn(0, 3, static_cast<G4int>(fExit.Size()));
  am->FillNtupleIColumn(0, 4, fNCreated);
  am->FillNtupleIColumn(0, 5, fNExitAll);
  am->AddNtupleRow(0);
}
