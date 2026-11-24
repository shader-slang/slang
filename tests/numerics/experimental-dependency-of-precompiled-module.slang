//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// `slang.tensor` is not an experimental module, but its precompiled module imports the
// experimental `slang.numerics`. Without `-experimental-feature` that dependency is refused, so
// `slang.tensor` cannot be loaded. The compiler reports the refused dependency and the failed
// import, instead of crashing on the null declarations that the rest of `slang.tensor` would
// otherwise deserialize in place of the ones it imports from `slang.numerics`.
//
// The import of `slang.runtime.core.simple_time` follows the refused one, and is not a dependency
// of `slang.tensor`. It loads without a diagnostic, because only errors reported while loading a
// precompiled module make that module's load fail.

import slang.tensor;
/*CHECK:
       ^^^^^ cannot open file 'slang/tensor.slang'
*/
//CHECK: 'slang/numerics' is an experimental module
//CHECK: import file not found
//CHECK: failed to find imported file 'slang/numerics'

import slang.runtime.core.simple_time;
