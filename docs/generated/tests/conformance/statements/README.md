---
generated: true
model: claude-sonnet-4-6
generated_at: 2026-06-02T07:58:55+00:00
source_commit: a01e79278cf0be9772deb8b2be52ee87687697a3
watched_paths_digest: 991fe196c741a3da23f65fd5eadd9c5caec361532b54395181ce8c0af1596b59
source_doc: docs/language-reference/statements.md
source_doc_digest: 00f8284df252413eabb2432a837bb4f83390efcf07dbd17ceadebd85178d551c
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for conformance/statements

## Intent

Tests verify statement-form claims from the **language reference** at
[`docs/language-reference/statements.md`](../../../../language-reference/statements.md).
Coverage strategy: one functional test (INTERPRET or COMPARE_COMPUTE -cpu) per claim expanded along
documented dimensions (zero-trip, one-trip, N-trip, nested, fallthrough, break/continue interactions,
coercion, early-exit), plus emission tests for compile-time-for unrolling and discard lowering, and
negative (DIAGNOSTIC_TEST) tests for all "is rejected" claims. Switch tests use COMPARE_COMPUTE -cpu
instead of INTERPRET because slangi's VM bytecode emitter does not support switch statements inside
called functions (finding: `slangi-switch-in-function-vm-bytecode-crash.yaml`).

## Claims

### Expression Statement

- **C1** An expression statement consists of an expression followed by a semicolon.

### Declaration Statement

- **C2** `let`, `var`, and typed variable declarations are valid in statement context.
- **C3** Currently only variable declarations are allowed in statement contexts (other declaration kinds are future work).

### Block Statement

- **C4** A block statement consists of zero or more statements wrapped in curly braces `{}`.
- **C5** A block statement provides local scoping: declarations in a block are visible to later statements in the same block but not to statements or expressions outside the block.

### Empty Statement

- **C6** A single semicolon (`;`) may be used as an empty statement equivalent to an empty block statement `{}`.

### Conditional Statements — If Statement

- **C7** An if statement `if(cond) stmt` executes the then-body when the condition is true.
- **C8** An if statement with an else clause `if(cond) stmt else stmt` executes the else-body when the condition is false.

### Conditional Statements — Switch Statement

- **C9** A switch statement consists of `switch(expr)` followed by a block body.
- **C10** The body of a switch statement must consist of switch case clauses.
- **C11** A case label has the form `case <const-int-expr>:`.
- **C12** A case expression must evaluate to a compile-time constant integer; a non-constant expression is rejected (E39999).
- **C13** A default label has the form `default:`.
- **C14** Multiple case labels may be stacked before a single body to form one switch case clause.
- **C15** A case or default label outside a switch body is a compile error (E39999).
- **C16** A switch case clause without a `break` falls through to the next case clause.
- **C17** A `break` statement in a switch body exits the switch.
- **C18** The default label fires when no case label matches the switch expression.
- **C19** Fallthrough on targets that do not support it natively (FXC/WGSL) is restructured by the compiler, emitting Warning 41026.

### Loop Statements — For Statement

- **C20** A `for(init; cond; incr) body` statement iterates the body while cond is true.
- **C21** The init part of a for-loop is optional; if present, it may declare a variable.
- **C22** A variable declared in the for-loop init has scope limited to the for statement.
- **C23** The cond part is optional; if absent, the condition is treated as always true.
- **C24** The cond part, if present, must be coercible to `bool`.
- **C25** The incr part is optional; if present, it is executed before testing the condition on each iteration after the first.
- **C26** A for-loop with a false initial condition executes the body zero times.

### Loop Statements — While Statement

- **C27** A `while(cond) body` statement executes the body repeatedly while the condition is true.
- **C28** A while-loop with an initially-false condition executes the body zero times.

### Loop Statements — Do-While Statement

- **C29** A `do body while(cond)` statement executes the body at least once before testing the condition.
- **C30** A do-while loop with a false condition still executes the body exactly once.

### Control Transfer — Break Statement

