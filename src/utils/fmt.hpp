#include <spdlog/fmt/ostr.h>

using RE;

#if FMT_VERSION >= 90000
    template <> struct fmt::formatter<BSFixedString> : ostream_formatter{};
#endif
