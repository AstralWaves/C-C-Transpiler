#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include <vector>
#include "intermediate_code.h"

class Optimizer {
public:
    std::vector<Quadruple> optimize(const std::vector<Quadruple>& code);
};

#endif