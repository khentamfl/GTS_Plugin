#include "hooks/headTracking.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	void Hook_HeadTracking::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook1{ RELOCATION_ID(36984, 38009) };                                    // 60D300, 635270
		REL::Relocation<uintptr_t> hook2{ RELOCATION_ID(36376, 37367) };                                    // 5D9BF0, 5FF320
		REL::Relocation<uintptr_t> hook3{ RELOCATION_ID(36220, 37200) };                                    // 5CFD60, 5F4320
		REL::Relocation<uintptr_t> hook4{ RELOCATION_ID(36540, 37541) };                                    // 5E8070, 60E050

		_SetHeadtrackTarget0 = trampoline.write_call<5>(hook1.address() + RELOCATION_OFFSET(0x592, 0x594), SetHeadtrackTarget0);    // 60D892
		_SetHeadtrackTarget4A = trampoline.write_call<5>(hook2.address() + RELOCATION_OFFSET(0xA3, 0xA3), SetHeadtrackTarget4A);   // 5D9C93
		_SetHeadtrackTarget4B = trampoline.write_call<5>(hook3.address() + RELOCATION_OFFSET(0x45C, 0x470), SetHeadtrackTarget4B);  // 5D01BC
		_SetHeadtrackTarget4C = trampoline.write_call<5>(hook3.address() + RELOCATION_OFFSET(0x5BF, 0x5D3), SetHeadtrackTarget4C);  // 5D031F
		_SetHeadtrackTarget4D = trampoline.write_call<5>(hook4.address() + RELOCATION_OFFSET(0x17E, 0x181), SetHeadtrackTarget4D);  // 5E81EE
	}

	void Hook_HeadTracking::SetHeadtrackTarget0(RE::AIProcess* a_this, RE::Actor* a_target)
	{
		auto owner = a_this->GetUserData();
		if (owner) {
			if (a_target) {
				//log::info("{} is head tracking: {}", owner->GetDisplayFullName(), a_target->GetDisplayFullName());
			}
		}
		_SetHeadtrackTarget0(a_this, a_target);
	}
	void Hook_HeadTracking::SetHeadtrackTarget4A(RE::AIProcess* a_this, RE::Actor* a_target)
	{
		auto owner = a_this->GetUserData();
		if (owner) {
			if (a_target) {
				//log::info("{} is head tracking: {}", owner->GetDisplayFullName(), a_target->GetDisplayFullName());
			}
		}
		_SetHeadtrackTarget4A(a_this, a_target);
	}
	void Hook_HeadTracking::SetHeadtrackTarget4B(RE::AIProcess* a_this, RE::Actor* a_target)
	{
		auto owner = a_this->GetUserData();
		if (owner) {
			if (a_target) {
				//log::info("{} is head tracking: {}", owner->GetDisplayFullName(), a_target->GetDisplayFullName());
			}
		}
		_SetHeadtrackTarget4B(a_this, a_target);
	}
	void Hook_HeadTracking::SetHeadtrackTarget4C(RE::AIProcess* a_this, RE::Actor* a_target)
	{
		auto owner = a_this->GetUserData();
		if (owner) {
			if (a_target) {
				//log::info("{} is head tracking: {}", owner->GetDisplayFullName(), a_target->GetDisplayFullName());
			}
		}
		_SetHeadtrackTarget4C(a_this, a_target);
	}
	void Hook_HeadTracking::SetHeadtrackTarget4D(RE::AIProcess* a_this, RE::Actor* a_target)
	{
		auto owner = a_this->GetUserData();
		if (owner) {
			if (a_target) {
				//log::info("{} is head tracking: {}", owner->GetDisplayFullName(), a_target->GetDisplayFullName());
			}
		}
		_SetHeadtrackTarget4D(a_this, a_target);
	}
}
