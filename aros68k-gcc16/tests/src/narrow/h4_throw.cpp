#include <cstdio>
int main() { try { throw 42; } catch (int v) { std::printf("h4 throw/catch: %d\n", v); } return 0; }
