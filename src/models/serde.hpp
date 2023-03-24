#pragma once
// Serde generalisation
//
#include <type_traits>
#include "toml.hpp"

using namespace std;
using namespace SKSE;

namespace Gts {
	/// Helper to Serialises a String
	void SerString(SerializationInterface* serde, string_view str);
	/// Helper to DeSerialises a String
	std::string DesString(SerializationInterface* serde);

	class Serde {
		public:
			// Serialisation (save) method
			virtual toml::value SaveData() = 0;

			// Desearilation (load) method
			virtual void LoadData(const toml::value& table) = 0;
	};

  // Unlike Serde which is done via a toml11 interfrace
  // this one manipulates the SerializationInterface
  // directly. It is used for loading the
  // legacy data
  class DirectSerde {
		public:
			// Desearilation (load) method
			virtual void LegacyLoadData(std::uint32_t key, std::uint32_t version, std::uint32_t size, SerializationInterface* serde) = 0;
	};
}

using namespace RE;
// Used to put in generic conversions from CommonLib
namespace toml {
    template<>
    struct into<NiPoint3> {
        static toml::value into_toml(const NiPoint3& f) {
            return toml::value{
              {"version", 1u32}
              {"x", f.x},
              {"y", f.y},
              {"z", f.z},
            };
        }
      };

    template<>
    struct from<NiPoint3> {
      static NiPoint3 from_toml(const toml::value& v) {
          return NiPoint3(
            find<float>(v, "x"),
            find<float>(v, "y"),
            find<float>(v, "z")
          );
      }
    };
}
