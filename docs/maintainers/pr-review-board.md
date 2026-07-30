# The Slang PR Tracking board (for assignees)

This page is for **maintainers and PR assignees** — the people who get pull
requests routed to them on the shared
[Slang PR Tracking](https://github.com/orgs/shader-slang/projects/13) project
board. It explains what the board's fields mean, what each `Status` tells you,
and when a PR actually needs your attention. (If you're just opening a PR, you
don't need this — see [CONTRIBUTING.md](../../CONTRIBUTING.md).)

> The board is a `shader-slang` org project, so it is only visible to **org
> members (committers)**. Outside contributors can't see it — that's expected;
> their PRs are still tracked on it by the committers who shepherd them.

You never set **Status** by hand: it is maintained automatically from PR
activity (open/close, pushes, reviews, CI results, merge-queue changes). Your
job is to **read the state and act when it asks you to**. **Source** is also
set by automation on first sight, but you may override it when it is wrong
(see below).

## How a PR lands on your plate: the `Source` field

Automation classifies each PR's **`Source`**, then uses that to pick a default
assignee (and reviewers, when appropriate). That is a best-effort default —
not a final verdict.

How Source is classified today:

- **Internal** — the author is a member of the org team the board is configured
  with (`source_internal_team`, by default `shader-slang/source-internal`;
  direct or nested membership, org-wide), **or** of a sibling
  `source-internal-*` team whose description includes `Scope:` listing this
  repository's short name (comma-separated inside brackets; e.g.
  `Scope: [slangpy, slangpy-samples]`). The author is assigned; no reviewer is
  auto-requested (they are expected to find one).
- **Community** — everyone else who is not a bot. A maintainer is assigned to
  shepherd it and arrange review.
- **Bot** — opened by an automated coworker. A maintainer is assigned to
  shepherd it to ready-for-review and merge.

If those team rosters cannot be read, automation leaves `Source` (and the
assignee) blank rather than guessing, and the nightly sweep retries.

Because Internal membership is org-wide (with optional per-repo scoped teams),
Source can still be wrong for a given repo. If it is wrong, change Source on
the board and reassign / request the right reviewers.

If you are the assignee, you are responsible for either driving the PR forward
or finding the correct assignee (and handing it off). Same idea for Internal
authors who still need a reviewer: pick someone who has recently touched the
same files, or ask in the team channel.

## What each `Status` means for you

| Status | What it means | Do you act? |
|---|---|---|
| **In Review** | The default for an open PR: awaiting review, CI still running, or a fresh commit not yet reviewed. (A **Bot draft** sits here too, so you can see and shepherd it.) | **Yes** — review it, or make sure a real reviewer is requested. |
| **Revising** | The author is working: a **human draft**, or a reviewer requested changes. (A **Bot** PR's failed CI also lands here — the bot fixes itself.) | **No** — it's on the author/bot until it moves. |
| **Snagged** | Needs a human's attention: a human PR's **CI failed**, **CI is awaiting your approval to run** (fork PRs from new contributors), or the PR is **approved + green but not in the merge queue** (it fell out, or needs someone to enqueue/merge it). | **Yes** — approve the CI run, help fix CI, or enqueue/merge. |
| **Approved** | Not a draft, already has an approving review, and is waiting only on CI or the merge queue. | **No** — automated; nothing to do. |
| **Done** | The PR is closed (merged or otherwise). Terminal. | No. |

In short: **`In Review` and `Snagged` are the two columns that want you.**
`Revising`/`Approved` are waiting on someone/something else, and `Done` is finished.

## How the state is decided (priority)

When several conditions are true at once, the first matching rule wins:

1. A reviewer's **changes-request** (made on the current commit) → **Revising**.
2. **Draft** → **In Review** (Bot) / **Revising** (human).
3. CI **needs approval** → **Snagged**; CI **failed** → **Revising** (Bot) /
   **Snagged** (human).
4. **Approved**: **Snagged** if CI is green and it's not in the merge queue,
   otherwise **Approved**.
5. Otherwise → **In Review**.

Two things worth knowing:

- A review only counts while it's on the **current** commit. A new push
  supersedes earlier feedback, so the PR returns to **In Review** until it's
  re-reviewed.
- The **only** difference between the Bot and human flows is what a **CI failure**
  does (Bot → `Revising`, human → `Snagged`); everything else is identical.

## The decision, as a flowchart

The board does not move a PR along edges between states — it **recomputes** the
status from the PR's current signals on every event. So the priority rules above
are best read as a decision tree (first match wins):

```mermaid
flowchart TD
  ev([PR event - recompute]) --> closed{"Closed?"}
  closed -->|yes| done["Done"]
  closed -->|no| cr{"Changes requested<br/>on current commit?"}
  cr -->|yes| rev["Revising"]
  cr -->|no| draft{"Draft?"}
  draft -->|"yes (human)"| rev
  draft -->|"yes (bot)"| inrev["In Review"]
  draft -->|no| ciappr{"CI awaiting approval?"}
  ciappr -->|yes| snag["Snagged"]
  ciappr -->|no| cifail{"CI failed?"}
  cifail -->|"yes (bot)"| rev
  cifail -->|"yes (human)"| snag
  cifail -->|no| appr{"Approved?"}
  appr -->|"yes, green and not queued"| snag
  appr -->|"yes, pending or queued"| approved["Approved"]
  appr -->|no| inrev
```

## Source of truth

This flowchart is an illustration. The authoritative rules live in the
`computeTarget` function in
[`.github/workflows/pr-board-sync.yml`](../../.github/workflows/pr-board-sync.yml),
which sets the board `Status` from PR events. If the diagram here ever disagrees
with that function, the function is correct — please fix the diagram.
