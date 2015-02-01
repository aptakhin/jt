
#include "interpreter.h"
#include <gtest/gtest.h>

#if JT_PLATFORM == JT_PLATFORM_WIN32
#	include <windows.h>
#endif

namespace jt {

std::vector<Node> listed(std::initializer_list<Node>&& init) {
	return std::move(std::vector<Node>(init));
}

template <typename T>
T& ref_cast(Term t) {
	return *((T*) t.impl<IntTermImpl>()->number());
}

Var make_var_none() {
	return Var();
}

Var jt_print(CallUnit*, FuncTermImpl* root, IntTermImpl* number) {
	auto& out = ref_cast<std::ostringstream>(root->init_args()->find("stream")->term());
	out << number->number();
	return make_var_none();
}

Var jt_printb(CallUnit*, FuncTermImpl* root, BoolTermImpl* b) {
	auto& out = ref_cast<std::ostringstream>(root->init_args()->find("stream")->term());
	out << b->boolean();
	return make_var_none();
}

Var jt_prints(CallUnit*, FuncTermImpl* root, StringTermImpl* s) {
	auto& out = ref_cast<std::ostringstream>(root->init_args()->find("stream")->term());
	out << s->str();
	return make_var_none();
}

Var jt_plus(CallUnit*, FuncTermImpl*, IntTermImpl* a, IntTermImpl* b) {
	return make_ivar(a->number() + b->number());
}

Var jt_pluss(CallUnit*, FuncTermImpl*, StringTermImpl* a, StringTermImpl* b) {
	return make_svar(a->str() + b->str());
}

Var jt_mul(CallUnit*, FuncTermImpl*, IntTermImpl* a, IntTermImpl* b) {
	return make_ivar(a->number() * b->number());
}

Var jt_eq(CallUnit*, FuncTermImpl*, IntTermImpl* a, IntTermImpl* b) {
	return make_bvar(a->number() == b->number());
}

Var jt_get(CallUnit* unit, FuncTermImpl*, StringTermImpl* varname) {
	return make_var(unit->get_var(varname->str()));
}

void BaseEnv::setup_env() {
	ctx_    = std::make_shared<Context>(nullptr);
	root_   = std::make_unique<FuncTermImpl>();
	run_    = std::make_unique<CallUnit>(nullptr, *root_.get(), ctx_);
	parser_ = std::make_unique<Parser>(root_.get(), ctx_);
	setup_std(ctx_);
	out_ << std::boolalpha;
}

void BaseEnv::setup_std(ContextSPtr ctx) {
	Seq print_init_args;
	Var v = make_ivar((int) &out_);
	v->set_name("stream");
	print_init_args->add(v);
	ctx->add_named("print",   def_func<IntTermImpl>(jt_print,  "to_print", print_init_args));
	ctx->add_named("print",   def_func<IntTermImpl>(jt_printb, "to_print", print_init_args));
	ctx->add_named("print",   def_func<IntTermImpl>(jt_prints, "to_print", print_init_args));
	ctx->add_named("op_get",  def_func<IntTermImpl>(jt_get,    "var"));
	ctx->add_named("op_plus", def_func<IntTermImpl>(jt_plus,   "a", "b"));
	ctx->add_named("op_plus", def_func<IntTermImpl>(jt_pluss,  "a", "b"));
	ctx->add_named("op_mul",  def_func<IntTermImpl>(jt_mul,    "a", "b"));
	ctx->add_named("op_eq",   def_func<BoolTermImpl>(jt_eq,    "a", "b"));
}

void BaseTest::SetUp() {
	setup_env();
	JT_TRACE(String() + "Running test " + ::testing::UnitTest::GetInstance()->current_test_info()->name());
}

void BaseTest::TearDown() {
	JT_TRACE("\n------------------------------------------------------------------\n");
}

void BaseTest::exec() {
	Inferencer inf(*root_.get(), ctx_);

	auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
	auto name = String() + "test-dumps/" + test_info->name() + ".txt";

	std::ofstream fout(name.c_str());
	fout << parser_->str() << std::endl << std::endl;

	AstPrinter print(fout);
	print.visit("", root_->flow());

	OstreamReportOut rep_out(fout); 
	Rep.add_out(&rep_out);

	JT_TRACE_SCOPE("Starting inferencer");
	inf.local(run_->flow());

	assembly_.open(String() + "assembly/" + test_info->name() + ".as");
	assembly_.push(root_->flow());

	run_->exec();

	Rep.remove_out(&rep_out);
}

void BaseTest::call_print(const String& t_out) {
	auto get   = FuncCall("op_get", make_svar(t_out));
	auto print = FuncCall("print", get);
	run_->flow()->add(print);
}

Interactive::Interactive(std::istream& in) 
:	in_(in) {
	setup_env();
}

int Interactive::exec(bool repl, const String& assembly) {
	if (!in_.good())
		return 0;

	int result =  repl? exec_interactive() : exec_stream();

	if (!assembly.empty()) {
		assembly_.open(assembly);
		assembly_.push(root_->flow());
	}
	return result;
}

int Interactive::exec_interactive() {
	size_t cur_exec = 0;
	String line;
	String prev_print;

	while (!in_.eof()) {
		std::getline(in_, line);
		parser_->push(line + "\n");

		Inferencer inf(*root_.get(), ctx_);
		inf.local(run_->flow());
		size_t commands = run_->flow()->flow().size();

		size_t to_eval = cur_exec;
		if (cur_exec >= commands)
			to_eval = commands - 1;
		
		auto term = run_->exec_node(run_->flow()->flow()[to_eval]);
		auto print = FuncCall("print", make_var(term));
		auto print_term = inf.local(print);
		print->set_term(print_term);
		run_->exec_node(print);
		if (cur_exec < commands || out_.str() != prev_print) {
			std::cout << "> " << out_.str() << std::endl;
			prev_print = out_.str();

			if (cur_exec < commands)
				++cur_exec;
		}
		out_.str(std::string());
	}
	
	return 0;
}

int Interactive::exec_stream() {
	String line;

	while (!in_.eof()) {
		std::getline(in_, line);
		parser_->push(line + "\n");
	}

	//#if JT_PLATFORM == JT_PLATFORM_WIN32
	//if (IsDebuggerPresent()) {
	//	JT_DBG_BREAK;
	//}
	//#endif // #if JT_PLATFORM == JT_PLATFORM_WIN32

	Inferencer inf(*root_.get(), ctx_);
	inf.local(run_->flow());

	run_->exec();
	std::cout << out_.str() << std::endl;
	
	return 0;
}

} // namespace jt {