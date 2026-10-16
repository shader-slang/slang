//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Data { float value; };

// Check that a condition recomputed in a loop can reach an uninitialized read
// even when its value on the preceding iteration prevented that read.
void loopDefinedCondition(bool initialized, bool again, int* flags, float* output)
{
    Data data;
    if (initialized) data.value = 3;
    bool first = true;
    do
    {
        bool condition = flags[0] != 0;
        if (first)
        {
            if (condition) return;
        }
        else
        {
            if (condition)
                output[0] = data.value;
//CHECK:                  ^ E41035
//CHECK:                  ^ variable 'data' may be uninitialized on some paths
        }
        first = false;
    }
    while (again);
}

[shader("compute")]
void main() {}
