#include "ir_visitor.hpp"
#include "ir.hpp"
#include "logging.hpp"
#include "parser.hpp"

using namespace moss;
using namespace ir;

IRVisitor::IRVisitor(Parser &parser) : parser(parser) {}

void PassManager::visit(Module &mod) {
    for (auto p: passes) {
        mod.accept(*p);
    }

    for (auto i : mod.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Class &cls) {
    for (auto p: passes) {
        cls.accept(*p);
    }

    for (auto i : cls.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Space &spc) {
    for (auto p: passes) {
        spc.accept(*p);
    }

    for (auto i : spc.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Function &fun) {
    for (auto p: passes) {
        fun.accept(*p);
    }

    for (auto a: fun.get_args()) {
        a->accept(*this);
    }

    for (auto i : fun.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(Else &els) {
    for (auto p: passes) {
        els.accept(*p);
    }

    for (auto i : els.get_body()) {
        i->accept(*this);
    }
}

void PassManager::visit(If &i) {
    for (auto p: passes) {
        i.accept(*p);
    }
    i.get_cond()->accept(*this);
    for (auto j : i.get_body()) {
        j->accept(*this);
    }
    auto els = i.get_else();
    if (els)
        els->accept(*this);
}

void PassManager::visit(Switch &swt) {
    for (auto p: passes) {
        swt.accept(*p);
    }

    swt.get_cond()->accept(*this);
    for (auto j : swt.get_body()) {
        j->accept(*this);
    }
}

void PassManager::visit(Case &cs) {
    for (auto p: passes) {
        cs.accept(*p);
    }
    
    for (auto v: cs.get_values()) {
        v->accept(*this);
    }

    for (auto j : cs.get_body()) {
        j->accept(*this);
    }
}

void PassManager::visit(Catch &ct) {
    for (auto p: passes) {
        ct.accept(*p);
    }

    ct.get_arg()->accept(*this);
    for (auto j : ct.get_body()) {
        j->accept(*this);
    }
}

void PassManager::visit(Finally &fnl) {
    for (auto p: passes) {
        fnl.accept(*p);
    }

    for (auto j : fnl.get_body()) {
        j->accept(*this);
    }
}

void PassManager::visit(Try &tr) {
    for (auto p: passes) {
        tr.accept(*p);
    }

    for (auto j : tr.get_body()) {
        j->accept(*this);
    }
    for (auto c: tr.get_catches()) {
        c->accept(*this);
    }
    if (tr.get_finally()) {
        tr.get_finally()->accept(*this);
    }
}

void PassManager::visit(While &whl) {
    for (auto p: passes) {
        whl.accept(*p);
    }
    whl.get_cond()->accept(*this);

    for (auto j : whl.get_body()) {
        j->accept(*this);
    }
}

void PassManager::visit(DoWhile &dwhl) {
    for (auto p: passes) {
        dwhl.accept(*p);
    }
    
    for (auto j : dwhl.get_body()) {
        j->accept(*this);
    }
    dwhl.get_cond()->accept(*this);
}

void PassManager::visit(ForLoop &frl) {
    for (auto p: passes) {
        frl.accept(*p);
    }
    
    frl.get_iterator()->accept(*this);
    frl.get_collection()->accept(*this);
    for (auto j : frl.get_body()) {
        j->accept(*this);
    }
}

void PassManager::visit(Import &imp) {
    for (auto p: passes) {
        imp.accept(*p);
    }

    for (auto n: imp.get_names()) {
        n->accept(*this);
    }
}

void PassManager::visit(Assert &a) {
    for (auto p: passes) {
        a.accept(*p);
    }

    a.get_cond()->accept(*this);
    if (a.get_msg())
        a.get_msg()->accept(*this);
}

void PassManager::visit(Raise &r) {
    for (auto p: passes) {
        r.accept(*p);
    }

    r.get_exception()->accept(*this);
}

void PassManager::visit(Return &ret) {
    for (auto p: passes) {
        ret.accept(*p);
    }
}

void PassManager::visit(Annotation &a) {
    for (auto p: passes) {
        a.accept(*p);
    }

    a.get_value()->accept(*this);
}

void PassManager::visit(BinaryExpr &be) {
    for (auto p: passes) {
        be.accept(*p);
    }

    be.get_left()->accept(*this);
    be.get_right()->accept(*this);
}

void PassManager::visit(UnaryExpr &ue) {
    for (auto p: passes) {
        ue.accept(*p);
    }

    ue.get_expr()->accept(*this);
}

void PassManager::visit(Multivar &mv) {
    for (auto p: passes) {
        mv.accept(*p);
    }

    for (auto v: mv.get_vars()) {
        v->accept(*this);
    }
}

void PassManager::visit(TernaryIf &ti) {
    for (auto p: passes) {
        ti.accept(*p);
    }

    ti.get_condition()->accept(*this);
    ti.get_value_true()->accept(*this);
    ti.get_value_false()->accept(*this);
}

void PassManager::visit(Lambda &fun) {
    for (auto p: passes) {
        fun.accept(*p);
    }

    for (auto a: fun.get_args()) {
        a->accept(*this);
    }
    fun.get_body()->accept(*this);
}

void PassManager::visit(Range &r) {
    for (auto p: passes) {
        r.accept(*p);
    }

    r.get_start()->accept(*this);
    if (r.get_second())
        r.get_second()->accept(*this);
    r.get_end()->accept(*this);
}

void PassManager::visit(Call &cl) {
    for (auto p: passes) {
        cl.accept(*p);
    }

    cl.get_fun()->accept(*this);
    for (auto a: cl.get_args()) {
        a->accept(*this);
    }
}

void PassManager::visit(List &lst) {
    for (auto p: passes) {
        lst.accept(*p);
    }

    for (auto v: lst.get_value()) {
        v->accept(*this);
    }

    if (lst.is_comprehension()) {
        if (lst.get_result())
            lst.get_result()->accept(*this);
        if (lst.get_else_result())
            lst.get_else_result()->accept(*this);
        if (lst.get_condition())
            lst.get_condition()->accept(*this);
        for (auto a: lst.get_assignments())
            a->accept(*this);
    }
}

void PassManager::visit(Dict &dct) {
    for (auto p: passes) {
        dct.accept(*p);
    }

    for (auto k: dct.get_keys())
        k->accept(*this);
    for (auto v: dct.get_values())
        v->accept(*this);
}

void PassManager::add_pass(IRVisitor *p) { 
    passes.push_back(p);
}