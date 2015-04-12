// jt
//
#pragma once

#include "common.h"
#include <iostream>
#include <vector>

namespace jt {

enum class ReportLevel {
	NOTIF        = 1,
	LEXER_NOTIF  = 1 << 1,
	PARSER_NOTIF = 1 << 2,
	SYMANT_NOTIF = 1 << 3,
	INFERR_NOTIF = 1 << 4,
	INTERP_NOTIF = 1 << 5,
	GENERN_NOTIF = 1 << 6,
	PYTHON_NOTIF = 1 << 7,

	USER_ERR  = 1 << 14,
	COMP_ERR  = 1 << 15,
};

class Report {
public:
	Report(ReportLevel level, const char* file, int line, const char* msg)
	:	level(level), file(file), line(line), msg(msg) {}

	Report(ReportLevel level, const char* file, int line, const String& m)
	:	level(level), file(file), line(line), msg(m) {}

	ReportLevel level;
	const char* file;
	int line;
	const String msg;
};

class IReportOut {
public:
	virtual ~IReportOut() {}
	virtual void out(const Report& report, int offset) = 0;
};

class Reports {
public:
	void add_out(IReportOut* out) { report_out_.push_back(out); }
	void remove_out(IReportOut* out) { report_out_.erase(std::find(report_out_.begin(), report_out_.end(), out)); }
	void report(const Report& report);

	void push_offset(int plus_offset) { offset_ += plus_offset; } 

protected:
	std::vector<IReportOut*> report_out_;

	int offset_ = 0;
};

extern Reports Rep;

class BaseReportFormatter {
public:
	static std::string format(const Report& report, int offset, bool show_file_path);
};

class OstreamReportOut : public IReportOut {
public:
	OstreamReportOut(std::ostream& out);
	virtual void out(const Report& report, int offset) override;

protected:
	void out_impl(const Report& report, int offset);

protected:
	std::ostream& out_;
};

class OstreamReportOutScopeOffset {
public:
	OstreamReportOutScopeOffset() {
		Rep.push_offset(1);
	}

	~OstreamReportOutScopeOffset() {
		Rep.push_offset(-1);
	}
};

#define JT_TRACE_SCOPE(msg) JT_TRACE((msg)); OstreamReportOutScopeOffset JT_CONCAT(trace_scope_, __LINE__);

#if JT_PLATFORM_WIN32

class Win32DbgReportOut : public IReportOut {
public:
	Win32DbgReportOut() {}
	virtual void out(const Report& report, int offset) override;

protected:
	void out_impl(const Report& report, int offset);
};

#endif // #if JT_PLATFORM_WIN32

#if JT_PLATFORM_WIN32
#	define JT_DBG_BREAK { asm {int 3} }
#else
#	define JT_DBG_BREAK { __asm__("int $3"); }
#endif

#define JT_TRAP(cond) { if ((cond)) { JT_DEBUG_BREAK; } }

#define JT_COMP_ERR(msg) { Rep.report(Report(ReportLevel::COMP_ERR, __FILE__, __LINE__, (msg))); JT_DBG_BREAK; }
#define JT_COMP_ASSERT(expr, msg) if (!(expr)){ JT_COMP_ERR(msg); }
#define JT_TRACE(msg) { Rep.report(Report(ReportLevel::NOTIF, __FILE__, __LINE__, (msg))); }
#define JT_USER_ERR(msg) { Rep.report(Report(ReportLevel::USER_ERR, __FILE__, __LINE__, (msg))); JT_DBG_BREAK; }

#define JT_TR(msg, level) { jt::Rep.report(jt::Report(jt::ReportLevel::level, __FILE__, __LINE__, (msg))); }


} // namespace jt {
