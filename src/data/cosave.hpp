#pragma once
// Module that holds data that is persistent across saves

#include "models/serde.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	class Cosave {
		public:
			[[nodiscard]] static Cosave& GetSingleton() noexcept;
			static void OnRevert(SKSE::SerializationInterface*);
			static void OnGameSaved(SKSE::SerializationInterface* serde);
			static void OnGameLoaded(SKSE::SerializationInterface* serde);

			static void RegisterForSerde(std::string tag, Serde* callback);
      static void RegisterForDirectSerde(std::string tag, Serde* callback);

			std::unordered_map<std::string, Serde*> serde_registry;
      std::unordered_map<std::uint32_t, DirectSerde*> directSerdeRegistry;

		private:
			Cosave() = default;

			mutable std::mutex _lock;
	};
}
