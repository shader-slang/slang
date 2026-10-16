//TEST:SIMPLE: -target spirv -no-codegen -warnings-as-errors all

struct Data { float value; };

// Check that returning on the uninitialized path protects a later read.
float earlyExit(bool condition)
{
    Data data;
    if (condition) data.value = 1;
    if (condition) {} else return 0;
    return data.value;
}

// Check that separate exits in both arms protect a read after the arms reconverge.
float splitGuard(bool condition, bool other, float* output)
{
    Data data;
    if (condition) data.value = 1;
    if (other) output[1] += 1;
    if (other)
    {
        if (condition) output[0] += 1;
        else return 0;
    }
    else
    {
        if (condition) output[0] += 2;
        else return 0;
    }
    return data.value;
}

[shader("compute")]
void main() {}
