/* Force-included (-include) when compiling the StdHep/mcfio C sources.
 *
 * Redirects xdr_array() to mcf_xdr_array() (mcf_xdr_array.c, same directory),
 * which calls the element routines (xdr_int, xdr_double, ...) itself instead
 * of relying on the C library's xdr_array. See external/stdhep/README.md. */
#ifndef MCF_XDR_COMPAT_H
#define MCF_XDR_COMPAT_H

#include <stdio.h>
#include <rpc/types.h>
#include <rpc/xdr.h>

/* elproc is one of xdr_int, xdr_double, ...: bool_t f(XDR *, T *). It is
 * declared void * here so that every call site in the StdHep sources compiles
 * unchanged whatever xdrproc_t looks like on the platform. */
extern bool_t mcf_xdr_array(XDR *xdrs, char **addrp, u_int *sizep, u_int maxsize,
                            u_int elsize, void *elproc);

#define xdr_array mcf_xdr_array

#endif
