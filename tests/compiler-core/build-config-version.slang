// Regression test for issue #12645.
//
// slang-test wraps compiler output in a banner before FileCheck sees it (`result code = N` /
// `standard error = { ... }`), and `-v` writes to stderr, so the two `-v` lines appear inside the
// `standard error` block. Anchoring the first CHECK on `standard error = {` makes the build-tag
// line the next line and the `build-config:` line the one after it. Matching {{on|off}} rather than
// a fixed value keeps the test config-agnostic, and rejecting `unknown` asserts the CMake
// compile-definition wiring is present.

//TEST:SIMPLE(filecheck=CHECK): -v

//CHECK: standard error = {
//CHECK-NEXT: {{.+}}
//CHECK-NEXT: build-config: release-lto={{on|off}} standard-module-develop-build={{on|off}}
