#include "RunAction.hh"

#include "ParticleRow.hh"

#include "G4AnalysisManager.hh"
#include "G4Run.hh"

RunAction::RunAction()
{
  auto* am = G4AnalysisManager::Instance();
  am->SetDefaultFileType("root");
  am->SetFileName("target_out");  // override with /analysis/setFileName
  am->SetNtupleMerging(true);     // one output file in MT mode
  am->SetVerboseLevel(1);

  // ntuple 0: one row per event
  am->CreateNtuple("primary", "Generated primary and event summary");
  am->CreateNtupleIColumn("event");     // 0
  am->CreateNtupleDColumn("E");         // 1 kinetic energy [MeV]
  am->CreateNtupleDColumn("thetaX");    // 2 atan(px/pz) [rad]
  am->CreateNtupleDColumn("thetaY");    // 3 atan(py/pz) [rad]
  am->CreateNtupleDColumn("x");         // 4 start position [mm]
  am->CreateNtupleDColumn("y");         // 5
  am->CreateNtupleDColumn("z");         // 6
  am->CreateNtupleDColumn("Edep");      // 7 energy deposited in target [MeV]
  am->CreateNtupleIColumn("nExit");     // 8 particles leaving the target
  am->CreateNtupleIColumn("nCreated");  // 9 secondaries created
  am->FinishNtuple();

  // ntuple 1: every particle leaving the target, at the exit point
  BookParticleNtuple(am, "exit", "Particles leaving the target");
  // ntuple 2: every secondary, at its creation point
  BookParticleNtuple(am, "created", "Secondaries at creation");
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
