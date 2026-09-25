#include "RunAction.hh"

#include "EventAction.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"

RunAction::RunAction(EventAction* ev)
{
  auto* am = G4AnalysisManager::Instance();
  am->SetDefaultFileType("root");
  am->SetFileName("beam_generator");  // override with /analysis/setFileName
  am->SetNtupleMerging(true);     // one output file in MT mode
  am->SetVerboseLevel(1);

  // One row per event (= one beam bunch). Vector columns hold one entry per
  // particle.
  am->CreateNtuple("events", "One row per beam bunch");
  am->CreateNtupleIColumn("event");     // 0
  am->CreateNtupleIColumn("nPrimary");  // 1 beam electrons in this bunch
  am->CreateNtupleDColumn("Edep");      // 2 energy deposited in target [MeV]
  am->CreateNtupleIColumn("nExit");     // 3 particles leaving the target and passing the filter
  am->CreateNtupleIColumn("nCreated");  // 4 secondaries created (counted even if not recorded)
  am->CreateNtupleIColumn("nExitAll");  // 5 particles leaving the target, before the filter

  // Beam electrons: index i is track ID i+1
  am->CreateNtupleDColumn("prim_E", ev->PrimE());            // kinetic energy [MeV]
  am->CreateNtupleDColumn("prim_thetaX", ev->PrimThetaX());  // atan(px/pz) [rad]
  am->CreateNtupleDColumn("prim_thetaY", ev->PrimThetaY());  // atan(py/pz) [rad]
  am->CreateNtupleDColumn("prim_x", ev->PrimX());            // start position [mm]
  am->CreateNtupleDColumn("prim_y", ev->PrimY());
  am->CreateNtupleDColumn("prim_z", ev->PrimZ());

  ev->Exit().Book(am, "exit_");        // particles leaving the target and passing the filter
  ev->Created().Book(am, "created_");  // secondaries, at creation
  am->FinishNtuple();
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  G4AnalysisManager::Instance()->OpenFile();
}

void RunAction::EndOfRunAction(const G4Run*)
{
  auto* am = G4AnalysisManager::Instance();
  am->Write();
  am->CloseFile();
}
