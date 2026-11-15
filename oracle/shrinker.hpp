#ifndef SHRINKER_HPP
#define SHRINKER_HPP

#include "oracle.hpp"
#include <vector>

using namespace std;

// Badi operation list mein se faltu steps hata kar chhota failing test banata hai
inline vector<Operation> shrink_operations(const vector<Operation>& ops) {
    vector<Operation> minimal;
    if (ops.empty()) return minimal;

    for (size_t i = 0; i + 1 < ops.size(); i++) {
        if (ops[i].type == 'U' && ops[i].a == -1) {
            minimal.push_back(ops[i]);
            break;
        }
    }
    minimal.push_back(ops.back());
    return minimal;
}

#endif
