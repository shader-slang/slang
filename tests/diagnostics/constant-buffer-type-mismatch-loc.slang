//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Regression test for https://github.com/shader-slang/slang/issues/12911
//
// A ConstantBuffer<A> vs ConstantBuffer<B> argument mismatch must produce a
// LOCATED E30019 (with a caret and an "expected/got" detail), exactly like the
// plain-struct and StructuredBuffer cases below. Before the fix the ConstantBuffer
// case emitted a bare, un-located error: `ConstantBuffer<T>` is a ParameterGroupType,
// so _coerce (source/slang/slang-check-conversion.cpp) implicitly dereferences it
// through a synthesized DerefExpr that never had its ->loc set. The failed element
// coercion then reported E30019 at an invalid SourceLoc, which the rich-diagnostics
// renderer prints without the caret block or the "expected/got" detail.
//
// Note: because the buffer is dereferenced to its element type before the failing
// coercion, the ConstantBuffer message reports the element type ('B'), not the full
// 'ConstantBuffer<B>' — a minor content difference from the StructuredBuffer case.

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
