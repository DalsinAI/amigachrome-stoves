/* t3f: the same pieces in plain pthreads: pthread_once, keys, mutex, cond. */
#include <pthread.h>
#include <stdio.h>
static pthread_once_t once = PTHREAD_ONCE_INIT; static int once_count;
static pthread_key_t key; static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER; static int keys_ok;
static void init(void) { ++once_count; }
static void *worker(void *arg)
{
    int mine = (int)(long)arg;
    pthread_once(&once, init);
    pthread_setspecific(key, (void *)(long)(mine + 1000));
    pthread_mutex_lock(&m); keys_ok += (long)pthread_getspecific(key) == mine + 1000; pthread_mutex_unlock(&m);
    return 0;
}
int main(void)
{
    pthread_t t[4]; int i, ok;
    pthread_key_create(&key, 0);
    for (i = 0; i < 4; i++) pthread_create(&t[i], 0, worker, (void *)(long)i);
    for (i = 0; i < 4; i++) pthread_join(t[i], 0);
    ok = once_count == 1 && keys_ok == 4;
    printf("%s pthread_once (%d), keys (%d of 4)\n", ok ? "PASS" : "FAIL", once_count, keys_ok);
    puts(ok ? "t3f OK" : "t3f FAILED");
    return ok ? 0 : 20;
}
