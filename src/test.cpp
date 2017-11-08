
#include "dev.h"

int main() {
    using namespace jt;

    auto term_seq = UNode_Seq{};
    term_seq.emplace_back(Make_Unique<Term_Node>(Make_Unique<Int_Term>(1)));

    auto flow = Seq<UNode>{};
    flow.emplace_back(Make_Unique<Call_Func_Node>(
        Make_Unique<Name_Node>("print"),
        std::move(term_seq)
    ));

    Runner run;
    run.load_std();
    run.load(std::move(flow));

    run.infer();
    run.run();

//    load_std(state);
//    load_flow(flow);
//    Runner runner{state};
//    runner.run();
    //parser->push("print(1)")

    return 0;
}