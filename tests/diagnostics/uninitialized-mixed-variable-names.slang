//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Data { float value; };

// Check that removing safe reads still reports the correct name for an unsafe variable.
void mixedVariables(bool condition, bool assigned, float* output)
{
    Data alwaysInitialized, guarded, missing;
    if (condition)
        alwaysInitialized.value = 1;
    else
        alwaysInitialized.value = 2;
    if (condition)
        guarded.value = 3;
    if (assigned)
        missing.value = 4;
    output[0] = alwaysInitialized.value;
    if (condition)
    {
        output[1] = guarded.value;
        output[2] = missing.value;
//CHECK:          ^ E41035
//CHECK:          ^ variable 'missing' may be uninitialized on some paths
    }
}

[shader("compute")]
void main() {}
