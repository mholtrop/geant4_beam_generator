/* xdr_array() with the semantics of the BSD/Sun RPC implementation:
 * the element count is coded first (as u_int), then every element is coded
 * with elproc(xdrs, element_address). Written out here because the StdHep
 * sources call xdr_array with arguments that recent C compilers only accept
 * with the diagnostics downgraded (function-pointer type mismatch for the
 * element routine), and its result was found to be wrong on macOS. */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

typedef bool_t (*mcf_elproc_t)(XDR *, void *);

bool_t mcf_xdr_array(XDR *xdrs, char **addrp, u_int *sizep, u_int maxsize,
                     u_int elsize, void *elproc)
{
  mcf_elproc_t proc = (mcf_elproc_t) elproc;
  char *target = *addrp;
  u_int i, c;
  bool_t stat = TRUE;

  if (!xdr_u_int(xdrs, sizep)) return FALSE;
  c = *sizep;
  if (xdrs->x_op != XDR_FREE) {
    if (c > maxsize) return FALSE;
    if (elsize != 0 && (UINT_MAX / elsize) < c) return FALSE;
  }

  if (target == NULL) {
    if (xdrs->x_op == XDR_DECODE) {
      if (c == 0) return TRUE;
      target = (char *) calloc(c, elsize);
      if (target == NULL) {
        fprintf(stderr, "mcf_xdr_array: out of memory\n");
        return FALSE;
      }
      *addrp = target;
    }
    else if (xdrs->x_op == XDR_FREE) {
      return TRUE;
    }
  }

  for (i = 0; i < c && stat; i++) {
    stat = (*proc)(xdrs, target);
    target += elsize;
  }

  if (xdrs->x_op == XDR_FREE) {
    free(*addrp);
    *addrp = NULL;
  }
  return stat;
}
