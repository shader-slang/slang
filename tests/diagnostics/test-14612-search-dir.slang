// DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK,exit-code=2):-I tests/diagnostics/test-14612/../test-14612

// Companion to test-14612.slang: the module is not resolvable relative to the importing file, so
// the import only succeeds through a search directory whose spelling contains `..`. That route
// combines the path through the SLANG_PATH_TYPE_DIRECTORY branch of the same calcCombinedPath, so
// this pins the second combine branch of the fix: the diagnostic must report the module under its
// collapsed spelling, without the `..` from the search directory.

import "mod14612.slang";
// CHECK: error[E30015]
// CHECK-NOT: ../
// CHECK: tests/diagnostics/test-14612/mod14612.slang:5