- **C31** A `break` statement transfers control to after the end of the closest lexically enclosing switch or loop statement.
- **C32** A `break` inside a nested loop exits only the innermost loop, not the outer loop.

### Control Transfer — Continue Statement

- **C33** A `continue` statement transfers control to the start of the next iteration of the enclosing loop statement.
- **C34** In a for-loop with a side-effect expression, the side-effect expression (increment) is evaluated when `continue` is used.

### Control Transfer — Return Statement

- **C35** `return;` (with no value) is valid in a `void` function and immediately exits it.
- **C36** `return expr;` in a non-void function supplies the return value to the caller.
- **C37** The value returned by `return expr;` must coerce to the function's declared result type.

### Control Transfer — Discard Statement

- **C38** `discard` can only be used in the context of a fragment shader; using it in a compute context is rejected (E36107).
- **C39** `discard` in a fragment shader emits a discard/kill instruction in the target output.

### Compile-Time For Statement

- **C40** A `$for(name in Range(initial, upper)) body` statement instantiates the body once for each integer value in `[initial, upper)`.
- **C41** The initial-value and upper-bound expressions in `Range` must be compile-time constant integers.
- **C42** `Range(N, N)` (equal bounds) produces zero instantiations.
- **C43** `Range(0, 1)` produces exactly one instantiation with the name bound to 0.
- **C44** The compile-time for is fully unrolled in emitted target code.

## Functional coverage

