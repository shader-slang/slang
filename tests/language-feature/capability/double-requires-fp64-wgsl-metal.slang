// WGSL and Metal have no 64-bit floating-point type, so `double` requires the `fp64` capability.
// A use of `double` reachable from an entry point is reported as a capability error at the user's
// code. The check runs before optimization, so a `double` that would fold away is reported too.
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
//DIAGNOSTIC_TEST:SIMPLE(diag=MEMBER, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=13
//DIAGNOSTIC_TEST:SIMPLE(diag=MEMBER, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=13
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT, non-exhaustive): -target wgsl -entry computeMain -stage compute -DCASE=11
//DIAGNOSTIC_TEST:SIMPLE(diag=DEFAULT, non-exhaustive): -target metal -entry computeMain -stage compute -DCASE=11
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT): -target wgsl -entry computeMain -stage compute -DCASE=12
//DIAGNOSTIC_TEST:SIMPLE(diag=EXPLICIT): -target metal -entry computeMain -stage compute -DCASE=12

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
}
//FOLDED: see using of 'double'

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

cbuffer Constants
{
    double scale;
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
//CBUFFER: see using of 'double'

#elif (CASE == 6)

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*CALL:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    outputBuffer[0] = (float)sqrt(double(outputBuffer[1]));
}
//CALL: see using of 'double'

#elif (CASE == 7)

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
/*MATRIX:
     ^^^^^^^^^^^ unavailable features in entry point
*/
{
    double2x2 m = double2x2(
        double(outputBuffer[0]),
        double(outputBuffer[1]),
        double(outputBuffer[2]),
        double(outputBuffer[3]));
    outputBuffer[0] = (float)mul(m, m)[1][1];
}
//MATRIX: see using of 'double'

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

#elif (CASE == 11)

RWStructuredBuffer<float> outputBuffer;

float scaled(float x = (float)(double(outputBuffer[2]) * 2.0LF))
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
//DEFAULT: see using of 'double'

#elif (CASE == 13)

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

#else

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

#endif
