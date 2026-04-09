---
name: slang-create-issue
description: Create clear, well-structured GitHub issues and pull requests for the Slang compiler repository. Use when the user wants to file a bug report, feature request, improvement proposal, or create a pull request for shader-slang/slang.
---

# Slang Issues and Pull Requests

**For**: Creating GitHub issues and PRs for the shader-slang/slang repository

**Core Principle**: Issues should be clear, concise, and actionable. Describe the problem and its impact.

## Issue Categories

Determine the category before writing:

| Category | Template | When to Use |
|----------|----------|-------------|
| **Compiler Bug** | Bug Report | Crash, ICE, wrong codegen, validation error |
| **Improvement** | Feature/Improvement | Missing feature, better diagnostics, UX improvement |

---

## Bug Report Template

Use this structure for compiler bugs, crashes, ICEs, and codegen errors.

```markdown
# [Short descriptive title]

## Issue Description

[1-3 sentences: What is broken. State the symptom clearly.]

## Reproducer Code

[Self-contained Slang code that triggers the issue. Minimize it.]

\`\`\`hlsl
// Minimal reproducer
[code here]
\`\`\`

**Command:**
\`\`\`bash
slangc -target [target] test.slang
\`\`\`

## Expected Behavior

[1-2 sentences: What should happen.]

## Actual Behavior

[Paste the actual error output or describe the wrong behavior.]

## Test Plan

[List the tests that revealed this issue or can be enabled or are affected by this issue.]

```

### Bug Report Rules

1. **Reproducer is mandatory** -- self-contained, minimal code that anyone can copy-paste and compile
2. **Use `slangc` single-dash options** -- `-target spirv` not `--target spirv`, `-dump-ir` not `--dump-ir`
3. **State the target** -- many bugs are target-specific (SPIRV is most common, followed by GLSL and HLSL)
4. **Include actual error output** -- paste the exact error message or crash output
5. **Do NOT include internal analysis** in the issue body (root cause analysis, file locations, line numbers belong in a separate plan document or PR description, not the issue)
6. **Keep it short** -- a maintainer should understand the problem in under 60 seconds
7. **Be cautious and critical when proposing solutions** -- the agent often lacks full context of the compiler's design constraints.

### What NOT to Include in Bug Reports

- Implementation details or source file references (e.g., "crash in slang-ir-typeflow-specialize.cpp line 1417")
- Detailed fix proposals with code changes
- IR dumps (unless specifically relevant to the reported behavior)
- Lengthy analysis of why something is architecturally impossible

---

## Improvement / Feature Request Template

Use this structure for missing features, better diagnostics, and UX improvements.

```markdown
# [Short descriptive title]

## Problem Description

[1-3 sentences: What is the problem or limitation. Why does it matter.]

## Preferred Solution

[Brief description of the desired outcome. Focus on WHAT, not HOW.]

## Alternative Solutions

[Other approaches considered, if any. Can be omitted if none.]

## Additional Context

[Links, related issues, downstream projects affected.]
```

### Improvement Rules

1. **Focus on the problem, not the solution** -- describe what's wrong or missing
2. **State the impact** -- who is affected and how (e.g., "all Vulkan users", "downstream project Falcor")
3. **Keep solution proposals brief and high-level** -- do not prescribe implementation details
4. **Reference related issues** if they exist -- duplicates are common in long-lived repositories

---

## Being Critical About Solutions

**This is the most important section.** When the agent has investigated a problem and wants to propose a solution:

### Before Proposing a Solution, Ask:

1. **Do I understand the full design context?** The Slang compiler has complex interactions between frontend (semantic checking), IR passes (100+ transformation passes), and multiple backends (SPIRV, HLSL, GLSL, CUDA, Metal, WGSL). A fix in one area may break assumptions in another.

2. **Is this the right layer to fix?** The compiler philosophy is to keep target code emission simple and do heavy lifting in IR passes. A fix in the emitter might be a band-aid for an IR problem.

3. **Could this be intentional?** Some behaviors that look like bugs may be deliberate design decisions (e.g., HLSL compatibility quirks, performance trade-offs).

4. **Am I confident in the scope?** The agent typically sees one test case. The actual fix may need to handle generics, interfaces, dynamic dispatch, autodiff, and cross-target interactions.

### Solution Confidence Levels

Use these to calibrate solution proposals:

| Level | When | What to Write |
|-------|------|---------------|
| **High** | Root cause is clear, fix is localized, pattern matches existing fixes | "The fix should [brief description]" |
| **Medium** | Root cause is likely but interactions are uncertain | "A possible approach is [brief description], but this needs investigation" |
| **Low** | Symptom is clear but root cause is uncertain | Omit solution proposal; describe only the problem |

### Default: Err on the Side of Less

- If unsure, **do not propose a solution**. A clear problem statement is more valuable than a wrong solution.
- Never propose changes to IR passes you haven't fully traced through.
- Never assume a check can be "simply added" to semantic analysis -- the checking pipeline has complex ordering constraints.

---

## Pull Request Format

