//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Regression test for https://github.com/shader-slang/slang/issues/12911:
// a ConstantBuffer<A> vs ConstantBuffer<B> argument mismatch must produce a
// LOCATED E30019 (caret + "expected/got" detail), like the plain-struct and
// StructuredBuffer cases below.
//
// The ConstantBuffer message reports the dereferenced element type ('B'), not
// the full 'ConstantBuffer<B>', because the buffer is implicitly dereferenced
// before the failing coercion.

struct A { float field; };
struct B { int field; };

void directConsumer(A a) {}
void directCaller(B b)
{
    directConsumer(b);
//CHECK:           ^ type mismatch in expression
//CHECK:           ^ expected an expression of type 'A', got 'B'
}

void constantConsumer(ConstantBuffer<A> abuf) {}
void constantCaller(ConstantBuffer<B> bbuf)
{
    constantConsumer(bbuf);
//CHECK:             ^^^^ type mismatch in expression
//CHECK:             ^^^^ expected an expression of type 'ConstantBuffer<A, DefaultDataLayout>', got 'B'
}

void structuredConsumer(StructuredBuffer<A> abuf) {}
void structuredCaller(StructuredBuffer<B> bbuf)
{
    structuredConsumer(bbuf);
//CHECK:               ^^^^ type mismatch in expression
//CHECK:               ^^^^ expected an expression of type 'StructuredBuffer<A, DefaultDataLayout>', got 'StructuredBuffer<B, DefaultDataLayout>'
}
