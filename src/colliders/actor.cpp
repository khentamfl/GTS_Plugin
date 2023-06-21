#include "colliders/actor.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ActorData::ActorData(Actor* actor): charCont(actor ? actor->GetCharController() : nullptr), ragdoll(actor ? GetRagdoll(actor) : nullptr) {
	}

  std::vector<ColliderData*> ActorData::GetChildren() {
    std::vector<ColliderData*> result = {};
    result.push_back(&this->charCont);
    result.push_back(&this->ragdoll);
    return result;
  }
}
