// test.cpp
#include <iostream>

int main()
{
    int sum = 0;
    for (int i = 0; i < 1000; ++i)
    {
        sum += i;
    }
    std::cout << "Sum: " << sum << std::endl;
    return 0;
}
