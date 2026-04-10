//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Passing an empty type pack to a generic with a `where nonempty(T)` constraint
// should produce error 30414.

struct S<each T> where nonempty(T) { int x; }

S<> badVar;
/*CHECK:
^ empty pack does not satisfy `nonempty(...)` constraint
^ pack argument here is empty
*/
