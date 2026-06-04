//TEST:SIMPLE(filecheck=CHECK):

module test_pragma_warning;

__include "pragma-warning-multifile-impl1.slang";
__include "pragma-warning-multifile-impl2.slang";

// impl1 wraps its `#pragma warning(disable: 30856)` in push/pop and pops it before impl1 ends,
// so the disable is balanced/scoped to impl1 and does not leak into impl2; impl2 has no disable,
// so its extension correctly emits E30856. (Whether a push/pop-scoped disable SHOULD persist
// across `__include` files is a separate semantic question — see shader-slang/slang#11473.)
// CHECK: warning[E30856]
// CHECK: pragma-warning-multifile-impl2.slang
