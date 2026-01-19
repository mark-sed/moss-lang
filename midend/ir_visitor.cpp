#include "ir_visitor.hpp"
#include "ir.hpp"
#include "logging.hpp"
#include "parser.hpp"

using namespace moss;
using namespace ir;

IRVisitor::IRVisitor(Parser &parser) : parser(parser), currently_visiting(nullptr) {}

template <typename T, typename Container>
void PassManager::visit_body(Container& nodes) {
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        T* old_node = *it;

        currently_visiting = old_node;
        IR* new_ir = old_node->accept(*this);

        if (new_ir == old_node)
            continue;

        // TODO:
        //assert(old_node->is_replaceable() &&
        //        "Attempt to replace non-replaceable IR node");

        if (new_ir == nullptr) {
            delete old_node;
            it = nodes.erase(it);
            --it;
            continue;
        }

        T* new_node = dynamic_cast<T*>(new_ir);
        assert(new_node && "Visitor returned incompatible replacement type");

        delete old_node;
        *it = new_node;
    }
}

template <typename T, typename Setter>
T* PassManager::visit_child(T* old_child, Setter set_func, const char* err_msg, bool allow_null) {
    if (!old_child)
        return nullptr;

    IR* ir_new = old_child->accept(*this);

    // Unchanged
    if (ir_new == old_child)
        return old_child;

    // Removed
    if (ir_new == nullptr) {
        assert(allow_null && err_msg);
        delete old_child;
        set_func(nullptr);
        return nullptr;
    }

    // Replaced
    T* new_child = dynamic_cast<T*>(ir_new);
    assert(new_child && err_msg);

    delete old_child;
    set_func(new_child);
    return new_child;
}

template <typename T>
IR* PassManager::try_replace(T& node) {
    for (auto p : passes) {
        IR* replaced = p->visit(node);
        if (replaced != &node)
            return replaced->accept(*this);
    }
    return &node;
}

IR *PassManager::visit(Module &mod) {
    for (auto p: passes) {
        IR *replaced = mod.accept(*p);
        assert(replaced == &mod && "Pass attempted to replace Module node");
    }

    visit_body<IR>(mod.get_body());
    return &mod;
}

IR *PassManager::visit(Class &cls) {
    for (auto p : passes) {
        IR* replaced = cls.accept(*p);
        assert(replaced == &cls && "Pass attempted to replace Class node");
    }

    visit_body<IR>(cls.get_body());
    return &cls;
}

IR *PassManager::visit(Space &spc) {
    for (auto p : passes) {
        IR* replaced = spc.accept(*p);
        assert(replaced == &spc && "Pass attempted to replace Space node");
    }

    visit_body<IR>(spc.get_body());
    return &spc;
}

IR *PassManager::visit(Argument &a) {
    visit_body<Expression>(a.get_types());
    visit_child(a.get_default_value(), [&a](Expression *new_i) { a.set_default_value(new_i); }, "Argument default value cannot be removed");
    return &a;
}

IR *PassManager::visit(Function &fun) {
    for (auto p : passes) {
        IR* replaced = fun.accept(*p);
        assert(replaced == &fun && "Pass attempted to replace Fun node");
    }

    visit_body<Argument>(fun.get_args());
    visit_body<IR>(fun.get_body());

    return &fun;
}

IR *PassManager::visit(Else &els) {
    IR* self = try_replace(els);
    if (self != &els)
        return self;

    visit_body<IR>(els.get_body());
    return try_replace(els);
}

IR* PassManager::visit(If &i) {
    IR* self = try_replace(i);
    if (self != &i)
        return self;

    visit_child(i.get_cond(), [&i](Expression *new_i) { i.set_cond(new_i); }, "If cond cannot be removed");

    // Rewrite the main body (sequence of statements)
    visit_body<IR>(i.get_body());   // handles deletion/replacement automatically

    // Rewrite the else branch if it exists
    if (i.get_else()) {
        visit_child(i.get_else(), [&i](Else *new_i) { i.set_else(new_i); }, "", true);
    }

    return try_replace(i);
}

IR *PassManager::visit(Switch &swt) {
    IR* self = try_replace(swt);
    if (self != &swt)
        return self;

    visit_child(swt.get_cond(), [&swt](Expression *new_i) { swt.set_cond(new_i); }, "Switch condition cannot be removed");
    visit_body<IR>(swt.get_body());
    return try_replace(swt);
}

IR *PassManager::visit(Case &cs) {
    IR* self = try_replace(cs);
    if (self != &cs)
        return self;

    visit_body<Expression>(cs.get_values());
    visit_body<IR>(cs.get_body());
    return try_replace(cs);
}

IR *PassManager::visit(Catch &ct) {
    for (auto p : passes) {
        IR* replaced = ct.accept(*p);
        assert (replaced == &ct && "Pass attempted to replace Catch node");
    }

    visit_child(ct.get_arg(), [&ct](Argument *new_i) { ct.set_arg(new_i); }, "Catch arg cannot be removed");
    visit_body<IR>(ct.get_body());
    return &ct;
}

