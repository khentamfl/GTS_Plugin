#pragma once

using namespace RE;
using namespace SKSE;

namespace Gts {

	template< typename ... Args >
	void Notify(std::string_view rt_fmt_str, Args&&... args) {
		try {
			DebugNotification(std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			log::info("Could not format notification, check valid format string: {}", e.what());
		}
	}

	bool starts_with(std::string_view arg, std::string_view prefix);

	bool matches(std::string_view str, std::string_view reg);

}
