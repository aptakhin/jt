
#include "report.h"
#include <string>
#include <string.h>

namespace jt {

Reports Rep;

std::string BaseReportFormatter::format(const Report& report, bool show_file_path) {
	std::string print_file = report.file;
	if (!show_file_path) {
		auto found = print_file.find_last_of("\\");
		if (~found)
			print_file = print_file.substr(found + 1, print_file.size() - found - 1);
	}
	std::ostringstream out;
	out << print_file << "(" << report.line << "): " << report.msg << std::endl;
	return out.str();
}

OstreamReportOut::OstreamReportOut(std::ostream& out)
:	out_(out) {}

void OstreamReportOut::out(const Report& report) {
	out_impl(report);
}

void OstreamReportOut::out_impl(const Report& report) {
	out_ << BaseReportFormatter::format(report, false);
}

void Win32DbgReportOut::out(const Report& report) {
	out_impl(report);
}

void Win32DbgReportOut::out_impl(const Report& report) {
	OutputDebugString(BaseReportFormatter::format(report, true).c_str());
}

void Reports::report(const Report& report) {
	for (auto& i : report_out_)
		i->out(report);
}

} // namespace jt {