#include "actorUtils.hpp"
#include "papyrusUtils.hpp"

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
		actor->criticalStage.set(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
	}

	void UnDisintegrate(Actor* actor) {
		actor->criticalStage.reset(ACTOR_CRITICAL_STAGE::kDisintegrateEnd);
	}

	void SetRestrained(Actor* actor) {
		actor->actorState1.lifeState = ACTOR_LIFE_STATE::kRestrained;
	}

	void SetUnRestrained(Actor* actor) {
		actor->actorState1.lifeState = ACTOR_LIFE_STATE::kAlive;
	}

	void SetDontMove(Actor* actor) {
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		auto vm = skyrimVM ? skyrimVM->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(true));
			auto actorPtr = GetObjectPtr(actor, "Actor", false);
			vm->DispatchMethodCall(actorPtr, "SetDontMove", args, callback);
		}
	}

	void SetMove(Actor* actor) {
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		auto vm = skyrimVM ? skyrimVM->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(false));
			auto actorPtr = GetObjectPtr(actor, "Actor", false);
			vm->DispatchMethodCall(actorPtr, "SetDontMove", args, callback);
		}
	}

	void KnockAreaEffect(Actor* actor, float afMagnitude, float afRadius) {
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		auto vm = skyrimVM ? skyrimVM->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(afMagnitude), std::move(afRadius));
			auto actorPtr = GetObjectPtr(actor, "Actor", false);
			vm->DispatchMethodCall(actorPtr, "KnockAreaEffect", args, callback);
		}
	}
}