IR *PassManager::visit(Finally &fnl) {
    for (auto p : passes) {
        IR* replaced = fnl.accept(*p);
        assert (replaced == &fnl && "Pass attempted to replace Finally node");
    }

    visit_body<IR>(fnl.get_body());
    return &fnl;
}

IR *PassManager::visit(Try &tr) {
    for (auto p : passes) {
        IR* replaced = tr.accept(*p);
        assert (replaced == &tr && "Pass attempted to replace Try node");
    }

    visit_body<IR>(tr.get_body());
    visit_body<Catch>(tr.get_catches());
    visit_child(tr.get_finally(), [&tr](Finally *new_i) { tr.set_finally(new_i); }, "Try finally cannot be removed");
    return &tr;
}

IR *PassManager::visit(While &whl) {
    for (auto p : passes) {
        IR* replaced = whl.accept(*p);
        assert (replaced == &whl && "Pass attempted to replace While node");
    }

    visit_child(whl.get_cond(), [&whl](Expression *new_i) { whl.set_cond(new_i); }, "While cond cannot be removed");
    visit_body<IR>(whl.get_body());
    return &whl;
}

IR *PassManager::visit(DoWhile &dwhl) {
    for (auto p : passes) {
        IR* replaced = dwhl.accept(*p);
        assert (replaced == &dwhl && "Pass attempted to replace DoWhile node");
    }

    visit_body<IR>(dwhl.get_body());
    visit_child(dwhl.get_cond(), [&dwhl](Expression *new_i) { dwhl.set_cond(new_i); }, "DoWhile cond cannot be removed");
    return &dwhl;
}

IR *PassManager::visit(ForLoop &frl) {
    for (auto p : passes) {
        IR* replaced = frl.accept(*p);
        assert (replaced == &frl && "Pass attempted to replace For node");
    }

    visit_child(frl.get_iterator(), [&frl](Expression *new_i) { frl.set_iterator(new_i); }, "For iterator cannot be removed");
    visit_child(frl.get_collection(), [&frl](Expression *new_i) { frl.set_collection(new_i); }, "For collection cannot be removed");
    visit_body<IR>(frl.get_body());
    return &frl;
}

IR *PassManager::visit(Import &imp) {
    for (auto p : passes) {
        IR* replaced = imp.accept(*p);
        assert (replaced == &imp && "Pass attempted to replace Import node");
    }

    visit_body<Expression>(imp.get_names());
    return &imp;
}

IR *PassManager::visit(Assert &a) {
    for (auto p : passes) {
        IR* replaced = a.accept(*p);
        assert (replaced == &a && "Pass attempted to replace Assert node");
    }

    visit_child(a.get_cond(), [&a](Expression *new_i) { a.set_cond(new_i); }, "Assert cond cannot be removed");
    if (a.get_msg())
        visit_child(a.get_msg(), [&a](Expression *new_i) { a.set_msg(new_i); }, "Assert msg cannot be removed");

    return &a;
}

IR *PassManager::visit(Raise &r) {
    for (auto p : passes) {
        IR* replaced = r.accept(*p);
        assert (replaced == &r && "Pass attempted to replace Raise node");
    }

    visit_child(r.get_exception(), [&r](Expression *new_i) { r.set_exception(new_i); }, "Raise exception cannot be removed");

    return &r;
}

IR *PassManager::visit(Break &b) {
    return try_replace(b);
}

IR *PassManager::visit(Continue &c) {
    return try_replace(c);
}

IR *PassManager::visit(Return &ret) {
    IR* self = try_replace(ret);
    if (self != &ret)
        return self;

    visit_child(ret.get_expr(), [&ret](Expression *new_i) { ret.set_expr(new_i); }, "Return expr cannot be removed");

    return try_replace(ret);
}

IR *PassManager::visit(Annotation &a) {
    IR* self = try_replace(a);
    if (self != &a)
        return self;

    visit_child(a.get_value(), [&a](Expression *new_i) { a.set_value(new_i); }, "Annotation value cannot be removed");

    return try_replace(a);
}

IR *PassManager::visit(BinaryExpr &be) {
    IR* self = try_replace(be);
    if (self != &be)
        return self;

    visit_child(be.get_left(), [&be](Expression *new_i) { be.set_left(new_i); }, "BE left cannot be removed");
    visit_child(be.get_right(), [&be](Expression *new_i) { be.set_right(new_i); }, "BE right cannot be removed");

    return try_replace(be);
}

IR *PassManager::visit(UnaryExpr &ue) {
    IR* self = try_replace(ue);
    if (self != &ue)
        return self;

    visit_child(ue.get_expr(), [&ue](Expression *new_i) { ue.set_expr(new_i); }, "UE value cannot be removed");

    return try_replace(ue);
}

IR *PassManager::visit(Multivar &mv) {
    for (auto p : passes) {
        IR* replaced = mv.accept(*p);
        assert (replaced == &mv && "Pass attempted to replace Multivar node");
    }

    visit_body<Expression>(mv.get_vars());

    return &mv;
}

