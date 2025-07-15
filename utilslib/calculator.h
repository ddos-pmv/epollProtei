#pragma once

#include <string>
#include <stack>
#include <sstream>

namespace protei
{
    int eval_expr(const std::string &expr)
    {
        int result = 0;
        int num = 0;
        char op = '+';

        std::istringstream in(expr);
        while (in)
        {
            char ch = in.peek();
            if (isdigit(ch))
            {
                in >> num;
                switch (op)
                {
                case '+':
                    result += num;
                    break;
                case '-':
                    result -= num;
                    break;
                case '*':
                    result *= num;
                    break;
                }
            }
            else
            {
                in >> op;
            }
        }
        return result;
    }

}