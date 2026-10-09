// Every read of a `__ref_writeonly` parameter is an error, while binding it as a location is not.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute

void takeIn(int v) {}
void takeInOut(inout int v) {}
void takeOut(out int v) { v = 0; }
void takeRef(__ref int v) {}
void takeWriteOnly(__ref_writeonly int v) { v = 1; }
void takeReadOnly(__ref_readonly int v) {}

struct S
{
    int data[2];
    __subscript(int i) -> int
    {
        get { return data[i]; }
        set { data[i] = newValue; }
    }
    int get() { return data[0]; }
    [mutating] void bump() { data[0]++; }
    [__ref] void clear() { data[0] = 0; }
}

int reads(__ref_writeonly int x, __ref_writeonly int a[4])
{
    int y = x;
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    let z = x;
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    y = x;
//CHECK:^ E30119
//CHECK:^ cannot read from writeonly, check modifiers.
    int s = x + 1;
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    takeIn(x);
//CHECK:   ^ E30119
//CHECK:   ^ cannot read from writeonly, check modifiers.
    takeInOut(x);
//CHECK:      ^ E30119
//CHECK:      ^ cannot read from writeonly, check modifiers.
    x += 1;
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    x++;
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    int e = a[0];
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    if (x > 0)
//CHECK:^ E30119
//CHECK:^ cannot read from writeonly, check modifiers.
        y = 2;
    switch (x)
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    {
    default:
        break;
    }
    return x;
//CHECK:   ^ E30119
//CHECK:   ^ cannot read from writeonly, check modifiers.
}

float swizzles(__ref_writeonly float4 v, __ref_writeonly float2x2 m, __ref_writeonly float f)
{
    float a = v.x;
//CHECK:        ^ E30119
//CHECK:        ^ cannot read from writeonly, check modifiers.
    float2 b = v.xy;
//CHECK:         ^^ E30119
//CHECK:         ^^ cannot read from writeonly, check modifiers.
    float c = m._11;
//CHECK:        ^^^ E30119
//CHECK:        ^^^ cannot read from writeonly, check modifiers.
    return f.x;
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
}

int receivers(__ref_writeonly S s)
{
    int a = s[0];
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    s.bump();
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
    return s.get();
//CHECK:   ^ E30119
//CHECK:   ^ cannot read from writeonly, check modifiers.
}

void locations(__ref_writeonly int x, __ref_writeonly int a[4])
{
    x = 1;
    a[0] = 1;
    takeOut(x);
    takeRef(x);
    takeWriteOnly(x);
    takeReadOnly(x);
}

void elementLocations(
    __ref_writeonly float4 v,
    __ref_writeonly S s,
    __ref_writeonly RWTexture2D<float> t)
{
    v.x = 1;
    v.yz = float2(2, 3);
    s[0] = 1;
    s.clear();
    t[uint2(0, 0)] = 1;
}

float textureRead(__ref_writeonly RWTexture2D<float> t)
{
    uint w, h;
    t.GetDimensions(w, h);
    return t[uint2(0, 0)];
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
}

[numthreads(1, 1, 1)]
void main()
{
}