| Claim                                                                                                                                  | Intent               | Anchor                                                                                                 | Tests                                                                                              |
| -------------------------------------------------------------------------------------------------------------------------------------- | -------------------- | ------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------- |
| C1 — An expression statement (expression followed by `;`) executes the expression.                                                     | functional           | [#expression-statement](../../../../language-reference/statements.md#expression-statement)             | [`expr-stmt-functional.slang`](expr-stmt-functional.slang)                                         |
| C2 — `let`, `var`, and typed variable declarations are valid in statement context.                                                     | functional           | [#declaration-statement](../../../../language-reference/statements.md#declaration-statement)           | [`decl-stmt-let-var-functional.slang`](decl-stmt-let-var-functional.slang)                         |
| C4/C5 — Block statement groups statements and scopes declarations locally; inner declaration shadows outer and is not visible outside. | functional           | [#block-statement](../../../../language-reference/statements.md#block-statement)                       | [`block-stmt-scoping-functional.slang`](block-stmt-scoping-functional.slang)                       |
| C6 — A lone `;` is a valid empty statement; accepted in a for-loop body position.                                                      | functional           | [#empty-statement](../../../../language-reference/statements.md#empty-statement)                       | [`empty-stmt-functional.slang`](empty-stmt-functional.slang)                                       |
| C7 — `if(cond) stmt` executes the then-body when cond is true, skips it when false.                                                    | functional           | [#if-statement](../../../../language-reference/statements.md#if-statement)                             | [`if-true-branch-functional.slang`](if-true-branch-functional.slang)                               |
| C8 — `if(cond) stmt else stmt` executes the else-body when cond is false.                                                              | functional           | [#if-statement](../../../../language-reference/statements.md#if-statement)                             | [`if-else-branch-functional.slang`](if-else-branch-functional.slang)                               |
| C9/C10/C11/C13/C17/C18 — switch routes to the matching case label; default fires for no-match; break exits the switch.                 | functional           | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-basic-functional.slang`](switch-basic-functional.slang)                                   |
| C12 — A case expression that is not a compile-time constant integer is rejected (E39999).                                              | negative             | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-case-non-const-rejected.slang`](switch-case-non-const-rejected.slang)                     |
| C14 — Multiple case labels stacked before a single body; both reach the same code.                                                     | functional           | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-multi-case-label-functional.slang`](switch-multi-case-label-functional.slang)             |
| C15 — `case` label outside a switch body is rejected (E39999).                                                                         | negative             | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-case-outside-switch-rejected.slang`](switch-case-outside-switch-rejected.slang)           |
| C15 — `default` label outside a switch body is rejected (E39999).                                                                      | negative             | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-default-outside-switch-rejected.slang`](switch-default-outside-switch-rejected.slang)     |
| C16 — A case without break falls through to the next case clause.                                                                      | functional           | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-fallthrough-functional.slang`](switch-fallthrough-functional.slang)                       |
| C18 — A switch with only a default label fires the default for every value.                                                            | functional, boundary | [#switch-statement](../../../../language-reference/statements.md#switch-statement)                     | [`switch-default-only-functional.slang`](switch-default-only-functional.slang)                     |
| C17 — break in a switch body exits the switch; unreachable code after break is skipped.                                                | functional           | [#break-statement](../../../../language-reference/statements.md#break-statement)                       | [`break-exits-switch-functional.slang`](break-exits-switch-functional.slang)                       |
| C20/C25 — `for(init;cond;incr) body` iterates the right number of times; incr fires each iteration.                                    | functional           | [#for-statement](../../../../language-reference/statements.md#for-statement)                           | [`for-basic-functional.slang`](for-basic-functional.slang)                                         |
| C26 — A for-loop with a false initial condition executes the body zero times.                                                          | boundary             | [#for-statement](../../../../language-reference/statements.md#for-statement)                           | [`for-zero-trip-functional.slang`](for-zero-trip-functional.slang)                                 |
| C20 — A for-loop with range [0,1) executes the body exactly once.                                                                      | boundary             | [#for-statement](../../../../language-reference/statements.md#for-statement)                           | [`for-one-trip-functional.slang`](for-one-trip-functional.slang)                                   |
| C21/C23 — All three for-loop parts may be omitted (`for(;;)`); absent cond loops until break.                                          | functional           | [#for-statement](../../../../language-reference/statements.md#for-statement)                           | [`for-optional-parts-functional.slang`](for-optional-parts-functional.slang)                       |
| C25/C34 — The for-loop increment executes after continue (not skipped).                                                                | functional           | [#for-statement](../../../../language-reference/statements.md#for-statement)                           | [`for-incr-executes-on-continue-functional.slang`](for-incr-executes-on-continue-functional.slang) |
| C22 — Variable declared in for-loop init is scoped to the for statement; can be redeclared in successive for loops.                    | functional           | [#for-statement](../../../../language-reference/statements.md#for-statement)                           | [`for-init-scope-functional.slang`](for-init-scope-functional.slang)                               |
| C27 — `while(cond) body` iterates the right number of times.                                                                           | functional           | [#while-statement](../../../../language-reference/statements.md#while-statement)                       | [`while-basic-functional.slang`](while-basic-functional.slang)                                     |
| C28 — `while(false) body` executes the body zero times.                                                                                | boundary             | [#while-statement](../../../../language-reference/statements.md#while-statement)                       | [`while-zero-trip-functional.slang`](while-zero-trip-functional.slang)                             |
| C29/C30 — `do body while(cond)` executes body at least once; with false condition, body runs exactly once.                             | functional, boundary | [#do-while-statement](../../../../language-reference/statements.md#do-while-statement)                 | [`do-while-basic-functional.slang`](do-while-basic-functional.slang)                               |
| C31 — `break` transfers control out of the closest enclosing loop.                                                                     | functional           | [#break-statement](../../../../language-reference/statements.md#break-statement)                       | [`break-exits-loop-functional.slang`](break-exits-loop-functional.slang)                           |
| C32 — `break` inside a nested loop exits only the innermost loop.                                                                      | functional           | [#break-statement](../../../../language-reference/statements.md#break-statement)                       | [`break-nested-loop-functional.slang`](break-nested-loop-functional.slang)                         |
| C33 — `continue` skips the rest of the loop body and starts the next iteration.                                                        | functional           | [#continue-statement](../../../../language-reference/statements.md#continue-statement)                 | [`continue-skips-rest-functional.slang`](continue-skips-rest-functional.slang)                     |
| C33 — `continue` in a while loop re-evaluates the condition, not just the body start.                                                  | functional           | [#continue-statement](../../../../language-reference/statements.md#continue-statement)                 | [`continue-while-functional.slang`](continue-while-functional.slang)                               |
| C35 — `return;` in a void function exits it immediately; code after return is not reached.                                             | functional           | [#return-statement](../../../../language-reference/statements.md#return-statement)                     | [`return-void-functional.slang`](return-void-functional.slang)                                     |
| C36 — `return expr;` supplies the return value to the caller.                                                                          | functional           | [#return-statement](../../../../language-reference/statements.md#return-statement)                     | [`return-value-functional.slang`](return-value-functional.slang)                                   |
| C37 — The returned value is coerced to the function's declared result type.                                                            | functional           | [#return-statement](../../../../language-reference/statements.md#return-statement)                     | [`return-coercion-functional.slang`](return-coercion-functional.slang)                             |
| C38 — `discard` in a compute shader is rejected (E36107).                                                                              | negative             | [#discard-statement](../../../../language-reference/statements.md#discard-statement)                   | [`discard-compute-rejected.slang`](discard-compute-rejected.slang)                                 |
| C39 — `discard` in a fragment shader emits a discard/kill instruction in HLSL and SPIR-V.                                              | functional           | [#discard-statement](../../../../language-reference/statements.md#discard-statement)                   | [`discard-emission.slang`](discard-emission.slang)                                                 |
| C40/C41 — `$for(i in Range(0, 4))` instantiates the body with i = 0, 1, 2, 3; sum confirms all ran.                                    | functional           | [#compile-time-for-statement](../../../../language-reference/statements.md#compile-time-for-statement) | [`compile-time-for-basic-functional.slang`](compile-time-for-basic-functional.slang)               |
| C42 — `Range(N, N)` (equal bounds) produces zero instantiations; body never executes.                                                  | boundary             | [#compile-time-for-statement](../../../../language-reference/statements.md#compile-time-for-statement) | [`compile-time-for-zero-range-functional.slang`](compile-time-for-zero-range-functional.slang)     |
| C43 — `Range(0, 1)` produces exactly one instantiation.                                                                                | boundary             | [#compile-time-for-statement](../../../../language-reference/statements.md#compile-time-for-statement) | [`compile-time-for-one-trip-functional.slang`](compile-time-for-one-trip-functional.slang)         |
| C44 — `$for` is fully unrolled in emitted HLSL; no loop structure remains.                                                             | functional           | [#compile-time-for-statement](../../../../language-reference/statements.md#compile-time-for-statement) | [`compile-time-for-emission.slang`](compile-time-for-emission.slang)                               |

## Untested claims

| Claim                                                                                                            | Reason         | Anchor                                                                                       | Why untested                                                                                                                                                                                                                         |
| ---------------------------------------------------------------------------------------------------------------- | -------------- | -------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| C3 — Currently only variable declarations are allowed in statement contexts; other declaration kinds are future. | (unclassified) | [#declaration-statement](../../../../language-reference/statements.md#declaration-statement) | The note documents a planned future feature, not a currently-observable behavior. There is no negative test that "other declaration kinds fail" because the doc doesn't specify which ones will be added or what error they produce. |
| C19 — Fallthrough on FXC/WGSL targets is restructured by the compiler with Warning W41026.                       | gpu-fxc-dxbc   | [#switch-statement](../../../../language-reference/statements.md#switch-statement)           | Requires FXC binary for DXBC output or WGSL toolchain; agent runtime has neither. CI nightly validates this path.                                                                                                                    |
| C24 — For-loop condition must be coercible to `bool`.                                                            | out-of-bundle  | [#for-statement](../../../../language-reference/statements.md#for-statement)                 | Coercion-to-bool semantics are covered by expression/type coercion bundles. The basic functional for-loop tests exercise this implicitly.                                                                                            |

## Doc gaps observed

| Anchor                                                                                 | Kind            | Gap                                                                                                                                     | Suggested addition                                                                    |
| -------------------------------------------------------------------------------------- | --------------- | --------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------- |
| [#continue-statement](../../../../language-reference/statements.md#continue-statement) | missing-example | The Continue Statement section shows `break;` (not `continue;`) as the code example, which appears to be a copy-paste error in the doc. | Replace the `break;` code example in the Continue Statement section with `continue;`. |
