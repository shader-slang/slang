# Feature Test Gap Analysis - AI Agent Guide

**Purpose**: Systematically analyze test coverage for a language feature, identify gaps, create tests, discover bugs.

**Inputs**: 
- Feature name
- Language reference manual/specification
- Access to `tests/` directory

**Outputs**: 
- Test coverage analysis
- New test files
- Bug reports (if found)
- Fixes (if bugs are fixable)

---

## 7-Step Workflow

### Step 1: Understand Feature

**Read reference manual** → Extract:
- Core concepts and capabilities
- Syntax and semantics  
- Restrictions and limitations
- Error conditions

**Search existing docs**:
```bash
find docs/ -type f | xargs grep -l "feature-keyword"
rg "feature-keyword" docs/ examples/
```

**Create**: `tmp/feature-name/README.md`
- Overview (2-3 paragraphs)
- Core concepts list
- Key behaviors from reference
- Known restrictions

---

### Step 2: Inventory Tests

**Find all related tests**:
```bash
find tests/ -name "*feature-name*" -type f
rg "feature-keyword" tests/ --files-with-matches
```

**For each test, record**: path, what it tests, test type (positive/negative/error)

**Categorize**: Basic functionality, Error handling, Edge cases, Integration

**Create**: `tmp/feature-name/test-coverage.md`
```markdown
## Test Inventory

### Category: Basic Functionality
**Status**: Well Covered / Partial / Missing
**Tests**: 
- `path/test.slang` - Description
**Covered**: ✅ X, ✅ Y
**Gaps**: ❌ Z

[Repeat for each category]

## Summary Table
| Category | Status | Files | Gaps |
|----------|--------|-------|------|
| Basic    | ✅     | 5     | 0    |
| Errors   | ⚠️     | 2     | 3    |
```

---

### Step 3: Identify Gaps

**For each capability in reference manual, check**:
- [ ] Positive case tested?
- [ ] Negative/error case tested?
- [ ] Edge cases tested?
- [ ] Interactions tested?

**Prioritize gaps**:
- **HIGH**: Core functionality untested, error cases missing
- **MEDIUM**: Edge cases partial
- **LOW**: Rare combinations

**Document each gap**:
```markdown
### Gap: [Name]
**Priority**: HIGH/MEDIUM/LOW
**Missing**: [What's not tested]
**Why Important**: [Impact]
**Example**: [Minimal code showing gap]
**Expected**: [Per reference manual]
```

**Update test-coverage.md**:
```markdown
## Priority Gaps
### High Priority
1. ❌ [Gap] - [Why critical]

### Medium Priority  
2. ⚠️ [Gap] - [Why useful]
```

---

### Step 4: Create Tests

**For each HIGH priority gap**:

1. **Study similar tests first**:
   ```bash
   cat tests/language-feature/similar-test.slang
   ```

2. **Follow pattern**:
   ```slang
   //TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -shaderobj -output-using-type
   //TEST_INPUT:ubuffer(data=[0 0 ...], stride=4):out,name=outputBuffer
   
   // Test: [Clear description]
   // Gap: [Which gap this addresses]
   // Reference: [Manual section]
   
   [Setup types/interfaces]
   
   RWStructuredBuffer<float> outputBuffer;
   
   [numthreads(1, 1, 1)]
   void computeMain() {
       uint idx = 0;
       
       // Test Case 1: [Description]
       outputBuffer[idx++] = result;
       // CHECK: [expected]
   }
   ```

3. **For error cases**:
   ```slang
   //TEST:SIMPLE:-target spirv -entry main -stage compute
   
   [Code that should error]
   
   // CHECK: error [number]
   ```

4. **Naming**: `feature-scenario.slang` or `error-condition.slang`

5. **Run test**:
   ```bash
   ./build/Release/bin/slang-test tests/path/to/test.slang
   ```

**Test quality checklist**:
- Verifies behavior (not just executes)
- Clear comments (what/why)
- Multiple related scenarios if appropriate
- Descriptive names

---

### Step 5: Investigate Failures

**When test fails**:

1. **Check test first**: Expected value correct? Setup correct?

2. **If test is correct → Potential bug**:

**Create**: `tmp/feature-name/bugs-found.md`
```markdown
## Bug: [Brief Description]

### Test
File: `tests/path/test.slang`

### Expected (per reference)
[What should happen]

### Actual
[What happens]
[Minimal reproduction]

### Analysis
**Root Cause**: [Location if found]
**Severity**: Low/Medium/High/Critical
**Impact**: [What's affected]

### Recommended Fix
[If determinable]
```

**Decision tree**:
```
Test fails
├─ Wrong expected value → Fix test
├─ Wrong setup → Fix test  
├─ Behavior matches docs → Fix test
└─ Behavior violates docs → BUG → Document → Step 6
```

---

### Step 6: Fix Bugs (If Found)

**Only if**: Root cause clear, fix straightforward, follows existing patterns

**Actions**:

1. **Locate bug**:
   ```bash
   rg "functionName" source/
   ```

