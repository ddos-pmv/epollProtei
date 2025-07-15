#pragma once

#include <string>
#include <vector>
#include <random>
#include <sstream>

namespace protei
{
    std::string generate_expression(int num_operands)
    {
        if (num_operands <= 0)
            throw std::invalid_argument("Must have at least one operand");

        static std::mt19937 rng{std::random_device{}()};
        std::uniform_int_distribution<int> number_dist(1, 100);
        std::uniform_int_distribution<int> op_dist(0, 2);
        const char *ops[] = {"+", "-", "*"};

        std::ostringstream oss;
        oss << number_dist(rng); // первое число

        for (int i = 1; i < num_operands; ++i)
        {
            const char *op = ops[op_dist(rng)];
            int number = number_dist(rng);
            oss << op << number;
        }

        return oss.str(); // пример: "13 + 5 * 2 - 8"
    }
}