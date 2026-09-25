#ifndef EventAction_h
#define EventAction_h 1

#include "ParticleBlock.hh"

#include "G4UserEventAction.hh"
#include "globals.hh"

#include <unordered_map>
#include <vector>

// One event = one beam bunch. Holds all per-event output; the vectors are
// bound to the columns of the "events" ntuple in RunAction.
class EventAction : public G4UserEventAction
{
  public:
    EventAction() = default;
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddEdep(G4double e) { fEdep += e; }
    void CountCreated() { ++fNCreated; }

    // Beam electron (primary track ID) each track descends from
    void SetAncestor(G4int trackID, G4int primaryID) { fAncestor[trackID] = primaryID; }
    G4int GetAncestor(G4int trackID) const
    {
      const auto it = fAncestor.find(trackID);
      return it != fAncestor.end() ? it->second : -1;
    }

    ParticleBlock& Exit() { return fExit; }
    ParticleBlock& Created() { return fCreated; }

    // Accessors used for ntuple booking
    std::vector<double>& PrimE() { return fPrimE; }
    std::vector<double>& PrimThetaX() { return fPrimThetaX; }
    std::vector<double>& PrimThetaY() { return fPrimThetaY; }
    std::vector<double>& PrimX() { return fPrimX; }
    std::vector<double>& PrimY() { return fPrimY; }
    std::vector<double>& PrimZ() { return fPrimZ; }

  private:
    G4double fEdep = 0.;
    G4int fNCreated = 0;
    std::unordered_map<G4int, G4int> fAncestor;

    // Beam electrons, index i corresponds to track ID i+1
    std::vector<double> fPrimE, fPrimThetaX, fPrimThetaY, fPrimX, fPrimY, fPrimZ;

    ParticleBlock fExit;
    ParticleBlock fCreated;
};

#endif
