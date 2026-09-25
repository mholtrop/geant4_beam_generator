#ifndef ParticleBlock_h
#define ParticleBlock_h 1

// A set of per-particle vectors that become vector columns (one entry per
// particle) in the per-event "events" ntuple. Used for the particles leaving
// the target (prefix "exit_") and the secondaries at creation ("created_").
//
// Columns (prefix + name):
//   track, parent        Geant4 track ID and parent track ID
//   primary              track ID of the beam electron this particle descends
//                        from (equal to track for a beam electron)
//   pdg, particle        PDG code and Geant4 particle name
//   process              creator process ("primary" for beam electrons)
//   E                    kinetic energy [MeV]
//   px, py, pz           momentum [MeV]
//   x, y, z              recorded point [mm]: exit point / creation point
//   vx, vy, vz           creation vertex of the track [mm]

#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"

#include <string>
#include <vector>

struct ParticleBlock
{
    std::vector<int> track, parent, primary, pdg;
    std::vector<std::string> particle, process;
    std::vector<double> E, px, py, pz, x, y, z, vx, vy, vz;

    void Book(G4AnalysisManager* am, const G4String& prefix)
    {
      am->CreateNtupleIColumn(prefix + "track", track);
      am->CreateNtupleIColumn(prefix + "parent", parent);
      am->CreateNtupleIColumn(prefix + "primary", primary);
      am->CreateNtupleIColumn(prefix + "pdg", pdg);
      am->CreateNtupleSColumn(prefix + "particle", particle);
      am->CreateNtupleSColumn(prefix + "process", process);
      am->CreateNtupleDColumn(prefix + "E", E);
      am->CreateNtupleDColumn(prefix + "px", px);
      am->CreateNtupleDColumn(prefix + "py", py);
      am->CreateNtupleDColumn(prefix + "pz", pz);
      am->CreateNtupleDColumn(prefix + "x", x);
      am->CreateNtupleDColumn(prefix + "y", y);
      am->CreateNtupleDColumn(prefix + "z", z);
      am->CreateNtupleDColumn(prefix + "vx", vx);
      am->CreateNtupleDColumn(prefix + "vy", vy);
      am->CreateNtupleDColumn(prefix + "vz", vz);
    }

    void Clear()
    {
      track.clear(); parent.clear(); primary.clear(); pdg.clear();
      particle.clear(); process.clear();
      E.clear(); px.clear(); py.clear(); pz.clear();
      x.clear(); y.clear(); z.clear(); vx.clear(); vy.clear(); vz.clear();
    }

    std::size_t Size() const { return track.size(); }

    void Add(const G4Track* trk, G4int primaryID, G4double ekin, const G4ThreeVector& mom,
             const G4ThreeVector& pos, const G4ThreeVector& vtx)
    {
      const auto* def = trk->GetParticleDefinition();
      const auto* creator = trk->GetCreatorProcess();
      track.push_back(trk->GetTrackID());
      parent.push_back(trk->GetParentID());
      primary.push_back(primaryID);
      pdg.push_back(def->GetPDGEncoding());
      particle.push_back(def->GetParticleName());
      process.push_back(creator != nullptr ? std::string(creator->GetProcessName())
                                           : std::string("primary"));
      E.push_back(ekin / MeV);
      px.push_back(mom.x() / MeV);
      py.push_back(mom.y() / MeV);
      pz.push_back(mom.z() / MeV);
      x.push_back(pos.x() / mm);
      y.push_back(pos.y() / mm);
      z.push_back(pos.z() / mm);
      vx.push_back(vtx.x() / mm);
      vy.push_back(vtx.y() / mm);
      vz.push_back(vtx.z() / mm);
    }
};

#endif
