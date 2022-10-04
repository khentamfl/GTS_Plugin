// Module that handles footsteps
#include "managers/collider/footIK.hpp"
#include "util.hpp"

namespace Gts {
	FootIkData::FootIkData() {
		this->ik = nullptr;
	}
	FootIkData::FootIkData(hkbFootIkDriver* ik) {
		this->ik = nullptr;
		this->ChangeIk(ik);
	}
	FootIkData::~FootIkData() {
		if (this->ik) {
			this->ik->RemoveReference();
		}
	}

	void FootIkData::ChangeIk(hkbFootIkDriver* ik) {
		if (this->ik) {
			this->ik->RemoveReference();
		}
		this->ik = ik;
		if (this->ik) {
			log::info("Foot IK count {}", this->ik->GetReferenceCount());
			this->ik->AddReference();
		}
	}

	void FootIkData::ApplyScale(const float& new_scale, const hkVector4& vecScale) {
		if (!this->ik) {
			return;
		}
		for (auto& leg: this->ik->m_internalLegData) {
			auto solver = leg.m_footIkSolver;
			if (solver) {
				log::info("m_footEndLS: {}", Vector2Str(solver->m_setup.m_footEndLS));
				log::info("m_footPlantedAnkleHeightMS: {}", Vector2Str(solver->m_setup.m_footPlantedAnkleHeightMS));
				log::info("m_footRaisedAnkleHeightMS: {}", Vector2Str(solver->m_setup.m_footRaisedAnkleHeightMS));
				log::info("m_maxAnkleHeightMS: {}", Vector2Str(solver->m_setup.m_maxAnkleHeightMS));
				log::info("m_minAnkleHeightMS: {}", Vector2Str(solver->m_setup.m_minAnkleHeightMS));
				log::info("m_raycastDistanceUp: {}", Vector2Str(solver->m_setup.m_raycastDistanceUp));
				log::info("m_raycastDistanceDown: {}", Vector2Str(solver->m_setup.m_raycastDistanceDown));
			}
		}
	}
	void FootIkData::ApplyPose(const hkVector4& origin, const float& new_scale) {

	}
}
