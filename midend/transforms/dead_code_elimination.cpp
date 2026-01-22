#include "dead_code_elimination.hpp"
#include "parser.hpp"
#include "ir.hpp"
#include "logging.hpp"
#include "opcode.hpp"
#include <algorithm>

using namespace moss;
using namespace ir;

IR *DeadCodeEliminationPass::visit(Function &fun) {
    // TODO: This could be smarter.
    // It could detect cases where we always return inside of some statement:
    //    fun is_ok(a) {
    //        if (a > 3) return true
    //        else return false
    //        c = "dead_code!" 
    //    }
    auto &body = fun.get_body();
    for (auto it = body.begin(); it != body.end(); ++it) {
        if (isa<ir::Return>(*it)) {
            // We found return, delete all IR after it in this block, if there
            // are any.
            body.erase(std::next(it), body.end());
            break;
        }
    }
    return &fun;
}

static void eliminate_code_after_break_continue(std::list<IR *>& body) {
    for (auto it = body.begin(); it != body.end(); ++it) {
        if (isa<ir::Break>(*it) || isa<ir::Continue>(*it)) {
            // We found break/continue, delete all IR after it in this block,
            // if there are any.
            body.erase(std::next(it), body.end());
            break;
        }
    }
}

IR *DeadCodeEliminationPass::visit(ForLoop &fl) {
    eliminate_code_after_break_continue(fl.get_body());
    return &fl;
}

IR *DeadCodeEliminationPass::visit(While &whl) {
    eliminate_code_after_break_continue(whl.get_body());
    return &whl;
}

IR *DeadCodeEliminationPass::visit(DoWhile &whl) {
    eliminate_code_after_break_continue(whl.get_body());
    return &whl;
}

IR *DeadCodeEliminationPass::visit(If &i) {
    eliminate_code_after_break_continue(i.get_body());
    return &i;
}

IR *DeadCodeEliminationPass::visit(Else &e) {
    eliminate_code_after_break_continue(e.get_body());
    return &e;
}

IR *DeadCodeEliminationPass::visit(Case &c) {
    eliminate_code_after_break_continue(c.get_body());
    return &c;
}

IR *DeadCodeEliminationPass::visit(Catch &c) {
    eliminate_code_after_break_continue(c.get_body());
    return &c;
}

IR *DeadCodeEliminationPass::visit(Finally &f) {
    eliminate_code_after_break_continue(f.get_body());
    return &f;
}

IR *DeadCodeEliminationPass::visit(Try &t) {
    eliminate_code_after_break_continue(t.get_body());
    return &t;
}