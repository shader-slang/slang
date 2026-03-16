---
name: slang-test-coverage
description: Comprehensive test coverage analysis and gap-filling for Slang language features. Use when the user wants to increase test coverage, maximize testing for a feature, perform coverage analysis, identify test gaps, or systematically test all aspects of a language feature.
---

# Feature-Oriented Test Coverage

**For**: Systematic test coverage improvement for specific Slang language features

**Core Principle**: Write tests that verify behavior and document intent, not tests that just increase coverage numbers.

## Workflow Overview: 7 Phases

1. **Understanding** → Research feature from all available sources
2. **Analysis** → Inventory existing tests, check coverage reports, verify code reachability
3. **Gap Identification** → Compare reference vs tests, prioritize gaps, score test value
4. **Test Design** → Transform coverage targets into functional requirements
5. **Implementation** → Create tests, run and validate, ensure cross-platform compatibility
6. **Bug Investigation** → Investigate failures, document bugs, fix if straightforward
7. **Documentation** → Update coverage analysis, create summary

---

## Phase 1: Understanding

### Information Sources (check in order)
1. **Formal Spec**: `external/spec/specification/` and `external/spec/proposals/`
   - Clone `https://github.com/shader-slang/spec.git` under `external/` if not present
2. **User Guide**: `docs/user-guide/` — search all chapters for feature mentions
3. **DeepWiki**: Use `mcp__deepwiki__ask_question` with repoName "shader-slang/slang"
4. **Diagnostic definitions**: `source/slang/slang-diagnostics-defs.h` for error codes
5. **Compiler source**: `source/slang/` for implementation details

**Extract from reference**: Core concepts, syntax, restrictions, error conditions

### Exhaustive Diagnostic Enumeration (mandatory)

Do NOT rely on keyword search alone to find relevant diagnostics. Extract
ALL diagnostic codes related to the feature from the source:

```bash
# Search diagnostic definition files for feature-related terms
# Use multiple keywords: the feature name, related concepts, synonyms
rg -i "generic|constraint|speciali|conform|type.param|pack|where.clause" \
  source/slang/slang-diagnostic*.h --context 2

# Extract the numeric codes and message text
# Organize into a complete table in research.md
```

For each diagnostic code found:
1. Record the code, name, and message text
2. Search `tests/` to determine if it is already tested
3. Mark as COVERED (test exists) or UNCOVERED (no test)

Include the complete diagnostic table in research.md. This is the
ground truth for error-path coverage -- keyword search of docs will
miss codes that use different terminology.

**Create** `tmp/feature-name/README.md` with feature overview, concepts, behaviors, restrictions.

### Line/Branch Coverage Baseline (optional)

The nightly coverage report at
`https://shader-slang.org/slang-coverage-reports/reports/latest/linux/index.html`
shows line and branch coverage for each compiler source file. Use it as
a signal, not a driver:

1. Identify key source files for the feature (e.g., for generics:
   `slang-check-constraint.cpp`, `slang-ir-specialize.cpp`)
2. Note their current line/branch coverage as a baseline
3. After writing tests, check if coverage improved
4. If a feature-related function has very low branch coverage, inspect
   the uncovered branches -- they may reveal untested error paths

Do NOT use line coverage % as a test-writing target. A covered line is
not the same as a verified behavior. The diagnostic enumeration and gap
traceability steps above are the primary drivers for what tests to write.

---

## Phase 2: Analysis

```bash
# Find related tests
find tests/ -name "*feature-name*" -type f
rg "feature-keyword" tests/ --files-with-matches

# Verify code reachability
grep -rn "functionName" source/
```

