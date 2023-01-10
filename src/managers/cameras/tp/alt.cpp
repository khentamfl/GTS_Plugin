#include "managers/cameras/tp/alt.hpp"
#include "data/runtime.hpp"

using namespace RE;

namespace Gts {
	NiPoint3 Alt::GetOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("cameraAlternateX"),
			0.0,
			Runtime::GetFloat("cameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffset(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("combatCameraAlternateX"),
			0.0,
			Runtime::GetFloat("combatCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCameraAlternateX"),
			0.0,
			Runtime::GetFloat("proneCameraAlternateY")
			);
	}

	NiPoint3 Alt::GetCombatOffsetProne(const NiPoint3& cameraPos) {
		return NiPoint3(
			Runtime::GetFloat("proneCombatCameraAlternateX"),
			0.0,
			Runtime::GetFloat("proneCombatCameraAlternateY")
			);
	}

	std::vector<std::string> Alt::GetBoneTargets() {
		//if (Function.GetValue() == 0.0) {
		//	return {};
		//};
		//else if (Function.GetValue() == 1.0) { // Spine0
			//return {
		//	 "NPC Spine [Spn0]",	
		//	};
		//};
	//	else if (Function.GetValue() == 2.0) { // Spine1
			//return {
			// "NPC Spine1 [Spn1]",	
			//};
		//};
		//else if (Function.GetValue() == 3.0) { // Breasts, non 3BA
			//return {
			// "NPC L Breast",	
			// "NPC R Breast",
			//};
		//};
	//	else if (Function.GetValue() == 4.0) { // 3BA Breasts
			//return {
			// "L Breast02",	
			// "R Breast02",
			//};
		//};
		//else if (Function.GetValue() == 5.0) { // Neck
			//return {
			// "NPC Neck [Neck]",	
			//};
		//};
		//else if (Function.GetValue() == 6.0) { // Butt
			//return {
			// "NPC L Butt",	
			// "NPC R Butt",	
			//};
		//};
		return {
		    "L Breast02",	
			"R Breast02",
		};
	}
}
