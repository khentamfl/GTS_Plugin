#include "managers/actorUtils.hpp"

using namespace RE;
using namespace Gts;

namespace {
	using VM = RE::BSScript::Internal::VirtualMachine;
	using ObjectPtr = RE::BSTSmartPointer<RE::BSScript::Object>;
	inline RE::VMHandle GetHandle(RE::TESForm* a_form)
	{
		auto vm = VM::GetSingleton();
		auto policy = vm->GetObjectHandlePolicy();
		return policy->GetHandleForObject(a_form->GetFormType(), a_form);
	}

	inline ObjectPtr GetObjectPtr(RE::TESForm* a_form, const char* a_class, bool a_create)
	{
		auto vm = VM::GetSingleton();
		auto handle = GetHandle(a_form);

		ObjectPtr object = nullptr;
		bool found = vm->FindBoundObject(handle, a_class, object);
		if (!found && a_create) {
			vm->CreateObject2(a_class, object);
			vm->BindObject(object, handle, false);
		}

		return object;
	}

}

namespace Gts {
	void PlayAnimation(Actor* actor, std::string_view animName) {
		actor->NotifyAnimationGraph(animName);
	}

	void TransferInventory(Actor* from, Actor* to, bool keepOwnership, bool removeQuestItems) {
		for (auto &[a_object, pair]: from->GetInventory()) {
			const auto& [count, entry] = invData;
			if (!removeQuestItems && entry.IsQuestObject()) {
				continue;
			}
			RE::ExtraDataList* a_extraList = new RE::ExtraDataList();
			if (keepOwnership) {
				a_extraList->SetOwner(entry.GetOwner());
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
		actor->criticalStage.set(ACTOR_CRITICAL_STAGE::kNone);
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

	void PlayImpactEffect(Actor* actor, BGSImpactDataSet* a_impactEffect, std::string_view node, NiPoint3& direction, float length, bool applyRotation, bool useLocalRotation) {
		auto impact = BGSImpactManager::GetSingleton();
		impact->PlayImpactEffect(actor, a_impactEffect, node, direction, length, applyRotation, useLocalRotation);
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