**Reachability decision**:
- Only definition appears → **DEAD CODE** → STOP (file issue, don't test)
- Multiple call sites → Reachable → Continue

**Create** `tmp/feature-name/test-coverage.md` categorizing: Basic functionality, Error handling, Edge cases, Integration.

### Duplicate Detection (mandatory before writing any test)

For each test you plan to write:
1. Search for existing tests by diagnostic code:
   `rg "30500\|pack.*param.*position" tests/ --files-with-matches`
2. Search by scenario keyword:
   `rg "nonempty\|pack.*query" tests/language-feature/<feature>/`
3. Read the top candidates and compare scenarios
4. If an existing test covers >= 80% of your planned scenario, SKIP
   or extend the existing test instead of creating a new file

Document overlap analysis in test-coverage.md:
- "Checked against: [file1, file2]. No significant overlap."
- or "Overlap with [file]. Extending existing test instead."

---

## Phase 3: Gap Identification

### Checklist

**For each capability, check**:
- [ ] Positive case tested?
- [ ] Negative/error case tested?
- [ ] Edge cases tested?
- [ ] All targets tested? (cpu, cuda, vk, hlsl, metal, wgsl)

**For interface-typed variables/parameters/return-values**:
- [ ] Direct interface type
- [ ] Composite containing interface (struct, array, tuple)

**For function return values**:
- [ ] Direct return
- [ ] Return via `out`/`inout` parameter

### Prioritize Gaps

- **HIGH**: Core functionality untested, error cases missing
- **MEDIUM**: Edge cases partial
- **LOW**: Rare combinations

### Evaluate Test Value (score 0-10)

| Criterion | 0 pts | 1 pt | 2 pts |
|-----------|-------|------|-------|
| **Coverage** | Explicitly tested | Incidentally covered | Not covered |
| **Clarity** | Existing tests clear | Existing unclear | No existing tests |
| **Errors** | Errors well-tested | Only success tested | Errors untested |
| **Docs** | Well-documented | Minimal docs | No documentation |
| **Risk** | Low regression risk | Medium risk | High risk |

**Decision**: 0-4 SKIP, 5 Maybe, 6-10 WRITE

### Gap Traceability (mandatory)

Every gap identified in research must map to one of:
- A specific test to write (with filename and sub-plan assignment)
- An explicit SKIP with documented reason

No gap may be silently dropped. In `test-coverage.md`, create a
traceability table:

```markdown
| Gap | Action | Target | Reason |
|-----|--------|--------|--------|
| 30400 generic-type-needs-args | WRITE | diagnose-generic-type-needs-args.slang | No test exists |
| 30404 invalid-equality-constraint | SKIP | — | Already tested in conjunction-equality-witness.slang |
| Coercion constraints | SKIP | — | Only 2 existing tests, low risk, score 3/10 |
| Constructor type inference | SKIP | — | Feature not implemented in compiler |
```

---

## Phase 4: Test Design

### Requirements

- Verify behavior (check outputs/errors), not just execute
- Cover variable positions: local, parameter, return, struct field, array element, global

### Target Availability
- **Always available**: `-cpu` (no hardware needed)
- **CI-only (no local GPU)**: `-vk`, `-cuda`, `-dx12`, `-metal`, `-wgsl`
- **Recommendation**: Write tests with `-cpu` first, add GPU targets for CI verification

### Writing Test Comments

Write comments that explain the **semantic restriction or behavior**, not test mechanics.

**Don't** (formulaic/clumsy):
```slang
// Test: Verify that X produces error 33180.
// Gap: "Some coverage gap name from internal docs"
```

**Do** (natural/explanatory):
```slang
// Extension methods require compile-time type resolution, which is
// incompatible with dynamic dispatch where types are resolved at runtime.
```

Guidelines:
- Explain *why* the behavior exists, not *what* error code it produces
- Use complete sentences in natural language
- Skip internal tracking terms like "Gap:", "Test:", "Coverage:"
- Include error codes only when users might search for them

### Test Templates

See the `slang-test-development` skill for complete test templates, syntax reference, and the test type decision tree. Key templates:

- **Compute tests**: `COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -shaderobj -output-using-type`
- **Compilation tests**: `SIMPLE(filecheck=CHECK): -target spirv`
- **Diagnostic tests**: `DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv` (see `docs/diagnostics.md`)
- **Interpreter tests**: `INTERPRET(filecheck=CHECK):`
- `-shaderobj` — Use shader-object-based parameter binding (preferred for new tests)

---

## Phase 5: Implementation

**Placement**: `tests/language-feature/` or `tests/diagnostics/`
**Naming**: `feature-scenario.slang` or `diagnose-error-condition.slang`

```bash
# Build if needed
cmake --build --preset release --target slangc

# Run test
./build/Release/bin/slang-test tests/path/to/test.slang

# Run full suite
./build/Release/bin/slang-test -use-test-server -server-count 8
```

**Self-review**: Is there an existing test just as good? Would I understand this in 6 months? Does it run on all targets?

---

## Phase 6: Bug Investigation

**Decision tree**:
```text
Test fails
├─ Wrong expected value → Fix test
├─ Wrong setup → Fix test
├─ Behavior matches docs → Fix test
└─ Behavior violates docs → BUG → Document → Fix if possible
```

**Fix only if**: Root cause clear, fix straightforward, follows existing patterns, can validate no regressions.

---

## Phase 7: Documentation

Update `tmp/feature-name/test-coverage.md` with final status.

Create `tmp/feature-name/SUMMARY.md`:
- Tests created (list with descriptions)
- Bugs found (list with status)
- Validation results

---

## Decision Rules

### WRITE test if:
- Reference describes behavior but no test exists (score ≥6)
- Error case should trigger but untested
- Existing tests unclear + your test notably better

### SKIP test if:
- Code unreachable (dead code)
- **Exact same scenario** already tested (not just "feature is tested")
- Value score <5

### Investigate as BUG if:
- Behavior violates documented semantics
- Error should trigger but doesn't

---

## Anti-Patterns

1. **Coverage %-driven**: Writing tests to hit percentage targets
2. **Dead code testing**: Not checking reachability first
3. **Test duplication**: Not checking existing coverage
4. **Execution-only tests**: Tests that run but don't verify behavior
5. **Testing unsupported features**: Attempting to write functional tests
   for compiler features that are not yet implemented (e.g., constructor
   type inference from arguments). Always verify the feature compiles
   with a quick `slangc` invocation before writing the full test. If the
   feature does not compile, file a bug/feature request instead of
   writing a test.
6. **Misleading comments**: Comments referencing wrong interface names,
   wrong error codes, or describing behavior that does not match the
   actual code. Always cross-check comment text against the real code.

---

## Output Structure

```text
tmp/feature-name/                    # Working directory (do NOT check in)
├── README.md          # Feature overview
├── test-coverage.md   # Analysis with gap status
└── SUMMARY.md         # Final results

tests/language-feature/feature-name/ # Test files (check in)
├── scenario-1.slang
├── scenario-2.slang
└── diagnose-error-case.slang
```

**Note**: The `tmp/` directory is for your reference during the coverage work and is gitignored. Only the test files under `tests/` should be committed.

---

## Core Principles

1. **Quality over quantity** - 5 excellent tests > 20 mediocre
2. **Test failures are valuable** - They reveal bugs or gaps in understanding
3. **Verify everything** - Run tests, check outputs, validate fixes
4. **No coverage increase is also valuable** - Dead code or already well-tested

**Goal**: Ensure features work correctly in all documented scenarios and fail gracefully in error cases.
