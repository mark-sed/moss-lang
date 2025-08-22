#include "ir.hpp"
#include "builtins.hpp"

using namespace moss;
using namespace ir;

unsigned long ir::Space::annonymous_id = 0;
unsigned long ir::Lambda::annonymous_id = 0;
unsigned long ir::List::annonymous_id = 0;

std::list<IR *> List::as_for() {
    std::vector<Expression *> empty_list;
    list_compr_var = new ir::BinaryExpr(new ir::Variable(compr_result_name, src_info), new ir::List(empty_list, src_info), Operator(OperatorKind::OP_SET), src_info);
    std::list<IR *> code { list_compr_var };

    IR *frbody = nullptr;
    if (condition) {
        std::vector<ir::Expression *> lval {result};
        std::list<IR *> ifbody {new ir::BinaryExpr(new ir::Variable(compr_result_name, src_info), new ir::List(lval, src_info), Operator(OperatorKind::OP_SET_PLUS), src_info)};
        ir::Else *else_stm = nullptr;
        if (else_result) {
            std::vector<ir::Expression *> elselval {else_result};
            std::list<IR *> elsebody {new ir::BinaryExpr(new ir::Variable(compr_result_name, src_info), new ir::List(elselval, src_info), Operator(OperatorKind::OP_SET_PLUS), src_info)};
            else_stm = new ir::Else(elsebody, src_info);
        }
        frbody = new ir::If(condition, ifbody, else_stm, src_info);
    } else {
        std::vector<ir::Expression *> lval {result};
        frbody = new ir::BinaryExpr(new ir::Variable(compr_result_name, src_info), new ir::List(lval, src_info), Operator(OperatorKind::OP_SET_PLUS), src_info);
    }

    for (auto riter = assignments.rbegin(); riter != assignments.rend(); ++riter) {
        auto asgn = (*riter);
        auto binE = dyn_cast<BinaryExpr>(asgn);
        assert(binE && "Assignemnt is not a binary expr");
        assert(binE->get_op().get_kind() == OperatorKind::OP_SET && "Not an assignemnt");
        std::list<IR *> frbody_list {frbody};
        auto fr = new ir::ForLoop(binE->get_left(), binE->get_right(), frbody_list, src_info);
        frbody = fr;
    }

    this->list_compr_for = frbody;
    code.push_back(frbody);
    return code;
}

std::ostream& Module::debug(std::ostream& os) const {
    for (Annotation *a: this->annotations) {
        os << *a << "\n";
    }
    os << "<Module>" << name << "{\n";
    for (auto d: body) {
        if (!d) os << "nullptr";
        else os << *d << "\n";
    }
    os << "}";
    return os;
}

std::ostream& Function::debug(std::ostream& os) const {
    for (Annotation *a: this->annotations) {
        os << *a << "\n";
    }
    os << (info.constructor ? "new " : "fun ") << name << "(";
    bool first = true;
    for (auto a: info.args) {
        if (first) {
            os << *a;
            first = false;
        }
        else {
            os << ", " << *a;
        }
    }
    os << ") {\n";
    for (auto d: body) {
        os << *d << "\n";
    }
    os << "}";
    return os;
}

StringLiteral *Class::get_internal_bind() {
    for (auto a: annotations) {
        if (a->get_name() == annots::INTERNAL_BIND) {
            auto v = dyn_cast<ir::StringLiteral>(a->get_value());
            if (v)
                return v;
        }
    }
    return nullptr;
}

bool Function::is_staticmethod() {
    for (auto a: annotations) {
        if (a->get_name() == "staticmethod" && isa<NilLiteral>(a->get_value())) {
            return true;
        }
    }
    return false;
}

bool Lambda::is_staticmethod() {
    for (auto a: annotations) {
        if (a->get_name() == "staticmethod" && isa<NilLiteral>(a->get_value())) {
            return true;
        }
    }
    return false;
}

void Function::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void Lambda::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void Class::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void Module::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void Space::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void Return::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void If::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}

void Else::accept(IRVisitor& visitor) {
    visitor.visit(*this);
}