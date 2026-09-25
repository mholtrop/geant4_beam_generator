#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4SystemOfUnits.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class DetectorConstruction;
class G4ParticleGun;
class G4GenericMessenger;
class G4Event;

// Beam model: one event = one beam bunch.
//  - number of electrons per bunch = current * bunchTime / e, rounded
//    (or Poisson distributed with that mean, or a fixed number if
//    electronsPerBunch > 0)
// Per electron:
//  - kinetic energy uniform in [Emin, Emax] (Emin == Emax -> monochromatic)
//  - thetaX = rotation about y (slope dx/dz = tan thetaX), uniform in [min, max]
//  - thetaY = rotation about x (slope dy/dz = tan thetaY), uniform in [min, max]
//  - beam spot (x0, y0) with Gaussian sigmas, defined in the plane of the
//    target centre (z = zCenter of the target)
//  - each primary starts at z = zStart, back-propagated along its direction
//    from its spot position, so the spot is where you specify it.
class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    explicit PrimaryGeneratorAction(const DetectorConstruction* detector);
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event* event) override;

    // Mean number of electrons per bunch from current and bunch time
    G4double MeanElectronsPerBunch() const;

    // Convenience setters: set min = max
    void SetEnergy(G4double e) { fEmin = e; fEmax = e; }
    void SetThetaX(G4double a) { fThetaXmin = a; fThetaXmax = a; }
    void SetThetaY(G4double a) { fThetaYmin = a; fThetaYmax = a; }

  private:
    void GenerateOneElectron(G4Event* event);

    const DetectorConstruction* fDetector = nullptr;
    G4ParticleGun* fGun = nullptr;
    G4GenericMessenger* fMessenger = nullptr;

    G4double fCurrent = 120. * nanoampere;
    G4double fBunchTime = 2. * ns;
    G4bool fPoisson = false;
    G4int fElectronsPerBunch = 0;  // > 0 overrides current * bunchTime / e

    G4double fEmin = 2.3 * GeV;
    G4double fEmax = 2.3 * GeV;
    G4double fThetaXmin = 0., fThetaXmax = 0.;
    G4double fThetaYmin = 0., fThetaYmax = 0.;
    G4double fX0 = 0., fY0 = 0.;
    G4double fSigmaX = 0., fSigmaY = 0.;
    G4double fZStart = -5. * mm;
};

#endif
