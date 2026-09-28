cmake_minimum_required(VERSION 3.16...3.27)
project(geant4_beam_generator)

option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 11.0 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 11.0 REQUIRED)
endif()
include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(geant4_beam_generator geant4_beam_generator.cc ${sources} ${headers})
target_include_directories(geant4_beam_generator PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(geant4_beam_generator ${Geant4_LIBRARIES})

# StdHep output: C subset of StdHep/mcfio from hps-mc, in external/stdhep.
# Needs the Sun RPC XDR routines (libtirpc on current Linux distributions).
option(WITH_STDHEP "Build with StdHep output (needs libtirpc)" ON)
if(WITH_STDHEP)
  find_path(TIRPC_INCLUDE_DIR rpc/xdr.h PATH_SUFFIXES tirpc)
  find_library(TIRPC_LIBRARY tirpc)
  if(NOT TIRPC_INCLUDE_DIR)
    message(FATAL_ERROR "rpc/xdr.h not found. Install the libtirpc development package "
                        "(libtirpc-dev / libtirpc-devel) or configure with -DWITH_STDHEP=OFF.")
  endif()
  file(GLOB stdhep_sources ${PROJECT_SOURCE_DIR}/external/stdhep/src/*.c)
  add_library(stdhep_mcfio STATIC ${stdhep_sources})
  target_include_directories(stdhep_mcfio PUBLIC ${PROJECT_SOURCE_DIR}/external/stdhep/include
                                                 ${TIRPC_INCLUDE_DIR})
  # The StdHep/mcfio code predates C99 and is kept unmodified. Compile it as
  # gnu89 and keep the diagnostics that recent compilers (GCC >= 14,
  # Clang >= 16, AppleClang) make errors by default as warnings; -w then
  # hides all warnings for this library only.
  set_target_properties(stdhep_mcfio PROPERTIES C_STANDARD 90 C_EXTENSIONS ON)
  target_compile_options(stdhep_mcfio PRIVATE -w
    -Wno-error=implicit-function-declaration
    -Wno-error=implicit-int
    -Wno-error=int-conversion
    -Wno-error=incompatible-pointer-types
    $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-error=incompatible-function-pointer-types>)
  if(TIRPC_LIBRARY)
    target_link_libraries(stdhep_mcfio PUBLIC ${TIRPC_LIBRARY})
  endif()
  target_compile_definitions(geant4_beam_generator PRIVATE GBG_WITH_STDHEP)
  # Reader built from the same library, to check files: stdhep_dump <file> [n]
  add_executable(stdhep_dump ${PROJECT_SOURCE_DIR}/tools/stdhep_dump.cc)
  target_link_libraries(stdhep_dump stdhep_mcfio)
  # Step-by-step check of the first record: stdhep_diag <file>
  add_executable(stdhep_diag ${PROJECT_SOURCE_DIR}/tools/stdhep_diag.cc)
  target_link_libraries(stdhep_diag stdhep_mcfio)
  target_link_libraries(geant4_beam_generator stdhep_mcfio)
  message(STATUS "StdHep output enabled (rpc/xdr.h in ${TIRPC_INCLUDE_DIR})")
endif()

foreach(_macro run.mac scan.mac init_vis.mac vis.mac)
  configure_file(${PROJECT_SOURCE_DIR}/macro/${_macro} ${PROJECT_BINARY_DIR}/${_macro} COPYONLY)
endforeach()
