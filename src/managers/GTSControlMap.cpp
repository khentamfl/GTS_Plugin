#include "managers/GTSControlMap.hpp"

#include "RE/B/BSInputDeviceManager.h"
#include "RE/U/UserEventEnabled.h"

namespace RE  /// Copied from CommonLib version of AlandtSE, since im dumb and unsure how to switch dependencies.
{
	GTSControlMap* GTSControlMap::GetSingleton()
	{
		REL::Relocation<GTSControlMap**> singleton{ Offset::GTSControlMap::Singleton };
		return *singleton;
	}

	std::int8_t GTSControlMap::AllowTextInput(bool a_allow)
	{
		if (a_allow) {
			if (GetRuntimeData().textEntryCount != -1) {
				++GetRuntimeData().textEntryCount;
			}
		} else {
			if (GetRuntimeData().textEntryCount != 0) {
				--GetRuntimeData().textEntryCount;
			}
		}

		return GetRuntimeData().textEntryCount;
	}

	bool GTSControlMap::GetButtonNameFromUserEvent(const BSFixedString& a_eventID, INPUT_DEVICE a_device, BSFixedString& a_buttonName)
	{
		for (const auto& inputContext : GTSControlMap) {
			if (!inputContext) {
				continue;
			}

			for (const auto& mapping : inputContext->deviceMappings[a_device]) {
				if (mapping.eventID == a_eventID) {
					if (mapping.inputKey == 0xFF) {
						break;
					}

					const auto inputDeviceManager = BSInputDeviceManager::GetSingleton();
					inputDeviceManager->GetButtonNameFromID(a_device, mapping.inputKey, a_buttonName);
					return true;
				}
			}
		}

		return false;
	}

	std::uint32_t GTSControlMap::GetMappedKey(std::string_view a_eventID, INPUT_DEVICE a_device, InputContextID a_context) const
	{
		assert(a_device < INPUT_DEVICE::kTotal);
		assert(a_context < InputContextID::kTotal);

		if (GTSControlMap[a_context]) {
			const auto&   mappings = GTSControlMap[a_context]->deviceMappings[a_device];
			BSFixedString eventID(a_eventID);
			for (auto& mapping : mappings) {
				if (mapping.eventID == eventID) {
					return mapping.inputKey;
				}
			}
		}

		return kInvalid;
	}

	bool GTSControlMap::GetMappingFromEventName(const BSFixedString& a_eventID, UserEvents::INPUT_CONTEXT_ID a_context, INPUT_DEVICE a_device, UserEventMapping& a_mapping)
	{
		const auto context = GTSControlMap[a_context];
		if (context) {
			for (auto& mapping : context->deviceMappings[a_device]) {
				if (mapping.eventID == a_eventID) {
					a_mapping = mapping;
					return true;
				}
			}
		}

		return false;
	}

	std::string_view GTSControlMap::GetUserEventName(std::uint32_t a_buttonID, INPUT_DEVICE a_device, InputContextID a_context) const
	{
		assert(a_device < INPUT_DEVICE::kTotal);
		assert(a_context < InputContextID::kTotal);

		if (GTSControlMap[a_context]) {
			const auto&      mappings = GTSControlMap[a_context]->deviceMappings[a_device];
			UserEventMapping tmp{};
			tmp.inputKey = static_cast<std::uint16_t>(a_buttonID);
			auto range = std::equal_range(
				mappings.begin(),
				mappings.end(),
				tmp,
				[](auto&& a_lhs, auto&& a_rhs) {
					return a_lhs.inputKey < a_rhs.inputKey;
				});

			if (std::distance(range.first, range.second) == 1) {
				return range.first->eventID;
			}
		}

		return ""sv;
	}

	void GTSControlMap::PopInputContext(InputContextID a_context)
	{
		using func_t = decltype(&GTSControlMap::PopInputContext);
		REL::Relocation<func_t> func{ RELOCATION_ID(67244, 68544) };
		return func(this, a_context);
	}

	void GTSControlMap::PushInputContext(InputContextID a_context)
	{
		using func_t = decltype(&GTSControlMap::PushInputContext);
		REL::Relocation<func_t> func{ RELOCATION_ID(67243, 68543) };
		return func(this, a_context);
	}

	void GTSControlMap::ToggleControls(UEFlag a_flags, bool a_enable)
	{
		auto oldState = GetRuntimeData().enabledControls;

		if (a_enable) {
			GetRuntimeData().enabledControls.set(a_flags);
			if (GetRuntimeData().unk11C != UEFlag::kInvalid) {
				GetRuntimeData().unk11C.set(a_flags);
			}
		} else {
			GetRuntimeData().enabledControls.reset(a_flags);
			if (GetRuntimeData().unk11C != UEFlag::kInvalid) {
				GetRuntimeData().unk11C.reset(a_flags);
			}
		}

		UserEventEnabled event{ GetRuntimeData().enabledControls, oldState };
		SendEvent(std::addressof(event));
	}
}