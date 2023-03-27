// Lowest level base class for an GActor
//
// Just the interface no data at this point
//

#include "models/serde.hpp"

class GActorBase {
  public:
    virtual std::string GetCls() = 0;

    virtual std::string Name() = 0;
    virtual FormID FormID() = 0;

    // TOML
    virtual void from_toml(const toml::value& v) = 0;
    virtual toml::value into_toml() const = 0;

}
