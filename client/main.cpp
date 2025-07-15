#include <generator.h>
#include <calculator.h>
#include <string>
#include <iostream>
int main()
{
    std::string expr = protei::generate_expression(5);

    std::cout << expr << std::endl;
    std::cout << "ans: " << protei::eval_expression(expr) << std::endl;
    return 0;
}