#include <iostream>
#include <string>

auto main() -> int {
    const auto message = std::string {"Hello world!"};
    std::cout << message << '\n';

    return 0;
}
