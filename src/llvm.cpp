
#include "llvm.h"

namespace jt {

Assembly::Assembly() {}

void Assembly::open(const String& filename) {
	out_.open(filename.c_str());
}

void Assembly::next(Node node, const String& var) {
	switch (node.type()) {
	case NodeType::FLOW: {
		auto flow = node.impl<FlowImpl>();
		for (auto& i: flow->flow()) {
			next(i, tmp());
		}
	}
	break;

	case NodeType::VAR: {
		auto v = node.impl<VarImpl>();
		
		if (v->term().is<IntTermImpl>()) {
			*this << 
			var + " = shl i32 " + std::to_string(v->term().as<IntTermImpl>()->number()) + ", 0";
		}
		else if (v->value().is<FuncCallImpl>()) {
			next(v->value(), var);
		}
		else {
			*this << 
			var + " = shl i32 0, 0; Unknown type";
		}
	}
	break;

	case NodeType::FUNC_CALL: {
		auto call = node.impl<FuncCallImpl>();

		auto name = call->name();

		if (name == "op_plus") {
			auto lhs = tmp(), rhs = tmp();
			next(call->flow()->flow()[0], lhs);
			next(call->flow()->flow()[1], rhs);
			*this << 
			var + " = add i32 " + lhs + ", " + rhs;
		}
		if (name == "print") {
			auto eq = tmp();
			next(call->flow()->flow()[0], eq);
			*this << 
			tmp() + " = call i32 @print_i(i32 " + eq + ")";
		}
	}
	}
}

void Assembly::push(Node node) {
	*this << 
	"; jt LLVM compiler production" <<
	"" <<
	"declare i32 @printf(i8* noalias , ...)" <<
	"" <<
	"@.int_print = private unnamed_addr constant [3 x i8] c\"%d\\00\"" << 
	"" <<
	"define i32 @print_i(i32 %a) {" <<
	"  %print_str = getelementptr [3 x i8]* @.int_print, i64 0, i64 0" <<
	"  %tmp = tail call i32 (i8*, ...)* @printf(i8* %print_str, i32 %a)" <<
	"  ret i32 0" <<
	"}";

	*this << 
	"define i32 @main() {";

	auto flow = node.impl<FlowImpl>();
	for (auto& i: flow->flow()) {
		next(i, tmp());
	}

	*this << 
	"ret i32 0" <<
	"} ; define i32 @main() {\n";
}

Assembly& Assembly::operator << (const String& str) {
	out_ << str << "\n";
	return *this;
}

String Assembly::tmp() {
	return "%tmp" + std::to_string(counter_++);
}

} // namespace jt {