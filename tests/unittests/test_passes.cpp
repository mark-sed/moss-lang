#include <gtest/gtest.h>
#include "testing_utils.hpp"
#include "ir.hpp"
#include "ir_pipeline.hpp"
#include "ir_visitor.hpp"
#include "parser.hpp"
#include <sstream>

namespace{

using namespace moss;
using namespace ir;

class AllIRsVisitor : public IRVisitor {
public:
    std::stringstream out;

    AllIRsVisitor(Parser &parser) : IRVisitor(parser) {}
    
    virtual void visit(Module &i) override {
        out << "Module " << i.get_name() << "\n";
    }

    virtual void visit(Space &i) override {
        out << "Space " << i.get_name() << "\n";
    }

    virtual void visit(Class &i) override {
        out << "Class " << i.get_name() << "\n";
    }

    virtual void visit(Function &i) override {
        out << "Function " << i.get_name() << "\n";
    }

    virtual void visit(Lambda &i) override {
        out << "Lambda " << i.get_name() << "\n";
    }

    virtual void visit(Return &i) override {
        out << "Return " << i.get_expr()->get_name() << "\n";
    }

    virtual void visit(Else &) override {
        out << "Else\n";
    }

    virtual void visit(If &i) override {
        out << "If " << i.get_cond()->get_name() << "\n";
    }

    virtual void visit(Switch &i) override {
        out << "Switch " << i.get_cond()->get_name() << "\n";
    }

    virtual void visit(Case &) override {
        out << "Case\n";
    }

    virtual void visit(Catch &i) override {
        out << "Catch " << i.get_arg()->get_name() << "\n";
    }

    virtual void visit(Finally &i) override {
        out << "Finally\n";
    }

    virtual void visit(Try &) override {
        out << "Try\n";
    }

    virtual void visit(While &i) override {
        out << "While " << i.get_cond()->get_name() << "\n";
    }

    virtual void visit(DoWhile &i) override {
        out << "DoWhile " << i.get_cond()->get_name() << "\n";
    }

    virtual void visit(ForLoop &i) override {
        out << "ForLoop " << i.get_iterator()->get_name() << "\n";
    }

    virtual void visit(Import &i) override {
        out << "Import " << i.get_name(0)->get_name() << "\n";
    }

    virtual void visit(Assert &i) override {
        out << "Assert " << i.get_cond()->get_name() << "\n";
    }

    virtual void visit(Raise &i) override {
        out << "Raise " << i.get_exception()->get_name() << "\n";
    }

    virtual void visit(Annotation &i) override {
        out << "Annotation " << i.get_value()->get_name() << "\n";
    }

    virtual void visit(BinaryExpr &i) override {
        out << "BinExp " << i.get_left()->get_name() << i.get_op() << i.get_right()->get_name() << "\n";
    }

    virtual void visit(UnaryExpr &i) override {
        out << "UnExp " << i.get_op() << i.get_expr()->get_name() << "\n";
    }

    virtual void visit(Multivar &i) override {
        out << "Multivar\n";
    }

    virtual void visit(TernaryIf &i) override {
        out << "TernIf " << i.get_condition()->get_name() << "\n";
    }

    virtual void visit(Range &i) override {
        out << "Range " << i.get_start()->get_name() << "\n";
    }

    virtual void visit(Call &i) override {
        out << "Call " << i.get_args().size() << "\n";
    }

    virtual void visit(List &i) override {
        out << "List\n";
    }

    virtual void visit(Dict &i) override {
        out << "Dict\n";
    }

    /*
    virtual void visit( &i) override {
        out << " " << i.get_name() << "\n";
    }
    */
};

/// Tests correct IR visiting
TEST(Passes, Visitors){
    ustring code = R"(
@!annotated("hi")
import FooModule
space Spc {
    space Names {
        class Cls1 {}
    }
}
class Cls2 {
    fun Cls2(a) {}
    fun get_a() = this.a
}
fun foo(a, b) {
    space M {}
    if (a) {
    } else if (b) {
    } else {}
    return M
}
fun lmb() = nil
if (true) {
} else {}

a = 4
switch(a) {
case 4: {
    if (a) {}
    try {
        a = a
    } catch (e) {
    } catch (e2) {
    } finally {}
}
}

while (a) {
    do {
    } while (a)
}

for (i, m, n: a) {
    for (j: i) {
        raise -j
    }
}

assert(a, "idk")
k,l,m = a
k ? m : l

for (i: a..10) {
    1, 3.. 12
}

[lmb(), foo(4, 3)]
{"a":1, "b": []}
)";

    ustring expected_out = "Module <one-liner>\n"
"Annotation <string-literal>\n"
"Import FooModule\n"
"Space Spc\n"
"Space Names\n"
"Class Cls1\n"
"Class Cls2\n"
"Function Cls2\n"
"Lambda get_a\n"
"BinExp <this-literal>.a\n"
"Function foo\n"
"Space M\n"
"If a\n"
"Else\n"
"If b\n"
"Else\n"
"Return M\n"
"Lambda lmb\n"
"If <bool-literal>\n"
"Else\n"
"BinExp a=<int-literal>\n"
"Switch a\n"
"Case\n"
"If a\n"
"Try\n"
"BinExp a=a\n"
"Catch e\n"
"Catch e2\n"
"Finally\n"
"While a\n"
"DoWhile a\n"
"ForLoop <multivar>\n"
"Multivar\n"
"ForLoop j\n"
"Raise <unary-expression>\n"
"UnExp -j\n"
"Assert a\n"
"BinExp <multivar>=a\n"
"Multivar\n"
"TernIf k\n"
"ForLoop i\n"
"Range a\n"
"Range <int-literal>\n"
"List\n"
"Call 0\n"
"Call 2\n"
"Dict\n"
"List\n"
;

    SourceFile sf(code, SourceFile::SourceType::STRING);
    Parser parser(sf);

    auto mod = dyn_cast<ir::Module>(parser.parse());
    ir::IRPipeline irp(parser);
    irp.get_pm().clear_passes();
    auto aip = new AllIRsVisitor(parser);
    irp.add_pass(aip);
    auto err = irp.run(mod);
    ASSERT_FALSE(err);
    ASSERT_EQ(aip->out.str(), expected_out);

    delete mod;
}

}