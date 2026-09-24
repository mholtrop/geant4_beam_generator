#include "DetectorConstruction.hh"

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4GenericMessenger.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4ios.hh"

#include <algorithm>

DetectorConstruction::DetectorConstruction()
{
  fMessenger = new G4GenericMessenger(this, "/tgt/det/",
                                      "Target geometry (set before /run/initialize)");
  fMessenger->DeclareProperty("material", fMaterial,
                              "NIST material name of the target (default G4_W)");
  fMessenger->DeclarePropertyWithUnit("thickness", "um", fThickness,
                                      "Target thickness along z (default 20 um)");
  fMessenger->DeclarePropertyWithUnit("zCenter", "mm", fZCenter,
                                      "z of the target centre (default -1.1 mm)");
  fMessenger->DeclarePropertyWithUnit("width", "mm", fWidth,
                                      "Full transverse size in x and y (default 100 mm)");
}

DetectorConstruction::~DetectorConstruction()
{
  delete fMessenger;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  auto* nist = G4NistManager::Instance();
  auto* vacuum = nist->FindOrBuildMaterial("G4_Galactic");
  auto* tgtMat = nist->FindOrBuildMaterial(fMaterial);
  if (tgtMat == nullptr) {
    G4ExceptionDescription msg;
    msg << "Unknown NIST material '" << fMaterial << "'";
    G4Exception("DetectorConstruction::Construct", "TGT001", FatalException, msg);
  }

  const G4double worldHalf = std::max(1. * m, fWidth);
  auto* worldS = new G4Box("World", worldHalf, worldHalf, worldHalf);
  auto* worldLV = new G4LogicalVolume(worldS, vacuum, "World");
  auto* worldPV =
    new G4PVPlacement(nullptr, G4ThreeVector(), worldLV, "World", nullptr, false, 0, true);

  auto* tgtS = new G4Box("Target", 0.5 * fWidth, 0.5 * fWidth, 0.5 * fThickness);
  auto* tgtLV = new G4LogicalVolume(tgtS, tgtMat, "Target");
  fTargetPV = new G4PVPlacement(nullptr, G4ThreeVector(0., 0., fZCenter), tgtLV, "Target",
                                worldLV, false, 0, true);

  G4cout << "\n=== Target: " << fMaterial << ", thickness " << fThickness / um << " um ("
         << fThickness / tgtMat->GetRadlen() << " X0), centre z = " << fZCenter / mm
         << " mm, faces at z = " << (fZCenter - 0.5 * fThickness) / mm << " and "
         << (fZCenter + 0.5 * fThickness) / mm << " mm ===\n"
         << G4endl;

  return worldPV;
}
