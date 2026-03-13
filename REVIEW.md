# PR Review Protocol

Follow this protocol. Do NOT skip steps. Do NOT write a free-form review. Do NOT post a review without dispatching all 6 teammates first.

## Step 1: Fetch the PR diff and changed files

```
gh pr diff <number> -R <repo>
gh pr view <number> -R <repo> --json files
```

## Step 2: Dispatch 6 reviewers in parallel

Make ALL 6 Agent calls in a SINGLE message so they run concurrently:

1. `Agent(subagent_type="code-quality-reviewer", run_in_background=true, ...)`
2. `Agent(subagent_type="ir-correctness-reviewer", run_in_background=true, ...)`
3. `Agent(subagent_type="security-code-reviewer", run_in_background=true, ...)`
4. `Agent(subagent_type="test-coverage-reviewer", run_in_background=true, ...)`
5. `Agent(subagent_type="cross-backend-reviewer", run_in_background=true, ...)`
6. `Agent(subagent_type="documentation-accuracy-reviewer", run_in_background=true, ...)`

Each agent's prompt MUST include:
- The FULL diff content and changed file list
- "Read CLAUDE.md for project context"
- "Use `mcp__deepwiki__ask_question` with repo `shader-slang/slang` as a SUPPLEMENTARY reference only — always prioritize the actual code, PR diff, and CLAUDE.md over DeepWiki answers"
- "For large files (>1000 lines), use Grep first then Read with offset/limit"
- "The Slang language spec has been cloned to `external/spec/` (if it exists). Check `external/spec/proposals/` when the PR implements or references a spec proposal"

All 6 MUST be dispatched regardless of PR size.

## Step 3: Wait for all 6 agents, then editorially filter

Wait for all 6 background agents to complete. You will be automatically notified as each one finishes.

Once ALL 6 have returned findings, combine them and filter:
- Drop false positives and low-value findings
- Drop formatting/style issues (CI enforces via `./extras/formatting.sh`)
- Merge duplicates across teammates
- Keep only findings with confidence >= 80

## Step 4: Analyze changes for the review

Before writing, prepare two things:

**A) Group changes by feature/module** (for PRs touching 3+ files):
- Example: "SER capability handling" = `slang-emit-spirv.cpp` + `slang-capability.cpp` + `tests/spirv/ser-*.slang`
- This goes into the "Changes Overview" section

**B) For each finding, prepare a detailed explanation**:
- What the code did BEFORE this PR (or what it does now that's wrong)
- What the IMPACT is (concrete scenario, example inputs/outputs, which users hit this)
- What the FIX should be (specific, actionable)

## Step 5: Post ONE review with inline comments

1. Create a PENDING review with `mcp__github__create_pending_pull_request_review`
2. Add inline comments via `mcp__github__add_comment_to_pending_review`:
   - Add an inline comment for EVERY finding (bugs, gaps, and questions)
   - Each inline comment goes on the specific diff line where the issue is
   - Each inline comment should be detailed (see Inline Comment Format below)
3. Submit with `mcp__github__submit_pending_pull_request_review` using event **"COMMENT"**

NEVER use "APPROVE" or "REQUEST_CHANGES". ALWAYS use "COMMENT".
Post EXACTLY ONE review. Do NOT post multiple reviews or separate comments.

---

## Inline Comment Format

Each inline comment should be a mini-analysis, not just a one-liner. Use this structure:

````
🔴 **Bug**: <short title>

<What this code does and why it's wrong — 2-3 sentences explaining the issue in context>

**Example**: <concrete scenario showing the bug, e.g., specific inputs that trigger wrong behavior>

**Suggested fix**:
```
<code snippet showing the fix>
```
````

For gaps and questions, adapt accordingly:

```
🟡 **Gap**: <short title>

<What's missing and why it matters — 2-3 sentences>

**Suggestion**: <specific actionable recommendation>
```

```
🔵 **Question**: <short title>

<What's unclear and why it matters — 1-2 sentences. Ask a specific question, don't just note confusion.>
```

---

## Review Body Format

The review body is the SUMMARY. The detailed analysis goes in inline comments. Use this structure:

```
**Verdict**: 🔴 Has issues — N bug(s), M gap(s) | OR | ✅ Clean — no significant issues found

<1-3 sentence TECHNICAL summary: what the PR changes and what problems were found.>

<details>
<summary>Changes Overview</summary>

**<Feature/Module Group 1>** (<list of files>)
- What changed: <1-2 sentences describing the before/after>

**<Feature/Module Group 2>** (<list of files>)
- What changed: <1-2 sentences describing the before/after>

</details>

<details>
<summary>Findings (N total)</summary>

| Severity | Location | Finding |
|----------|----------|---------|
| 🔴 Bug | `file:line` | <one-line description — detail is in the inline comment> |
| 🟡 Gap | `file:line` | <one-line description — detail is in the inline comment> |
| 🔵 Question | `file:line` | <one-line description — detail is in the inline comment> |

</details>
```

Omit the Findings section if there are 0 findings. Changes Overview is ALWAYS included.

### Severity badges:
- 🔴 **Bug** — correctness issue, crash, UB, or security vulnerability
- 🟡 **Gap** — missing backend/test/doc coverage, inconsistency
- 🔵 **Question** — intent unclear, needs author clarification

---

## Tone Rules

- Be TECHNICAL. Every sentence must convey information.
- Do NOT praise the code. No "well-structured", "clean", "good", "nice", "solid", "well-designed", "LGTM".
- Do NOT say "Approve" or "looks good to merge" anywhere in the review.
- Do NOT use sections like "What works well", "Positive aspects", "Highlights".
- If no issues found, say `✅ Clean — no significant issues found` and provide the Changes Overview only.
- Put the DETAIL in inline comments, keep the body as a summary/index.
