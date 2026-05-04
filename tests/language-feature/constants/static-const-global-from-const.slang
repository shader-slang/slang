//TEST:SIMPLE(filecheck=CHECK):-target spirv

// Negative test for gh-8550: a `static const` global initialized from another
// `static const` global is a valid compile-time constant and must NOT produce E31226.
// A local `static const` initialized from a cbuffer member is also valid.

cbuffer C { int x; }

static const int base = 25;
static const int derived = base * 2;

// CHECK-NOT: static const global initializer must be a compile-time constant

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // A local static const initialized from a non-const runtime value is valid
    // (it is not a module-level global), so E31226 must NOT fire.
    static const int local = x;
    (void)local;
}
