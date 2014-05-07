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

} // namespace jt {