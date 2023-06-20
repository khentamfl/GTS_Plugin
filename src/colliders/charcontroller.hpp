#pragma once
// Takes a char controller and extracts its collision objects
#include "colliders/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class CharContData {
		public:
			CharContData(bhkCharacterController* charCont);
			void ReInit(bhkCharacterController* charCont);

			bhkCharacterController* charCont = nullptr;

      void DisableCollisions();
      void EnableCollisions();

		private:
      void AddRB(hkpRigidBody* rb);
			void AddPhantom(hkpPhantom* phantom);

			std::unordered_map<hkpRigidBody*, hkRefPtr<hkpRigidBody>> rbs;
			std::unordered_map<hkpPhantom*, hkRefPtr<hkpPhantom>> phantoms;
	};
}
