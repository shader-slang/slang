---
name: slang-investigate
description: Root cause investigation for Slang compiler bugs. Reproduces, classifies, traces code paths, analyzes design context, and searches for prior art. Produces investigation.md. Can be used standalone or from slang-fix-bug.
---

# Slang Investigate

**For**: Deep root cause investigation of Slang compiler bugs — from symptom to violated invariant.

**Core Principle**: Understand before fixing. The first plausible explanation is often wrong. Trace
the full code path before proposing any fix.

**Usage**: `/slang-investigate <bug-source>`

Where `<bug-source>` is a GitHub issue, test file, CI log, or symptom description.

---

## Step 1: Reproduce Locally

Build if needed (see `slang-build` skill), then confirm the bug:

```bash
# For ICE/crash
./build/<preset>/bin/slangc -target <target> test.slang

# For SPIRV validation
SLANG_RUN_SPIRV_VALIDATION=1 ./build/<preset>/bin/slangc -target spirv test.slang

# For wrong codegen (compute test)
./build/<preset>/bin/slang-test tests/path/to/test.slang
```

If the bug does not reproduce, document why and stop.

---

## Step 2: Classify the Error

| Error Type | What to Look For | Investigation Strategy |
|------------|------------------|----------------------|
| ICE 99999 | `SLANG_UNIMPLEMENTED_X`, exception, `SLANG_UNEXPECTED` | Find the exact crash site via error message text search |
| Assertion | `SLANG_ASSERT`, `assert failure:` | Search for the assertion text in source |
| Segfault | No error, process killed | Use `-dump-ir` to find the last pass before crash |
| Wrong codegen | Output differs from expected | Compare IR at different pass stages |
| Missing diagnostic | No error emitted for invalid code | Check semantic checker for the relevant validation |
| SPIRV validation | `spirv-val` error message | Compare SPIRV output with `-emit-spirv-via-glsl` reference |

---

## Step 3: Locate the Crash Site

```bash
# Search for the error message text
rg "error message text" source/slang/ source/compiler-core/

# For SLANG_UNIMPLEMENTED_X crashes
rg "SLANG_UNIMPLEMENTED_X" source/slang/ --context 5

# For assertion failures
rg "assert.*message text" source/slang/
```

---

## Step 4: Trace the Code Path

Use IR dumps to understand which pass is involved:

```bash
# Dump IR at every pass (split into per-pass files)
./build/<preset>/bin/slangc -dump-ir -target <target> -o /dev/null test.slang 2>&1 | \
  python extras/split-ir-dump.py

# Dump IR around a suspected pass
./build/<preset>/bin/slangc \
  -dump-ir-before <pass-name> \
  -dump-ir-after <pass-name> \
  -target <target> -o /dev/null test.slang > pass-dump.txt 2>&1
```

Always combine `-dump-ir` with `-target` and `-o <file>` to avoid mixing output.

---

## Step 5: Understand the Design Context

Before proposing any fix, answer these questions:

### Which compiler stage owns this behavior?

| Stage | Files | Role |
|-------|-------|------|
| Frontend | `slang-check-*.cpp`, `slang-parser.cpp` | Parsing, type checking, name resolution |
| IR generation | `slang-lower-to-ir.cpp` | AST to IR conversion |
| IR passes | `slang-ir-*.cpp` | Transformation, optimization, legalization |
| Code emission | `slang-emit-*.cpp` | Target-specific output |

### What invariant is violated?

- A type that should have been rejected reached a pass that cannot handle it
- A transformation produced invalid IR
- An instruction was not lowered before reaching emission
- A layout calculation produced incorrect results

### Is this the right layer to fix?

**Compiler philosophy**: The Slang compiler is a pipeline of IR passes. Each pass
has a clear purpose. Keep emission simple — do heavy lifting in IR passes.

**Three levels of fix quality** (prefer higher):

1. **Best: Add or extend an IR pass** — If the bug is caused by IR that wasn't
   transformed into the right shape, the fix belongs in an IR pass, not at the
   crash site. If no existing pass has the right theme, add a new one.

2. **Acceptable: Annotate early, act later** — When a fix needs information from
   an early compiler stage but must act in a later pass, annotate the IR with
   decorations in the early pass and read them in the later pass. Do not
   pattern-match arbitrary IR shapes in the later pass — that approach is fragile.

3. **Last resort: Spot-fix at the symptom** — Only patch the crash site or emitter
   when the problem is genuinely local (e.g., a missing case in a switch that follows
   an established pattern). If you find yourself adding complex conditional logic to
   an emitter or an existing pass whose theme doesn't match, step back — the fix
   probably belongs upstream.

**Don't shoehorn fixes into existing passes**: If the purpose of an existing pass
doesn't align with what your fix needs to do, add a new pass rather than complicating
an existing one with unrelated logic.

### What related code paths exist?

- Does a similar construct work on other targets?
- Does a similar type/pattern work in the same context?
- Is there existing validation for a related case?

### Could this be intentional?

Some behaviors that look like bugs may be deliberate (HLSL compatibility,
performance trade-offs, known limitations documented in issues).

---

## Step 6: Search for Related Issues and Prior Art

```bash
# Search for related issues
gh issue list --repo shader-slang/slang --search "error message keywords" --limit 10

# Search for related fixes in git history
git log --all --oneline --grep="error message keywords" -- source/slang/

# Check for TODOs near the crash site
rg "TODO|FIXME|HACK|WORKAROUND" source/slang/<file>.cpp --context 3
```

### External References

For target-specific bugs, consult the relevant specification:
- **SPIRV**: Khronos SPIRV specification
- **HLSL**: Microsoft HLSL spec and DXC reference
- **GLSL**: Khronos GLSL specification
- **Metal**: Apple Metal Shading Language specification
- **WGSL**: W3C WGSL specification

Use `mcp__deepwiki__ask_question` with repoName "shader-slang/slang" for implementation-level questions.

---

## Output

Write `tmp/<issue-repository>-<bug-id>/investigation.md` (or `tmp/<bug-id>/` for local-only bugs):

```markdown
# Root Cause Investigation

## Crash Site
- File: `source/slang/<file>.cpp`
- Function: `functionName()`
- Line: N
- Error: [exact error message or assertion]

## Code Path
[How the input reaches the crash site — which passes transform it, what decisions lead here]

## Violated Invariant
[What assumption is broken and why]

## Design Context
- Stage: [frontend / IR pass / emission]
- Right layer: [yes/no — if no, which layer should handle it]
- Related patterns: [similar constructs that work, and why they work]

## Related Issues
- #NNNN: [relationship]
- Prior fix in commit abc123: [what it did]

## Potential Fix Locations
1. [Location 1]: [brief description of what a fix here would look like]
2. [Location 2]: [brief description]
3. [Location 3]: [brief description]

## Recommended Approach
[Which fix location and why, considering the "prefer IR pass" principle]
```

### Sharing Results

If a GitHub issue exists for this bug, offer to post a summary of the investigation
as a comment on the issue. Ask the user before posting. Prefix with `[Agent]`
(see `slang-create-issue` GitHub Comment Rules).
