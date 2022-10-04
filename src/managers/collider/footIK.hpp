#pragma once
// Module that handles footsteps
#include "hooks/RE.hpp"
#include "managers/collider/footIKSolverData.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class FootIkData {
		public:
			FootIkData();
			FootIkData(hkbFootIkDriver* ik);
			~FootIkData();
			void ChangeIk(hkbFootIkDriver* ik);

			void ApplyScale(const float& new_scale, const hkVector4& vecScale);
			void AddSolver(hkaFootPlacementIkSolver* solver);
			void UpdateColliders(hkbFootIkDriver* ik);
			void PruneColliders(Actor* actor);

			hkbFootIkDriver* ik = nullptr;
		private:
			mutable std::mutex _lock;

			std::unordered_map<hkaFootPlacementIkSolver*, FootIkSolverData> solver_data;
	};
}
