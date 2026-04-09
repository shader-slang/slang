---
name: slang-review-pr
description: Review a Slang compiler PR for correctness, evaluate review feedback, implement fixes, and manage review threads. Use when the user wants to review a PR, address review comments, or iterate on PR feedback for shader-slang/slang.
---

# Slang PR Review

**For**: Reviewing PRs on the shader-slang/slang repository, evaluating whether the solution
fixes the root cause optimally, addressing review feedback, and managing review threads.

**Core Principle**: A good fix addresses the root cause, not symptoms. Evaluate each PR for
long-term correctness and each review comment for actionability before acting.

**Usage**: `/slang-review-pr <pr-url-or-number>`

Where `<pr-url-or-number>` is:
- A PR URL (e.g., `https://github.com/shader-slang/slang/pull/10759`)
- A PR number (e.g., `10759`)

---

## Phase 1: GATHER CONTEXT

Collect all information about the PR, its linked issues, and review feedback in parallel.

### Step 1: Fetch PR Details

```bash
# PR metadata, body, and reviews
gh pr view <number> --repo shader-slang/slang \
  --json title,body,state,headRefName,baseRefName,url,isCrossRepository,headRepository,headRepositoryOwner

# Review comments (inline code comments)
gh api repos/shader-slang/slang/pulls/<number>/comments

# Reviews (top-level review bodies)
gh api repos/shader-slang/slang/pulls/<number>/reviews
```

### Step 2: Fetch Linked Issues

Extract issue numbers from the PR body (e.g., `Fixes #10153`) and fetch them:

```bash
gh issue view <number> --repo shader-slang/slang --json title,body,labels
```

### Step 3: Fetch Review Thread Status

Paginate to avoid missing unresolved threads on larger PRs:

```bash
# Fetch all review threads (paginate if hasNextPage is true)
gh api graphql -f query='
{
  repository(owner: "shader-slang", name: "slang") {
    pullRequest(number: N) {
      reviewThreads(first: 100) {
        pageInfo { hasNextPage endCursor }
        nodes {
          id
          isResolved
          comments(first: 100) {
            nodes {
              databaseId
              author { login }
              body
            }
          }
        }
      }
    }
  }
}'
```

If `hasNextPage` is true, re-query with `after: "<endCursor>"` until all threads are fetched.

### Step 4: Sync the Branch

```bash
# Preferred: works for same-repo and fork-based PRs
gh pr checkout <number>

# Manual fallback if you need explicit control:
# Same-repo PR
git fetch origin <headRefName>
git checkout -B <headRefName> --track origin/<headRefName>

# Fork-based PR
git remote get-url pr-author >/dev/null 2>&1 || \
  git remote add pr-author https://github.com/<headRepositoryOwner.login>/<headRepository.name>.git
git fetch pr-author <headRefName>
git checkout -B <headRefName> --track pr-author/<headRefName>
```

If you add the fork remote manually, add it only once. Reuse the existing remote
on later review iterations.

---

## Phase 2: REVIEW THE APPROACH

**This is YOUR independent assessment of the code.** Phase 3 (next) is your response
to EACH reviewer comment. These are distinct — a reviewer may raise something you
missed, or you may disagree with a reviewer. Do not skip Phase 3 even if you feel
you covered the comments during Phase 2.

Read the actual code changes and evaluate the solution against the linked issue.

### Step 1: Read the Changed Files

Read every file touched by the PR. Do not evaluate code you haven't read.

Focus on:
- **Source changes** (`source/slang/`): The core fix
- **Diagnostic definitions** (`slang-diagnostics.lua`): New error codes, message clarity
- **Tests** (`tests/`): Coverage of the fix, edge cases, negative tests

### Step 2: Evaluate Root Cause Fix

Answer these questions:

1. **Does this fix the root cause?** Or does it mask a symptom?
   - A root cause fix prevents the problem from occurring
   - A symptom fix catches the problem after it occurs (e.g., adding a null check
     without asking why the null happened)

