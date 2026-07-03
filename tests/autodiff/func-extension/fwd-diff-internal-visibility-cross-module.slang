//DISABLE_TEST:SIMPLE(diag=CHECK): -experimental-feature

// TODO: re-enable when func-extension visibility is implemented separately.

// Test: an internal func-extension is not accessible from another module,
// while a public func-extension from the same imported module remains usable.

import fwd_diff_visibility_cross_module_helper;

float ok()
{
    return fwd_diff(cubePublic)(diffPair(2.0, 1.0)).d;
}

float bad()
{
    return fwd_diff(cubeInternal)(diffPair(2.0, 1.0)).d;
/*CHECK:
           ^^^^^^^^ declaration not accessible
           ^^^^^^^^ 'fwd_diff' is not accessible from the current context.
*/
}
