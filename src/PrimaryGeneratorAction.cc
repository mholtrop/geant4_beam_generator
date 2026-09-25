#include "PrimaryGeneratorAction.hh"

#include "DetectorConstruction.hh"

#include "G4Event.hh"
#include "G4PhysicalConstants.hh"
#include "G4Poisson.hh"
#include "G4ios.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction(const DetectorConstruction* detector)
  : fDetector(detector)
{
  fGun = new G4ParticleGun(1);
  fGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));

  fMessenger = new G4GenericMessenger(this, "/tgt/gun/", "Beam settings");
  fMessenger->DeclarePropertyWithUnit("current", "nA", fCurrent,
                                      "Beam current (default 120 nA)");
  fMessenger->DeclarePropertyWithUnit("bunchTime", "ns", fBunchTime,
                                      "Time per bunch = per event (default 2 ns)");
  fMessenger->DeclareProperty("poisson", fPoisson,
                              "Poisson-fluctuate the electrons per bunch (default false)");
  fMessenger->DeclareProperty("electronsPerBunch", fElectronsPerBunch,
                              "If > 0, fixed electrons per bunch, ignoring current (default 0)");
  fMessenger->DeclareMethodWithUnit("energy", "GeV", &PrimaryGeneratorAction::SetEnergy,
                                    "Monochromatic kinetic energy (sets Emin = Emax)");
  fMessenger->DeclarePropertyWithUnit("Emin", "GeV", fEmin, "Minimum kinetic energy");
  fMessenger->DeclarePropertyWithUnit("Emax", "GeV", fEmax, "Maximum kinetic energy");

  fMessenger->DeclareMethodWithUnit("thetaX", "mrad", &PrimaryGeneratorAction::SetThetaX,
                                    "Fixed angle in x-z plane (rotation about y)");
  fMessenger->DeclarePropertyWithUnit("thetaXmin", "mrad", fThetaXmin, "");
  fMessenger->DeclarePropertyWithUnit("thetaXmax", "mrad", fThetaXmax, "");
  fMessenger->DeclareMethodWithUnit("thetaY", "mrad", &PrimaryGeneratorAction::SetThetaY,
                                    "Fixed angle in y-z plane (rotation about x)");
  fMessenger->DeclarePropertyWithUnit("thetaYmin", "mrad", fThetaYmin, "");
  fMessenger->DeclarePropertyWithUnit("thetaYmax", "mrad", fThetaYmax, "");

  fMessenger->DeclarePropertyWithUnit("x0", "mm", fX0, "Beam spot centre x at the target centre plane");
  fMessenger->DeclarePropertyWithUnit("y0", "mm", fY0, "Beam spot centre y at the target centre plane");
  fMessenger->DeclarePropertyWithUnit("sigmaX", "mm", fSigmaX, "Gaussian spot sigma in x");
  fMessenger->DeclarePropertyWithUnit("sigmaY", "mm", fSigmaY, "Gaussian spot sigma in y");
  fMessenger->DeclarePropertyWithUnit("zStart", "mm", fZStart,
                                      "z where primaries start (must be upstream of target)");
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fMessenger;
  delete fGun;
}

G4double PrimaryGeneratorAction::MeanElectronsPerBunch() const
{
  // Charge in Geant4 internal units is in units of eplus
  return fCurrent * fBunchTime / eplus;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
  G4int n = fElectronsPerBunch;
  if (n <= 0) {
    const G4double mean = MeanElectronsPerBunch();
    n = fPoisson ? static_cast<G4int>(G4Poisson(mean)) : static_cast<G4int>(std::lround(mean));
  }

  if (event->GetEventID() == 0) {
    G4cout << "=== Beam: ";
    if (fElectronsPerBunch > 0) G4cout << fElectronsPerBunch << " electrons per event (fixed)";
    else G4cout << fCurrent / nanoampere << " nA x " << fBunchTime / ns << " ns = "
                << MeanElectronsPerBunch() << " electrons per event"
                << (fPoisson ? " (Poisson mean)" : " (rounded)");
    G4cout << " ===" << G4endl;
  }

  for (G4int i = 0; i < n; ++i) GenerateOneElectron(event);
}

void PrimaryGeneratorAction::GenerateOneElectron(G4Event* event)
{
  const G4double ekin = fEmin + (fEmax - fEmin) * G4UniformRand();
  const G4double tx = fThetaXmin + (fThetaXmax - fThetaXmin) * G4UniformRand();
  const G4double ty = fThetaYmin + (fThetaYmax - fThetaYmin) * G4UniformRand();

  const G4ThreeVector dir = G4ThreeVector(std::tan(tx), std::tan(ty), 1.).unit();

  const G4double xs = fX0 + (fSigmaX > 0. ? G4RandGauss::shoot(0., fSigmaX) : 0.);
  const G4double ys = fY0 + (fSigmaY > 0. ? G4RandGauss::shoot(0., fSigmaY) : 0.);

  // Move from (xs, ys, zCenter of target) back to the plane z = zStart along dir
  const G4double dz = fZStart - fDetector->GetZCenter();
  const G4ThreeVector pos(xs + dz * dir.x() / dir.z(), ys + dz * dir.y() / dir.z(), fZStart);

  fGun->SetParticleEnergy(ekin);
  fGun->SetParticleMomentumDirection(dir);
  fGun->SetParticlePosition(pos);
  fGun->GeneratePrimaryVertex(event);
}
