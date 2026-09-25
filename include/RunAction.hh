#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"

class EventAction;
class G4Run;

class RunAction : public G4UserRunAction
{
  public:
    explicit RunAction(EventAction* eventAction);
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;
};

#endif
