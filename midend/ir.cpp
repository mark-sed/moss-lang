#include "ir.hpp"
#include "builtins.hpp"

using namespace moss;
using namespace ir;

unsigned long ir::Space::anonymous_id = 0;
unsigned long ir::Lambda::anonymous_id = 0;
unsigned long ir::List::anonymous_id = 0;

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

bool IR::has_annotation(ustring name) {
    for (auto a: annotations) {
        if (a->get_name() == name) {
            return true;
        }
    }
    return false;
}

bool Function::is_staticmethod() {
    for (auto a: annotations) {
        if (a->get_name() == annots::STATIC_METHOD && isa<NilLiteral>(a->get_value())) {
            return true;
        }
    }
    return false;
}

bool Lambda::is_staticmethod() {
    for (auto a: annotations) {
        if (a->get_name() == annots::STATIC_METHOD && isa<NilLiteral>(a->get_value())) {
            return true;
        }
    }
    return false;
}

IR *Argument::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Function::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Lambda::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Class::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Module::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Space::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Return::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *If::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Else::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Switch::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Case::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Catch::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Finally::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Try::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *While::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *DoWhile::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *ForLoop::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

//IR *Enum::accept(IRVisitor& visitor) {
//    return visitor.visit(*this);
//}

IR *Import::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Assert::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Raise::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Break::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Continue::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Annotation::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *BinaryExpr::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *UnaryExpr::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Multivar::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *TernaryIf::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Range::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Call::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *List::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}

IR *Dict::accept(IRVisitor& visitor) {
    return visitor.visit(*this);
}