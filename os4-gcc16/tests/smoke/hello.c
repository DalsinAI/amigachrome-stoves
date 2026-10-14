/* os4-gcc16 smoke test: a C program that calls dos.library through its
 * interface, built with each C library the stove has. */
#include <proto/exec.h>
#include <proto/dos.h>
#include <string.h>
#include <stdio.h>

int main(void)
{
    char line[64];
    snprintf(line, sizeof line, "os4-gcc16 %d.%d", __GNUC__, __GNUC_MINOR__);
    IDOS->Printf("%s: hello, %ld bytes\n", line, (int32)strlen(line));
    return RETURN_OK;
}
