#include <fmt/core.h>
#include <spdlog/fmt/ostr.h>

using namespace RE;

template <> struct fmt::formatter<BSFixedString> : formatter<string_view> {
  std::string format(BSFixedString v, format_context& ctx) const;
};


template <> struct fmt::formatter<ACTOR_VALUE_MODIFIER>: formatter<string_view> {
  std::string format(ACTOR_VALUE_MODIFIER v, format_context& ctx) const;
};
