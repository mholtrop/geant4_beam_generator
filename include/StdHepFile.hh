#ifndef StdHepFile_h
#define StdHepFile_h 1

// Thin C++ wrapper around the StdHep (mcfio/XDR) C library for writing.
// No Geant4 dependence. Not thread safe: the StdHep library works through
// global common blocks, so the caller must serialise all calls.
//
// Units follow the HEPEVT convention: momenta, energy and mass in GeV,
// vertex position in mm, vertex time in mm/c.

#include <string>
#include <vector>

struct StdHepParticle
{
    int status = 1;  // ISTHEP: 1 = final-state particle
    int pdg = 0;     // IDHEP
    double px = 0., py = 0., pz = 0., E = 0., m = 0.;  // PHEP [GeV]
    double x = 0., y = 0., z = 0., t = 0.;            // VHEP [mm, mm/c]
};

class StdHepFile
{
  public:
    StdHepFile() = default;
    ~StdHepFile() { Close(); }

    // nEventsExpected is passed to mcfio as the expected number of events.
    bool Open(const std::string& fileName, const std::string& title, int nEventsExpected);

    // Writes one event. Returns the number of particles written, which is
    // less than particles.size() if that exceeds MaxParticles(). Returns -1
    // on a write error. An event with no particles is not written (the
    // StdHep library skips empty events); 0 is returned.
    int WriteEvent(int eventNumber, const std::vector<StdHepParticle>& particles);

    void Close();

    bool IsOpen() const { return fOpen; }
    const std::string& FileName() const { return fFileName; }
    int EventsWritten() const { return fNWritten; }

    static int MaxParticles();  // NMXHEP of the StdHep library

  private:
    bool fOpen = false;
    std::string fFileName;
    int fNWritten = 0;
    static bool fMcfioInitDone;
    static constexpr int kStream = 0;  // StdHep stream index
};

#endif
