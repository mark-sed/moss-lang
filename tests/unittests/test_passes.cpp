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
    
    virtual IR *visit(Module &i) override {
        out << "Module " << i.get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Space &i) override {
        out << "Space " << i.get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Class &i) override {
        out << "Class " << i.get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Function &i) override {
        out << "Function " << i.get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Lambda &i) override {
        out << "Lambda " << i.get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Return &i) override {
        out << "Return " << i.get_expr()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Else &i) override {
        out << "Else\n";
        return &i;
    }

    virtual IR *visit(If &i) override {
        out << "If " << i.get_cond()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Switch &i) override {
        out << "Switch " << i.get_cond()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Case &i) override {
        out << "Case\n";
        return &i;
    }

    virtual IR *visit(Catch &i) override {
        out << "Catch " << i.get_arg()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Finally &i) override {
        out << "Finally\n";
        return &i;
    }

    virtual IR *visit(Try &i) override {
        out << "Try\n";
        return &i;
    }

    virtual IR *visit(While &i) override {
        out << "While " << i.get_cond()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(DoWhile &i) override {
        out << "DoWhile " << i.get_cond()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(ForLoop &i) override {
        out << "ForLoop " << i.get_iterator()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Import &i) override {
        out << "Import " << i.get_name(0)->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Assert &i) override {
        out << "Assert " << i.get_cond()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Raise &i) override {
        out << "Raise " << i.get_exception()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Annotation &i) override {
        out << "Annotation " << i.get_value()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(BinaryExpr &i) override {
        out << "BinExp " << i.get_left()->get_name() << i.get_op() << i.get_right()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(UnaryExpr &i) override {
        out << "UnExp " << i.get_op() << i.get_expr()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Multivar &i) override {
        out << "Multivar\n";
        return &i;
    }

    virtual IR *visit(TernaryIf &i) override {
        out << "TernIf " << i.get_condition()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Range &i) override {
        out << "Range " << i.get_start()->get_name() << "\n";
        return &i;
    }

    virtual IR *visit(Call &i) override {
        out << "Call " << i.get_args().size() << "\n";
        return &i;
    }

    virtual IR *visit(List &i) override {
        out << "List\n";
        return &i;
    }

    virtual IR *visit(Dict &i) override {
        out << "Dict\n";
        return &i;
    }
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
"Annotation <string-literal>\n"
"Import FooModule\n"
"Space Spc\n"
"Space Names\n"
"Class Cls1\n"
"Class Cls2\n"
"Function Cls2\n"
"Lambda get_a\n"
"BinExp <this-literal>.a\n"
"BinExp <this-literal>.a\n"
"Lambda get_a\n"
"Function foo\n"
"Space M\n"
"If a\n"
"Else\n"
"If b\n"
"Else\n"
"Else\n"
"If b\n"
"Else\n"
"If a\n"
"Return M\n"
"Return M\n"
"Lambda lmb\n"
"Lambda lmb\n"
"If <bool-literal>\n"
"Else\n"
"Else\n"
"If <bool-literal>\n"
"BinExp a=<int-literal>\n"
"BinExp a=<int-literal>\n"
"Switch a\n"
"Case\n"
"If a\n"
"If a\n"
"Try\n"
"BinExp a=a\n"
"BinExp a=a\n"
"Catch e\n"
"Catch e2\n"
"Finally\n"
"Case\n"
"Switch a\n"
"While a\n"
"DoWhile a\n"
"ForLoop <multivar>\n"
"Multivar\n"
"ForLoop j\n"
"Raise <unary-expression>\n"
"UnExp -j\n"
"UnExp -j\n"
"Assert a\n"
"BinExp <multivar>=a\n"
"Multivar\n"
"BinExp <multivar>=a\n"
"TernIf k\n"
"TernIf k\n"
"ForLoop i\n"
"Range a\n"
"Range a\n"
"Range <int-literal>\n"
"Range <int-literal>\n"
"List\n"
"Call 0\n"
"Call 0\n"
"Call 2\n"
"Call 2\n"
"List\n"
"Dict\n"
"List\n"
"List\n"
"Dict\n"
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