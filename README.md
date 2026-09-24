# targetSim

Electrons through a thin target (default 20 um W centred at z = -1.1 mm).
Requires Geant4 >= 11.0 with its data sets installed.

## Build and run

    source /path/to/geant4-install/bin/geant4.sh
    mkdir build && cd build
    cmake ..
    make -j
    ./targetSim run.mac -t 8          # batch, 8 threads
    ./targetSim scan.mac -s 12345     # energy/angle ranges, fixed seed
    ./targetSim                       # interactive with visualisation

Options: -t nThreads, -p physicsList (default FTFP_BERT_EMZ), -s seed.

## Commands

Geometry (before /run/initialize):
  /tgt/det/material, /tgt/det/thickness, /tgt/det/zCenter, /tgt/det/width

Beam (after /run/initialize; in MT mode these commands only exist after init):
  /tgt/gun/energy E             monochromatic kinetic energy
  /tgt/gun/Emin, /tgt/gun/Emax  uniform kinetic-energy range
  /tgt/gun/thetaX a             angle in x-z plane (rotation about y), dx/dz = tan(a)
  /tgt/gun/thetaXmin/thetaXmax  uniform range
  /tgt/gun/thetaY, thetaYmin/thetaYmax   same in y-z plane
  /tgt/gun/x0, y0, sigmaX, sigmaY        Gaussian spot, defined at the target centre z
  /tgt/gun/zStart (default -5 mm)
  /gun/particle                 particle type (e- by default)
Units can be given explicitly, e.g. "/tgt/gun/thetaX 1.7 deg".

Output:
  /analysis/setFileName name    (use a new name for each /run/beamOn)
  /tgt/created/record true|false
  /tgt/exit/kill true|false     kill tracks after they leave the target

## Output (ROOT file, energies/momenta in MeV, lengths in mm, angles in rad)

primary : one row per event
  event, E, thetaX, thetaY, x, y, z (start point), Edep (in target), nExit, nCreated

exit    : one row per track crossing the target surface outward, at the exit point
created : one row per secondary, at its creation point
  both: event, track, parent, pdg, particle, process (creator process, "primary"
  for the beam particle), E (kinetic), px, py, pz, x, y, z (recorded point),
  vx, vy, vz (creation vertex)

The scattered beam electron is the row in "exit" with track == 1. Geant4 keeps
the primary as track 1 through brems and ionisation; for Moller scattering the
delta ray is at most half the kinetic energy, so the primary keeps the larger
share. Backscattered particles appear in "exit" with pz < 0.

Example (Python, uproot):

    import uproot
    f  = uproot.open("target_mono.root")
    ex = f["exit"].arrays(library="np")
    beam = ex["track"] == 1
    E_scat = ex["E"][beam]

## Physics notes

- Production cuts decide which secondaries become tracks. Below the threshold,
  energy loss is treated as continuous. The default 0.7 mm range cut is large
  compared with a 20 um foil, so the macros use /run/setCut 10 um. The
  resulting energy thresholds per particle are printed in the couple table at
  /run/initialize; check them. There is also a lower edge on thresholds
  (/cuts/setLowEdge).
- The combined "general" gamma process is switched off so that creator
  processes are reported individually (phot, compt, conv, Rayl).
- To see which processes are attached to each particle after /run/initialize:
  /particle/select e-  then  /particle/process/dump
  In particular, check whether a direct electron pair-production (trident)
  process is present for e- in the list you use; do not assume it is.
- Photo- and electro-nuclear processes come from G4EmExtraPhysics in the
  reference hadronic lists.
- Atomic de-excitation: /process/em/fluo true, /process/em/auger true
  (before /run/initialize) if you need fluorescence/Auger products.
