//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):

// Regression test for shader-slang/slang#14612. A module imported through a path with `..`
// segments used to be recorded and reported under the unsimplified spelling, because
// `_calcCombinedPath` combined paths verbatim and, unlike `#include` handling, `import`
// never simplified the found path afterwards. On Windows that longest spelling can push a
// path past the legacy ~260-character limit and fail, so the combined path is now simplified
// at construction. The error below must be reported under the collapsed spelling, and that
// diagnostic must not show the `..` spelling. (The later `import failed` diagnostic echoes
// the import string as written in the source, so it is outside the CHECK-NOT range.)

import "test-14612/../test-14612/mod14612.slang";
//CHECK: error[E30015]
//CHECK-NOT: ../
//CHECK: tests/diagnostics/test-14612/mod14612.slang:5
