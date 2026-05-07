//TEST:SIMPLE(filecheck=CHECK):-target spirv

// Negative test for gh-8550: a `static const` global initialized from another
// `static const` global is a valid compile-time constant and must NOT produce E31226.

static const int base = 25;
static const int derived = base * 2;

// CHECK-NOT: static const global initializer must be a compile-time constant

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain() {}
