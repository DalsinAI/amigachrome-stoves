// t3e: std::async and a future.
#include <cstdio>
#include <future>
int main()
{
    auto f = std::async(std::launch::async, [] { return 6 * 7; });
    int v = f.get();
    std::puts(v == 42 ? "PASS async" : "FAIL async");
    std::puts(v == 42 ? "t3e OK" : "t3e FAILED");
    return v == 42 ? 0 : 20;
}
