#pragma once

#include <string>
#include <stack>
#include <sstream>

namespace protei
{
    int eval_expression(const std::string &expr)
    {
        std::istringstream iss(expr);
        std::stack<int> values;
        std::stack<char> ops;

        auto apply = [](int a, int b, char op) -> int
        {
            switch (op)
            {
            case '+':
                return a + b;
            case '-':
                return a - b;
            case '*':
                return a * b;
            case '/':
                return b == 0 ? 0 : a / b;
            default:
                throw std::runtime_error("Invalid operator");
            }
        };

        auto precedence = [](char op) -> int
        {
            if (op == '+' || op == '-')
                return 1;
            if (op == '*' || op == '/')
                return 2;
            return 0;
        };

        int num;
        while (iss >> num)
        {
            values.push(num);
            char op;
            if (!(iss >> op))
                break;

            while (!ops.empty() && precedence(ops.top()) >= precedence(op))
            {
                int b = values.top();
                values.pop();
                int a = values.top();
                values.pop();
                char prev_op = ops.top();
                ops.pop();
                values.push(apply(a, b, prev_op));
            }
            ops.push(op);
        }

        while (!ops.empty())
        {
            int b = values.top();
            values.pop();
            int a = values.top();
            values.pop();
            char op = ops.top();
            ops.pop();
            values.push(apply(a, b, op));
        }

        return values.top();
    }

}