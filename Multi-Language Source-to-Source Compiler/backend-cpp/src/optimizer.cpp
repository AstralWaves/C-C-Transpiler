// PHASE 5: CODE OPTIMIZATION
#include "../include/optimizer.h"

std::vector<Quadruple> Optimizer::optimize(const std::vector<Quadruple>& code) {
    std::vector<Quadruple> optimized;
    
    // Phase 5: Simple Code Optimization
    // 1. Constant folding (simplified)
    // 2. Dead code removal (simplified)
    // For now, we just pass through but we could add logic here
    
    for (const auto& quad : code) {
        // Example: skip redundant assignments like x = x
        if (quad.op == Quadruple::OP_ASSIGN && quad.arg1 == quad.result) {
            continue;
        }
        optimized.push_back(quad);
    }
    
    return optimized;
}
