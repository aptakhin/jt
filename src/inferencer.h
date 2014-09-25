// jt
//
#pragma once

#include "common.h"
#include "ast.h"
#include "runner.h"

namespace jt {

class Inferencer {
public:
	Inferencer(FuncTermImpl& root, ContextSPtr ctx);

	Term local(Node node);

protected:
	FuncTermImpl& root_;

	ContextStack stack_;
};

String type_name(Node node);
String type_name(Term term);

String print_node(Node node);

} // namespace jt {