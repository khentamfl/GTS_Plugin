#pragma once
#include <fmt/core.h>
#include <spdlog/fmt/ostr.h>

using namespace RE;

template <> struct fmt::formatter<BSFixedString> : formatter<string_view> {
  auto format(BSFixedString v, format_context& ctx) const {
    string_view name = "<empty>";
    if v.c_str() {
      name = v.c_str();
    }
    return formatter<string_view>::format(name, ctx);
  }
};


template <> struct fmt::formatter<ACTOR_VALUE_MODIFIER>: formatter<string_view> {
  auto format(ACTOR_VALUE_MODIFIER v, format_context& ctx) const {
    string_view name = "unknown";
    switch (v) {
      case ACTOR_VALUE_MODIFIER::kPermanent: name = "kPermanent"; break;
      case ACTOR_VALUE_MODIFIER::kTemporary: name = "kTemporary"; break;
      case ACTOR_VALUE_MODIFIER::kDamage: name = "kDamage"; break;
    }
    return formatter<string_view>::format(name, ctx);
  }
};
