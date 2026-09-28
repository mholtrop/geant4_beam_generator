#ifdef GBG_WITH_STDHEP

#include "StdHepFile.hh"

#include <algorithm>

extern "C" {
#include "stdhep.h"
#include "stdhep_mcfio.h"
}

bool StdHepFile::fMcfioInitDone = false;

int StdHepFile::MaxParticles()
{
  return NMXHEP;
}

bool StdHepFile::Open(const std::string& fileName, const std::string& title,
                      int nEventsExpected)
{
  Close();
  // The C interface takes non-const char*
  std::vector<char> name(fileName.begin(), fileName.end());
  name.push_back('\0');
  std::vector<char> ttl(title.begin(), title.end());
  ttl.push_back('\0');
  const int ntries = std::max(nEventsExpected, 1);

  const int rc = fMcfioInitDone ? StdHepXdrWriteOpen(name.data(), ttl.data(), ntries, kStream)
                                : StdHepXdrWriteInit(name.data(), ttl.data(), ntries, kStream);
  fMcfioInitDone = true;
  if (rc != 0) return false;

  StdHepXdrWrite(100, kStream);  // begin-run record
  fOpen = true;
  fFileName = fileName;
  fNWritten = 0;
  return true;
}

int StdHepFile::WriteEvent(int eventNumber, const std::vector<StdHepParticle>& particles)
{
  if (!fOpen) return -1;
  const int n = std::min(static_cast<int>(particles.size()), static_cast<int>(NMXHEP));
  if (n == 0) return 0;

  hepevt_.nevhep = eventNumber;
  hepevt_.nhep = n;
  for (int i = 0; i < n; ++i) {
    const StdHepParticle& p = particles[i];
    hepevt_.isthep[i] = p.status;
    hepevt_.idhep[i] = p.pdg;
    hepevt_.jmohep[i][0] = 0;
    hepevt_.jmohep[i][1] = 0;
    hepevt_.jdahep[i][0] = 0;
    hepevt_.jdahep[i][1] = 0;
    hepevt_.phep[i][0] = p.px;
    hepevt_.phep[i][1] = p.py;
    hepevt_.phep[i][2] = p.pz;
    hepevt_.phep[i][3] = p.E;
    hepevt_.phep[i][4] = p.m;
    hepevt_.vhep[i][0] = p.x;
    hepevt_.vhep[i][1] = p.y;
    hepevt_.vhep[i][2] = p.z;
    hepevt_.vhep[i][3] = p.t;
  }
  if (StdHepXdrWrite(1, kStream) != 0) return -1;
  ++fNWritten;
  return n;
}

void StdHepFile::Close()
{
  if (!fOpen) return;
  StdHepXdrWrite(200, kStream);  // end-run record
  StdHepXdrEnd(kStream);
  fOpen = false;
}

#endif  // GBG_WITH_STDHEP
