# Prompt: docs/generated/tests/conformance/statements/

See [`_common.md`](_common.md).

## Target

Bundle at `docs/generated/tests/conformance/statements/`,
anchored to
[`docs/language-reference/statements.md`](../../../../language-reference/statements.md).

## Sub-areas and claim extraction strategy

The doc has the following normative sections:

### Expression Statement

A single behavioral claim: an expression followed by `;` is a statement.
The note "may warn on no-effect expression statement" is softly normative (may-warn → testable
only as doc-gap, not as a must-warn). All function-call and assignment forms in the example
are normative.

### Declaration Statement

`let`, `var`, and typed declaration forms are all valid in statement context. One normative
note carves out that currently only variable declarations are allowed (other decl kinds are future).
That note is a normative constraint: testing `let`, `var`, and typed variable declarations as
statements covers it.

### Block Statement

Grammar claim: zero or more statements in `{}` constitute a block. Scoping claim: declarations
in a block are visible to later statements in the same block but not to statements outside.
Two claims: (1) block-as-statement works, (2) block provides local scoping.

### Empty Statement

Single claim: a lone `;` is a valid statement equivalent to an empty block `{}`. Testable
via parse-and-compile (no reject) and via runtime behavior (appears in contexts like for-init
and for-increment).

### Conditional Statements — If Statement

Three claims:

- `if(cond) stmt` executes stmt when cond is true.
- `if(cond) stmt else stmt` executes the else-branch when cond is false.
- Condition must be coercible to bool (implicit claim — the form `if(expr)` is legal).

### Conditional Statements — Switch Statement

Many claims in this section:

- Switch expression in `()` followed by a block body.
- Body must consist of switch case clauses (not bare statements).
- Case label: `case <const-int>:`.
- Default label: `default:`.
- Fallthrough: a case clause without `break` falls through to the next.
- Error: `case`/`default` outside switch body is rejected.
- Error: bare statement inside switch body not in a clause is rejected.
- `break` exits the switch.
- Fallthrough note: FXC/WGSL targets restructure fall-through (Warning 41026). This is normative
  as a warning claim. Testable on supported targets.

### Loop Statements — For Statement

Six claims:

- `for(init; cond; incr) body` form.
- `init` is optional (may declare a variable).
- `cond` is optional; if absent, loop runs as if condition is always true.
- `cond` must be coercible to bool.
- `incr` is optional; executed for effects before testing condition on each iteration after first.
- Variable declared in `init` has scope limited to the for statement.

### Loop Statements — While Statement

Two claims:

- `while(cond) body` form.
- Equivalent to `for(; cond;) body`.

### Loop Statements — Do-While Statement

Two claims:

- `do body while(cond)` form.
- Equivalent to the `for(;;) { body; if(cond) continue; else break; }` form (body executes at
  least once regardless of condition).

### Control Transfer — Break Statement

One claim: `break` transfers control to after the end of the closest lexically enclosing switch
or loop statement.

### Control Transfer — Continue Statement

Two claims:

- `continue` transfers control to start of next loop iteration.
- In a `for` with a side-effect expression, the side-effect expression is evaluated when
  `continue` is used.

### Control Transfer — Return Statement

Three claims:

- `return;` is valid in a `void` function.
- `return expr;` is valid in a non-void function; expr supplies the return value.
- The returned value must coerce to the function's declared result type.

### Control Transfer — Discard Statement

Two claims:

- `discard` can only be used in a fragment shader context.
- Before the `discard`, side effects already executed remain visible.

### Compile-Time For Statement

Five claims:

- `$for(name in Range(initial, upper)) body` form.
- `initial` and `upper` must be compile-time constant integers.
- Semantics: body is instantiated once per integer in `[initial, upper)` with `name` bound
  to each successive value.
- Zero-trip: `Range(N, N)` produces an empty expansion.
- Range boundaries: `Range(0, 1)` instantiates body exactly once.

## What counts as a claim vs. non-normative

**Claims**: grammar productions; "must"/"is an error" sentences; documented equivalences;
behavioral statements describing observable runtime effects.

**Non-normative**: the "work in progress" caveat at the top; note remarks marked with `>` unless
they add behavioral content (the fallthrough/FXC note is normative). The `>Note: Currently only
variable declarations are allowed` is normative as a constraint.

## What NOT to test here

- `discard` on GPU fragment pipeline — requires `gpu-non-compute` runner; we can test the negative
  (discard in compute context is rejected) and the emission (discard statement emits correctly).
- Fallthrough warning (W41026) on FXC/WGSL targets — requires `gpu-fxc-dxbc` or a WGSL toolchain.
  Record as untested with the appropriate reason.
- Formal equivalence proofs between `while`/`do-while` and `for` — we test observable behavior
  (body executes right number of times) rather than internal IR equivalence.
- Module-scope `static` data members via `slangi` — known limitation (use `-target hlsl` instead).
