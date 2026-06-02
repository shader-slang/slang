//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv

// Diamond include: D includes B and C, both include A (which defines ERRORING_MACRO).
// Both B and C expand ERRORING_MACRO, which should each produce an "expanded from" note
// pointing to their respective call sites (not each other's).
#include "diamond-b.h"
#include "diamond-c.h"

// Each expansion should report the correct call site.
// B's invocation is in diamond-b.h; C's is in diamond-c.h.
// CHECK: diamond-b.h
// CHECK: expanded from macro 'ERRORING_MACRO'
// CHECK: diamond-c.h
// CHECK: expanded from macro 'ERRORING_MACRO'
