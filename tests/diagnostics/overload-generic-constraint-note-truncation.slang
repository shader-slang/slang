//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Exercises the shared display budget for issue #12965. The retained constraint-failed generics
// are printed by the same ten-candidate cap and "N more" tail as the concrete `bestCandidates`
// loop. With eleven non-conforming generic overloads and no concrete candidate, the first ten are
// listed (each: an E40011 signature plus its E40022 constraint note, ordered by declaration
// location) and the eleventh is folded into the "more overload candidates" tail (E40015). SIMPLE
// mode is exhaustive, so annotating exactly ten candidate/note pairs plus the tail asserts the cap
// holds and nothing is double-counted across the two loops.

interface I0 {}
interface I1 {}
interface I2 {}
interface I3 {}
interface I4 {}
interface I5 {}
interface I6 {}
interface I7 {}
interface I8 {}
interface I9 {}
interface I10 {}

struct W {}

T f<T : I0>(T x) { return x; }
T f<T : I1>(T x) { return x; }
T f<T : I2>(T x) { return x; }
T f<T : I3>(T x) { return x; }
T f<T : I4>(T x) { return x; }
T f<T : I5>(T x) { return x; }
T f<T : I6>(T x) { return x; }
T f<T : I7>(T x) { return x; }
T f<T : I8>(T x) { return x; }
T f<T : I9>(T x) { return x; }
T f<T : I10>(T x) { return x; }

void use(W w)
{
    f(w);
}

//CHECK: no overload for 'f' applicable to arguments of type (W)
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I0'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I1'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I2'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I3'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I4'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I5'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I6'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I7'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I8'
//CHECK: candidate: func f<T> -> T
//CHECK: does not conform to the required interface 'I9'
//CHECK: 1 more overload candidates
