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
