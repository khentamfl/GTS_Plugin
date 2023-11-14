#pragma once
// Module that handles footsteps
#include "colliders/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class FootIkSolverData {
		public:
			FootIkSolverData(hkaFootPlacementIkSolver* ik);
			~FootIkSolverData();

			void ApplyScale(const float& new_scale, const hkVector4& vecScale);

			hkaFootPlacementIkSolver* solver;
		private:
			hkVector4 m_footEndLS;
            hkVector4 m_kneeAxisLS;

			hkVector4 m_worldUpDirectionWS;
            hkVector4 m_modelUpDirectionMS;

			float m_footPlantedAnkleHeightMS;
			float m_footRaisedAnkleHeightMS;
			float m_maxAnkleHeightMS;
			float m_minAnkleHeightMS;
			float m_raycastDistanceUp;
			float m_raycastDistanceDown;
            float m_originalGroundHeight;
	};
}