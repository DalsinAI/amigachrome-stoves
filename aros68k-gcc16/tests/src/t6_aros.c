/* t6: the AROS system API through the compiler: exec and dos library calls,
   which go through register-argument library stubs. */
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>

static int pass, fail;
#define CHECK(name, cond) do { if (cond) pass++; else { fail++; printf("FAIL %s (line %d)\n", name, __LINE__); } } while (0)

int main(void)
{
    struct Task *me = FindTask(NULL);
    CHECK("FindTask", me != NULL && me->tc_Node.ln_Name != NULL);
    UBYTE *mem = AllocVec(65536, MEMF_ANY | MEMF_CLEAR);
    CHECK("AllocVec clear", mem != NULL && mem[0] == 0 && mem[65535] == 0);
    if (mem) { memset(mem, 0x5A, 65536); CHECK("AllocVec write", mem[40000] == 0x5A); FreeVec(mem); }
    CHECK("AvailMem", AvailMem(MEMF_ANY) > 65536);
    struct Library *lib = OpenLibrary("utility.library", 0);
    CHECK("OpenLibrary", lib != NULL && lib->lib_Version >= 39);
    if (lib) CloseLibrary(lib);
    struct MsgPort *port = CreateMsgPort();
    CHECK("CreateMsgPort", port != NULL);
    if (port) {
        struct Message msg; memset(&msg, 0, sizeof msg); msg.mn_Length = sizeof msg;
        PutMsg(port, &msg);
        CHECK("PutMsg/GetMsg", GetMsg(port) == &msg && GetMsg(port) == NULL);
        DeleteMsgPort(port);
    }
    BPTR lock = Lock("SYS:", SHARED_LOCK);
    CHECK("Lock SYS:", lock != 0);
    if (lock) {
        char name[256];
        CHECK("NameFromLock", NameFromLock(lock, (STRPTR)name, sizeof name) && strchr(name, ':') != NULL);
        printf("  SYS: is %s\n", name);
        UnLock(lock);
    }
    struct DateStamp ds; DateStamp(&ds);
    CHECK("DateStamp", ds.ds_Days > 0);
    BPTR fh = Open("T:gcc16-test.txt", MODE_NEWFILE);
    CHECK("Open/Write", fh != 0 && Write(fh, "gcc16\n", 6) == 6);
    if (fh) Close(fh);
    char rd[8] = {0}; fh = Open("T:gcc16-test.txt", MODE_OLDFILE);
    CHECK("Read back", fh != 0 && Read(fh, rd, 6) == 6 && memcmp(rd, "gcc16\n", 6) == 0);
    if (fh) Close(fh);
    printf("  Task \"%s\", %lu bytes free\n", me->tc_Node.ln_Name, (unsigned long)AvailMem(MEMF_ANY));
    printf("t6 AROS API: %d passed, %d failed\n", pass, fail);
    printf("%s\n", fail ? "RESULT FAIL" : "RESULT PASS");
    return fail ? 10 : 0;
}
