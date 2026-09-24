#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

class EventAction : public G4UserEventAction
{
  public:
    EventAction() = default;
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddEdep(G4double e) { fEdep += e; }
    void CountExiting() { ++fNExit; }
    void CountCreated() { ++fNCreated; }

  private:
    G4double fEdep = 0.;
    G4int fNExit = 0;
    G4int fNCreated = 0;
};

#endif
