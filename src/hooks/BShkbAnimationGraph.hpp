#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_BShkbAnimationGraph
	{
		public:
			static void Hook();
		private:

			static void AddRagdollToWorld(BShkbAnimationGraph* a_this);
			static inline REL::Relocation<decltype(AddRagdollToWorld)> _AddRagdollToWorld;

			static void RemoveRagdollFromWorld(BShkbAnimationGraph* a_this);
			static inline REL::Relocation<decltype(RemoveRagdollFromWorld)> _RemoveRagdollFromWorld;

			static void ResetRagdoll(BShkbAnimationGraph* a_this);
			static inline REL::Relocation<decltype(RemoveRagdollFromWorld)> _ResetRagdoll;

			static void SetRagdollConstraintsFromBhkConstraints(BShkbAnimationGraph* a_this);
			static inline REL::Relocation<decltype(RemoveRagdollFromWorld)> _SetRagdollConstraintsFromBhkConstraints;
	};
}