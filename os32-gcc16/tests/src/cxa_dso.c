/* SPDX-License-Identifier: MIT */
/* For the tests built with -fuse-cxa-atexit (t12c, t13c, t19c): code built so
   passes &__dso_handle to __cxa_thread_atexit, and the stove's libnix does not
   define it. One program image needs only a unique address. */
void *__dso_handle = &__dso_handle;
