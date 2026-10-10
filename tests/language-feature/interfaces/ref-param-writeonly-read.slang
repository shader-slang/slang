// Every read of a `__ref_writeonly` parameter is an error, while binding its memory location to an
// `out` or `__ref_writeonly` parameter, or assigning to it, is not.

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

void memoryLocationBindings(__ref_writeonly int x, __ref_writeonly int a[4])
{
    x = 1;
    a[0] = 1;
    takeOut(x);
    takeWriteOnly(x);
}

// `__ref` and `__ref_readonly` parameters may read their argument.
void readingReferenceBindings(__ref_writeonly int x)
{
    takeRef(x);
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    takeReadOnly(x);
//CHECK:         ^ E30119
//CHECK:         ^ cannot read from writeonly, check modifiers.
}

void elementMemoryLocationBindings(
    __ref_writeonly float4 v,
    __ref_writeonly S s,
    __ref_writeonly RWTexture2D<float> t)
{
    v.x = 1;
    v.yz = float2(2, 3);
    s[0] = 1;
    t[uint2(0, 0)] = 1;
}

// A `[__ref]` method may read its receiver.
void referenceReceiver(__ref_writeonly S s)
{
    s.clear();
/*CHECK:
    ^ E30119
    ^ cannot read from writeonly, check modifiers.
*/
}

struct P
{
    int a;
    int b;
}

struct Q
{
    int v;
    __init(int x) { v = x; }
}

struct QW
{
    int v;
    __init(__ref_writeonly int x)
    {
        x = 0;
        v = 1;
    }
}

struct QO
{
    int v;
    __init(out int x)
    {
        x = 0;
        v = 1;
    }
}

struct R
{
    int v;
}

void takeArray(int a[1]) {}
void takeR(R r) {}

// Constructing a value from a `__ref_writeonly` parameter reads it, unless the constructor
// parameter is itself `out` or `__ref_writeonly`.
void construction(__ref_writeonly int x)
{
    int v1[1] = { x };
//CHECK:          ^ E30119
//CHECK:          ^ cannot read from writeonly, check modifiers.
    P p1 = { x, 2 };
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    int v2[2] = { 1, x };
//CHECK:             ^ E30119
//CHECK:             ^ cannot read from writeonly, check modifiers.
    let v3 = int[1](x);
//CHECK:            ^ E30119
//CHECK:            ^ cannot read from writeonly, check modifiers.
    Q q1 = Q(x);
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    Q q2 = (Q)x;
//CHECK:      ^ E30119
//CHECK:      ^ cannot read from writeonly, check modifiers.
    Q q3 = { x };
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    R r1 = { x };
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    R r2 = R(x);
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    int2 i2 = { x, 1 };
//CHECK:        ^ E30119
//CHECK:        ^ cannot read from writeonly, check modifiers.
    P p2 = P(x, 2);
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    takeArray({ x });
//CHECK:        ^ E30119
//CHECK:        ^ cannot read from writeonly, check modifiers.
    takeR({ x });
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    float f1 = float(x);
//CHECK:             ^ E30119
//CHECK:             ^ cannot read from writeonly, check modifiers.
    float f2 = (float)x;
//CHECK:              ^ E30119
//CHECK:              ^ cannot read from writeonly, check modifiers.
    uint u1 = uint(x);
//CHECK:           ^ E30119
//CHECK:           ^ cannot read from writeonly, check modifiers.
    QW w1 = QW(x);
    QW w2 = (QW)x;
    QW w3 = { x };
    QO o1 = QO(x);
    QO o2 = { x };
}

// An implicit conversion or an identity conversion of a `__ref_writeonly` parameter is
// reported once, where the result is read.
void conversions(__ref_writeonly int x)
{
    float f = x;
//CHECK:      ^ E30119
//CHECK:      ^ cannot read from writeonly, check modifiers.
    int y = int(x);
//CHECK:        ^ E30119
//CHECK:        ^ cannot read from writeonly, check modifiers.
    int z = -x;
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
    int w = ~x;
//CHECK:     ^ E30119
//CHECK:     ^ cannot read from writeonly, check modifiers.
}

interface IGet
{
    int get();
}

struct G : IGet
{
    int v;
    int get() { return v; }
}

struct Base
{
    int a;
    int f() { return a; }
}

struct Derived : Base
//CHECK:         ^^^^ warning E30816
{
    int b;
}

// A member reached through a base type, a generic constraint or an existential is part of the
// same memory location, so reading it is reported.
int viewedReceivers<T : IGet>(
    __ref_writeonly T t,
    __ref_writeonly IGet e,
    __ref_writeonly Derived d)
{
    int a = t.get();
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    int b = e.get();
//CHECK:    ^ E30119
//CHECK:    ^ cannot read from writeonly, check modifiers.
    int c = d.a;
//CHECK:      ^ E30119
//CHECK:      ^ cannot read from writeonly, check modifiers.
    return d.f();
//CHECK:   ^ E30119
//CHECK:   ^ cannot read from writeonly, check modifiers.
}

// Dereferencing a pointer-valued `__ref_writeonly` parameter reads the pointer.
int pointerMember(__ref_writeonly P* p)
{
    return p.a;
//CHECK:   ^ E30119
//CHECK:   ^ cannot read from writeonly, check modifiers.
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
