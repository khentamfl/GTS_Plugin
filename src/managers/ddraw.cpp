#include "managers/ddraw.hpp"
#include "UI/DebugAPI.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	DDraw& DDraw::GetSingleton() noexcept {
		static DDraw instance;
		return instance;
	}

	void DDraw::Update() {
		if (!this->enabled.load()) {
			return;
		}
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}
		if (!player->Is3DLoaded()) {
			return;
		}

		auto model = player->GetCurrent3D();
		if (model) {
			auto spine_node = find_node(player, "NPC Spine [Spn0]", Person::Current);
			if (spine_node) {
				DebugAPI::DrawSphere(Ni2Glm(spine_node->world.translate), meter_to_unit(2.0));
			}
		}
	}
}