IR *PassManager::visit(TernaryIf &ti) {
    IR* self = try_replace(ti);
    if (self != &ti)
        return self;

    visit_child(ti.get_condition(), [&ti](Expression *new_i) { ti.set_condition(new_i); }, "TernIf cond cannot be removed");
    visit_child(ti.get_value_true(), [&ti](Expression *new_i) { ti.set_value_true(new_i); }, "TernIf true cannot be removed");
    visit_child(ti.get_value_false(), [&ti](Expression *new_i) { ti.set_value_false(new_i); }, "TernIf false cannot be removed");

    return try_replace(ti);
}

IR *PassManager::visit(Lambda &fun) {
    IR* self = try_replace(fun);
    if (self != &fun)
        return self;

    visit_body<Argument>(fun.get_args());
    visit_child(fun.get_body(), [&fun](Expression *new_i) { fun.set_body(new_i); }, "Lambda body cannot be removed");

    return try_replace(fun);
}

IR *PassManager::visit(Range &r) {
    IR* self = try_replace(r);
    if (self != &r)
        return self;

    visit_child(r.get_start(), [&r](Expression *new_i) { r.set_start(new_i); }, "Range start cannot be removed");
    if (r.get_second())
        visit_child(r.get_second(), [&r](Expression *new_i) { r.set_second(new_i); }, "Range second cannot be removed");
    visit_child(r.get_end(), [&r](Expression *new_i) { r.set_end(new_i); }, "Range end cannot be removed");

    return try_replace(r);
}

IR *PassManager::visit(Call &cl) {
    IR* self = try_replace(cl);
    if (self != &cl)
        return self;

    visit_child(cl.get_fun(), [&cl](Expression *new_i) { cl.set_fun(new_i); }, "Call fun cannot be removed");
    visit_body<Expression>(cl.get_args());

    return try_replace(cl);
}

IR *PassManager::visit(List &lst) {
    IR* self = try_replace(lst);
    if (self != &lst)
        return self;

    visit_body<Expression>(lst.get_value());

    if (lst.is_comprehension()) {        
        if (lst.get_result())
            visit_child(lst.get_result(), [&lst](Expression *new_i) { lst.set_result(new_i); }, "List result cannot be removed");
        if (lst.get_else_result())
            visit_child(lst.get_else_result(), [&lst](Expression *new_i) { lst.set_else_result(new_i); }, "List else result cannot be removed");
        if (lst.get_condition())
            visit_child(lst.get_condition(), [&lst](Expression *new_i) { lst.set_condition(new_i); }, "List cond cannot be removed");
        
        visit_body<Expression>(lst.get_assignments());
    }

    return try_replace(lst);
}

IR *PassManager::visit(Dict &dct) {
    IR* self = try_replace(dct);
    if (self != &dct)
        return self;

    visit_body<Expression>(dct.get_keys());
    visit_body<Expression>(dct.get_values());

    return try_replace(dct);
}

void PassManager::add_pass(IRVisitor *p) { 
    passes.push_back(p);
}

IR *IRVisitor::visit(class Module &i) { return &i; }
IR *IRVisitor::visit(class Space &i) { return &i; }
IR *IRVisitor::visit(class Class &i) { return &i; }
IR *IRVisitor::visit(class Argument &i) { return &i; }
IR *IRVisitor::visit(class Function &i) { return &i; }
IR *IRVisitor::visit(class Lambda &i) { return &i; }
IR *IRVisitor::visit(class Return &i) { return &i; }
IR *IRVisitor::visit(class Else &i) { return &i; }
IR *IRVisitor::visit(class If &i) { return &i; }
IR *IRVisitor::visit(class Switch &i) { return &i; }
IR *IRVisitor::visit(class Case &i) { return &i; }
IR *IRVisitor::visit(class Catch &i) { return &i; }
IR *IRVisitor::visit(class Finally &i) { return &i; }
IR *IRVisitor::visit(class Try &i) { return &i; }
IR *IRVisitor::visit(class While &i) { return &i; }
IR *IRVisitor::visit(class DoWhile &i) { return &i; }
IR *IRVisitor::visit(class ForLoop &i) { return &i; }
IR *IRVisitor::visit(class Import &i) { return &i; }
IR *IRVisitor::visit(class Assert &i) { return &i; }
IR *IRVisitor::visit(class Raise &i) { return &i; }
IR *IRVisitor::visit(class Break &i) { return &i; }
IR *IRVisitor::visit(class Continue &i) { return &i; }
IR *IRVisitor::visit(class Annotation &i) { return &i; }
IR *IRVisitor::visit(class BinaryExpr &i) { return &i; }
IR *IRVisitor::visit(class UnaryExpr &i) { return &i; }
IR *IRVisitor::visit(class Multivar &i) { return &i; }
IR *IRVisitor::visit(class TernaryIf &i) { return &i; }
IR *IRVisitor::visit(class Range &i) { return &i; }
IR *IRVisitor::visit(class Call &i) { return &i; }
IR *IRVisitor::visit(class List &i) { return &i; }
IR *IRVisitor::visit(class Dict &i) { return &i; }