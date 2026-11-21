// WGSL and Metal have no 64-bit floating-point type, so `double` requires the `fp64` capability.
// A use of `double` reachable from an entry point is reported as a capability error at the user's
// code. The check runs before optimization, so a `double` that would fold away is reported too.
// A struct's instance fields count wherever a value of the struct type is used (CASE 3-4, 8-10,
// 14, 18). A resource field counts through its element type only, because type legalization
// moves the field out of the struct but Metal keeps it in a parameter block's argument buffer
// (CASE 16, 21). The default value of an omitted argument counts at the call (CASE 11-12), and
// the initializer of a static variable where the variable is read (CASE 19-20). A conformance
// linked with `-conformance` is checked against the target when it is created (CASE 22-23).
// CASE 15-17 and 24 must compile.
//DIAGNOSTIC_TEST:SIMPLE(diag=BUFFER, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=0
//DIAGNOSTIC_TEST:SIMPLE(diag=BUFFER, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=0
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=1
//DIAGNOSTIC_TEST:SIMPLE(diag=PARAM, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=1
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=2
//DIAGNOSTIC_TEST:SIMPLE(diag=FOLDED, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=2
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=3
//DIAGNOSTIC_TEST:SIMPLE(diag=FIELD, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=3
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=4
//DIAGNOSTIC_TEST:SIMPLE(diag=NESTED, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=4
//DIAGNOSTIC_TEST:SIMPLE(diag=CBUFFER, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=5
//DIAGNOSTIC_TEST:SIMPLE(diag=CBUFFER, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=5
//DIAGNOSTIC_TEST:SIMPLE(diag=CALL, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=6
//DIAGNOSTIC_TEST:SIMPLE(diag=CALL, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=6
//DIAGNOSTIC_TEST:SIMPLE(diag=MATRIX, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=7
//DIAGNOSTIC_TEST:SIMPLE(diag=MATRIX, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=7
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=8
//DIAGNOSTIC_TEST:SIMPLE(diag=GENERIC, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=8
//DIAGNOSTIC_TEST:SIMPLE(diag=INHERITED, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=9
//DIAGNOSTIC_TEST:SIMPLE(diag=INHERITED, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=9
//DIAGNOSTIC_TEST:SIMPLE(diag=MEMBER, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=10
//DIAGNOSTIC_TEST:SIMPLE(diag=MEMBER, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=10
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=11
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=11
//DIAGNOSTIC_TEST:SIMPLE(diag=CASTDEFAULT, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=12
//DIAGNOSTIC_TEST:SIMPLE(diag=CASTDEFAULT, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=12
//DIAGNOSTIC_TEST:SIMPLE(diag=GLSLMUL, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=13
//DIAGNOSTIC_TEST:SIMPLE(diag=GLSLMUL, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=13
//DIAGNOSTIC_TEST:SIMPLE(diag=GATED, non-exhaustive): -target spirv -entry computeMain -stage compute -DCASE=14
//DIAGNOSTIC_TEST:SIMPLE(diag=GATED, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=14
//DIAGNOSTIC_TEST:SIMPLE(diag=ASSOC, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=18
//DIAGNOSTIC_TEST:SIMPLE(diag=ASSOC, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=18
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=19
//DIAGNOSTIC_TEST:SIMPLE(diag=STATIC, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=19
//DIAGNOSTIC_TEST:SIMPLE(diag=STATICMEMBER, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=20
//DIAGNOSTIC_TEST:SIMPLE(diag=STATICMEMBER, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=20
//DIAGNOSTIC_TEST:SIMPLE(diag=PBLOCK, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=21
//DIAGNOSTIC_TEST:SIMPLE(diag=PBLOCK, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=21
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFMETHOD, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=22 -conformance "FloatValue:IValue=0" -conformance "DoubleValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFMETHOD, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=22 -conformance "FloatValue:IValue=0" -conformance "DoubleValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFFIELD, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=23 -conformance "FloatValue:IValue=0" -conformance "DoubleValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFFIELD, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=23 -conformance "FloatValue:IValue=0" -conformance "DoubleValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFOK, exit-code=0): -target wgsl -entry computeMain -stage compute -DCASE=24 -conformance "FloatValue:IValue=0" -conformance "IntValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFOK, exit-code=0): -target metal -entry computeMain -stage compute -DCASE=24 -conformance "FloatValue:IValue=0" -conformance "IntValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=CONFOK, exit-code=0): -target spirv -entry computeMain -stage compute -DCASE=22 -conformance "FloatValue:IValue=0" -conformance "DoubleValue:IValue=1"
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT, exit-code=0): -target wgsl -entry computeMain -stage compute -DCASE=15
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT, exit-code=0): -target metal -entry computeMain -stage compute -DCASE=15
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE, exit-code=0): -target wgsl -entry computeMain -stage compute -DCASE=16
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE, exit-code=0): -target metal -entry computeMain -stage compute -DCASE=16
//DIAGNOSTIC_TEST:SIMPLE(diag=RESOURCE, exit-code=0): -target cuda -entry computeMain -stage compute -DCASE=16
//DIAGNOSTIC_TEST:SIMPLE(diag=QUALIFIER, exit-code=0): -target wgsl -entry computeMain -stage compute -DCASE=17
//DIAGNOSTIC_TEST:SIMPLE(diag=QUALIFIER, exit-code=0): -target metal -entry computeMain -stage compute -DCASE=17

