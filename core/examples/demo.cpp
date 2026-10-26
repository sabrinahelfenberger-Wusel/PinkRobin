#include "pinkrobin/core.hpp"
#include <iostream>
const char* expression(pinkrobin::Expression value) {
    switch (value) {
    case pinkrobin::Expression::sleeping: return "sleeping";
    case pinkrobin::Expression::curious: return "curious";
    case pinkrobin::Expression::happy: return "happy";
    case pinkrobin::Expression::heart_eyes: return "heartEyes";
    }
    return "unknown";
}
int main() {
    pinkrobin::Core robin;
    auto show = [&] { const auto s = robin.snapshot(); std::cout << s.time_ms << " ms: " << expression(s.expression) << '\n'; };
    show();
    robin.advance_to(4000); show();
    robin.advance_to(7100); show();
    robin.dispatch(pinkrobin::Event::head_started, 8000); show();
    robin.advance_to(13000); show();
    robin.dispatch(pinkrobin::Event::head_ended, 14000); show();
}
