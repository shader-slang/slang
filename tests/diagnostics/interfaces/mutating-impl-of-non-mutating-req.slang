// mutating-impl-of-non-mutating-req.slang

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main
interface IThing
{
    int processValue(int inValue);
//CHECK:^^^^^^^^^^^^ see interface requirement declaration of 'processValue'
}

struct Counter : IThing
{
    int state;

    [mutating] int processValue(int inValue)
//CHECK:           ^^^^^^^^^^^^ interface requirement mismatch
//CHECK:           ^^^^^^^^^^^^ member 'processValue' does not match interface requirement.
    {
        int result = state;
        state += inValue;
        return state;
    }
}

int helper<T : IThing>(T thing, int value)
{
    return thing.processValue(value);
}

int test(int value)
{
    Counter counter = { value };
    return helper(counter, value);
}

cbuffer C
{
    int gValue;
}

[shader("fragment")]
int main() : SV_Target
{
    return test(gValue);
}