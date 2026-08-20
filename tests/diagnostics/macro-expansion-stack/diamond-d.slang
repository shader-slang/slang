//TEST:SIMPLE(filecheck=CHECK):-target spirv

// Diamond include: D includes B and C, both include A (which defines ERRORING_MACRO).
// Both B and C expand ERRORING_MACRO, which should each produce an "expanded from" note
// pointing to their respective call sites.
#include "diamond-b.h"
#include "diamond-c.h"

// Verify two independent "expanded from" chains using the human-readable renderer output.
// The patterns below confirm:
//   1. The error message text appears for each invocation.
//   2. Each "expanded from" note names the correct include file (b.h before c.h),
//      proving the two invocations are attributed independently.
//
//CHECK: undefined identifier 'UndefinedType'.
//CHECK: expanded from macro 'ERRORING_MACRO'
//CHECK: diamond-b.h
//CHECK: undefined identifier 'UndefinedType'.
//CHECK: expanded from macro 'ERRORING_MACRO'
//CHECK: diamond-c.h
