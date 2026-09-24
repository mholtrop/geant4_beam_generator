cmake_minimum_required(VERSION 3.16...3.27)
project(TargetSim)

option(WITH_GEANT4_UIVIS "Build with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 11.0 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 11.0 REQUIRED)
endif()
include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

add_executable(targetSim targetSim.cc ${sources} ${headers})
target_include_directories(targetSim PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(targetSim ${Geant4_LIBRARIES})

foreach(_macro run.mac scan.mac init_vis.mac vis.mac)
  configure_file(${PROJECT_SOURCE_DIR}/${_macro} ${PROJECT_BINARY_DIR}/${_macro} COPYONLY)
endforeach()
