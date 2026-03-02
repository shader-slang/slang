//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
// Using non-exhaustive because of cascading import failures

// A file that recursively imports itself
// (including transitive cases) should be diagnosed.

import recursive_import_extra;
//CHECK: recursive module import
