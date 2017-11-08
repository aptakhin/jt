#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <iostream>

namespace jt {

using String = std::string;

class Runner;

//template <class T>
//class U {
//public:
//    using SelfType = U<T>;
//
//    template <class... TArgs>
//    static SelfType create(TArgs&&... args) {
//        U<T> ut;
//        ut.ptr_.reset(new T(args...));
//        return ut;
//    }
//
////    U<TOther> operator() {
////        return U<TOther>(ptr)
////    }
//
//    U() = default;
//
//    template <class TOther>
//    U(U<TOther>&& other)
//        :   ptr_(std::move(other.ptr_)) {
//    }
//
//private:
//
//
//private:
//    std::unique_ptr<T> ptr_;
//};

template <class T>
using U = std::unique_ptr<T>;

template <class T, class... Args>
inline U<T> Make_Unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <class T>
class W {
private:
    T* ptr_;
};

//
//template <class T>
//class Seq {
//public:
//    Seq(std::initializer_list<T>&& elems) {
//
//    }
//
//
//private:
//    std::vector<T> seq_;
//};
template <class T>
using Seq = std::vector<T>;


enum class ENode {
    UNDEFINED,
    NAME,
    TERM,
    CALL_FUNC,
};

class Node {
public:
    Node(const ENode type)
        :   type_(type) {
    }

    virtual ~Node() {}

    ENode type() const { return type_; }

    virtual bool is_abstract() const { return true; };

    virtual Node* do_clone() const = 0;

    template <class T>
    T* as() {
        if (type() != T::TYPE) {
            std::cerr << "ERROR casting" << std::endl;
        }
        return static_cast<T*>(this);
    }

    template <class T>
    const T* as() const {
        if (type() != T::TYPE) {
            std::cerr << "ERROR casting" << std::endl;
        }
        return static_cast<const T*>(this);
    }

    template <class T>
    static T* as(U<Node>& holder) {
        if (holder->type() != T::TYPE) {
            std::cerr << "ERROR casting" << std::endl;
        }
        return static_cast<T*>(holder.get());
    }

    template <class T>
    static const T* as(const U<Node>& holder) {
        if (holder->type() != T::TYPE) {
            // ERROR!!
            std::cerr << "ERROR casting" << std::endl;
        }
        return static_cast<const T*>(holder.get());
    }

private:
    ENode type_;
};
using UNode = U<Node>;

enum class ETerm {
    UNDEFINED,
    INT,
    MODULE,
    FUNC,
    NATIVE_FUNC,

};

class Term {
public:
    Term(ETerm type)
        :   type_(type) {
    }

    virtual ~Term() {};

    ETerm type() const { return type_; }

    virtual Term* do_clone() const = 0;

    template <class T>
    T* as() {
        if (type() != T::TYPE) {
            // ERROR!!
            std::cerr << "ERROR casting" << std::endl;
        }
        return static_cast<T*>(this);
    }

    template <class T>
    const T* as() const {
        if (type() != T::TYPE) {
            // ERROR!!
            std::cerr << "ERROR casting" << std::endl;
        }
        return static_cast<const T*>(this);
    }

private:
    const ETerm type_ = ETerm::UNDEFINED;
};
using UTerm = U<Term>;

class Term_Contract {
public:
    virtual ~Term_Contract() {}

    virtual bool match(ETerm type) const = 0;
    virtual bool match(Term* term) const = 0;
};

class Simple_Type_Term_Contract : public Term_Contract {
public:
    Simple_Type_Term_Contract(ETerm type)
        :   type_(type) {
    }

    bool match(ETerm type) const override {
        return type == type_;
    }

    bool match(Term* term) const override {
        return term->type() == type_;
    }

private:
    ETerm type_;
};

class Int_Term : public Term {
public:
    using Self = Int_Term;

    const static ETerm TYPE = ETerm::INT;

    Int_Term(int value)
        :   Term(Self::TYPE),
            value_(value) {
    }

    int value() const { return value_; }

    virtual Int_Term* do_clone() const override;

private:
    const int value_;
};

//class Func_Term : public Term {
//
//};

class Native_Func_Term : public Term {
public:
    using Self = Native_Func_Term;

    const static ETerm TYPE = ETerm::NATIVE_FUNC;

    Native_Func_Term(void* func);

    virtual Native_Func_Term* do_clone() const override;

    void call(const Seq<UTerm>& args) const;



private:
    void* const func_;
};

class Module_Term : public Term {
public:
    using Self = Int_Term;

    const static ETerm TYPE = ETerm::MODULE;

    Module_Term()
        :   Term(Self::TYPE) {

    }

    void add(const String& name, UTerm&& term) {
        context_.insert(std::make_pair(name, std::move(term)));
    }

    virtual Module_Term* do_clone() const override;

private:
    std::unordered_multimap<String, UTerm> context_;
};

class State {
public:
    friend class Runner;



private:
    Module_Term root_;
};

class Func_Arguments_Item {
public:


private:
    String name_;
    U<Term_Contract> type_req_;
};

class Func_Arguments {
private:
    std::vector<Func_Arguments_Item> args_;
};
//
//class Func_Node : public Node {
//private:
//    Func_Arguments args_;
//
//    //std::unordered_map<String, U<Term>> context_;
//
//    std::vector<U<Node>> flow_;
//};

class Name_Node : public Node {
public:
    using Self = Name_Node;
    static const ENode TYPE = ENode::NAME;

    Name_Node(const String& name)
        :   Node(Self::TYPE),
            name_(name) {
    }

    virtual Name_Node* do_clone() const override;

private:
    const String name_;
};

class Term_Node : public Node {
public:
    using Self = Term_Node;
    static const ENode TYPE = ENode::TERM;

    Term_Node(UTerm&& t)
        :   Node(Self::TYPE),
            t_(std::move(t)) {
    }

    const Term* term() const { return t_.get(); }

    virtual Term_Node* do_clone() const override;

private:
    UTerm t_;
};


using UNode_Seq = Seq<UNode>;

class Call_Func_Node : public Node {
public:
    static const ENode TYPE = ENode::CALL_FUNC;

    Call_Func_Node(U<Name_Node>&& name, UNode_Seq&& args);
    Call_Func_Node(UTerm&& term, UNode_Seq&& args);

    virtual bool is_abstract() const override;

    virtual Call_Func_Node* do_clone() const override;

    const Name_Node* name() const { return name_.get(); }
    const Term* term() const { return term_.get(); }
    const Seq<UNode>& args() const { return args_; }

private:
    Call_Func_Node(const Call_Func_Node& cpy);

private:
    U<Name_Node> name_;
    UTerm term_;
    Seq<UNode> args_;
};

class Runner {
public:
    void load_std();
    void load(Seq<UNode>&& seq);

    void infer();
    UNode infer_node(const UNode& node);

    void run();

private:
    State state_;

    Seq<UNode> ast_flow_;
    Seq<UNode> exec_flow_;
};

} // namespace jt {