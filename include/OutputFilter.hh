#ifndef OutputFilter_h
#define OutputFilter_h 1

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class G4GenericMessenger;
class G4ParticleDefinition;

// Selection applied to particles leaving the target before they are written
// to the exit_* columns:
//  - charged particles: kinetic energy >= chargedEmin
//  - all particles:     thetaYmin <= abs(thetaY) <= thetaYmax,
//                       thetaY = atan2(py, pz) (same definition as prim_thetaY)
// The defaults pass everything.
class OutputFilter
{
  public:
    OutputFilter();
    ~OutputFilter();

    G4bool Pass(const G4ParticleDefinition* def, G4double ekin, const G4ThreeVector& mom) const;
    G4bool KeepEmptyEvents() const { return fKeepEmptyEvents; }

  private:
    G4double fChargedEmin = 500.; //MeV
    G4double fNeutralEmin = 50.;   //MeV
    G4double fThetaYmin = 0.0125; // rad
    G4double fThetaYmax = 0.1;    // rad
    G4bool fKeepEmptyEvents = false;
    G4GenericMessenger* fMessenger = nullptr;
};

#endif
