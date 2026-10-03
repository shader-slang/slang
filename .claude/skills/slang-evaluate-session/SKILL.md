---
name: slang-evaluate-session
description: Evaluate the current session's skill effectiveness. Only invoke when explicitly called via /slang-evaluate-session.
---

# Evaluate Session

**For**: Post-session review of skill effectiveness — which instructions helped, which wasted tokens, which were missing.

**Usage**: `/slang-evaluate-session`

Run this at the end of a work session to get a structured assessment.

---

## What to Analyze

Scan the current conversation and identify:

1. **Skills triggered** — any skills invoked or referenced (project skills, user skills, or built-in skills)
2. **Tasks performed** — what concrete work was done (bug fix, test writing, PR review, issue filing, etc.)
3. **Outcome** — what was produced (commits, PRs, issues, test files, investigation notes)

---

## For Each Skill Used, Evaluate

### USEFUL rules
Instructions that actively influenced behavior in a positive way:
- Prevented a known mistake (e.g., used single-dash CLI options, wrote negative test companion)
- Guided toward better solution (e.g., investigated IR pass instead of emission fix)
- Saved time (e.g., referenced correct build preset, knew platform limitations)

### WASTEFUL rules
Instructions that consumed tokens without benefit:
- Not relevant to the task at hand
- Too generic to influence behavior
- Duplicated information already in CLAUDE.md
- Long sections that were never consulted

### MISSING rules
Mistakes or friction that a rule could have prevented:
- A pattern the LLM got wrong that should be documented
- A workflow step that was forgotten or done in wrong order
- Platform-specific issue that wasn't covered

---

## Report Format

Present to the user:

```
## Session Evaluation

### Tasks
- [list of what was done]

### Skills Used
- [list of skills triggered]

### Verdict

| Skill | Rule/Section | Rating | Evidence |
|-------|-------------|--------|----------|
| slang-fix-bug | "prefer IR pass over emission fix" | USEFUL | Redirected from emitter patch to legalization pass |
| slang-fix-bug | parallel agent strategy (Phase 3) | WASTEFUL | Fix was unambiguous, section was skipped entirely |
| slang-write-test | negative test companion rule | USEFUL | Added constraint violation test that would have been missed |
| simplify | code reuse check | USEFUL | Caught duplicated helper that could use existing utility |
| user-custom-skill | deployment checklist | WASTEFUL | Not relevant to this task |
| (none) | autodiff interaction guidance | MISSING | Hit autodiff edge case with no skill guidance |

### Summary
- **Useful**: N rules actively helped
- **Wasteful**: N rules consumed tokens without benefit
- **Missing**: N gaps identified

### Suggested Skill Improvements
- [concrete suggestions based on findings]
```

---

## Guidelines

- Be honest and specific — vague assessments like "skills were helpful" are not useful
- Cite the exact rule or section name, not just the skill name
- For WASTEFUL, distinguish between "irrelevant to this task" (acceptable) and "never useful to any task" (should be removed)
- For MISSING, propose the specific rule text that would help
- Keep the report concise — focus on actionable findings, not exhaustive enumeration
