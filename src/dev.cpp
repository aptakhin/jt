
#include "dev.h"
#include <iostream>

namespace jt {

void jt_print(int a) {
    std::cout << a;
}

Int_Term* Int_Term::do_clone() const {
    return new Int_Term{value_};
}

Native_Func_Term* Native_Func_Term::do_clone() const {
    return new Native_Func_Term{func_};
}

Name_Node* Name_Node::do_clone() const {
    return new Name_Node{name_};
}

Term_Node* Term_Node::do_clone() const {
    return new Term_Node{UTerm{t_->do_clone()}};
}

Module_Term* Module_Term::do_clone() const {
    return new Module_Term{};
}

Native_Func_Term::Native_Func_Term(void* const func)
    :   Term(Native_Func_Term::TYPE),
        func_(func) {
}

void Native_Func_Term::call(const Seq<UTerm>& args) const {
    const auto func_one = reinterpret_cast<void (*)(int)>(func_);

    if (args.size() != 1 || args[0]->type() != ETerm::INT) {
        std::cerr << "Expected one Int argument" << std::endl;
        throw std::runtime_error("Expected one Int argument");
    }

    auto t = args[0]->as<Int_Term>();
    func_one(t->value());
}

Call_Func_Node::Call_Func_Node(U<Name_Node>&& name, Seq<UNode>&& args)
    :   Node(Call_Func_Node::TYPE),
        name_(std::move(name)),
        args_(std::move(args)) {
}

Call_Func_Node::Call_Func_Node(UTerm&& term, Seq<UNode>&& args)
    :   Node(Call_Func_Node::TYPE),
        term_(std::move(term)),
        args_(std::move(args)) {
}

Call_Func_Node::Call_Func_Node(const Call_Func_Node& cpy)
    :   Node(Call_Func_Node::TYPE),
        name_(cpy.name_->do_clone()),
        term_(cpy.term_->do_clone()) {
    throw std::runtime_error("Not implemented Call_Func_Node::Call_Func_Node");
}


Call_Func_Node* Call_Func_Node::do_clone() const {
    return new Call_Func_Node{*this};
}

bool Call_Func_Node::is_abstract() const {
    return bool(name_);
}

void Runner::load_std() {
    state_.root_.add("print", Make_Unique<Native_Func_Term>((void*)jt_print));
}

void Runner::load(Seq<UNode>&& seq) {
    ast_flow_ = std::move(seq);
}

void Runner::infer() {
    for (const auto& node : ast_flow_) {
        switch (node->type()) {
            case ENode::CALL_FUNC: {
                std::cout << "CallFuncNode" << std::endl;

                const auto call_func = Node::as<Call_Func_Node>(node);
                const auto name = call_func->name();

                auto node_args = UNode_Seq{};
                //t_args.emplace_back(Make_Unique<Int_Term>(2));
                for (const auto& arg : call_func->args()) {
                    std::cout << "W";
                    node_args.emplace_back(infer_node(arg));
                }

                auto call_node = Make_Unique<Call_Func_Node>(
                    Make_Unique<Native_Func_Term>((void*) jt_print),
                    std::move(node_args)
                );
                exec_flow_.emplace_back(std::move(call_node));

                break;
            }

            default: {
                std::cout << "OTHER" << std::endl;
                break;
            }
        }
    }
}

UNode Runner::infer_node(const UNode& node) {
    switch (node->type()) {
        case ENode::TERM: {
            return UNode(node->do_clone());
        }

        default: {
            break;
        }
    }
    return UNode{};
}

//UTerm Runner::eval() {
//
//}

void Runner::run() {
    for (const auto& node : exec_flow_) {
        if (node->is_abstract()) {
            std::cout << "ERROR in runner. Node is abstract!!" << std::endl;
        }

        switch (node->type()) {
            case ENode::CALL_FUNC: {
                std::cout << "CallFuncNode2" << std::endl;

                const auto call_func = Node::as<Call_Func_Node>(node);
                const auto term = call_func->term();
                const auto func_term = term->as<Native_Func_Term>();

                auto t_args = Seq<UTerm>{};
                t_args.emplace_back(Make_Unique<Int_Term>(2));
//                for (const auto& arg : call_func->args()) {
//                    std::cout << "Q";
//                    t_args.emplace_back(Make_Unique<Int_Term>(2));
//                }

                func_term->call(t_args);

                break;
            }

            default: {
                std::cout << "OTHER" << std::endl;
                break;
            }
        }
    }
}

} // namespace jt {