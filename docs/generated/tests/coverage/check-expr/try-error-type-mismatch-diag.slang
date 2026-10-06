//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=A `try` call whose callee throws a different error type than the caller declares is rejected as incompatible error types (E30095).
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes visitTryExpr's error-type compatibility check, which is the last
// of three: the enclosing function must exist, must not be non-throwing, and its
// declared error type must equal the callee's. Here both functions throw, so the
// first two checks pass and only the equality check fails, which is what isolates
// this diagnostic from the "uncaught try in non-throwing function" one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

enum CalleeError : int { CalleeFailed }
enum CallerError : int { CallerFailed }

int mayFail() throws CalleeError { return 1; }

int caller() throws CallerError { return try mayFail(); }
//CHECK:                                 ^^^ E30095
//CHECK:                                 ^^^ the error type `CalleeError` of callee `mayFail` is not compatible with the caller's error type `CallerError`.

int test() { return 0; }
