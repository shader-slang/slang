//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

interface ISelfOp
{
    This clone();
    bool matches(This other);
}

struct Impl : ISelfOp
{
    float v;
    This clone() { return Impl(v); }
    bool matches(Impl other) { return v == other.v; }
}

void test()
{
    ISelfOp obj = Impl(5.0);
    ISelfOp copy = obj.clone();
    obj.matches(copy);
    //CHECK: type mismatch in expression
    //CHECK: expected an expression of type 'test..This', got 'test..This'
    //CHECK: the concrete 'This' type identity is lost after type erasure into interface type 'ISelfOp'; consider using a generic function with a type constraint instead
}

// Negative test: a regular type mismatch (not ExtractExistentialType on both sides)
// should NOT produce the E30021 erasure note. The exhaustive checker ensures no
// extra diagnostics appear beyond those annotated here.
void testNoErasureNote()
{
    ISelfOp obj = Impl(5.0);
    int x = obj.clone();
    //CHECK: type mismatch
    //CHECK: expected an expression of type 'int', got 'testNoErasureNote...This'
}

// Negative test: both sides are ExtractExistentialType but from different interfaces.
// The equals() guard should prevent E30021 from firing here.
interface IOther
{
    This make();
}

struct Impl2 : ISelfOp, IOther
{
    float v;
    This clone() { return Impl2(v); }
    bool matches(Impl2 other) { return v == other.v; }
    This make() { return Impl2(v); }
}

void testDifferentInterfaces()
{
    ISelfOp a = Impl2(1.0);
    IOther b = Impl2(2.0);
    a.matches(b.make());
    //CHECK: type mismatch
    //CHECK: expected an expression of type 'testDifferentInterfaces..This', got 'testDifferentInterfaces..This'
}
