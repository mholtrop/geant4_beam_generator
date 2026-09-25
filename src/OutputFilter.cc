#include "OutputFilter.hh"

#include "G4GenericMessenger.hh"
#include "G4ParticleDefinition.hh"

#include <cmath>

OutputFilter::OutputFilter()
{
   fMessenger = new G4GenericMessenger(this, "/tgt/filter/",
                                      "Selection of particles leaving the target");
   fMessenger->DeclarePropertyWithUnit("chargedEmin", "MeV", fChargedEmin,
                                      "Minimum kinetic energy for charged particles (default 500 MeV)");
   fMessenger->DeclarePropertyWithUnit("neutralEmin", "MeV", fNeutralEmin,
                                      "Minimum kinetic energy for neutral particles (default 50 MeV)");
   fMessenger->DeclarePropertyWithUnit("thetaYmin", "mrad", fThetaYmin,
                                      "Minimum abs(thetaY) = abs(atan2(py,pz)), all particles (default 12.5 mrad)");
   fMessenger->DeclarePropertyWithUnit("thetaYmax", "mrad", fThetaYmax,
                                      "Maximum abs(thetaY) = abs(tan2(py,pz)), all particles (default 100 mrad)");
   fMessenger->DeclareProperty("keepEmptyEvents", fKeepEmptyEvents,
                              "Write events with no particle passing the filter (default false)");
}

OutputFilter::~OutputFilter()
{
  delete fMessenger;
}

G4bool OutputFilter::Pass(const G4ParticleDefinition* def, G4double ekin,
                          const G4ThreeVector& mom) const
{
   if (def->GetPDGCharge() == 0. && ekin < fNeutralEmin) return false;
   if (def->GetPDGCharge() != 0. && ekin < fChargedEmin) return false;
   const G4double thetaY = std::abs(std::atan2(mom.y(), mom.z()));
   return thetaY >= fThetaYmin && thetaY <= fThetaYmax;
}