2. **Study similar code**:
   ```bash
   rg "bool.*check.*Type" source/slang/
   ```

3. **Implement fix** following project patterns

4. **Validate**:
   ```bash
   # Build
   cmake --build build --config Release --target slangc
   
   # Test specific case
   ./build/Release/bin/slang-test tests/path/failing-test.slang
   
   # Test feature suite
   ./build/Release/bin/slang-test tests/language-feature/feature-name/
   
   # Test all (no regressions)
   ./build/Release/bin/slang-test -use-test-server -server-count 8
   ```

**Update bugs-found.md**:
```markdown
### Status: ✅ FIXED
**Location**: `source/file.cpp:lines`
**Validation**: All N tests pass, no regressions
```

**If not fixable**: Document only, proceed to Step 7

---

### Step 7: Document

**Update test-coverage.md** with final status:
```markdown
## Final Status
| Category | Status | Files | Gaps |
|----------|--------|-------|------|
| ...      | ✅     | 8     | 0    |

**Created**: [N] tests
**Bugs Found**: [N]
**Bugs Fixed**: [N]
**Pass Rate**: [X/X] (100%)
```

**Create**: `tmp/feature-name/SUMMARY.md`
```markdown
# Feature Testing Summary

## Work Completed
1. Feature Understanding - README.md
2. Coverage Analysis - [N] existing tests, [M] gaps
3. Tests Created - [list with descriptions]
4. Bugs Found - [list with status]
5. Validation - [X/X] pass, no regressions

## Deliverables
- Documentation: README.md, test-coverage.md, SUMMARY.md
- Tests: [N] files in tests/language-feature/feature-name/
- Bug reports: bugs-found.md (if any)
- Code changes: [files] (if fixes)

## Results
[Test pass rate summary]

## Key Findings
[Important discoveries]
```

**If bugs fixed**: `tmp/feature-name/PR.md`
```markdown
# [Brief title]

## Summary
[2-3 sentences]

## Bug Fixed
**Issue**: [description]
**Impact**: [severity]
**Root Cause**: [technical]

## Solution
[Technical fix description]

## Tests Added
[List with descriptions]

## Validation
- ✅ All [N] tests pass
- ✅ No regressions
```

---

## Decision Rules

### Write Test If:
- ✅ Reference describes behavior but no test exists
- ✅ Error case should trigger but untested
- ✅ Edge case could fail but untested
- ✅ Feature interaction untested

### Skip Test If:
- ❌ Same scenario already tested
- ❌ Redundant with existing
- ❌ Feature deprecated
- ❌ Unreachable code

### Investigate As Bug If:
- Expectation matches reference but behavior differs
- Error should trigger (per docs) but doesn't
- Behavior violates documented semantics
- Inconsistent across similar cases

### Fix Bug If:
- ✅ Root cause clear
- ✅ Fix straightforward
- ✅ Follows existing patterns
- ✅ Can validate no regressions

### Document Only If:
- ❌ Root cause unclear
- ❌ Needs major refactoring
- ❌ Architectural decision needed

---

## Quality Standards

**Tests**:
- Clear purpose in comments
- Verifies outputs/errors
- Self-contained
- Focused on one aspect

**Documentation**:
- Complete (all deliverables)
- Accurate (matches reality)
- Clear and organized
- Actionable recommendations

**Code** (if fixing bugs):
- Handles edge cases
- Follows project patterns
- Commented for clarity
- Validated with tests

---

## Expected Output Structure

```
tmp/feature-name/
├── README.md              # Feature overview
├── test-coverage.md       # Analysis
├── SUMMARY.md            # Complete summary
├── bugs-found.md         # If bugs found
└── PR.md                 # If fixes made

tests/language-feature/feature-name/
├── scenario-1.slang
├── scenario-2.slang
├── error-case-1.slang
└── ...

source/slang/
└── [file.cpp]            # If bugs fixed
```

---

## Key Commands

```bash
# Finding
find tests/ -name "*keyword*" -type f
rg "pattern" tests/ --files-with-matches
rg "functionName" source/

# Running
./build/Release/bin/slang-test tests/path/test.slang
./build/Release/bin/slang-test tests/language-feature/dir/
./build/Release/bin/slang-test -use-test-server -server-count 8

# Building
cmake --build build --config Release --target slangc
```

---

## Success Criteria

**Minimum**:
- Feature documented
- Coverage analyzed
- Gaps identified/prioritized
- High-priority gaps tested
- All tests pass

**Full**:
- Above + bugs documented + fixes (if fixable) + comprehensive docs + no regressions

**Outstanding**:
- Above + unknown bugs found + significant coverage improvement

---

## Core Principles

1. **Follow all 7 steps** - Don't skip
2. **Test failures are valuable** - They reveal bugs or gaps in understanding
3. **Quality over quantity** - 5 excellent tests > 20 mediocre
4. **Document as you go** - Don't wait until end
5. **Verify everything** - Run tests, check outputs, validate fixes

**Goal**: Ensure feature works correctly in all documented scenarios, fails gracefully in error cases.
