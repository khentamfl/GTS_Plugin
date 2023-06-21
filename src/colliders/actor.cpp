#include "colliders/actor.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ActorData::ActorData(Actor* actor): CharContData(actor ? actor->GetCharController() : nullptr) RagdollData(actor ? GetRagdoll(actor) : nullptr) {
	}

  std::vector<ColliderData*> ActorData::GetChildren() {
    return {
      &charCont,
      &ragdoll
    };
  }
}