2. **Is it optimal?** Could the same result be achieved with less code, fewer edge cases,
   or in a more maintainable way?

3. **Is it focused?** Does the PR do only what's needed, or does it include unrelated
   refactoring, unnecessary abstractions, or speculative features?

4. **Is it correct long-term?** Will this hold up as the codebase evolves, or does it
   rely on fragile assumptions?

5. **Are there missing cases?** Does the fix handle all variants of the problem, or only
   the specific reproducer from the issue?

### Step 3: Evaluate the Diagnostic Messages (if applicable)

For diagnostic-related PRs:
- Is the error message accurate for all cases where it fires?
- Could reusing an existing diagnostic code be misleading in the new context?
- Does the message give actionable guidance to the user?

### Step 4: Evaluate Test Coverage

Check that tests cover:
- The positive case (the bug is fixed / the diagnostic fires)
- Negative cases (the fix doesn't break valid code / the diagnostic doesn't fire incorrectly)
- Edge cases (related patterns, different targets, generic/parametric types)
- The original reproducer from the linked issue
- **All applicable backends**: Tests should have `//TEST` lines for all relevant
  targets, not just one. Target-independent features need at minimum `-cpu` and
  `-spirv`. Flag tests that only test a single backend when the fix applies broadly.

---

## Phase 3: EVALUATE REVIEW FEEDBACK

**CHECKPOINT**: Before proceeding, list every unresolved review thread by ID/file.
Each one MUST appear in the classification table below. If there are zero threads
to evaluate, state that explicitly. Do not skip this phase — even if you feel you
already covered the comments during Phase 2, the structured evaluation ensures
nothing is missed and gives the user a clear action list.

### Step 1: Enumerate all threads

List every unresolved thread: `#{number} — {file}:{line} — {one-line summary}`

### Step 2: Classify each thread

| Category | Criteria | Action |
|----------|----------|--------|
| **Valid + Actionable** | Points to a real bug, missing case, or incorrect behavior | Implement the fix |
| **Valid + Out of Scope** | Correct observation but unrelated to this PR's purpose | Reply acknowledging, don't fix |
| **Valid + Nice-to-Have** | Improves quality but not critical | Implement if easy, otherwise acknowledge |
| **Incorrect** | Based on wrong assumptions about the code | Reply explaining why |
| **Trivial Nitpick** | Style, wording, minor formatting | Apply if trivial, otherwise acknowledge |

### Priority Order

Address feedback in this order:
1. **Bugs / correctness issues** (e.g., missing ErrorType guard, cascading diagnostics)
2. **Missing test coverage** (e.g., negative tests, edge cases)
3. **Diagnostic message accuracy** (e.g., misleading error text)
4. **Code clarity** (e.g., comments, variable names)
5. **Out-of-scope suggestions** (reply only)

### Step 3: Present findings to user

**STOP and present** the full classification table and your recommended actions.
Ask: "Which items should I implement? All actionable, specific items (list numbers), or none?"

The user may also provide additional context or disagree with classifications.

---

## Phase 4: IMPLEMENT FIXES

Only implement items the user approved in Phase 3.

### Step 1: Build if Needed

If you switched from another branch, the binary may be stale. See the `slang-build` skill for
platform-aware build instructions and preset selection.

### Step 2: Make Changes

- Fix one concern per commit when possible
- Follow existing code patterns in the file
- Add comments only for non-obvious logic
- For new diagnostics: choose a code number adjacent to related diagnostics

### Step 3: Test

```bash
# Run the specific test(s) affected by changes
./build/RelWithDebInfo/bin/slang-test tests/path/to/test.slang

# If modifying compiler source, also run related tests
./build/RelWithDebInfo/bin/slang-test tests/path/to/related-tests/
```

### Step 4: Format

```bash
./extras/formatting.sh
```

### Step 5: Commit and Push

