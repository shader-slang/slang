// Companion to `operator-name-on-non-function.slang`: the *legal* uses of an `operator <op>` name.
// A plain operator function, a generic operator function, and `operator()` must all compile with no
// diagnostic. Exhaustive mode (no `non-exhaustive`) fails if any of these regresses to emit E20020.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Vec2 { float x; float y; }
Vec2 operator+(Vec2 a, Vec2 b) { return { a.x + b.x, a.y + b.y }; }
T operator-<T>(T a, T b) { return a; }
int operator()(int a) { return a; }