Unlike issues, PRs **should** include technical implementation details. The PR description is read by reviewers who need to understand *what was changed and why at a code level*. Include root cause analysis, affected files, design decisions, and trade-offs.

When creating a PR for a fix or feature:

```markdown
## Summary
[1-3 bullet points describing what this PR does]

## Motivation
[Why this change is needed. Link to the issue if one exists.]

## Technical Details
[Explain the root cause, the approach taken, and key design decisions.
Include file names, function names, and reasoning for the chosen approach.
Mention alternative approaches considered and why they were rejected.
This is where implementation-level analysis belongs -- not in the issue.
Do not narrate the diff line by line -- reviewers read the code themselves.]

## Test Plan
[How this was tested]
- [ ] New test added: `tests/path/to/test.slang`
- [ ] Existing tests pass: `slang-test -use-test-server -server-count 8`
- [ ] SPIRV validation: `SLANG_RUN_SPIRV_VALIDATION=1 slangc -target spirv test.slang`
```

### Commit Rules

These rules apply to ALL commits across all skills.

1. **Never amend**: Always create new commits, never `git commit --amend` or `git rebase -i` to squash/edit. This preserves branch history so earlier attempts and intermediate states remain findable. If a previous commit had a problem, fix it in a new commit — do not rewrite or back out the original.
2. **Do NOT mention AI tools** in commit messages or PR descriptions
3. **No issue numbers in commit summary**: Reference the issue in the PR body (e.g., "Fixes #1234"), not the commit message — GitHub adds it during merge.

### PR Rules

1. **Label the PR**: Use `pr: non-breaking` (default) or `pr: breaking` (for ABI/language-breaking changes)
2. **Assign to author**: Always use `--assignee @me` when creating PRs
3. **Include tests**: Add regression tests as `.slang` files under `tests/`
4. **Keep PRs focused**: One issue per PR when possible
5. **Include technical depth**: Root cause, design rationale, trade-offs. Do not narrate the diff -- reviewers read the code themselves
6. **Suggest reviewers**: Use `git log --format='%an' -- <changed-files> | sort | uniq -c | sort -rn` to identify 2-5 reviewers based on recent authorship of the affected code. Include 1-2 sentences per reviewer explaining the rationale.

### Test Quality Rules (for PRs adding tests)

When a PR adds test files, verify each test before committing:

1. **Filename matches content**: If the test verifies "no applicable
   generic", the file must be named accordingly, not after a different
   scenario.
2. **Comments match code**: All interface names, error codes, and
   behavior descriptions in comments must match the actual code.
3. **No dead code**: Every function, struct, or variable in the test
   must be called or used.
4. **No duplicates**: Search existing tests for the same scenario before
   adding a new file. Extend existing tests when possible.
5. **Feature verified**: For functional tests, confirm the feature
   compiles before writing the full test. Do not test unsupported
   features.
6. **Diagnostic tests**: Prefer exhaustive mode. Only use
   `non-exhaustive` when there is a documented reason.
7. **All tests pass locally**: Run every new test before committing.

---

## Issue Title Conventions

Titles should be scannable and specific. Follow patterns from existing issues:

**Bug titles:**
- `[BUG]: SPIRV validation error when [specific scenario]`
- `Internal compiler error when [specific scenario]`
- `[target] Wrong codegen for [specific construct]`
- `Crash when compiling [specific pattern]`

**Improvement titles:**
- `[Feature area] Support [specific capability]`
- `Improve diagnostic for [specific error case]`
- `[Target] Add support for [specific extension/feature]`

**Avoid:**
- Vague titles: "Slang doesn't work", "Compilation fails"
- Overly long titles with full error messages
- Titles that describe the solution instead of the problem

---

## Common Slang Issue Patterns

| Pattern | Notes |
|---------|-------|
| SPIRV validation errors | Include `SLANG_RUN_SPIRV_VALIDATION=1` output |
| ICE (Internal Compiler Error) | Error 99999 with exception. Always a bug |
| Wrong codegen | Include expected vs actual output comparison |
| Missing HLSL compatibility | Reference HLSL spec or DXC behavior |
| Generics/interface issues | Often complex interactions, be cautious with solutions |
| Diagnostic quality | User-facing error messages need improvement |

For deep investigation of any of these, use the `slang-investigate` skill.

---

## Output Files

Write issue content to `tmp/<topic>/issue.md`. This file is gitignored and for draft purposes only.

If a detailed implementation plan exists, keep it in a separate `tmp/<topic>/plan.md`. Do not merge plan details into the issue.

---

## Checklist Before Filing

- [ ] Title is specific and scannable
- [ ] Reproducer code is self-contained and minimal
- [ ] Expected vs actual behavior is clear
- [ ] Issue is under ~50 lines of Markdown (excluding code blocks)
- [ ] No internal implementation details in the issue body
- [ ] Solution proposal (if any) matches confidence level
- [ ] Checked for duplicate issues
- [ ] Test Plan section is filled in
- [ ] Issue is assigned to author (`--assignee @me`)
