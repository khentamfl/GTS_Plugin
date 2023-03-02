#include "data/cosave.hpp"

using namespace SKSE;
using namespace RE;
 
namespace Gts {
	Cosave& Cosave::GetSingleton() noexcept {
		static Cosave instance;
		return instance;
	}

	void Cosave::OnRevert(SerializationInterface*) {
	}

	void Cosave::RegisterForSerde(std::uint32_t tag, Serde* callback) {
		Cosave::GetSingleton().serde_registry.try_emplace(tag, callback);
	}

	void Cosave::OnGameLoaded(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		std::uint32_t key;
		std::uint32_t size;
		std::uint32_t version;

		while (serde->GetNextRecordInfo(key, version, size)) {
			try {
				auto& record = Cosave::GetSingleton().serde_registry.at(key);
				record->De(serde, version);
			} catch (const std::out_of_range& oor) {
				log::warn("Unknown record type in Cosave ({}).", key);
			}
		}
	}

	void Cosave::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		for (auto const& [key, record]: Cosave::GetSingleton().serde_registry) {
			auto version = record->SerVersion();
			if (!serde->OpenRecord(key, version)) {
				log::error("Unable to open {} record to write Cosave data.", record->SerdeName());
				continue;
			} else {
				record->Ser(serde, version);
			}
		}
	}
}
