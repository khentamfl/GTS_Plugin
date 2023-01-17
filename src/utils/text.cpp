#include "utils/text.hpp"

using namespace RE;
using namespace SKSE;

namespace Gts {

	bool starts_with(std::string_view arg, std::string_view prefix) {
		return arg.compare(0, prefix.size(), prefix);
	}

	bool matches(std::string_view str, std::string_view reg) {
		std::regex the_regex(std::string(reg).c_str());
		return std::regex_match(std::string(str), the_regex);
	}

}
