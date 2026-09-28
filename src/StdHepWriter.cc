#include "StdHepWriter.hh"

#include "ParticleBlock.hh"
#include "StdHepFile.hh"

#include "G4AnalysisManager.hh"
#include "G4AutoLock.hh"
#include "G4Exception.hh"
#include "G4GenericMessenger.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

#include <vector>

namespace
{
G4Mutex stdhepMutex = G4MUTEX_INITIALIZER;
}

StdHepWriter* StdHepWriter::Instance()
{
  // Never destroyed: its messenger must not outlive the UI manager.
  static StdHepWriter* instance = new StdHepWriter;
  return instance;
}

StdHepWriter::StdHepWriter()
{
  fMessenger = new G4GenericMessenger(this, "/tgt/stdhep/", "StdHep output of exit_* particles");
  auto& cmdWrite = fMessenger->DeclareProperty("write", fEnabled,
                                               "Write an StdHep file (default false)");
  auto& cmdName = fMessenger->DeclareProperty(
    "fileName", fFileName, "StdHep file name (default: ROOT file name with .stdhep)");
  // The file is handled on the master; do not pass these commands to workers.
  cmdWrite.command->SetToBeBroadcasted(false);
  cmdName.command->SetToBeBroadcasted(false);
}

void StdHepWriter::BeginOfRun(G4int nEventsExpected)
{
  if (!fEnabled) return;
#ifdef GBG_WITH_STDHEP
  G4String name = fFileName;
  if (name.empty()) {
    name = G4AnalysisManager::Instance()->GetFileName();
    if (name.size() > 5 && name.substr(name.size() - 5) == ".root") {
      name = name.substr(0, name.size() - 5);
    }
    name += ".stdhep";
  }
  if (fFile == nullptr) fFile = new StdHepFile;
  if (!fFile->Open(name, "geant4_beam_generator", nEventsExpected)) {
    G4ExceptionDescription msg;
    msg << "Cannot open StdHep file " << name << "; no StdHep output for this run.";
    G4Exception("StdHepWriter::BeginOfRun", "STDHEP001", JustWarning, msg);
    return;
  }
  fNTruncated = 0;
  fNEmpty = 0;
  fOpen = true;
  G4cout << "=== StdHep output: " << name << " ===" << G4endl;
#else
  (void)nEventsExpected;
  G4Exception("StdHepWriter::BeginOfRun", "STDHEP000", JustWarning,
              "/tgt/stdhep/write is set, but this program was built without StdHep "
              "support (WITH_STDHEP=OFF). No StdHep output.");
#endif
}

void StdHepWriter::WriteEvent(G4int eventID, const ParticleBlock& b)
{
#ifdef GBG_WITH_STDHEP
  if (!fOpen) return;

  // Convert outside the lock
  const std::size_t n = b.Size();
  std::vector<StdHepParticle> particles(n);
  constexpr G4double toGeV = MeV / GeV;              // block holds MeV
  const G4double nsToMmOverC = (c_light * ns) / mm;  // block holds ns
  for (std::size_t i = 0; i < n; ++i) {
    StdHepParticle& p = particles[i];
    p.status = 1;
    p.pdg = b.pdg[i];
    p.px = b.px[i] * toGeV;
    p.py = b.py[i] * toGeV;
    p.pz = b.pz[i] * toGeV;
    p.m = b.mass[i] * toGeV;
    p.E = (b.E[i] + b.mass[i]) * toGeV;  // E holds kinetic energy
    p.x = b.x[i];
    p.y = b.y[i];
    p.z = b.z[i];
    p.t = b.t[i] * nsToMmOverC;
  }

  G4AutoLock lock(&stdhepMutex);
  if (!fOpen) return;
  const G4int written = fFile->WriteEvent(eventID, particles);
  if (written < 0) {
    G4ExceptionDescription msg;
    msg << "Error writing event " << eventID << " to " << fFile->FileName();
    G4Exception("StdHepWriter::WriteEvent", "STDHEP002", JustWarning, msg);
  }
  else if (written == 0) {
    ++fNEmpty;
  }
  else if (static_cast<std::size_t>(written) < n) {
    if (fNTruncated == 0) {
      G4ExceptionDescription msg;
      msg << "Event " << eventID << " has " << n << " particles; StdHep holds at most "
          << StdHepFile::MaxParticles() << ". Extra particles are dropped from the StdHep "
          << "file (not from the ROOT file). Further occurrences are counted, not reported.";
      G4Exception("StdHepWriter::WriteEvent", "STDHEP003", JustWarning, msg);
    }
    ++fNTruncated;
  }
#else
  (void)eventID;
  (void)b;
#endif
}

void StdHepWriter::EndOfRun()
{
#ifdef GBG_WITH_STDHEP
  if (!fOpen) return;
  G4AutoLock lock(&stdhepMutex);
  fOpen = false;
  G4cout << "=== StdHep output: " << fFile->EventsWritten() << " events written to "
         << fFile->FileName();
  if (fNEmpty > 0) G4cout << ", " << fNEmpty << " empty events not written";
  if (fNTruncated > 0) G4cout << ", " << fNTruncated << " events truncated";
  G4cout << " ===" << G4endl;
  fFile->Close();
#endif
}
