#include "managers/ShrinkToNothingManager.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "actorUtils.hpp"
#include "papyrusUtils.hpp"
#include "util.hpp"

#include <random>

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace Gts {
	ShrinkToNothingManager& ShrinkToNothingManager::GetSingleton() noexcept {
		static ShrinkToNothingManager instance;
		return instance;
	}

	std::string ShrinkToNothingManager::DebugName() {
		return "ShrinkToNothingManager";
	}

	void ShrinkToNothingManager::Update() {
		for (auto &[tiny, data]: this->data) {
			auto giant = data.giant;
			if (!tiny) {
				continue;
			}
			if (!giant) {
				continue;
			}
			//if (!tiny->Is3DLoaded()) {
				//continue;
			//}
			//if (!giant->Is3DLoaded()) {
				//continue;
			//}
			if (data.state == ShrinkState::Healthy) {
				tiny->KillImmediate();
				data.state = ShrinkState::Shrinking;
			} else if (data.state == ShrinkState::Shrinking) {
				if (data.delay.ShouldRun()) {
					if (!tiny->IsDead()) {
						// They cannot be shrunked to nothing if they aint dead
						this->ResetActor(tiny);
						return;
					}

					// Do shrink
					float currentSize = get_visual_scale(tiny);
					if (currentSize > 0.01) {
						set_target_scale(tiny, 0.005);
					} else {
						// Fully shrunk
						if (giant->formID == 0x14 && Runtime::GetBool("GtsEnableLooting")) {
							Actor* into = giant;
							TransferInventory(tiny, into, false, true);
						} else if (giant->formID != 0x14 && Runtime::GetBool("GtsNPCEnableLooting")) {
							Actor* into = giant;
							TransferInventory(tiny, into, false, true);
						}
						Disintegrate(tiny);

						std::random_device rd;
						std::mt19937 gen(rd());
						std::uniform_real_distribution<float> dis(-0.2, 0.2);

						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Head", NiPoint3{dis(gen), 0, -1}, 512, true, true);
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC L Foot [Lft]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC R Foot [Rft]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Spine [Spn0]", NiPoint3{dis(gen), 0, -1}, 512, true, false);

						data.state = ShrinkState::Shrinked;
					}

				}
			}
		}
	}

	void ShrinkToNothingManager::Reset() {
		this->data.clear();
	}

	void ShrinkToNothingManager::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void ShrinkToNothingManager::Shrink(Actor* giant, Actor* tiny) {
		if (ShrinkToNothingManager::CanShrink(giant, tiny)) {
			ShrinkToNothingManager::GetSingleton().data.try_emplace(tiny, giant, tiny);
		}
	}

	bool ShrinkToNothingManager::AlreadyShrinked(Actor* actor) {
		auto& m = ShrinkToNothingManager::GetSingleton().data;
		return !(m.find(actor) == m.end());
	}

	bool ShrinkToNothingManager::CanShrink(Actor* giant, Actor* tiny) {
		if (ShrinkToNothingManager::AlreadyShrinked(tiny)) {
			return false;
		}

		return true;
	}

	ShrinkData::ShrinkData(Actor* giant, Actor* tiny) :
		delay(Timer(0.01)),
		state(ShrinkState::Healthy),
		giant(giant) {
	}
}
