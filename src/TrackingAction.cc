#include "TrackingAction.hh"

#include "EventAction.hh"

#include "G4GenericMessenger.hh"
#include "G4Track.hh"

TrackingAction::TrackingAction(EventAction* eventAction) : fEventAction(eventAction)
{
  fMessenger = new G4GenericMessenger(this, "/tgt/created/", "Secondary-creation output");
  fMessenger->DeclareProperty("record", fRecord,
                              "Fill the created_* columns (default true)");
}

TrackingAction::~TrackingAction()
{
  delete fMessenger;
}

void TrackingAction::PreUserTrackingAction(const G4Track* track)
{
  // A parent is always tracked before its secondaries, so its ancestor is known.
  const G4int id = track->GetTrackID();
  const G4int parent = track->GetParentID();
  const G4int ancestor = (parent == 0) ? id : fEventAction->GetAncestor(parent);
  fEventAction->SetAncestor(id, ancestor);

  if (parent == 0) return;  // beam electron
  fEventAction->CountCreated();
  if (!fRecord) return;

  // At the start of tracking the track is at its creation point
  fEventAction->Created().Add(track, ancestor, track->GetKineticEnergy(), track->GetMomentum(),
                              track->GetPosition(), track->GetPosition());
}
