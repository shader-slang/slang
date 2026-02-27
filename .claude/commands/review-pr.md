---
allowed-tools: Bash(gh pr comment:*),Bash(gh pr diff:*),Bash(gh pr view:*),Bash(git diff*),Bash(git log*),Bash(git show*),mcp__github__create_pending_pull_request_review,mcp__github__add_comment_to_pending_review,mcp__github__submit_pending_pull_request_review,mcp__github__add_issue_comment,mcp__github__get_pull_request,mcp__github__get_pull_request_files,mcp__github__get_pull_request_reviews
description: Review a pull request for the Slang shader compiler
---

Perform a comprehensive code review of this PR for the Slang shader compiler using subagents for key areas:

- code-quality-reviewer
- ir-correctness-reviewer
- security-code-reviewer
- test-coverage-reviewer
- cross-backend-reviewer
- documentation-accuracy-reviewer

Instruct each subagent to read CLAUDE.md for project context before reviewing. Each should read the full diff (`gh pr diff`) and the changed files in full, then report only noteworthy findings with confidence ≥80.

Once all subagents finish, review their combined feedback and apply a second editorial filter:
- Drop any finding you judge to be a false positive or low-value
- Drop formatting/style issues (CI enforces formatting via `./extras/formatting.sh`)
- Merge duplicate findings across agents

Then post the filtered results to the PR:
1. Create a PENDING review with `mcp__github__create_pending_pull_request_review`
2. Add inline comments via `mcp__github__add_comment_to_pending_review` for critical issues only
3. Submit the review with `mcp__github__submit_pending_pull_request_review` using event "COMMENT" (NEVER "APPROVE" or "REQUEST_CHANGES")

The review body should have a 1-3 sentence executive summary, then collapsible `<details>` sections:
- Correctness & IR Invariants
- Security & Undefined Behavior
- Cross-Backend Consistency
- Test Coverage Gaps
- Documentation & Comments
- Questions for Author
- Workflow Reminders (proposal docs, API docs, feature maturity)

Omit empty categories. Be terse. Zero inline comments is the norm for solid code.
