//DIAGNOSTIC_TEST:SIMPLE:

float foo(out float3 v)
{
    // This should error as we haven't set v before we read from it
    float r = v.x + 1.0;
    // This should warn as we haven't set v before we return
    return r;
}

// This should warn as we return without x being initialized
float bar(out float x)
{
    return 0;
}

// This should also warn pointing at the implicit return
void baz(out float x) {}

void twoReturns(bool b, out float y)
{
    if(b)
    {
        // Should warn
        return;
    }
    y = 0;
    // Shouldn't warn
    return;
}

void twoOkReturns(bool b, out float y)
{
    if(b)
    {
        // Shouldn't warn
        unused(y);
        return;
    }
    y = 0;
    // Shouldn't warn
    return;
}

// TODO: This should warn that n is potentially uninitialized
int ok(bool b, out int n)
{
    if(b)
        n = 0;
    return n;
}

// TODO: This should warn that arr isn't fully initialized
void partial(out float arr[2])
{
    arr[0] = 1;
}
