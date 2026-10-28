#pragma once

#include <exception>
#include <initializer_list>
#include <iostream>
#include <stdexcept>

#define CHECK(condition) do { if (!(condition)) throw std::runtime_error(#condition); } while (false)

namespace test_support {

struct TestCase {
    const char* name;
    void (*run)();
};

inline int run_tests(std::initializer_list<TestCase> cases) {
    for (const auto& test : cases) {
        try {
            test.run();
            std::cout << "Passed: " << test.name << '\n';
        } catch (const std::exception& error) {
            std::cerr << "Failed: " << test.name << ": " << error.what() << '\n';
            return 1;
        }
    }
    return 0;
}

} // namespace test_support
