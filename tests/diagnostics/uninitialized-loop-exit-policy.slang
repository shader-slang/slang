//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Disabled: the loop-exit rule treats a loop containing a store as initializing
// the variable even when the store is conditional. Tracking the repeated guard
// excludes the path that skips the loop, so this read receives no warning.
// Enable when loop-exit analysis accounts for paths that skip the store.

struct Data { float value; };

// Check that a store made only on some loop iterations, under a repeated guard,
// does not guarantee initialization at the read.
float guardedConditionalStoreInLoop(bool condition, bool store, bool again)
{
    Data data;
    if (condition)
    {
        do
        {
            if (store)
                data.value = 1;
        } while (again);
    }
    if (condition)
        return data.value;
//CHECK:^ E41035
//CHECK:^ variable 'data' may be uninitialized on some paths
    return 0;
}

[shader("compute")]
void main() {}
