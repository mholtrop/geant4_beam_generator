#ifndef SteppingAction_h
#define SteppingAction_h 1

#include "G4UserSteppingAction.hh"
#include "globals.hh"

class DetectorConstruction;
class EventAction;
class G4GenericMessenger;

// Accumulates energy deposit in the target and records every track that
// crosses the target boundary outward ("exit" ntuple). By default such
// tracks are then killed: the world is vacuum, nothing further happens.
class SteppingAction : public G4UserSteppingAction
{
  public:
    SteppingAction(const DetectorConstruction* detector, EventAction* eventAction);
    ~SteppingAction() override;

    void UserSteppingAction(const G4Step* step) override;

  private:
    const DetectorConstruction* fDetector = nullptr;
    EventAction* fEventAction = nullptr;
    G4bool fKillOnExit = true;
    G4GenericMessenger* fMessenger = nullptr;
};

#endif
