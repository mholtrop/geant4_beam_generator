# StdHep / mcfio (C subset)

C sources and headers needed to write StdHep files: the `libstdhepC` and
`libFmcfio` C code of StdHep 5.6.1, copied unmodified from
https://github.com/JeffersonLab/hps-mc (tools/stdhep-lib/src, commit 308e27b):

- `src/stdhep/*.c`                                   -> `src/`
- the C files of libFmcfio listed in `mcfio/src/GNUmakefile` (CL_F_SRC,
  without the Fortran file mcfio_FPrintDictionary.F) -> `src/`
- the headers these files include (`src/inc`, `mcfio/src`) -> `include/`

StdHep was written at Fermilab (Lynn Garren et al.); see the copyright
notices in the individual files. Requires the Sun RPC XDR routines
(libtirpc on current Linux distributions).

## Local additions (not from hps-mc)

`compat/mcf_xdr_array.c` and `compat/mcf_xdr_compat.h` replace the C library's
`xdr_array()` in the StdHep sources: CMake force-includes the header
(`-include`) when compiling `src/*.c`, which redirects each `xdr_array` call to
`mcf_xdr_array()`. The sources in `src/` stay unmodified. Same behaviour as
the standard routine (element count, then each element via xdr_int,
xdr_double, ...). Disable with `-DSTDHEP_OWN_XDR_ARRAY=OFF`.

Reason: files written on macOS had every array element replaced by one
repeated stack-address value (event table, event headers and the particle
arrays), so they could not be read back. `tools/xdr_selftest.c` (built as
`xdr_selftest`) tests the C library's `xdr_array` and `mcf_xdr_array` on the
current platform.
