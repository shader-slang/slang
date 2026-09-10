//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=The three shape rules around `try` — the operand must be a call, the callee must throw, and a throwing callee must be wrapped in `try` — each get their own diagnostic.
//META: intent=characterization
//META: covers=source/slang/slang-check-expr.cpp
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes the well-formedness rules that visitTryExpr and invoke checking
// apply to error handling, using three sibling functions so each rule fires on
// its own. `tryOnNonCall` violates the rule that the operand of `try` is an
// invocation; `tryOnNonThrowing` satisfies that but names a callee with no error
// type; `missingTry` is the mirror image — a throwing callee reached without any
// `try`. Together they pin which of the three distinct diagnostics belongs to
// which mistake, since all three are easy to confuse from the source text alone.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry test -stage fragment

enum MyError : int { Failed }

int throwing() throws MyError { return 1; }

int nonThrowing() { return 2; }

int tryOnNonCall() throws MyError { return try (1 + 2); }
//CHECK:                                   ^^^ E30090
//CHECK:                                   ^^^ expression in a 'try' clause must be a call to a function or operator overload.

int tryOnNonThrowing() throws MyError { return try nonThrowing(); }
//CHECK:                                       ^^^ E30091
//CHECK:                                       ^^^ 'nonThrowing' called from a 'try' clause does not throw an error, make sure the callee is marked as 'throws'

int missingTry() throws MyError { return throwing(); }
//CHECK:                                         ^ E30094
//CHECK:                                         ^ the callee may throw an error, and therefore must be called within a 'try' clause

int test() { return 0; }