One commit per logical fix. Follow commit rules from the `slang-create-issue` skill.
Commit message should reference the review feedback:

```bash
git add <files>
git commit -m "$(cat <<'EOF'
Address review: <short description of what was fixed>

<1-2 lines explaining what changed and why>
EOF
)"

# If the branch already tracks the PR head remote, plain `git push` is enough
git push

# If the branch has no upstream yet, set it explicitly
git push -u <head-remote> HEAD:<headRefName>
```

If push is rejected (remote has new commits), rebase first:

```bash
git pull --rebase
git push
```

---

## Phase 5: SUGGEST REPLIES

**Do NOT post replies automatically.** Present draft replies for user approval.

### Step 1: Draft replies for each thread

For each thread from Phase 3, prepare a suggested reply. Use these formats:

- **Implemented**: "[Agent] Applied. <brief description of what changed>. See <commit-hash>."
- **Out of scope**: "[Agent] Acknowledged. Out of scope for this PR. Will address separately."
- **Won't fix**: "[Agent] This is intentional because <reason>."
- **Question answered**: "[Agent] <concise answer>"

### Step 2: Present to user

Show all draft replies in a table:

| Thread | File | Draft Reply | Post? |
|--------|------|-------------|-------|
| #1 | slang-ir-foo.cpp:123 | [Agent] Applied. Added null check... | ? |
| #2 | slang-ir-foo.cpp:456 | [Agent] Out of scope for this PR... | ? |

**STOP and ask**: "Which replies should I post? All, some (list numbers), or none?"

### Step 3: Post approved replies

Only post the replies the user approved:

```bash
gh api repos/shader-slang/slang/pulls/<number>/comments/<comment-id>/replies \
  -f body="<approved-reply>"
```

### Step 4: Resolve threads (only for posted replies)

Only resolve threads where a reply was posted and the concern was addressed:

```graphql
mutation {
  resolveReviewThread(input: {threadId: "<thread-id>"}) {
    thread { isResolved }
  }
}
```

Do NOT resolve threads when:
- The user chose not to reply
- The reviewer explicitly asked for follow-up discussion
- You're unsure whether the response addresses the concern

---

## Phase 6: REPORT

Present a concise summary to the user:

### When Reviewing a New PR

```
## PR #N Review

**Root cause fix?** Yes/No — [1 sentence explanation]
**Optimal?** Yes/No — [1 sentence]
**Long-term correct?** Yes/No — [1 sentence]

## Review Feedback

| Thread | Feedback | Valid? | Action |
|--------|----------|--------|--------|
| ... | ... | ... | ... |

All N threads resolved / M remain.
```

### When Addressing Feedback

```
## Changes Made (commit <hash>)

| Thread | Action |
|--------|--------|
| ... | ... |

All N threads resolved.
```

---

## Iteration

The user may ask to check for new comments after pushing fixes. Repeat from Phase 1
Step 3 (fetch thread status) — only process unresolved threads.

When iterating:
- Always rebuild if the branch was used by another PR in between
- Only read files that are relevant to the new comments
- Don't re-reply to already-resolved threads

---

## Anti-Patterns

1. **Blindly applying all suggestions**: Evaluate each comment. Bot reviewers sometimes
   suggest changes that are incorrect, out of scope, or unnecessary.

2. **Resolving without replying**: Always reply before resolving. The reply is the record
   of what was done.

3. **Large omnibus commits**: One commit per logical fix makes review easier.

4. **Not rebuilding after branch switch**: The binary in `build/RelWithDebInfo/bin/` corresponds
   to whatever branch was last built. Always rebuild after switching branches.

5. **Ignoring exhaustive test mode**: `DIAGNOSTIC_TEST:SIMPLE` with exhaustive mode catches
   unexpected diagnostics. If adding a negative test case, annotate ALL diagnostics it
   produces, not just the one you're interested in.

6. **Pushing without testing**: Always run the affected tests before pushing. A broken push
   triggers CI and wastes reviewer time.
