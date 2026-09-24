#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4GenericMessenger.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "Randomize.hh"

#include <cmath>

PrimaryGeneratorAction::PrimaryGeneratorAction()
{
  fGun = new G4ParticleGun(1);
  fGun->SetParticleDefinition(G4ParticleTable::GetParticleTable()->FindParticle("e-"));

  fMessenger = new G4GenericMessenger(this, "/tgt/gun/", "Beam settings");
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

  fMessenger->DeclarePropertyWithUnit("x0", "mm", fX0, "Beam spot centre x at z = zSpot");
  fMessenger->DeclarePropertyWithUnit("y0", "mm", fY0, "Beam spot centre y at z = zSpot");
  fMessenger->DeclarePropertyWithUnit("sigmaX", "mm", fSigmaX, "Gaussian spot sigma in x");
  fMessenger->DeclarePropertyWithUnit("sigmaY", "mm", fSigmaY, "Gaussian spot sigma in y");
  fMessenger->DeclarePropertyWithUnit("zSpot", "mm", fZSpot,
                                      "Plane where the spot is defined (default -1.1 mm)");
  fMessenger->DeclarePropertyWithUnit("zStart", "mm", fZStart,
                                      "z where primaries start (must be upstream of target)");
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fMessenger;
  delete fGun;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* event)
{
   // We generate the beam primaries in the z-axis direction, with the 0.0305 beam rotation
   // already applied.
  const G4double ekin = fEmin + (fEmax - fEmin) * G4UniformRand();
  const G4double tx = fThetaXmin + (fThetaXmax - fThetaXmin) * G4UniformRand();
  const G4double ty = fThetaYmin + (fThetaYmax - fThetaYmin) * G4UniformRand();

  const G4ThreeVector dir = G4ThreeVector(std::tan(tx), std::tan(ty), 1.).unit();

  const G4double xs = fX0 + (fSigmaX > 0. ? G4RandGauss::shoot(0., fSigmaX) : 0.);
  const G4double ys = fY0 + (fSigmaY > 0. ? G4RandGauss::shoot(0., fSigmaY) : 0.);

  // Move from (xs, ys, zSpot) back to the plane z = zStart along dir
  const G4double dz = fZStart - fZSpot;
  const G4ThreeVector pos(xs + dz * dir.x() / dir.z(), ys + dz * dir.y() / dir.z(), fZStart);

  fGun->SetParticleEnergy(ekin);
  fGun->SetParticleMomentumDirection(dir);
  fGun->SetParticlePosition(pos);
  fGun->GeneratePrimaryVertex(event);
}
