#include "TrackingAction.hh"

#include "EventAction.hh"
#include "ParticleRow.hh"

#include "G4GenericMessenger.hh"
#include "G4Track.hh"

TrackingAction::TrackingAction(EventAction* eventAction) : fEventAction(eventAction)
{
  fMessenger = new G4GenericMessenger(this, "/tgt/created/", "Secondary-creation output");
  fMessenger->DeclareProperty("record", fRecord,
                              "Fill the 'created' ntuple (default true)");
}

TrackingAction::~TrackingAction()
{
  delete fMessenger;
}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  if (track->GetParentID() == 0) return;  // primary
  fEventAction->CountCreated();
  if (!fRecord) return;

  // At the start of tracking the track is at its creation point
  FillParticleRow(2, track, track->GetKineticEnergy(), track->GetMomentum(),
                  track->GetPosition(), track->GetPosition());
}
