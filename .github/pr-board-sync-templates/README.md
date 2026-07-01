# PR board-sync onboarding templates

These are **copy-me** caller workflows for onboarding any `shader-slang` repo to
the shared **"Slang PR Tracking"** ProjectsV2 board. They live here (outside
`.github/workflows/`, so GitHub does not execute them) next to the reusable
workflow they call, [`../workflows/pr-board-sync.yml`](../workflows/pr-board-sync.yml),
so the templates and the engine stay in sync.

A reusable workflow cannot declare `on:` triggers itself, so each consuming repo
keeps a thin caller that declares the triggers and maps the one secret. All board
IDs default to the shared board, so callers pass only `SLANG_PR_BOT_TOKEN`.

`shader-slang/slang` itself consumes the same reusable workflow via the local
`./.github/workflows/pr-board-sync.yml` path; these templates are the cross-repo
form (`uses: shader-slang/slang/.github/workflows/pr-board-sync.yml@master`).
Each template mirrors one of slang's own live callers, so this directory is also
the reference for what slang runs.

## The templates

| Template | Copy to | Customize? | Purpose |
|---|---|---|---|
| `example-pr-maintenance.yml` | `.github/workflows/pr-maintenance.yml` | no | PR lifecycle + reviews (`pull_request_target`, `pull_request_review`, `check_suite`). |
| `example-pr-checks-complete.yml` | `.github/workflows/pr-checks-complete.yml` | **yes** — list this repo's workflow names | CI / gating-check results via `workflow_run` (Actions CI does not emit usable `check_suite`). |
| `example-pr-commit-status.yml` | `.github/workflows/pr-commit-status.yml` | no | External commit statuses via the `status` event. |
| `example-pr-review-fork-bridge.yml` | `.github/workflows/pr-review-fork-bridge.yml` | no | Optional: stage 1 of the real-time fork-PR review relay. |
| `example-pr-review-fork-apply.yml` | `.github/workflows/pr-review-fork-apply.yml` | no | Optional: stage 2 (privileged) of the fork-PR review relay. |

## Onboarding a repo

1. Copy `example-pr-maintenance.yml`, `example-pr-commit-status.yml`, and
   `example-pr-checks-complete.yml` into the repo's `.github/workflows/`
   (dropping the `example-` prefix).
2. In `pr-checks-complete.yml`, replace the `workflows:` list with **this repo's**
   gating workflow `name:`s (at minimum its CI workflow). `workflow_run` can only
   reference workflows in the same repo, by name. Do **not** list the board-sync's
   own callers, or you create a `workflow_run` loop.
3. (Optional, for real-time fork-PR reviews) also copy the two
   `example-pr-review-fork-*.yml` files.
4. Ensure the org-level secret `SLANG_PR_BOT_TOKEN` (a dedicated bot PAT with org
   Projects RW + Members read; repo Contents read, Pull requests write, Issues
   write, Metadata read) is available to the repo. The callers map only that
   secret; the reusable workflow uses it for every call, so callers need no
   `permissions:` block.

The nightly sweep (`pr_sweep.py`, run on a cadence) remains the idempotent
backstop for anything the per-event path cannot see (missed webhooks, fork-PR CI,
repos not yet onboarded), and reconciles the whole board.

## Why `workflow_run`/`status` instead of `check_suite` for CI

GitHub deliberately does **not** deliver `check_suite`/`check_run` events for
suites created by GitHub Actions, to prevent recursive workflow runs. Since these
repos' CI is GitHub Actions, a `check_suite` trigger never fires for it. The
events that *are* delivered:

- **`workflow_run`** — fires when an Actions workflow completes; carries CI/check
  results. Used by `pr-checks-complete.yml` (and the fork-review relay).
- **`status`** — fires for external commit statuses (posted by apps/PATs, not the
  recursion-suppressed `GITHUB_TOKEN`). Used by `pr-commit-status.yml`.

`check_suite` is kept in `pr-maintenance.yml` only to catch any non-Actions,
GitHub-App-created check suites a repo might have.
