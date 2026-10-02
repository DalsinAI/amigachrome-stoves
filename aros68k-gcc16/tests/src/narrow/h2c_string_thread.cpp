#include <cstdio>
#include <string>
#include <pthread.h>
static void *nothing(void *) { return nullptr; }
int main() { pthread_t t; pthread_create(&t, nullptr, nothing, nullptr); pthread_join(t, nullptr);
             std::string s = "abc"; s += "def"; std::printf("h2c std::string after one thread: %s\n", s.c_str()); return 0; }
