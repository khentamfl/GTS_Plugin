#include "actorUtils.hpp"
#include "papyrusUtils.hpp"
#include "data/runtime.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	void PlayAnimation(Actor* actor, std::string_view animName) {
		actor->NotifyAnimationGraph(animName);
	}

	void TransferInventory(Actor* from, Actor* to, bool keepOwnership, bool removeQuestItems) {
		for (auto &[a_object, invData]: from->GetInventory()) {
			const auto& [count, entry] = invData;
			if (!removeQuestItems && entry->IsQuestObject()) {
				continue;
			}
			RE::ExtraDataList* a_extraList = new RE::ExtraDataList();
			if (keepOwnership) {
				a_extraList->SetOwner(entry->GetOwner());
			} else {
				a_extraList->SetOwner(to);
			}
			to->AddObjectToContainer(a_object, a_extraList, count, from);
		}
	}

	void Disintegrate(Actor* actor) {
		CallFunctionOn(actor, "SetCriticalStage", 4);
	}

	void UnDisintegrate(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetCriticalStage", 0);
	}

	void SetRestrained(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetRestrained", true);
	}

	void SetUnRestrained(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetRestrained", false);
	}

	void SetDontMove(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetDontMove", true);
	}

	void SetMove(Actor* actor) {
		CallFunctionOn(actor, "Actor", "SetDontMove", true);
	}

	void KnockAreaEffect(Actor* actor, float afMagnitude, float afRadius) {
		CallFunctionOn(actor, "Actor", "KnockAreaEffect", afMagnitude, afRadius);
	}

	bool IsDragon(Actor* actor) {
		return (
			std::string(actor->GetDisplayFullName()).find("ragon") != std::string::npos
			|| Runtime::IsRace(actor, "dragonRace")
			);
	}
}
