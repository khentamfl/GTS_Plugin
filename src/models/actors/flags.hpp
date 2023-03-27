// This is similar to actorvalues but deals with BOOLs
//

using namespace Gts;

namespace Gts {
  class GFlag {
    public:
      std::string name = "";
      bool value = false;

      GFlag(std::string name, bool value) : name(name), value(value) {}

      bool Get() {
        return this->value;
      }

      void Set(bool v) {
        this->value = v;
      }

      void On() {
        this->Set(true);
      }

      void Off() {
        this->Set(false);
      }
  };

  class FlagHolder {
    public:

      GFlag& Flag(std::string_view namesv) {
        std::string name = std::string(namesv);
        if (this->flags.count(name) == 0) {
          log::error("Flag named {} was not found", name);
        }
        this->flags.try_emplace(name, name, false); // Emplace a default if not there
        return this->flags.at(name);
      }

      void CreateFlag(std::string_view namesv, bool value) {
        std::string name = std::string(namesv);
        this->gactorValues.try_emplace(name, name, value);
      }

      // Used for the toml coversions
      virtual void from_toml(const toml::value& v)
      {
        for (const auto& [k, v]: toml::find<std::unordered_map<std::string, bool>(v, "flags")) {
          this->try_emplace(std::string(k), std::string(k), v);
        }
      }
      virtual toml::value into_toml() const {
        toml::table flags = {};
        for (const auto &[k, v]: this->flags) {
          flags.try_emplace(toml::key(k), v.Get());
        }

        return toml::value(
          {
            {"flags", flags},
          }
        );
      }
    protected:
      std::unordered_map<std::string, GFlag> flags;
  }
}
