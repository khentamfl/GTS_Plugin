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
		if (this->ik == ik) {
			return;
		}
		if (this->ik) {
			this->ik->RemoveReference();
		}
		this->ik = ik;
		if (this->ik) {
			log::info("Foot IK count {}", this->ik->GetReferenceCount());
			this->ik->AddReference();

			for (auto& leg: this->ik->m_internalLegData) {
				auto solver = leg.m_footIkSolver;
				this->AddSolver(solver);
			}
		}
	}

	void FootIkData::ApplyScale(const float& new_scale, const hkVector4& vecScale) {
		for (auto &[key, data]: this->solver_data) {
			data.ApplyScale(new_scale, vecScale);
		}
	}

	void FootIkData::PruneColliders(Actor* actor) {
		// for (auto i = this->solver_data.begin(); i != this->solver_data.end();) {
		// 	auto& data = (*i);
		// 	auto key = data.first;
		// 	if (key->GetReferenceCount() == 1) {
		// 		i = this->solver_data.erase(i);
		// 	} else {
		// 		++i;
		// 	}
		// }
	}

	void FootIkData::AddSolver(hkaFootPlacementIkSolver* solver) {
		if (solver) {
			log::info("Foot IK Solver count {}", solver->GetReferenceCount());
			log::info("Foot IK Solver Name {}", GetRawName(solver));
			this->solver_data.try_emplace(solver, solver);
		}
	}
}
