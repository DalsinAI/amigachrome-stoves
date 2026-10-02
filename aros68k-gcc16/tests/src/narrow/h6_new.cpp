#include <cstdio>
#include <vector>
int main() { std::vector<int> v(1000, 3); int *p = new int[10]; delete[] p; std::printf("h6 new/vector: %d\n", v[999]); return 0; }
