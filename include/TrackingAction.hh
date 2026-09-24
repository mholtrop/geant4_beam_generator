#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class EventAction;
class G4GenericMessenger;

// Records every secondary track at its creation point ("created" ntuple).
class TrackingAction : public G4UserTrackingAction
{
  public:
    explicit TrackingAction(EventAction* eventAction);
    ~TrackingAction() override;

    void PreUserTrackingAction(const G4Track* track) override;

  private:
    EventAction* fEventAction = nullptr;
    G4bool fRecord = true;
    G4GenericMessenger* fMessenger = nullptr;
};

#endif