#if (CASE == 0)

RWStructuredBuffer<double> buffer;

[numthreads(1, 1, 1)]
void computeMain()
/*BUFFER:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    buffer[2] = buffer[0] + buffer[1];
/*BUFFER:
    ^^^^^^ see using of 'double'
*/
}

#elif (CASE == 1)

RWStructuredBuffer<float> outputBuffer;

void storeValue(double v)
{
    outputBuffer[0] = (float)v;
}

[numthreads(1, 1, 1)]
void computeMain()
/*PARAM:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    storeValue(0.0LF);
/*PARAM:
    ^^^^^^^^^^ see using of 'double'
*/
}

#elif (CASE == 2)

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*FOLDED:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = (float)(1.5LF);
/*FOLDED:
                             ^ see using of 'double'
*/
}

#elif (CASE == 3)

// Only `a` is read, but declaring `S` for the buffer also declares the `double` field.
struct S
{
    float a;
    double b;
//FIELD:   ^ see using of 'double'
};

RWStructuredBuffer<S> sb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*FIELD:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = sb[0].a;
}

#elif (CASE == 4)

struct Inner
{
    double x;
//NESTED:  ^ see using of 'double'
};

struct Outer
{
    float a;
    Inner inner;
};

RWStructuredBuffer<Outer> sb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*NESTED:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = sb[0].a;
}

#elif (CASE == 5)

// Only `offset` is read, but declaring the constant buffer also declares `scale`.
cbuffer Constants
{
    double scale;
/*CBUFFER:
           ^^^^^ see using of 'double'
*/
    float offset;
};

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*CBUFFER:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = offset;
}

#elif (CASE == 6)

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*CALL:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = (float)sqrt(double(outputBuffer[1]));
/*CALL:
                                 ^ see using of 'double'
*/
}

#elif (CASE == 7)

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*MATRIX:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    double2x2 m = double2x2(
/*MATRIX:
    ^^^^^^^^^ see using of 'double'
*/
        double(outputBuffer[0]),
        double(outputBuffer[1]),
        double(outputBuffer[2]),
        double(outputBuffer[3]));
    outputBuffer[0] = (float)mul(m, m)[1][1];
}

#elif (CASE == 8)

struct Pair<T>
{
    T first;
    double second;
//GENERIC: ^^^^^^ see using of 'double'
};

RWStructuredBuffer<Pair<float>> sb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*GENERIC:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = sb[0].first;
}

#elif (CASE == 9)

struct Base
{
    double d;
/*INHERITED:
           ^ see using of 'double'
*/
};

struct Derived : Base
{
    float a;
};

RWStructuredBuffer<Derived> sb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*INHERITED:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = sb[0].a;
}

#elif (CASE == 10)

struct Outer<T>
{
    struct Inner
    {
        double d;
//MEMBER:      ^ see using of 'double'
        float f;
    };
};

RWStructuredBuffer<Outer<int>.Inner> sb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*MEMBER:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = sb[0].f;
}

#elif (CASE == 11)

RWStructuredBuffer<float> outputBuffer;

float scaled(float x = (float)(double(outputBuffer[2]) * 2.0LF))
/*DEFAULT:
                              ^ see using of 'double'
*/
{
    return x;
}

[numthreads(1, 1, 1)]
void computeMain()
/*DEFAULT:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = scaled();
}

#elif (CASE == 12)

RWStructuredBuffer<float> outputBuffer;

// A cast that selects a constructor fills in the constructor's default arguments, like a call.
struct Wide
{
    float v;
    __init(float a, float s = (float)(double(outputBuffer[2]) * 2.0LF)) { v = a * s; }
/*CASTDEFAULT:
                                     ^ see using of 'double'
*/
};

[numthreads(1, 1, 1)]
void computeMain()
/*CASTDEFAULT:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = ((Wide)outputBuffer[1]).v;
}

#elif (CASE == 13)

import glsl;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*GLSLMUL:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    double3x3 m = double3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
/*GLSLMUL:
    ^^^^^^^^^ see using of 'double'
*/
    double3x3 n = m;
    m *= n;
    outputBuffer[0] = (float)m[0][0];
}

#elif (CASE == 14)

