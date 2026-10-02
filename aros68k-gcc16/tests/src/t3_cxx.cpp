// t3: modern C++ and the standard library. Built as C++20 by GCC 16; GCC 6.5
// gets the C++14 half (the newer half is compiled out and counted as skipped).
#include <cstdio>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <numeric>
#include <functional>
#if __cplusplus >= 201703L
#include <optional>
#include <variant>
#include <string_view>
#endif
#if __cplusplus >= 202002L
#include <span>
#include <ranges>
#include <concepts>
#if __has_include(<format>)
#include <format>
#endif
#endif

static int pass, fail, skipped;
#define CHECK(name, cond) do { if (cond) pass++; else { fail++; std::printf("FAIL %s (line %d)\n", name, __LINE__); } } while (0)

struct Shape { virtual ~Shape() = default; virtual int area() const = 0; };
struct Sq : Shape { int s; explicit Sq(int s) : s(s) {} int area() const override { return s * s; } };
static int destroyed;
struct Tracker { ~Tracker() { destroyed++; } };

#if __cplusplus >= 202002L
template <std::integral T> constexpr T sum_to(T n) { T t{}; for (T i = 1; i <= n; ++i) t += i; return t; }
static_assert(sum_to(100) == 5050);
#endif

int main()
{
    std::vector<std::unique_ptr<Shape>> shapes;
    for (int i = 1; i <= 5; i++) shapes.push_back(std::make_unique<Sq>(i));
    int total = 0; for (auto &s : shapes) total += s->area();
    CHECK("virtual + unique_ptr", total == 55);
    std::string a = "Amiga", b = std::string(3, '!');
    CHECK("string", a + b == "Amiga!!!" && a.find("ig") == 2 && std::to_string(-1234) == "-1234");
    std::map<std::string, int> m{{"zeta", 1}, {"alpha", 2}};
    CHECK("map order", m.begin()->first == "alpha");
    std::unordered_map<int, int> h; for (int i = 0; i < 1000; i++) h[i * 7] = i;
    CHECK("unordered_map", h.size() == 1000 && h[700] == 100);
    std::vector<int> v(1000); std::iota(v.begin(), v.end(), -500);
    std::sort(v.begin(), v.end(), std::greater<int>());
    CHECK("sort + iota", v.front() == 499 && v.back() == -500 && std::accumulate(v.begin(), v.end(), 0) == -500);
    { auto t = std::make_shared<Tracker>(); auto t2 = t; } CHECK("shared_ptr", destroyed == 1);
    int k = 3; auto add = [k](int x) { return x + k; }; std::function<int(int)> f = add;
    CHECK("lambda + function", f(4) == 7);
#if __cplusplus >= 201703L
    std::optional<int> o; CHECK("optional", !o && o.value_or(9) == 9);
    std::variant<int, std::string> var = std::string("x"); CHECK("variant", std::holds_alternative<std::string>(var));
    std::string_view sv = "hello world"; CHECK("string_view", sv.substr(6) == "world");
    auto [q, r] = std::make_pair(17 / 5, 17 % 5); CHECK("structured bindings", q == 3 && r == 2);
#else
    skipped += 4;
#endif
#if __cplusplus >= 202002L
    int arr[] = {5, 1, 4}; std::span<int> sp(arr); CHECK("span", sp.size() == 3 && sp[2] == 4);
    auto evens = std::views::iota(1, 11) | std::views::filter([](int x) { return x % 2 == 0; }) | std::views::transform([](int x) { return x * x; });
    int s2 = 0; for (int x : evens) s2 += x; CHECK("ranges", s2 == 220);
    CHECK("concepts + constexpr", sum_to(10L) == 55L);
#if __has_include(<format>) && defined(__cpp_lib_format)
    CHECK("std::format", std::format("{:>6}|{:x}|{:.2f}", 42, 255, 3.14159) == "    42|ff|3.14");
#else
    skipped++; std::printf("  std::format not available\n");
#endif
#else
    skipped += 4;
#endif
    std::printf("t3 C++ (%ld): %d passed, %d failed, %d skipped\n", (long)__cplusplus, pass, fail, skipped);
    std::printf("%s\n", fail ? "RESULT FAIL" : "RESULT PASS");
    return fail ? 10 : 0;
}
