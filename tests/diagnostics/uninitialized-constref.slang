//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry computeMain

void observe(__constref int value)
{
}

static int uninitializedGlobal;

void readUninitializedGlobalThroughConstRef()
{
    // A borrowed input reads its argument. It cannot initialize the supplied storage.
    observe(uninitializedGlobal);
//CHECK:   ^ use of uninitialized global variable
//CHECK:   ^ use of uninitialized global variable 'uninitializedGlobal'
}

void readUninitializedLocalThroughConstRef()
{
    int uninitializedLocal;

    // The same read-only contract applies to local storage.
    observe(uninitializedLocal);
//CHECK:   ^ use of uninitialized variable
//CHECK:   ^ use of uninitialized variable 'uninitializedLocal'
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}
