#pragma once

#include <vector>
#include <random>
#include <sstream>
#include <string>

namespace protei
{
    std::string generate_expression(int n)
    {
        static std::mt19937 rng{std::random_device{}()};
        static std::uniform_int_distribution<int> num_dist(1, 100);
        static std::uniform_int_distribution<int> op_dist(0, 1);
        static const char ops[] = {'+', '-'};

        std::ostringstream expr;
        expr << num_dist(rng);
        for (int i = 1; i < n; ++i)
        {
            expr << ops[op_dist(rng)] << num_dist(rng);
        }
        return expr.str();
    }
}