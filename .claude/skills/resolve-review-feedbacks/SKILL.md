---
name: resolve-review-feedbacks
description: Resolve GitHub PR review feedback and CI failures. Use when asked to monitor a PR, reply to and resolve LLM review threads, leave human review threads for human resolution, fix failing checks, rebase merge conflicts, and push updates until the PR is clean.
argument-hint: "<PR URL or number>"
allowed-tools:
  - Bash
  - Read
  - Write
  - Edit
  - Grep
  - Glob
---

# Resolve GitHub Review Feedback

Use this skill to keep a GitHub PR moving until all CI checks pass, LLM review threads have been addressed and resolved by the agent, and human-owned threads have been resolved by their reviewers.

## Prerequisites

- GitHub CLI (`gh`) is installed and authenticated for the PR repository.
- The `gh` token can read PR reviews/checks and push to the PR branch.
- A PR URL or PR number is provided in `$ARGUMENTS`. If it is missing, ask the user for the PR.

Check before making changes:

```bash
gh auth status
git status --short
gh pr view "$PR" --json number,title,url,baseRefName,headRefName,headRepository,headRepositoryOwner,mergeStateStatus,isDraft
```

Do not overwrite unrelated local changes. If the worktree is dirty, inspect the changes and preserve the user's work.

## Main Loop

Repeat this workflow every 30 minutes until the PR has no unresolved review feedback and all required checks pass. Use `sleep 1800` between polling iterations when there is nothing immediate to fix.

1. Check out the PR branch:

   ```bash
   gh pr checkout "$PR"
   git fetch --all --prune
   ```

2. Inspect PR state, checks, mergeability, and review threads.
3. Fix actionable review feedback and CI failures.
4. Commit PR modifications as new commits and push them to the PR branch.
5. Reply to LLM review feedback and resolve only the LLM-owned threads that have been addressed.
6. Leave human-owned threads unresolved for the human reviewer to resolve manually.
7. Wait for CI and repeat until clean.

Stop early only if blocked by missing credentials, missing push permission, an ambiguous human decision, or local changes that cannot be safely preserved.

## Commit Policy

When the PR is modified for any reason, preserve the change history by creating a new commit for the modification. Do not use `git commit --amend` for review fixes, CI fixes, conflict-resolution follow-up edits, formatting changes, or any other PR update.

Use concise commit messages that describe the reason for the follow-up change, for example:

```bash
git add <changed-files>
git commit -m "Address review feedback"
git push
```

## Review Threads

Use GitHub GraphQL to list review threads, because `gh pr view` does not expose all thread resolution state:

```bash
PR_NUMBER="$(gh pr view "$PR" --json number --jq .number)"
PR_URL="$(gh pr view "$PR" --json url --jq .url)"
OWNER_REPO="$(printf '%s\n' "$PR_URL" | sed -E 's#https://github.com/([^/]+/[^/]+)/pull/[0-9]+#\1#')"
OWNER="${OWNER_REPO%/*}"
REPO="${OWNER_REPO#*/}"

gh api graphql -F owner="$OWNER" -F repo="$REPO" -F pr="$PR_NUMBER" -f query='
query($owner:String!, $repo:String!, $pr:Int!) {
  repository(owner:$owner, name:$repo) {
    pullRequest(number:$pr) {
      reviewThreads(first:100) {
        pageInfo { hasNextPage endCursor }
        nodes {
          id
          isResolved
          isOutdated
          path
          line
          startLine
          comments(first:100) {
            nodes {
              id
              url
              body
              author { login __typename }
              createdAt
            }
          }
        }
      }
    }
  }
}'
```

Classify threads conservatively:

- **LLM review feedback**: the author is clearly an automated LLM reviewer, such as Copilot, CodeRabbit, Claude, Codex, OpenAI, or another bot whose comment identifies itself as AI review feedback.
- **Human feedback**: the author is a person, or the source is ambiguous.
- **CI/static-analysis bot output**: handle it as CI feedback unless it is clearly an LLM review thread.

For each unresolved LLM thread:

1. Read the full thread and relevant code.
2. Apply the fix, or determine that the suggestion is invalid with evidence.
3. Run focused validation.
4. Push the fix if code changed.
5. Reply on the thread with what changed, what validation ran, or why no code change was needed.
6. Resolve the thread only after the reply is posted and the issue is actually addressed.

Reply to an LLM thread:

```bash
gh api graphql -F thread="$THREAD_ID" -F body="$REPLY_BODY" -f query='
mutation($thread:ID!, $body:String!) {
  addPullRequestReviewThreadReply(input:{pullRequestReviewThreadId:$thread, body:$body}) {
    comment { url }
  }
}'
```

Resolve an addressed LLM thread:

```bash
gh api graphql -F thread="$THREAD_ID" -f query='
mutation($thread:ID!) {
  resolveReviewThread(input:{threadId:$thread}) {
    thread { id isResolved }
  }
}'
```

For human threads, do not mark them resolved. If you fixed the issue, reply with a concise summary and ask the reviewer to resolve the thread if satisfied.

If `pageInfo.hasNextPage` is true, paginate and inspect every review thread before deciding that the PR has no remaining feedback.
For pagination, repeat the query with an `$after:String` variable and `reviewThreads(first:100, after:$after)` set to the previous `endCursor`.

## CI Failures

Inspect checks with:

```bash
gh pr checks "$PR"
gh run list --branch "$(git branch --show-current)" --limit 10
gh run view "$RUN_ID" --log-failed
```

For each failure:

1. Identify the failing job and command from the logs.
2. Reproduce locally when feasible.
3. Fix the code or test.
4. Run the narrowest reliable validation first, then broader validation when the change warrants it.
5. Push to the PR branch.
6. Continue monitoring until the new checks finish.

If checks are still running and there is no review work to do, wait for them:

```bash
gh pr checks "$PR" --watch
```

## Merge Conflicts And Auto-Rebase Failures

If GitHub reports that auto-merge or auto-rebase cannot continue because conflicts must be resolved, update the PR branch manually.

Inspect merge state:

```bash
gh pr view "$PR" --json baseRefName,headRefName,mergeStateStatus,headRepository,headRepositoryOwner
```

Resolve by rebasing onto the latest base branch:

```bash
BASE="$(gh pr view "$PR" --json baseRefName --jq .baseRefName)"
HEAD_BRANCH="$(gh pr view "$PR" --json headRefName --jq .headRefName)"
git fetch upstream "$BASE"
git rebase "upstream/$BASE"
```

Resolve conflicts in the files, then continue:

```bash
git add <resolved-files>
git rebase --continue
```

Run relevant validation, then push with a lease:

```bash
git push --force-with-lease origin "HEAD:$HEAD_BRANCH"
```

If the PR branch belongs to a fork or a non-`origin` remote, push to the remote reported by `gh pr view` only after confirming the local remote matches that owner/repository.

## Completion Criteria

The skill is complete only when all of these are true:

- `gh pr checks "$PR"` shows all required checks passing.
- There are no unresolved LLM review threads.
- There are no unresolved human review threads left. Human-owned threads must be resolved by a human reviewer, not by the agent.
- `gh pr view "$PR" --json mergeStateStatus` does not report a conflict state.
- All local commits needed for the fixes have been pushed to the PR branch.

If only unresolved human threads remain, report that the PR is waiting for manual human resolution and continue the 30-minute polling loop if the user asked for continuous monitoring.
