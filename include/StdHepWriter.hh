#ifndef StdHepWriter_h
#define StdHepWriter_h 1

// Writes the exit_* particles of every written event (after the output
// filter) to an StdHep file: one StdHep event per Geant4 event (bunch),
// NEVHEP = Geant4 event ID. One file per run, shared by all worker threads.
//
// Per particle: ISTHEP = 1, IDHEP = PDG code, no mother/daughter links,
// PHEP = (px, py, pz, E_total, mass) in GeV, VHEP = exit point (mm) and
// global time at the exit point (mm/c).
//
// Commands (/tgt/stdhep/, executed on the master only):
//   write true|false   enable StdHep output (default false)
//   fileName name      output file; default: ROOT file name with .stdhep
//
// If the program is built without StdHep support (-DWITH_STDHEP=OFF) the
// commands exist but enabling the output gives a warning.

#include "globals.hh"

#include <atomic>

class G4GenericMessenger;
class StdHepFile;
struct ParticleBlock;

class StdHepWriter
{
  public:
    static StdHepWriter* Instance();

    void BeginOfRun(G4int nEventsExpected);  // master only
    void EndOfRun();                         // master only
    void WriteEvent(G4int eventID, const ParticleBlock& block);  // any thread

  private:
    StdHepWriter();
    ~StdHepWriter() = delete;

    G4bool fEnabled = false;
    G4String fFileName;
    std::atomic<bool> fOpen{false};
    StdHepFile* fFile = nullptr;
    G4int fNTruncated = 0;
    G4int fNEmpty = 0;
    G4GenericMessenger* fMessenger = nullptr;
};

#endif
