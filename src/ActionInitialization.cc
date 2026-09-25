#include "ActionInitialization.hh"

#include "EventAction.hh"
#include "PrimaryGeneratorAction.hh"
#include "RunAction.hh"
#include "SteppingAction.hh"
#include "TrackingAction.hh"

void ActionInitialization::BuildForMaster() const
{
  // The master books the same ntuple layout (needed for merging); this
  // EventAction only provides the vectors for booking and is never run.
  // Same pattern as example B5.
  auto* eventAction = new EventAction;
  SetUserAction(new RunAction(eventAction));
}

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction(fDetector));
  auto* eventAction = new EventAction;
  SetUserAction(eventAction);
  SetUserAction(new RunAction(eventAction));
  SetUserAction(new TrackingAction(eventAction));
  SetUserAction(new SteppingAction(fDetector, eventAction));
}
