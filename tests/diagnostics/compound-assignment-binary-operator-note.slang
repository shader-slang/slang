//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=-1):-target spirv

// Issue #14038: Slang does not derive `x op= y` from `operator op`. When no `operator op=`
// applies but `x = x op y` would type-check, the E39999 error gets a note (E40026) that says to
// declare `operator op=` with an `inout` first parameter, or to write `x = x op y`. When the
// binary operator does not apply either, or its result does not convert back to the type of
// `x`, there is no note.

// The repro from the issue.
struct S3
{
    float a, b, c;
}
S3 operator-(S3 x, S3 y) { return { x.a - y.a, x.b - y.b, x.c - y.c }; }
S3 operator*(S3 x, float s) { return { x.a * s, x.b * s, x.c * s }; }

void subtractS3(inout S3 x, S3 y)
{
    x -= y;
}
// CHECK: error[E39999]: no overload for '-=' applicable to arguments of type (S3, S3)
// CHECK: note[E40026]: '-' applies to these arguments, but '-=' is not derived from it: declare 'operator-=' with an 'inout' first parameter, or write 'x = x - y'

// The right operand has a different type from the left one. Here the builtin scalar `*=` is the
// one closest candidate, so the error is its type mismatch rather than E39999; the note is the
// same.
void scale(inout S3 x, float s)
{
    x *= s;
}
// CHECK: error[E30019]: type mismatch in expression
// CHECK: note[E40026]: '*' applies to these arguments, but '*=' is not derived from it: declare 'operator*=' with an 'inout' first parameter, or write 'x = x * y'

// Bitwise and shift compound operators.
struct Flags
{
    uint bits;
}
Flags operator|(Flags x, Flags y) { return { x.bits | y.bits }; }
Flags operator<<(Flags x, int n) { return { x.bits << n }; }

void setFlags(inout Flags f, Flags g)
{
    f |= g;
    f <<= 2;
}
// CHECK: error[E39999]: no overload for '|=' applicable to arguments of type (Flags, Flags)
// CHECK: note[E40026]: '|' applies to these arguments, but '|=' is not derived from it: declare 'operator|=' with an 'inout' first parameter, or write 'x = x | y'
// CHECK: error[E39999]: no overload for '<<=' applicable to arguments of type (Flags, int)
// CHECK: note[E40026]: '<<' applies to these arguments, but '<<=' is not derived from it: declare 'operator<<=' with an 'inout' first parameter, or write 'x = x << y'

// No note when the binary operator does not apply: `S3` has no `operator+` and no `operator/`.
void addAndDivide(inout S3 x, S3 y)
{
    x += y;
    x /= y;
}
// CHECK: error[E39999]: no overload for '+=' applicable to arguments of type (S3, S3)
// CHECK-NOT: E40026
// CHECK: error[E39999]: no overload for '/=' applicable to arguments of type (S3, S3)
// CHECK-NOT: E40026

// No note when the binary operator applies but its result is not the left operand's type, so
// `x = x ^ y` would not compile either.
struct Mask
{
    uint bits;
}
S3 operator^(Mask x, Mask y) { return { 0, 0, 0 }; }

void toggle(inout Mask m, Mask n)
{
    m ^= n;
}
// CHECK: error[E39999]: no overload for '^=' applicable to arguments of type (Mask, Mask)
// CHECK-NOT: E40026

// No note when the left operand is not assignable: `x = x - y` would not compile either.
S3 makeS3() { return { 0, 0, 0 }; }

void subtractFromTemporary(S3 y)
{
    makeS3() -= y;
}
// CHECK: error[E39999]: no overload for '-=' applicable to arguments of type (S3, S3)
// CHECK-NOT: E40026

// No note when the right operand cannot be passed to the binary operator's `inout` or `out`
// parameter: `x = x & makeS3()` and `x = x | makeS3()` would not compile either.
S3 operator&(S3 x, inout S3 y) { return x; }
S3 operator|(S3 x, out S3 y) { y = x; return x; }

void combineWithTemporary(inout S3 x)
{
    x &= makeS3();
    x |= makeS3();
}
// CHECK: error[E39999]: no overload for '&=' applicable to arguments of type (S3, S3)
// CHECK-NOT: E40026
// CHECK: error[E39999]: no overload for '|=' applicable to arguments of type (S3, S3)
// CHECK-NOT: E40026

// Generics: the note follows overload resolution of the binary operator, including generic
// operators and their constraints.
interface ISubtractable
{
    This subtract(This other);
}
T operator-<T : ISubtractable>(T x, T y) { return x.subtract(y); }

struct Vec2 : ISubtractable
{
    float x, y;
    Vec2 subtract(Vec2 other) { return { x - other.x, y - other.y }; }
}

// A concrete type that meets the generic operator's constraint gets the note.
void subtractVec2(inout Vec2 a, Vec2 b)
{
    a -= b;
}
// CHECK: error[E39999]: no overload for '-=' applicable to arguments of type (Vec2, Vec2)
// CHECK: note[E40026]: '-' applies to these arguments, but '-=' is not derived from it

// So does a type parameter that meets it.
void subtractGeneric<T : ISubtractable>(inout T a, T b)
{
    a -= b;
}
// CHECK: error[E39999]: no overload for '-=' applicable to arguments of type (T, T)
// CHECK: note[E40026]: '-' applies to these arguments, but '-=' is not derived from it

// A type that does not meet the constraint gets no note.
struct NotSubtractable
{
    float x;
}

void subtractUnconstrained(inout NotSubtractable a, NotSubtractable b)
{
    a -= b;
}
// CHECK: error[E39999]: no overload for '-=' applicable to arguments of type (NotSubtractable, NotSubtractable)
// CHECK-NOT: E40026

// An unconstrained type parameter gets no note.
void subtractAny<T>(inout T a, T b)
{
    a -= b;
}
// CHECK: error[E39999]: no overload for '-=' applicable to arguments of type (T, T)
// CHECK-NOT: E40026
