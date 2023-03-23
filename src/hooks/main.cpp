#include "hooks/main.hpp"
#include "managers/Attributes.hpp"
#include "events.hpp"
#include "data/time.hpp"
#include "data/plugin.hpp"
#include "timer.hpp"
#include "Config.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	void PopulateHook(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon) {
		//auto& attributes = AttributeManager::GetSingleton();
		//if (Attributes) {
		//a_this->totalDamage *= GetAttributeBonus(a_aggressor, ActorValue::kAttackDamageMult);
		//}
		//log::info("Actor {} HP before: {}", a_target->GetDisplayFullName(), a_target->GetActorValue(ActorValue::kHealth));
		//log::info("Before: Att: {}, totalDamage: {}, physical: {}, bonusHealthDamageMult: {}, targetedLimbDamage: {}", a_aggressor->GetDisplayFullName(), a_this->totalDamage, a_this->physicalDamage, a_this->bonusHealthDamageMult, a_this->targetedLimbDamage);
		a_this->totalDamage *= 0;
		a_this->physicalDamage *= 0;
		a_this->bonusHealthDamageMult *= 0;
		a_this->targetedLimbDamage *= 0;
		//log::info("Actor {} HP After: {}", a_target->GetDisplayFullName(), a_target->GetActorValue(ActorValue::kHealth));
		//log::info("After: Att: {}, totalDamage: {}, physical: {}, bonusHealthDamageMult: {}, targetedLimbDamage: {}", a_aggressor->GetDisplayFullName(), a_this->totalDamage, a_this->physicalDamage, a_this->bonusHealthDamageMult, a_this->targetedLimbDamage);
	}

	//bool CheckHealthGate(Actor* receiver, float damage) {

	//}
}



namespace Hooks
{

	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};
		logger::info("Gts applying Main Update Hook at {:X}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x11F, 0x160), Update);

		/*if (REL::Module::IsSE()) {
		        // auto offsetHelper = REL::IDDatabase::Offset2ID();
		        // log::info("Dumping OFFSETS");
		        // for (auto& offsetData: offsetHelper) {
		        // 	log::info("{}:{:X}:{}", offsetData.id, offsetData.offset, offsetData.offset);
		        // }

		        REL::Relocation<uintptr_t*> hook1(REL::ID(37606), REL::Offset(0xb6));
		        logger::info("Applying PopulateHook1: {:X}:{:X}", hook1.address(), *hook1.get());
		        _PopulateHook1 = trampoline.write_call<5>(hook1.address(), PopulateHook1);
		        logger::info("  - Applied PopulateHook1");

		        REL::Relocation<uintptr_t*> hook2(REL::ID(37673), REL::Offset(0x1b7));
		        logger::info("Applying PopulateHook2: {:X}:{:X}", hook2.address(), *hook2.get());
		        _PopulateHook2 = trampoline.write_call<5>(hook1.address(), PopulateHook2);
		        logger::info("  - Applied PopulateHook2");

		        REL::Relocation<uintptr_t*> hook3(REL::ID(37674), REL::Offset(0xeb));
		        logger::info("Applying PopulateHook3: {:X}:{:X}", hook3.address(), *hook3.get());
		        _PopulateHook3 = trampoline.write_call<5>(hook1.address(), PopulateHook3);
		        logger::info("  - Applied PopulateHook3");

		        REL::Relocation<uintptr_t*> hook4(REL::ID(42830), REL::Offset(0x83));
		        logger::info("Applying PopulateHook4: {:X}:{:X}", hook4.address(), *hook4.get());
		        _PopulateHook4 = trampoline.write_call<5>(hook1.address(), PopulateHook4);
		        logger::info("  - Applied PopulateHook4");
		   }*/
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);
		Time::GetSingleton().Update();

		static std::atomic_bool started = std::atomic_bool(false);
		Plugin::SetOnMainThread(true);
		if (Plugin::Live()) {
			// We are not loading or in the mainmenu
			// Player loaded and not paused
			if (started.exchange(true)) {
				// Not first updated
				EventDispatcher::DoUpdate();
			} else {
				// First update this load
				EventDispatcher::DoStart();
			}
		} else if (!Plugin::InGame()) {
			// Loading or in main menu
			started.store(false);
		}
		Plugin::SetOnMainThread(false);

		if (Config::GetSingleton().GetDebug().ShouldProfile()) {
			static Timer timer = Timer(5.0);
			if (timer.ShouldRun()) {
				EventDispatcher::ReportProfilers();
			}
		}
	}

	void Hook_MainUpdate::PopulateHook1(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon) {
		_PopulateHook1(a_this, a_aggressor, a_target, a_weapon);
		PopulateHook(a_this, a_aggressor, a_target, a_weapon);
	}

	void Hook_MainUpdate::PopulateHook2(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon) {
		_PopulateHook2(a_this, a_aggressor, a_target, a_weapon);
		PopulateHook(a_this, a_aggressor, a_target, a_weapon);
	}

	void Hook_MainUpdate::PopulateHook3(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon) {
		_PopulateHook3(a_this, a_aggressor, a_target, a_weapon);
		PopulateHook(a_this, a_aggressor, a_target, a_weapon);
	}

	void Hook_MainUpdate::PopulateHook4(HitData* a_this, Actor* a_aggressor, Actor* a_target, InventoryEntryData* a_weapon) {
		_PopulateHook4(a_this, a_aggressor, a_target, a_weapon);
		PopulateHook(a_this, a_aggressor, a_target, a_weapon);
	}
}
