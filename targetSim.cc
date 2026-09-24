// targetSim: electrons through a thin target, recording everything that
// leaves the target and (optionally) every secondary created in it.
//
// Usage: targetSim [macro] [-t nThreads] [-p physicsList] [-s seed]
//   no macro -> interactive session with visualisation (init_vis.mac)

#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

#include "G4EmParameters.hh"
#include "G4PhysListFactory.hh"
#include "G4RunManagerFactory.hh"
#include "G4UIExecutive.hh"
#include "G4UImanager.hh"
#include "G4VModularPhysicsList.hh"
#include "G4VisExecutive.hh"
#include "Randomize.hh"

#include <cstdlib>
#include <string>

namespace
{
void PrintUsage()
{
  G4cout << "Usage: targetSim [macro] [-t nThreads] [-p physicsList] [-s seed]\n"
         << "  no macro       : interactive session with visualisation\n"
         << "  -t nThreads    : number of worker threads (MT builds)\n"
         << "  -p physicsList : reference list name, default FTFP_BERT_EMZ\n"
         << "  -s seed        : random seed" << G4endl;
}
}  // namespace

int main(int argc, char** argv)
{
  G4String macro;
  G4String physListName = "FTFP_BERT_EMZ";
  G4int nThreads = 0;
  long seed = 0;

  for (G4int i = 1; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg == "-t" && i + 1 < argc) nThreads = std::atoi(argv[++i]);
    else if (arg == "-p" && i + 1 < argc) physListName = argv[++i];
    else if (arg == "-s" && i + 1 < argc) seed = std::atol(argv[++i]);
    else if (arg == "-h" || arg == "--help") { PrintUsage(); return 0; }
    else if (!arg.empty() && arg[0] != '-') macro = arg;
    else { PrintUsage(); return 1; }
  }

  G4UIExecutive* ui = nullptr;
  if (macro.empty()) ui = new G4UIExecutive(argc, argv);

  if (seed != 0) G4Random::setTheSeed(seed);

  auto* runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);
  if (nThreads > 0) runManager->SetNumberOfThreads(nThreads);

  // Physics list
  G4PhysListFactory factory;
  G4VModularPhysicsList* physics = nullptr;
  if (factory.IsReferencePhysList(physListName)) {
    physics = factory.GetReferencePhysList(physListName);
  }
  if (physics == nullptr) {
    G4cerr << "Unknown or unusable physics list: " << physListName << G4endl;
    return 1;
  }
  runManager->SetUserInitialization(physics);

  // Use the individual gamma processes (phot, compt, conv, Rayl) instead of the
  // combined "general" gamma process, so creator-process names stay specific.
  G4EmParameters::Instance()->SetGeneralProcessActive(false);

  auto* detector = new DetectorConstruction;
  runManager->SetUserInitialization(detector);
  runManager->SetUserInitialization(new ActionInitialization(detector));

  auto* uiManager = G4UImanager::GetUIpointer();
  G4VisManager* visManager = nullptr;

  if (ui == nullptr) {
    uiManager->ApplyCommand("/control/execute " + macro);
  }
  else {
    visManager = new G4VisExecutive("Quiet");
    visManager->Initialize();
    uiManager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;
  return 0;
}
