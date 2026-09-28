/* xdr_selftest: checks the XDR array routines outside of StdHep.
 * Encodes known int and double arrays with the C library's xdr_array and with
 * mcf_xdr_array, once into memory and once through stdio (as StdHep does),
 * and compares the resulting bytes with the expected big-endian XDR values.
 * Exit status 0 if all tests pass. Usage: xdr_selftest */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

extern bool_t mcf_xdr_array(XDR *xdrs, char **addrp, u_int *sizep, u_int maxsize,
                            u_int elsize, void *elproc);

#define NI 6
#define ND 3

static int expect_ints[NI] = {11, 22, -33, 44, 55, 66};
static double expect_dbls[ND] = {3.5, -0.125, 1.0e10};

static void put_be32(unsigned char* p, unsigned int v)
{
  p[0] = v >> 24; p[1] = v >> 16; p[2] = v >> 8; p[3] = v;
}

/* Build the expected bytes: count, then the elements */
static int build_expected_int(unsigned char* out)
{
  int i;
  put_be32(out, NI);
  for (i = 0; i < NI; ++i) put_be32(out + 4 + 4 * i, (unsigned int) expect_ints[i]);
  return 4 + 4 * NI;
}

static int build_expected_dbl(unsigned char* out)
{
  int i, k;
  put_be32(out, ND);
  for (i = 0; i < ND; ++i) {
    unsigned char raw[8];
    double d = expect_dbls[i];
    unsigned long long u = 0;
    memcpy(&u, &d, 8);
    for (k = 0; k < 8; ++k) raw[k] = (unsigned char) (u >> (56 - 8 * k));  /* assumes IEEE, host-endian independent */
    memcpy(out + 4 + 8 * i, raw, 8);
  }
  return 4 + 8 * ND;
}

static void dump(const char* label, const unsigned char* b, int n)
{
  int i;
  printf("      %s:", label);
  for (i = 0; i + 3 < n; i += 4) printf(" %02x%02x%02x%02x", b[i], b[i + 1], b[i + 2], b[i + 3]);
  printf("\n");
}

typedef bool_t (*array_fn)(XDR*, char**, u_int*, u_int, u_int, void*);

static bool_t system_array(XDR* x, char** a, u_int* s, u_int m, u_int e, void* p)
{
  /* the call exactly as the StdHep sources make it */
  return xdr_array(x, a, s, m, e, (xdrproc_t) p);
}

static int run(const char* name, array_fn fn, int viaStdio, int isDouble)
{
  unsigned char got[256], want[256];
  int nWant, nGot;
  XDR x;
  u_int n;
  char* p;
  int* ints = (int*) malloc(sizeof(int) * NI);
  double* dbls = (double*) malloc(sizeof(double) * ND);
  int ok = 0;

  memcpy(ints, expect_ints, sizeof(expect_ints));
  memcpy(dbls, expect_dbls, sizeof(expect_dbls));
  nWant = isDouble ? build_expected_dbl(want) : build_expected_int(want);
  n = isDouble ? ND : NI;
  p = isDouble ? (char*) dbls : (char*) ints;

  if (!viaStdio) {
    xdrmem_create(&x, (char*) got, sizeof(got), XDR_ENCODE);
    if (isDouble) ok = fn(&x, &p, &n, ND, sizeof(double), (void*) xdr_double);
    else ok = fn(&x, &p, &n, NI, sizeof(int), (void*) xdr_int);
    nGot = (int) xdr_getpos(&x);
  }
  else {
    FILE* f = tmpfile();
    if (f == NULL) { printf("  %-28s cannot create temporary file\n", name); return 0; }
    xdrstdio_create(&x, f, XDR_ENCODE);
    if (isDouble) ok = fn(&x, &p, &n, ND, sizeof(double), (void*) xdr_double);
    else ok = fn(&x, &p, &n, NI, sizeof(int), (void*) xdr_int);
    nGot = (int) xdr_getpos(&x);
    xdr_destroy(&x);
    fflush(f);
    rewind(f);
    nGot = (int) fread(got, 1, sizeof(got), f);
    fclose(f);
  }
  free(ints);
  free(dbls);

  ok = ok && nGot == nWant && memcmp(got, want, nWant) == 0;
  printf("  %-28s %s %-7s %s\n", name, viaStdio ? "stdio " : "memory", isDouble ? "double" : "int",
         ok ? "PASS" : "FAIL");
  if (!ok) {
    dump("expected", want, nWant);
    dump("got     ", got, nGot);
  }
  return ok;
}

int main(void)
{
  int nFail = 0;
  printf("sizeof: int %zu, long %zu, u_int %zu, bool_t %zu, void* %zu\n", sizeof(int),
         sizeof(long), sizeof(u_int), sizeof(bool_t), sizeof(void*));
  printf("C library xdr_array:\n");
  nFail += !run("xdr_array", system_array, 0, 0);
  nFail += !run("xdr_array", system_array, 0, 1);
  nFail += !run("xdr_array", system_array, 1, 0);
  nFail += !run("xdr_array", system_array, 1, 1);
  printf("mcf_xdr_array (used by the StdHep code unless disabled):\n");
  nFail += !run("mcf_xdr_array", mcf_xdr_array, 0, 0);
  nFail += !run("mcf_xdr_array", mcf_xdr_array, 0, 1);
  nFail += !run("mcf_xdr_array", mcf_xdr_array, 1, 0);
  nFail += !run("mcf_xdr_array", mcf_xdr_array, 1, 1);
  printf("%s\n", nFail == 0 ? "all tests passed" : "SOME TESTS FAILED");
  return nFail == 0 ? 0 : 1;
}
