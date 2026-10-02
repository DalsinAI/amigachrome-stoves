#include <cstdio>
#include <memory>
int main() { auto p = std::make_shared<int>(7); auto q = p; std::printf("h5 shared_ptr: %ld uses\n", (long)p.use_count()); return 0; }
