# geant4_beam_generator

Simulate the interactions of electrons going through a thin target (default 20 um W centred at z = -1.1 mm).
Default is for 120nA in 2ns bunches (1500 e- per event). Output is a ROOT file with the events.
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

## Commands for the macros

Geometry (before /run/initialize):
  /tgt/det/material, /tgt/det/thickness, /tgt/det/zCenter, /tgt/det/width

Beam (after /run/initialize; in MT mode these commands only exist after init):
  One event = one beam bunch. Electrons per bunch = current * bunchTime / e,
  rounded (1498 for 120 nA and 2 ns). /run/beamOn N simulates N bunches.
  /tgt/gun/current I            beam current (default 120 nA)
  /tgt/gun/bunchTime t          time per bunch (default 2 ns)
  /tgt/gun/poisson true|false   Poisson-fluctuate the number (default false)
  /tgt/gun/electronsPerBunch n  if > 0, use n electrons per event instead
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
  Filter on particles leaving the target (exit_* columns only):
  /tgt/filter/chargedEmin E     charged particles need kinetic energy >= E (default 0)
  /tgt/filter/thetaYmin a       all particles need thetaYmin <= thetaY <= thetaYmax,
  /tgt/filter/thetaYmax a       thetaY = atan2(py, pz) (defaults -pi, +pi)
  /tgt/filter/keepEmptyEvents   write events with no passing particle (default true)
  /tgt/exit/kill true|false     kill tracks after they leave the target

## Output (ROOT file, energies/momenta in MeV, lengths in mm, angles in rad)

One TTree, "events", with one row per event (bunch). Per-particle quantities
are vector columns (RVec in RDataFrame), one entry per particle.

Scalars: event, nPrimary, Edep (in target), nExit (passing the filter),
  nCreated, nExitAll (leaving the target, before the filter)

Beam electrons (index i corresponds to Geant4 track ID i+1):
  prim_E, prim_thetaX, prim_thetaY, prim_x, prim_y, prim_z (start point)

Particles leaving the target, at the exit point (prefix exit_), and
secondaries at their creation point (prefix created_):
  track, parent, primary (track ID of the beam electron it descends from),
  pdg, particle, process (creator process, "primary" for beam electrons),
  E (kinetic), px, py, pz, x, y, z (recorded point), vx, vy, vz (vertex)

Scattered beam electrons are the exit_ entries with exit_parent == 0. Geant4
keeps a primary's track ID through brems and ionisation; for Moller scattering
the delta ray is at most half the kinetic energy, so the primary keeps the
larger share. Backscattered particles have exit_pz < 0.

Example (PyROOT RDataFrame):

    import ROOT as R
    df = R.RDataFrame("events", "target_mono.root")
    df = df.Define("scat_E", "exit_E[exit_parent == 0]")
    h  = df.Histo1D(("h", "scattered e- energy;E [MeV]", 200, 0, 4000), "scat_E")

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
