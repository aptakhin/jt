
#include "report.h"
#include <string>
#include <string.h>

#define NOMINMAX
#include <windows.h>

namespace jt {

Reports Rep;

std::string BaseReportFormatter::format(const Report& report, int offset, bool show_file_path) {
	std::string print_file = report.file;
	if (!show_file_path) {
		auto found = print_file.find_last_of("\\");
		if (~found)
			print_file = print_file.substr(found + 1, print_file.size() - found - 1);
	}
	std::ostringstream out;
	while (offset--)
		out << "  ";
	out << print_file << "(" << report.line << "): " << report.msg;
	return out.str();
}

OstreamReportOut::OstreamReportOut(std::ostream& out)
:	out_(out) {}

void OstreamReportOut::out(const Report& report, int offset) {
	out_impl(report, offset);
}

void OstreamReportOut::out_impl(const Report& report, int offset) {
	out_ << BaseReportFormatter::format(report, offset, false) << std::endl;
	out_.flush();
}

#if JT_PLATFORM == JT_PLATFORM_WIN32
void Win32DbgReportOut::out(const Report& report, int offset) {
	out_impl(report, offset);
}

void Win32DbgReportOut::out_impl(const Report& report, int) {
	OutputDebugString((BaseReportFormatter::format(report, 0, true) + "\n").c_str());
}
#endif // #if JT_PLATFORM == JT_PLATFORM_WIN32

void Reports::report(const Report& report) {
	for (auto& i : report_out_)
		i->out(report, offset_);
}

} // namespace jt {