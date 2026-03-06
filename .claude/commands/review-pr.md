---
allowed-tools: Agent,Bash(gh pr diff *),Bash(gh pr diff*),Bash(gh pr view *),Bash(gh pr view*),Bash(git diff*),Bash(git log*),Bash(git show*),mcp__github__create_pending_pull_request_review,mcp__github__add_comment_to_pending_review,mcp__github__submit_pending_pull_request_review,mcp__github__get_pull_request,mcp__github__get_pull_request_files,mcp__github__get_pull_request_reviews
description: Review a pull request for the Slang shader compiler
---

Perform a comprehensive code review of this PR for the Slang shader compiler using subagents for key areas:

- code-quality-reviewer
- ir-correctness-reviewer
- security-code-reviewer
- test-coverage-reviewer
- cross-backend-reviewer
- documentation-accuracy-reviewer

First, fetch the PR diff yourself with `gh pr diff <number>` and the changed files list. Then you MUST dispatch ALL subagents listed above in parallel, regardless of PR size. Do NOT skip any — even for small PRs, each agent catches different classes of issues. Pass the full diff content and changed file list in each subagent's prompt. Instruct each to read CLAUDE.md for project context. Subagents can read local files (base branch) for surrounding context but cannot run Bash — you must provide the diff to them. Each should report only noteworthy findings with confidence ≥80.

Once all subagents finish, review their combined feedback and apply a second editorial filter:
- Drop any finding you judge to be a false positive or low-value
- Drop formatting/style issues (CI enforces formatting via `./extras/formatting.sh`)
- Merge duplicate findings across agents
- Keep only findings with confidence ≥80

Then post the filtered results to the PR:
1. Create a PENDING review with `mcp__github__create_pending_pull_request_review`
2. Add inline comments via `mcp__github__add_comment_to_pending_review` for critical issues only (max 5 inline comments, prioritized by severity — put the rest in the review body)
3. Submit the review with `mcp__github__submit_pending_pull_request_review` using event "COMMENT" (NEVER "APPROVE" or "REQUEST_CHANGES")

### Review body format

Start with a verdict line, a 1-3 sentence summary, and an optional highlights line. Then collapsible `<details>` sections by category. Omit empty categories. Be terse.

Use severity badges on each finding:
- 🔴 **Bug** — correctness issue, crash, UB, or security vulnerability
- 🟡 **Gap** — missing backend/test/doc coverage, inconsistency
- 🔵 **Question** — intent unclear, needs author clarification

**Template:**

```
**Verdict**: 🔴 Has issues — N bugs, M gaps | OR | ✅ Clean — no significant issues found

<1-3 sentence summary of what the PR does and the key findings>

**Highlights**: <one sentence noting what was done well — omit if nothing stands out>

<details>
<summary>Correctness & IR Invariants (N findings)</summary>

- 🔴 **Bug** `file:line` — description
- 🟡 **Gap** `file:line` — description

</details>

<details>
<summary>Security & Undefined Behavior (N findings)</summary>

- ...

</details>

<details>
<summary>Cross-Backend Consistency (N findings)</summary>

- ...

</details>

<details>
<summary>Test Coverage (N findings)</summary>

- ...

</details>

<details>
<summary>Documentation & Comments (N findings)</summary>

- ...

</details>

<details>
<summary>Clarifications Needed</summary>

- 🔵 `file:line` — question for the author

</details>
```

Zero inline comments is the norm for solid code.
