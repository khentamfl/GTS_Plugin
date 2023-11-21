#include "utils/fmt.hpp"

std::string fmt::formatter<BSFixedString>::format(BSFixedString v, format_context& ctx) const {
  string_view name = "<empty>";
  if v.c_str() {
    name = v.c_str();
  }
  return formatter<string_view>::format(name, ctx);
}

std::string fmt::formatter<ACTOR_VALUE_MODIFIER>::format(ACTOR_VALUE_MODIFIER v, format_context& ctx) const {
  string_view name = "unknown";
  switch (v) {
    case ACTOR_VALUE_MODIFIER::kPermanent: name = "kPermanent"; break;
    case ACTOR_VALUE_MODIFIER::kTemporary: name = "kTemporary"; break;
    case ACTOR_VALUE_MODIFIER::kDamage: name = "kDamage"; break;
  }
  return formatter<string_view>::format(name, ctx);
}
