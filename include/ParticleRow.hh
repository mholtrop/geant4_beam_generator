#ifndef ParticleRow_h
#define ParticleRow_h 1

// Shared column layout of the "exit" (id 1) and "created" (id 2) ntuples.
//  0 event   1 track   2 parent   3 pdg   4 particle (string)  5 process (string)
//  6 E  (kinetic, MeV)      7-9  px, py, pz (MeV)
// 10-12 x, y, z (mm): where the row was recorded (exit point / creation point)
// 13-15 vx, vy, vz (mm): creation vertex of the track

#include "G4AnalysisManager.hh"
#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

inline void BookParticleNtuple(G4AnalysisManager* am, const G4String& name,
                               const G4String& title)
{
  am->CreateNtuple(name, title);
  am->CreateNtupleIColumn("event");
  am->CreateNtupleIColumn("track");
  am->CreateNtupleIColumn("parent");
  am->CreateNtupleIColumn("pdg");
  am->CreateNtupleSColumn("particle");
  am->CreateNtupleSColumn("process");
  am->CreateNtupleDColumn("E");
  am->CreateNtupleDColumn("px");
  am->CreateNtupleDColumn("py");
  am->CreateNtupleDColumn("pz");
  am->CreateNtupleDColumn("x");
  am->CreateNtupleDColumn("y");
  am->CreateNtupleDColumn("z");
  am->CreateNtupleDColumn("vx");
  am->CreateNtupleDColumn("vy");
  am->CreateNtupleDColumn("vz");
  am->FinishNtuple();
}

inline void FillParticleRow(G4int id, const G4Track* track, G4double ekin,
                            const G4ThreeVector& mom, const G4ThreeVector& pos,
                            const G4ThreeVector& vtx)
{
  auto* am = G4AnalysisManager::Instance();
  const G4int evt = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
  const auto* def = track->GetParticleDefinition();
  const auto* creator = track->GetCreatorProcess();

  am->FillNtupleIColumn(id, 0, evt);
  am->FillNtupleIColumn(id, 1, track->GetTrackID());
  am->FillNtupleIColumn(id, 2, track->GetParentID());
  am->FillNtupleIColumn(id, 3, def->GetPDGEncoding());
  am->FillNtupleSColumn(id, 4, def->GetParticleName());
  am->FillNtupleSColumn(id, 5, creator != nullptr ? creator->GetProcessName() : G4String("primary"));
  am->FillNtupleDColumn(id, 6, ekin / MeV);
  am->FillNtupleDColumn(id, 7, mom.x() / MeV);
  am->FillNtupleDColumn(id, 8, mom.y() / MeV);
  am->FillNtupleDColumn(id, 9, mom.z() / MeV);
  am->FillNtupleDColumn(id, 10, pos.x() / mm);
  am->FillNtupleDColumn(id, 11, pos.y() / mm);
  am->FillNtupleDColumn(id, 12, pos.z() / mm);
  am->FillNtupleDColumn(id, 13, vtx.x() / mm);
  am->FillNtupleDColumn(id, 14, vtx.y() / mm);
  am->FillNtupleDColumn(id, 15, vtx.z() / mm);
  am->AddNtupleRow(id);
}

#endif