// The field rule applies to every capability: `H` is available only on HLSL, so a struct that
// contains an `H` is too, even when only another field is read.
[require(hlsl)]
struct H
{
    int x;
};

struct S
{
    float a;
    H h;
/*GATED:
      ^ see using of 'H'
*/
};

RWStructuredBuffer<S> sb;

[numthreads(1, 1, 1)]
void computeMain()
/*GATED:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    sb[0].a = 1;
}

#elif (CASE == 15)

// A call that passes the argument does not evaluate the `double` default value, so it compiles.
RWStructuredBuffer<float> outputBuffer;

float scaled(float x = (float)(double(outputBuffer[2]) * 2.0LF))
{
    return x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = scaled(outputBuffer[1]);
}

#elif (CASE == 16)

// Type legalization moves resource fields out of the struct, so declaring `G` declares neither
// `b` nor `rw`, and their types' requirements do not apply to code that only uses `o`. Metal keeps
// parameter-block resources in its argument buffer, but these element types need nothing Metal
// lacks.
struct G
{
    Buffer<float> b;
    RWBuffer<float> rw;
    RWStructuredBuffer<float> o;
};

ParameterBlock<G> g;

[numthreads(1, 1, 1)]
void computeMain()
{
    g.o[0] = 1;
}

#elif (CASE == 17)

// A generic struct that only qualifies a name declares no value of itself, so its `double`
// field adds nothing to code that uses `Outer<int>.Inner` or calls `Outer<float>.one()`.
struct Outer<T>
{
    double unused;
    struct Inner
    {
        float f;
    };
    static float one() { return 1.0; }
};

RWStructuredBuffer<Outer<int>.Inner> sb;

[numthreads(1, 1, 1)]
void computeMain()
{
    sb[0].f = Outer<float>.one();
}

#elif (CASE == 18)

// Two specializations of one generic struct can have different field types. `Box<A>` is walked
// first and has no `double`; `Box<B>` must still be walked, because its `T.Assoc` field is `double`.
interface IFoo
{
    associatedtype Assoc;
}
struct A : IFoo
{
    typedef float Assoc;
}
struct B : IFoo
{
    typedef double Assoc;
}
struct Box<T : IFoo>
{
    float a;
    T.Assoc f;
//ASSOC:    ^ see using of 'double'
};
struct Pair
{
    Box<A> x;
    Box<B> y;
};

RWStructuredBuffer<Pair> sb;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*ASSOC:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = sb[0].x.a;
}

#elif (CASE == 19)

// The entry point reads `kScale`, so the initializer that computes it runs for the entry point.
RWStructuredBuffer<float> outputBuffer;

static float kScale = (float)(double(outputBuffer[1]) * 2.0LF);
//STATIC:                    ^ see using of 'double'

[numthreads(1, 1, 1)]
void computeMain()
/*STATIC:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = kScale;
}

#elif (CASE == 20)

// The same holds for a static member of a struct.
RWStructuredBuffer<float> outputBuffer;

struct Settings
{
    static float scale = (float)(double(outputBuffer[1]) * 2.0LF);
//STATICMEMBER:                 ^ see using of 'double'
};

[numthreads(1, 1, 1)]
void computeMain()
/*STATICMEMBER:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = Settings.scale;
}

#elif (CASE == 21)

// `wide` is never used, but Metal keeps it in the parameter block's argument buffer.
struct Globals
{
    RWStructuredBuffer<double> wide;
//PBLOCK:                      ^^^^ see using of 'double'
    RWStructuredBuffer<float> output;
};

ParameterBlock<Globals> globals;

[numthreads(1, 1, 1)]
void computeMain()
/*PBLOCK:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    globals.output[0] = 1.0f;
}

#elif (CASE >= 22)

// No entry point names `DoubleValue` or `IntValue`, but `-conformance` links each of them, with
// its fields and methods, for dynamic dispatch.
interface IValue
{
    float get();
}

struct FloatValue : IValue
{
    float v;
    float get() { return v; }
};

#if (CASE == 22)
struct DoubleValue : IValue
/*CONFMETHOD:
       ^^^^^^^^^^^ dependencies not compatible on target
*/
{
    float v;
    float get() { return (float)(double(v) * 2.0LF); }
//CONFMETHOD:                   ^ see using of 'double'
};
#elif (CASE == 23)
struct DoubleValue : IValue
/*CONFFIELD:
       ^^^^^^^^^^^ dependencies not compatible on target
*/
{
    double v;
/*CONFFIELD:
           ^ see using of 'double'
*/
    // `get` uses no `double`, so the only requirement is the field.
    float get() { return 1.0f; }
};
#else
struct IntValue : IValue
{
    int v;
    float get() { return float(v); }
};
#endif

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uniform int id)
{
    IValue value = createDynamicObject<IValue>(id, 1.0f);
    outputBuffer[0] = value.get();
}

#endif
