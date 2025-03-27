#include "ir.hpp"

using namespace moss;
using namespace ir;

unsigned long ir::Space::annonymous_id = 0;
unsigned long ir::Lambda::annonymous_id = 0;
unsigned long ir::List::annonymous_id = 0;

std::list<IR *> List::as_for() {
    ir::Variable *result_var = new ir::Variable(compr_result_name);
    transform_irs.push_back(result_var);
    std::vector<Expression *> empty_list;
    std::list<IR *> code { new ir::BinaryExpr(result_var, new ir::List(empty_list), Operator(OperatorKind::OP_SET)) };

    IR * frbody = nullptr;
    if (condition) {
        std::vector<ir::Expression *> lval {result};
        std::list<IR *> ifbody {new ir::BinaryExpr(result_var, new ir::List(lval), Operator(OperatorKind::OP_SET_PLUS))};
        ir::Else *else_stm = nullptr;
        if (else_result) {
            std::vector<ir::Expression *> elselval {else_result};
            std::list<IR *> elsebody {new ir::BinaryExpr(result_var, new ir::List(elselval), Operator(OperatorKind::OP_SET_PLUS))};
            else_stm = new ir::Else(elsebody);
        }
        frbody = new ir::If(condition, ifbody, else_stm);
    } else {
        std::vector<ir::Expression *> lval {result};
        frbody = new ir::BinaryExpr(result_var, new ir::List(lval), Operator(OperatorKind::OP_SET_PLUS));
    }

    for (auto riter = assignments.rbegin(); riter != assignments.rend(); ++riter) {
        auto asgn = (*riter);
        auto binE = dyn_cast<BinaryExpr>(asgn);
        assert(binE && "Assignemnt is not a binary expr");
        assert(binE->get_op().get_kind() == OperatorKind::OP_SET && "Not an assignemnt");
        // TODO: Need to clone to free correctly or assign nullptr or dont delete on compr
        std::list<IR *> frbody_list {frbody};
        auto fr = new ir::ForLoop(binE->get_left(), binE->get_right(), frbody_list);
        frbody = fr;
    }

    if(frbody)
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
    os << (constructor ? "new " : "fun ") << name << "(";
    bool first = true;
    for (auto a: args) {
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