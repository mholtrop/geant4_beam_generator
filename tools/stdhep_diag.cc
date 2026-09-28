// stdhep_diag: step through the first record of an StdHep file the way
// mcfioC_NextEvent does, printing each intermediate value, to find out where
// reading fails on a given platform.  Usage: stdhep_diag <file.stdhep>

#include <cstdio>  // FILE, needed by mcf_xdr.h

extern "C" {
#include <rpc/types.h>
#include <rpc/xdr.h>
#include "mcf_xdr.h"
#include "mcfio_Direct.h"
#include "mcfio_Util1.h"
void mcfioC_Init(void);
int mcfioC_OpenReadDirect(char* filename);
}

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

// Raw 32-bit big-endian words from the file, independent of XDR/mcfio
static void DumpWords(const char* file, unsigned int pos, int nWords)
{
  FILE* f = std::fopen(file, "rb");
  if (f == nullptr || std::fseek(f, static_cast<long>(pos), SEEK_SET) != 0) {
    std::printf("  (cannot read raw bytes at %u)\n", pos);
    if (f) std::fclose(f);
    return;
  }
  std::printf("  raw words at %u:", pos);
  for (int i = 0; i < nWords; ++i) {
    unsigned char b[4];
    if (std::fread(b, 1, 4, f) != 4) break;
    const unsigned int w = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
    std::printf("%s %08x", (i % 8 == 0) ? "\n   " : "", w);
  }
  std::printf("\n");
  std::fclose(f);
}

// Look for event headers (id 4, ntot, version string "2.00") after pos
static void ScanForEventHeaders(const char* file, unsigned int pos, unsigned int range)
{
  FILE* f = std::fopen(file, "rb");
  if (f == nullptr) return;
  std::vector<unsigned char> buf(range);
  std::fseek(f, static_cast<long>(pos), SEEK_SET);
  const std::size_t n = std::fread(buf.data(), 1, range, f);
  std::fclose(f);
  int found = 0;
  for (std::size_t i = 0; i + 16 <= n && found < 5; i += 4) {
    const unsigned char* b = &buf[i];
    const bool idOk = b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 4;
    const bool lenOk = b[8] == 0 && b[9] == 0 && b[10] == 0 && b[11] == 4;
    if (idOk && lenOk && std::memcmp(b + 12, "2.00", 4) == 0) {
      std::printf("  event header pattern found at %zu\n", pos + i);
      ++found;
    }
  }
  if (found == 0) std::printf("  no event header pattern in %u bytes after %u\n", range, pos);
}

static const char* BlockName(int id)
{
  switch (id) {
    case FILEHEADER: return "FILEHEADER";
    case EVENTTABLE: return "EVENTTABLE";
    case EVENTHEADER: return "EVENTHEADER";
    case SEQUENTIALHEADER: return "SEQUENTIALHEADER";
    case GENERIC: return "GENERIC";
    default: return "?";
  }
}

int main(int argc, char** argv)
{
  if (argc < 2) {
    std::fprintf(stderr, "Usage: %s <file.stdhep>\n", argv[0]);
    return 1;
  }
  std::printf("sizeof: int %zu, long %zu, u_int %zu, bool_t %zu, void* %zu\n", sizeof(int),
              sizeof(long), sizeof(u_int), sizeof(bool_t), sizeof(void*));

  std::vector<char> name(argv[1], argv[1] + std::strlen(argv[1]) + 1);
  mcfioC_Init();
  const int stream = mcfioC_OpenReadDirect(name.data());
  if (stream < 1) {
    std::printf("open failed\n");
    return 1;
  }
  mcfStream* str = McfStreamPtrList[stream - 1];
  XDR* xdrs = str->xdr;
  std::printf("after file header: currentPos %u, xdr_getpos %u, numevts %u, dimTable %d,"
              " nBlocks %d, nNTuples %d\n",
              str->currentPos, xdr_getpos(xdrs), str->fhead->numevts, str->fhead->dimTable,
              str->fhead->nBlocks, str->fhead->nNTuples);

  // 1. What block follows the file header?
  const u_int p1 = xdr_getpos(xdrs);
  int id = -1, ntot = -1;
  bool_t ok = xdr_mcfast_headerBlock(xdrs, &id, &ntot, McfGenericVersion);
  std::printf("block at %u: decode %s, id %d (%s), ntot %d, version '%s'\n", p1,
              ok ? "OK" : "FAILED", id, BlockName(id), ntot, *McfGenericVersion);
  if (!ok) return 2;
  std::printf("setpos back to %u: %s\n", p1, xdr_setpos(xdrs, p1) ? "OK" : "FAILED");

  // 2. Decode the event table
  auto* table = static_cast<mcfxdrEventTable*>(std::calloc(1, sizeof(mcfxdrEventTable)));
  table->dim = str->fhead->dimTable;
  ok = xdr_mcfast_eventtable(xdrs, &id, &ntot, McfGenericVersion, &table);
  std::printf("event table: decode %s, version '%s', nextLocator %d, numevts %d, dim %u,"
              " xdr_getpos after %u\n",
              ok ? "OK" : "FAILED", *McfGenericVersion, table->nextLocator, table->numevts,
              table->dim, xdr_getpos(xdrs));
  if (!ok) return 3;
  for (int i = 0; i < 5 && i < table->numevts; ++i) {
    std::printf("  entry %d: evtnum %d, ptrEvents %u\n", i, table->evtnums[i],
                table->ptrEvents[i]);
  }
  const u_int tableEnd = xdr_getpos(xdrs);
  DumpWords(argv[1], tableEnd, 24);
  ScanForEventHeaders(argv[1], tableEnd, 1 << 20);
  if (table->numevts <= 0) return 4;

  // 3. Position at the first event header and decode it
  const u_int pEvt = table->ptrEvents[0];
  ok = xdr_setpos(xdrs, pEvt);
  std::printf("setpos to first event %u: %s, xdr_getpos %u\n", pEvt, ok ? "OK" : "FAILED",
              xdr_getpos(xdrs));
  DumpWords(argv[1], pEvt, 24);
  if (!ok) return 5;
  auto* ehead = static_cast<mcfxdrEventHeader*>(std::calloc(1, sizeof(mcfxdrEventHeader)));
  ok = xdr_mcfast_eventheader(xdrs, &id, &ntot, McfGenericVersion, &ehead);
  std::printf("event header: decode %s, id %d (%s), ntot %d, version '%s'\n",
              ok ? "OK" : "FAILED", id, BlockName(id), ntot, *McfGenericVersion);
  std::printf("  evtnum %d, storenum %d, runnum %d, trigMask %d, nBlocks %u, dimBlocks %u,"
              " nNTuples %u, dimNTuples %u\n",
              ehead->evtnum, ehead->storenum, ehead->runnum, ehead->trigMask, ehead->nBlocks,
              ehead->dimBlocks, ehead->nNTuples, ehead->dimNTuples);
  if (ok) {
    for (u_int i = 0; i < ehead->nBlocks && ehead->blockIds != nullptr; ++i) {
      std::printf("  block %u: id %d at %u\n", i, ehead->blockIds[i], ehead->ptrBlocks[i]);
    }
  }
  return ok ? 0 : 6;
}
