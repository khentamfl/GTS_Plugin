#include "data/cosave.hpp"
#include <sstream>

using namespace SKSE;
using namespace RE;

namespace {
  inline const auto TomlRecord = _byteswap_ulong('TOML');

  void FixUpFormIDs(toml::value& v, SerializationInterface* serde) {
    if (v.is_array()) {
      for (auto& child: v.as_array()) {
        FixUpFormIDs(child, serde);
      }
    } else if (v.is_table()) {
      for (auto& [key, child]: v.as_table()) {
        if (str_tolower(key) == "formid") {
          auto maybeFormID = toml::expect<RE::FormID>(child);
          if (maybeFormID.is_ok()) {
            FormID oldID = maybeFormID.unwrap();
            FormID newID;
            if (serde->ResolveFormID(oldID, newID)) {
              child = newID;
            } else {
              child = 0;
              log::warn("Could not find formID {} after game load", oldID);
            }
          }
        } else {
          FixUpFormIDs(child, serde);
        }
      }
    }
  }
}

namespace Gts {
	Cosave& Cosave::GetSingleton() noexcept {
		static Cosave instance;
		return instance;
	}

	void Cosave::OnRevert(SerializationInterface*) {
	}

	void Cosave::RegisterForSerde(std::string tag, Serde* callback) {
		Cosave::GetSingleton().serde_registry.try_emplace(tag, callback);
	}

	void Cosave::OnGameLoaded(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		std::uint32_t key;
		std::uint32_t size;
		std::uint32_t version;

		while (serde->GetNextRecordInfo(key, version, size)) {
      if (key == TomlRecord) {
        std::string serData = DesString(serde);
        std::istringstream serStream(serData);
        toml::value allData = toml::parse(serStream, "COSAVE.toml");
        FixUpFormIDs(allData);
        // Fix up all formIDs
        for (const auto& [key, value]: allData.as_table()) {
          try {
            auto& record = Cosave::GetSingleton().serde_registry.at(key);
            record->LoadData(value, serde);
          } catch (const std::out_of_range& oor) {
    				log::warn("Unknown TOML table in Cosave ({}).", key);
    			}
        }
        for (auto const& [key, record]: ) {
        }
      } else {
  			try {
  				auto& record = Cosave::GetSingleton().directSerdeRegistry.at(key);
  				record->LegacyLoadData(key, version, size, serde);
  			} catch (const std::out_of_range& oor) {
  				log::warn("Unknown record type in Cosave ({}).", key);
  			}
      }
		}
	}

	void Cosave::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		if (!serde->OpenRecord(TomlRecord, 1)) {
			log::error("Unable to open record to write Cosave data.");
			continue;
		} else {
      toml::table data;

      for (auto const& [key, record]: Cosave::GetSingleton().serde_registry) {
        data.try_emplace(key, record.SaveData());
      }

      toml::value allData(data);
      std::string serData = toml::format(allData, 0);
      log::debug("Saved: {}", serData);
      SerString(serde, serData);
		}
	}
}
