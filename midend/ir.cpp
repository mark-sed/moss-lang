#include "ir.hpp"

using namespace moss;
using namespace ir;

bool can_be_annotated(IR *decl) {
    return isa<Construct>(decl);
}

std::vector<ustring> ir::encode_fun(ustring name, std::vector<Argument *> args) {
    std::vector<ustring> names;

    std::vector<std::vector<ustring>> arg_types;

    // This generates vector of vectors of all the type names so that
    // all the combinations can be generated.
    // Eg: {{"_"}, {"Bool", "Int"}, {"ClassC", "ClassA", "ClassD"}}
    for (unsigned i = 0; i < args.size(); ++i) {
        std::vector<ustring> types;
        auto a = args[i];
        if (a->is_vararg()) {
            types.push_back("...");
        }
        else if (a->is_typed()) {
            for (auto t: a->get_types()) {
                types.push_back(t->as_string());
            }
        }
        else {
            types.push_back("_");
        }
        arg_types.push_back(types);
    }
 
    // to keep track of next element in each of
    // the n arrays
    std::vector<int> indices(arg_types.size(), 0);
 
    while (true) {
        std::stringstream ss;
        ss << name << "(";
        // print current combination
        for (unsigned i = 0; i < arg_types.size(); i++) {
            if (i == 0)
                ss << arg_types[i][indices[i]];
            else
                ss << "," << arg_types[i][indices[i]];
        }
        ss << ")";
        names.push_back(ss.str());
 
        // find the rightmost array that has more  elements left after the
        // current element  in that array
        int next = arg_types.size() - 1;
        while (next >= 0 && (indices[next] + 1 >= static_cast<int>(arg_types[next].size())))
            next--;
 
        // no such array is found so no more combinations are left
        if (next < 0)
            break;
 
        // if found move to next element in that array
        indices[next]++;
 
        // for all arrays to the right of this array current index again points
        // to the first element
        for (int i = next + 1; i < static_cast<int>(arg_types.size()); i++)
            indices[i] = 0;
    }

    return names;
}