// os4-gcc16 smoke test: C++ with the standard library, exceptions included.
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

int main()
{
    std::vector<std::string> words { "stove", "os4", "gcc16" };
    std::sort(words.begin(), words.end());
    std::string all;
    for (const auto &w : words) all += w + " ";
    try {
        throw std::runtime_error(all);
    } catch (const std::exception &e) {
        std::cout << "caught: " << e.what() << std::endl;
    }
    return 0;
}
