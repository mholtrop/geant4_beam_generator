// stdhep_dump: read an StdHep file with the same StdHep/mcfio library (and
// the same compiler settings) that geant4_beam_generator writes with.
//
// Usage: stdhep_dump <file.stdhep> [n_print]
//   Prints the file header, the first n_print events in full (default 1),
//   and a summary: records read, events, particles, first/last NEVHEP.

extern "C" {
#include "stdhep.h"
#include "stdhep_mcfio.h"
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::fprintf(stderr, "Usage: %s <file.stdhep> [n_print]\n", argv[0]);
    return 1;
  }
  const int nPrint = (argc > 2) ? std::atoi(argv[2]) : 1;
  std::vector<char> name(argv[1], argv[1] + std::strlen(argv[1]) + 1);

  int nRecordsInHeader = 0;
  if (StdHepXdrReadInitNTries(name.data(), &nRecordsInHeader, 0) != 0) {
    std::fprintf(stderr, "stdhep_dump: cannot open %s\n", argv[1]);
    return 1;
  }

  long nRecords = 0, nEvents = 0, nParticles = 0;
  int firstEvt = -1, lastEvt = -1, maxN = 0;
  int ilbl = 0;
  int rc = 0;
  while ((rc = StdHepXdrRead(&ilbl, 0)) == 0) {
    ++nRecords;
    if (ilbl != 1 && ilbl != 4) {
      std::printf("record %ld: ilbl = %d (not an event)\n", nRecords, ilbl);
      continue;
    }
    ++nEvents;
    nParticles += hepevt_.nhep;
    if (firstEvt < 0) firstEvt = hepevt_.nevhep;
    lastEvt = hepevt_.nevhep;
    if (hepevt_.nhep > maxN) maxN = hepevt_.nhep;
    if (nEvents <= nPrint) {
      std::printf("event %ld: nevhep = %d, nhep = %d\n", nEvents, hepevt_.nevhep, hepevt_.nhep);
      std::printf("  %4s %6s %11s %11s %11s %11s %11s %9s %9s %9s %9s\n", "i", "id",
                  "px[GeV]", "py", "pz", "E", "m", "x[mm]", "y", "z", "t[mm/c]");
      for (int i = 0; i < hepevt_.nhep; ++i) {
        std::printf("  %4d %6d %11.5g %11.5g %11.5g %11.5g %11.5g %9.4g %9.4g %9.4g %9.4g\n", i,
                    hepevt_.idhep[i], hepevt_.phep[i][0], hepevt_.phep[i][1],
                    hepevt_.phep[i][2], hepevt_.phep[i][3], hepevt_.phep[i][4],
                    hepevt_.vhep[i][0], hepevt_.vhep[i][1], hepevt_.vhep[i][2],
                    hepevt_.vhep[i][3]);
      }
    }
  }
  StdHepXdrEnd(0);

  std::printf("\nSummary: %ld records read (header says %d), %ld events, %ld particles"
              " (max %d per event), NEVHEP first %d last %d; read stopped with %s\n",
              nRecords, nRecordsInHeader, nEvents, nParticles, maxN, firstEvt, lastEvt,
              rc == 1 ? "end of file" : "an ERROR");
  return rc == 1 ? 0 : 2;
}
