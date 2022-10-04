#include "managers/collider/footIKSolverData.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	FootIkSolverData::FootIkSolverData(hkaFootPlacementIkSolver* solver) {
		this->solver = solver;
		this->solver->AddReference();
		log::info("solver memSizeAndFlags: {:X}", this->solver->memSizeAndFlags);
		this->m_footEndLS = solver->m_setup.m_footEndLS;
		this->m_footPlantedAnkleHeightMS = solver->m_setup.m_footPlantedAnkleHeightMS;
		this->m_footRaisedAnkleHeightMS = solver->m_setup.m_footRaisedAnkleHeightMS;
		this->m_maxAnkleHeightMS = solver->m_setup.m_maxAnkleHeightMS;
		this->m_minAnkleHeightMS = solver->m_setup.m_minAnkleHeightMS;
		this->m_raycastDistanceUp = solver->m_setup.m_raycastDistanceUp;
		this->m_raycastDistanceDown = solver->m_setup.m_raycastDistanceDown;
	}
	FootIkSolverData::~FootIkSolverData() {
		this->solver->RemoveReference();
	}

	void FootIkSolverData::ApplyScale(const float& new_scale, const hkVector4& vecScale) {
		log::info("Checking: {}, RawName: {}", reinterpret_cast<std::uintptr_t>(this->solver), GetRawName(this->solver));
		this->solver->m_setup.m_footEndLS = this->m_footEndLS * vecScale;
		this->solver->m_setup.m_footPlantedAnkleHeightMS = this->m_footPlantedAnkleHeightMS * new_scale;
		this->solver->m_setup.m_footRaisedAnkleHeightMS = this->m_footRaisedAnkleHeightMS * new_scale;
		this->solver->m_setup.m_maxAnkleHeightMS = this->m_maxAnkleHeightMS  * new_scale;
		this->solver->m_setup.m_minAnkleHeightMS = this->m_minAnkleHeightMS * new_scale;
		this->solver->m_setup.m_raycastDistanceUp = this->m_raycastDistanceUp * new_scale;
		this->solver->m_setup.m_raycastDistanceDown = this->m_raycastDistanceDown * new_scale;
	}
}
