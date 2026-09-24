#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4SystemOfUnits.hh"
#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4VPhysicalVolume;
class G4GenericMessenger;

// Vacuum world with a single target slab. The target is a box of full size
// width x width x thickness, centred at (0, 0, zCenter).
class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction();
    ~DetectorConstruction() override;

    G4VPhysicalVolume* Construct() override;

    const G4VPhysicalVolume* GetTargetPV() const { return fTargetPV; }

    // z of the target centre. Once the geometry is built this is taken from the
    // placed volume, so it always matches the target actually being simulated.
    G4double GetZCenter() const;

  private:
    G4String fMaterial = "G4_W";
    G4double fThickness = 20. * um;
    G4double fZCenter = -1.1 * mm;
    G4double fWidth = 100. * mm;

    G4VPhysicalVolume* fTargetPV = nullptr;
    G4GenericMessenger* fMessenger = nullptr;
};

#endif
