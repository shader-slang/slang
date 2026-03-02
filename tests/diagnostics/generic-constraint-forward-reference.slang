//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):
// Test forward references in generic constraints

interface IFloat {}
interface IFoo<T : IFloat> {}
interface IBar<T, U> {}

// Test case 1: Simple forward reference in interface constraint
void test1<Foo : IFoo<T>, T : IFloat>() {}
//CHECK:         ^^^^ forward reference in generic constraint
//CHECK:         ^^^^ generic constraint for parameter 'Foo' references type parameter 'T' before it is declared

// Test case 2: Multiple forward references in a single constraint
void test2<A : IBar<B, C>, B : IFloat, C : IFloat>() {}
//CHECK:       ^^^^ forward reference in generic constraint
//CHECK:       ^^^^ generic constraint for parameter 'A' references type parameter 'B' before it is declared
//CHECK:       ^^^^ forward reference in generic constraint
//CHECK:       ^^^^ generic constraint for parameter 'A' references type parameter 'C' before it is declared

// Valid case that should NOT produce our error:
// Test case 3: Correct order - type parameter declared before use
void test3<T : IFloat, Foo : IFoo<T>>() {}
