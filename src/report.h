// jt
//
#pragma once

#include "common.h"
#include <iostream>
#include <vector>

namespace jt {

enum class ReportLevel {
	NOTIF = 1,
	USER_ERR = 1 << 1,
	COMP_ERR = 1 << 2
};

class Report {
public:
	Report(ReportLevel level, const char* file, int line, const char* msg)
	:	level(level), file(file), line(line), msg(msg) {}

	ReportLevel level;
	const char* file;
	int line;
	const char* msg;
};

class IReportOut {
public:
	virtual ~IReportOut() {}
	virtual void out(const Report& report) = 0;
};

class BaseReportFormatter {
public:
	static std::string format(const Report& report, bool show_file_path);
};

class OstreamReportOut : public IReportOut {
public:
	OstreamReportOut(std::ostream& out);
	virtual void out(const Report& report) override;

protected:
	void out_impl(const Report& report);

protected:
	std::ostream& out_;
};

class Win32DbgReportOut : public IReportOut {
public:
	Win32DbgReportOut() {}
	virtual void out(const Report& report) override;

protected:
	void out_impl(const Report& report);
};

class Reports {
public:
	void add_out(IReportOut* out) { report_out_.push_back(out); }
	void report(const Report& report);

protected:
	std::vector<IReportOut*> report_out_;
};

extern Reports Rep;

#define JT_COMP_ERR(msg) { Rep.report(Report(ReportLevel::COMP_ERR, __FILE__, __LINE__, (msg))); _CrtDbgBreak(); }
#define JT_COMP_ASSERT(expr, msg) if (!(expr)){ JT_COMP_ERR(msg); }
#define JT_USER_ERR(msg) { Rep.report(Report(ReportLevel::USER_ERR, __FILE__, __LINE__, (msg))); _CrtDbgBreak(); }

} // namespace jt {
