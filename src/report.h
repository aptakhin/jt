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
	static std::string format(const Report& report, int offset, bool show_file_path);
};

class OstreamReportOut : public IReportOut {
public:
	OstreamReportOut(std::ostream& out);
	virtual void out(const Report& report) override;

	void push_offset(int plus_offset) { offset_ += plus_offset; } 

protected:
	void out_impl(const Report& report);

protected:
	std::ostream& out_;

	int offset_ = 0;
};

class OstreamReportOutScopeOffset {
public:
	OstreamReportOutScopeOffset(OstreamReportOut& out) : out_(out) {
		out_.push_offset(1);
	}

	~OstreamReportOutScopeOffset() {
		out_.push_offset(-1);
	}

private:
	OstreamReportOut& out_;
};

extern OstreamReportOut RepOut; // See test.cpp for definition

#define JT_TRACE_SCOPE(msg) JT_TRACE(msg); OstreamReportOutScopeOffset trace_scope_##__LINE__(RepOut);

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
#define JT_TRACE(msg) { Rep.report(Report(ReportLevel::NOTIF, __FILE__, __LINE__, (msg))); }
#define JT_USER_ERR(msg) { Rep.report(Report(ReportLevel::USER_ERR, __FILE__, __LINE__, (msg))); _CrtDbgBreak(); }

} // namespace jt {
