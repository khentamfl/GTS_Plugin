#include <fmt/core.h>
#include <spdlog/fmt/ostr.h>

using namespace RE;

template <> struct fmt::formatter<BSFixedString> : ostream_formatter{};


template <> struct fmt::formatter<ActorValue>: formatter<string_view> {
  auto format(ActorValue c, format_context& ctx) const;
};

template <> struct fmt::formatter<ACTOR_VALUE_MODIFIER>: formatter<string_view> {
  auto format(ACTOR_VALUE_MODIFIER c, format_context& ctx) const;
};
