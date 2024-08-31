#include "ir.hpp"

using namespace moss;
using namespace ir;

bool can_be_annotated(IR *decl) {
    return isa<Construct>(decl);
}