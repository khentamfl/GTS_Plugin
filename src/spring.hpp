#pragma once
// Critically Damped Springs
//
#include "events.hpp"
#include "models/serde.hpp"

using namespace SKSE;

namespace Gts {
	class SpringBase {
		public:
			virtual void Update(float delta) = 0;
		protected:
			void UpdateValues(float& value, const float& target, float & velocity, const float& halflife, const float& dt);
	};

	class Spring : public SpringBase {
		public:
			void Ser(SerializationInterface* serde) override;
			void Des(SerializationInterface* serde) override;

			float value = 0.0;
			float target = 0.0;
			float velocity = 0.0;
			float halflife = 1.0;

			void Update(float dt) override;

			Spring();
			Spring(float initial, float halflife);

			~Spring();
	};

	class Spring3 : public SpringBase {
		public:
			NiPoint3 value = NiPoint3(0.0, 0.0, 0.0);
			NiPoint3 target = NiPoint3(0.0, 0.0, 0.0);
			NiPoint3 velocity = NiPoint3(0.0, 0.0, 0.0);
			float halflife = 1.0;

			void Update(float delta) override;

			Spring3();
			Spring3(NiPoint3 initial, float halflife);

			~Spring3();
	};

	class SpringManager : public EventListener {
		public:
			static SpringManager& GetSingleton();

			static void AddSpring(SpringBase* spring);
			static void RemoveSpring(SpringBase* spring);

			virtual std::string DebugName() override;
			virtual void Update() override;

			std::unordered_set<SpringBase*> springs;
	};
}

namespace toml {
    template<>
    struct into<Gts::Spring> {
        static toml::value into_toml(const Gts::Spring& f) {
            return toml::value{
              {"version", 1u32}
              {"value", f.value},
              {"target", f.target},
              {"velocity", f.velocity},
              {"halflife", f.halflife},
            };
        }
      };

    template<>
    struct from<Gts::Spring> {
      static Gts::Spring from_toml(const toml::value& v) {
          Gts::Spring f = Spring();
          f.value = find<float>(v, "value")
          f.target = find<float>(v, "target")
          f.velocity = find<float>(v, "velocity")
          f.halflife = find<float>(v, "halflife")

          return f;
      }
    };

    template<>
    struct into<Gts::Spring3> {
        static toml::value into_toml(const Gts::Spring3& f) {
            return toml::value{
              {"version", 1u32}
              {"value", f.value},
              {"target", f.target},
              {"velocity", f.velocity},
              {"halflife", f.halflife},
            };
        }
      };

    template<>
    struct from<Gts::Spring3> {
      static Gts::Spring3 from_toml(const toml::value& v) {
          Gts::Spring3 f = Spring();
          f.value = find<NiPoint3>(v, "value")
          f.target = find<NiPoint3>(v, "target")
          f.velocity = find<NiPoint3>(v, "velocity")
          f.halflife = find<float>(v, "halflife")

          return f;
      }
    };
} // toml
