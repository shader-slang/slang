//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct Data { float value; };

// Check that a write on the true branch does not initialize a read on the false branch.
float oppositeGuard(bool condition)
{
    Data data;
    if (condition)
        data.value = 1;
    if (condition)
        return 0;
    else
        return data.value;
//CHECK:^^^^^^ E41035
//CHECK:^^^^^^ variable 'data' may be uninitialized on some paths
}

[shader("compute")]
void main() {}
