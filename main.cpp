#include <iostream>

#include "eunsortedvector.hpp"

int main(int, char**)
{

    eunsortedvector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);

    std::cout << "Hello, from eunsortedvector!\n";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i] << " ";
    }
    std::cout << "\n";
}
