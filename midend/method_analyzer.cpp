#include "method_analyzer.hpp"

using namespace moss;
using namespace ir;

void MethodAnalyzer::check_constructor(Function &method, ustring class_name) {
    if (method.get_name() == class_name) {
        LOGMAX("Setting " << method.get_name() << " as constructor");
        method.set_constructor(true);
    }
}

void MethodAnalyzer::visit(Class &cls) {
    for (auto i : cls.get_body()) {
        if (auto f = dyn_cast<Function>(i)) {
            check_constructor(*f, cls.get_name());
            LOGMAX("Setting " << f->get_name() << " as method");
            f->set_method(true);
        }
    }
}