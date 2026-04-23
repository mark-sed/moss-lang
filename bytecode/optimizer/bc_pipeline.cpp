#include "bc_pipeline.hpp"
#include "register_reuse_pass.hpp"
#include <vector>
#include <unordered_set>

using namespace moss;
using namespace opcode;

std::list<BCPass *> moss::opcode::O1Pipeline{
    new RegisterReusePass()
};

std::vector<BCBlob*> opcode::collect_all_blobs(BCBlob* root) {
    std::vector<BCBlob*> result;
    if (!root)
        return result;

    std::unordered_set<BCBlob*> visited;
    std::vector<BCBlob*> stack;

    stack.push_back(root);

    while (!stack.empty()) {
        BCBlob* blob = stack.back();
        stack.pop_back();

        if (!blob || visited.find(blob) != visited.end())
            continue;

        visited.insert(blob);
        result.push_back(blob);

        // Push children onto stack
        for (BCBlob* child : blob->get_inner_blobs()) {
            stack.push_back(child);
        }
    }

    return result;
}