/* os4-gcc16 smoke test: built like a device (no start files, no C library,
 * only libgcc), calling exec through the interface it is handed. */
#include <exec/exec.h>
#include <proto/exec.h>

int32 _start(void) { return RETURN_FAIL; }

uint64 count_down(struct ExecIFace *iexec, uint64 n, uint32 d)
{
    iexec->Forbid();
    n = n / d + n % d;                  /* 64-bit division comes from libgcc */
    iexec->Permit();
    return n;
}
