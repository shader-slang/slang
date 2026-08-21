# GitHub Actions workflows

This directory holds Slang's GitHub Actions workflows. This README is a
big-picture map: it groups the 62 workflow files by **when they run** and **what
job they do**, so you can find the right file without opening each one.

For narrower, deeper documentation see:

- [`../../docs/ci.md`](../../docs/ci.md) — prose notes on the build/test CI
  (LLVM caching, sccache, which runners run the full test suite).
- [`pr-board-sync.md`](pr-board-sync.md) — architecture of the `pr-*` family
  that keeps the "Slang PR Tracking" ProjectsV2 board in sync.
- [`../pr-board-sync-templates/README.md`](../pr-board-sync-templates/README.md)
  — copy-me caller templates for onboarding another repo to the board sync.

> **Keeping this current:** the tables below are a hand-maintained snapshot;
> nothing regenerates them. When you add, remove, or rename a workflow — or
> change its trigger — update the matching row in the same PR. The `name:` and
> `on:` fields inside each `.yml` remain the source of truth.

## Trigger vocabulary

| Trigger                                        | Meaning                                                                 |
| ---------------------------------------------- | ----------------------------------------------------------------------- |
| `pull_request`                                 | Runs on an open PR, against the PR's merge-preview commit.              |
| `merge_group`                                  | Re-runs in the merge queue, against the queue's tentative merge commit. |
| `pull_request_target`                          | PR events, but run in the base repo's privileged context with secrets.  |
| `workflow_call`                                | Reusable: invoked by another workflow, never triggered on its own.      |
| `schedule`                                     | Cron-driven; the cadence is given in words in each table.               |
| `workflow_dispatch`                            | Run manually from the Actions tab.                                      |
| `workflow_run`                                 | Runs after another named workflow completes (privileged, no checkout).  |
| `repository_dispatch`                          | Triggered by a slash command relayed from a PR comment.                 |
| `push` / `status` / `issues` / `issue_comment` | Plain repository events.                                                |

---

## 1. Per-PR and merge-queue gates

These build, test, or lint a change before it can land. `ci.yml` is the umbrella
that fans out into the reusable build/test workflows of
[group 2](#2-reusable-building-blocks-workflow_call); its `check-ci` job is the
single aggregate status that branch protection and the merge queue require. The
`check-*` workflows are cheap, focused gates that run independently of `ci.yml`;
most are path-filtered so they stay off PRs they cannot be affected by.

### Per-PR versus merge queue — what the difference means

A change is validated **twice**, at two different commits, and the two runs are
distinct GitHub Actions events:

- **Per-PR (`pull_request`)** — fires while the PR is open, on every push and on
  `ready_for_review`. It tests a preview merge of your branch into `master` _as
  master looked when the event fired_. This is the feedback loop you iterate
  against. `ci.yml` skips draft PRs entirely (see its `filter` job), so a draft
  gets no per-PR CI until it is marked ready — that is why the bot dispatches
  `ci.yml` manually while iterating on a draft.
- **Merge queue (`merge_group`)** — fires after the PR is approved and enqueued,
  on the queue's _tentative_ merge commit: your change stacked on top of master
  plus any PRs ahead of you in the queue. It is what catches a semantic conflict
  between two independently-green PRs. Nothing about your branch changed between
  the two runs; the base did.

So a workflow that declares **both** triggers gates the merge itself, while a
workflow that declares **only `pull_request`** is a per-PR-only check — it is
never re-evaluated against the tentative merge commit.

The 1a/1b split below is just a reading of the `on:` blocks as they stand today,
not a policy: which bucket a given check sits in can change with any PR that
edits its triggers. Check the file itself before relying on it.

### 1a. Both per-PR and merge queue (`pull_request` + `merge_group`)

These seven files declare both triggers, so each one runs twice per change: once
on the open PR, and again on the queue's tentative merge commit.

Running in the merge queue is **not** the same as blocking the merge. Only
_required status checks_ do that, and which checks are required is branch
protection configuration — it lives in repo settings, not in this directory. As
configured today, `master` requires exactly three contexts: **`check-ci`** (the
aggregate job in `ci.yml`), **`check-formatting`** (the job in
`check-formatting.yml`), and **`SlangPy Tests`** (the external status posted by
the run that `ci-slangpy-trigger-test.yml` dispatches). Everything else in this
table runs in the queue but is advisory there: if `check-actionlint.yml`,
`check-python-core.yml`, `check-submodules.yml`, or `check-workflow-scripts.yml`
fails on a queue entry, the queue still merges it once those three pass, and the
failed run is left on the temporary `gh-readonly-queue/...` branch that is
deleted right after — so nothing goes red on the PR or on `master`. Look in the
workflow's own run list in the Actions tab to find one.

| Workflow                      | Per-PR trigger                        | Merge-queue trigger | Other triggers      | Purpose                                                                                                                   |
| ----------------------------- | ------------------------------------- | ------------------- | ------------------- | ------------------------------------------------------------------------------------------------------------------------- |
| `ci.yml`                      | `pull_request` (drafts skipped)       | `merge_group`       | `workflow_dispatch` | The build+test umbrella. Skips work on docs-only changes, fans out the build/test matrix, and aggregates into `check-ci`. |
| `check-actionlint.yml`        | `pull_request`                        | `merge_group`       | `workflow_dispatch` | Lints the workflow YAML in this directory with actionlint.                                                                |
| `check-formatting.yml`        | `pull_request`                        | `merge_group`       | —                   | Runs `extras/formatting.sh --check-only`; comment `/format` on the PR to auto-fix.                                        |
| `check-python-core.yml`       | `pull_request` (paths: Python)        | `merge_group`       | —                   | Compile-checks the repo's Python scripts under `extras/` and `tools/compile-perf/`.                                       |
| `check-submodules.yml`        | `pull_request` (paths: `external/**`) | `merge_group`       | —                   | Verifies the `external/**` submodule commit pins are reachable.                                                           |
| `check-workflow-scripts.yml`  | `pull_request` (paths: `.github/**`)  | `merge_group`       | —                   | Runs the unit tests for the JavaScript under `.github/scripts/` that the board-sync workflows embed.                      |
| `ci-slangpy-trigger-test.yml` | `pull_request_target`                 | `merge_group`       | `workflow_dispatch` | Dispatches SlangPy's own CI against this Slang change and reports back as an external commit status.                      |

### 1b. Per-PR only (no `merge_group`)

These run on the open PR and are **never** re-evaluated in the merge queue.

| Workflow                          | Per-PR trigger                               | Other triggers      | Purpose                                                                                        |
| --------------------------------- | -------------------------------------------- | ------------------- | ---------------------------------------------------------------------------------------------- |
| `check-pr-label.yml`              | `pull_request`                               | —                   | Requires exactly one `pr:` classification label on the PR.                                     |
| `check-toc.yml`                   | `pull_request` (paths: `docs/user-guide/**`) | —                   | Checks `docs/user-guide/toc.html` is current; comment `/regenerate-toc` to auto-fix.           |
| `check-spirv-generated.yml`       | `pull_request` (paths: SPIRV externals)      | —                   | Verifies the committed SPIR-V generated files match the pinned SPIRV-Tools/Headers submodules. |
| `check-container-consistency.yml` | `pull_request` (paths: `*-container.yml`)    | `push` (master)     | Verifies every `*-container.yml` workflow pins the same CI container image tag.                |
| `reuse-compliance.yml`            | `pull_request`                               | `push`              | REUSE/SPDX license-header compliance check.                                                    |
| `claude-pr-review.yml`            | `pull_request_target` (paths: source/docs)   | `workflow_dispatch` | Automated Claude review of the PR diff. Advisory, not a gate.                                  |

### 1c. Neither — runs after a CI run completes

| Workflow               | Trigger                                           | Purpose                                                                                                  |
| ---------------------- | ------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| `check-ir-version.yml` | `workflow_run` after `CI`, PR-triggered runs only | Reports the IR-version check result as a PR comment. Does not run the check itself — see the note below. |

The IR-version check runs **inside** CI: `ci-slang-build.yml` invokes
`extras/check-inst-version-changes.sh` on the Linux debug build of a
`pull_request` event, and uploads an `ir-version-check-results` artifact when it
has something to report. `check-ir-version.yml` then fires on that CI run's
completion, downloads the artifact, and creates-or-updates the PR comment; with
no artifact it no-ops. The two are split because posting the comment needs a
privileged token that the build job — which may be running a fork's code — must
not have.

Two more gates live as jobs **inside** `ci.yml` rather than as their own files,
because they reuse an already-built artifact instead of building again:
`check-cmdline-ref` (verifies `docs/command-line-slangc-reference.md` matches
`slangc -help`; `/regenerate-cmdline-ref` auto-fixes it) and
`check-capability-atoms-ref` (verifies
`docs/user-guide/a4-02-reference-capability-atoms.md` matches
`source/slang/*.capdef`).

## 2. Reusable building blocks (`workflow_call`)

These have no trigger of their own — they are called by `ci.yml`, the nightlies,
`cmake-options.yml`, or `sccache-populate.yml` to build or test one matrix
entry. Edit one of these to change how that kind of build or test runs
everywhere at once. The `*-container` variants run inside the Linux CI container
images published by `container-publish-images.yml`.

| Workflow                            | Called by                                    | Purpose                                                                                  |
| ----------------------------------- | -------------------------------------------- | ---------------------------------------------------------------------------------------- |
| `ci-slang-build.yml`                | `ci.yml`, `sccache-populate.yml`             | Build Slang (and optionally LLVM) for one os/compiler/platform/config entry.             |
| `ci-slang-build-container.yml`      | `ci.yml`, `sccache-populate.yml`             | Same, inside the Linux CI container.                                                     |
| `ci-slang-test.yml`                 | `ci.yml`                                     | Run `slang-test` for one platform, with CPU-only / GPU-API-only / GPU-tier variants.     |
| `ci-slang-test-container.yml`       | `ci.yml`                                     | Run `slang-test` on the containerized self-hosted GPU pool.                              |
| `ci-rhi-test.yml`                   | `ci.yml`                                     | Run the slang-rhi test suite for one platform.                                           |
| `ci-rhi-test-container.yml`         | `ci.yml`                                     | Same, on the containerized GPU pool.                                                     |
| `ci-slang-sanitizer.yml`            | `ci.yml`, `nightly-slang-sanitizer-test.yml` | ASan/UBSan-instrumented build and test run.                                              |
| `ci-slang-coverage-test.yml`        | `nightly-slang-coverage-test.yml`            | Instrumented build plus coverage report, optionally deployed to Pages or posted on a PR. |
| `ci-falcor-test.yml`                | `ci.yml`                                     | Compile Falcor's shaders with the new Slang build (self-hosted Windows `falcor` runner). |
| `ci-slang-regression-test.yml`      | `ci.yml`                                     | Compile-regression suite on the self-hosted Windows `regression-test` runner.            |
| `ci-mdl-benchmark-test.yml`         | `ci.yml`                                     | MDL benchmark run on the self-hosted Windows `benchmark` runner.                         |
| `ci-materialx-regression-test.yml`  | `ci.yml`                                     | MaterialX integration/regression test.                                                   |
| `cmake-options-build.yml`           | `cmake-options.yml`                          | Build one CMake-option combination.                                                      |
| `cmake-options-build-container.yml` | `cmake-options.yml`                          | Same, inside the container.                                                              |
| `pr-board-sync.yml`                 | the five `pr-*` callers below                | The whole PR-board reconciliation engine; see [`pr-board-sync.md`](pr-board-sync.md).    |

## 3. Frequent scheduled workflows (sub-daily)

Keep caches warm and keep an eye on CI itself. None of these gate a PR.

| Workflow                   | Cadence                                             | Purpose                                                                                            |
| -------------------------- | --------------------------------------------------- | -------------------------------------------------------------------------------------------------- |
| `ci-health.yml`            | every 15 min (+ manual)                             | Samples GitHub-hosted runner-cap saturation and publishes a CI health signal.                      |
| `sccache-populate.yml`     | every 30 min (+ manual)                             | Builds master through the reusable build workflows purely to populate the sccache entries PRs hit. |
| `ci-retry-yielded-bot.yml` | `workflow_run` after `CI`, hourly at :17 (+ manual) | Reruns bot CI runs that yielded their runner slot to human/merge-queue work.                       |

## 4. Nightly / daily

Longer or noisier suites that would be too expensive per-PR. The staggered UTC
hours are deliberate — they keep the heavy Linux and self-hosted pools from
competing with each other.

| Workflow                           | Cadence (UTC)          | Purpose                                                                                   |
| ---------------------------------- | ---------------------- | ----------------------------------------------------------------------------------------- |
| `nightly-slang-coverage-test.yml`  | daily 02:00 (+ manual) | Full coverage run across platforms; publishes the coverage report.                        |
| `nightly-slang-sanitizer-test.yml` | daily 02:00 (+ manual) | ASan/UBSan run over the full test suite.                                                  |
| `nightly-remix-test.yml`           | daily 03:00 (+ manual) | Compiles all RTX Remix shaders with a fresh Slang build.                                  |
| `nightly-slang-test.yml`           | daily 04:00 (+ manual) | Runs the LLM-generated, doc-anchored suite under `docs/generated/tests/` (Linux, no GPU). |
| `nightly-slang-sascha-test.yml`    | daily 04:00 (+ manual) | Compiles the Sascha Willems Vulkan sample shaders against the latest merge-queue build.   |
| `nightly-mdl-perf-test.yml`        | daily 05:00 (+ manual) | Compile-performance suite for the MDL workloads; publishes tracking data and pages.       |
| `ci-analytics.yml`                 | daily 06:00 (+ manual) | Collects CI run statistics and publishes them to the analytics repo.                      |
| `nightly-slang-vkglcts-test.yml`   | daily 07:00 (+ manual) | Runs the Vulkan CTS (VK-GL-CTS) with Slang as the shader compiler.                        |
| `pr-sweep-nightly.yml`             | daily 07:00 (+ manual) | Board-sync backstop: reconciles every open PR, catching events the per-event path missed. |
| `release-linux-glibc-2-27.yml`     | daily 02:00            | Also nightly — see [group 7](#7-release-and-tag).                                         |
| `release-linux-glibc-2-28.yml`     | daily 02:00            | Also nightly — see [group 7](#7-release-and-tag).                                         |

## 5. Weekly

| Workflow            | Cadence                       | Purpose                                                                                                                                                                                                        |
| ------------------- | ----------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `cmake-options.yml` | Saturday 08:00 UTC (+ manual) | Builds the matrix of non-default CMake option combinations (`.github/cmake-options-matrix.json`). Deliberately not in the merge queue: the 10-job matrix used to starve the org's GitHub-hosted runner budget. |

## 6. PR board sync and bot automation

The `pr-*` workflows are thin callers around the reusable `pr-board-sync.yml`;
each exists because a different GitHub event is the only one that carries a
particular signal (and, for fork PRs, the only one that carries secrets). Read
[`pr-board-sync.md`](pr-board-sync.md) before changing any of them.

| Workflow                    | Trigger                                                      | Purpose                                                                                                                      |
| --------------------------- | ------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------------------- |
| `pr-maintenance.yml`        | `pull_request_target`, `pull_request_review`, `check_suite`  | Main board-sync caller for PR and review events (origin PRs).                                                                |
| `pr-ci-complete.yml`        | `workflow_run` after the gating checks                       | Recomputes board Status when a gating Actions workflow finishes — `check_suite` is not delivered for Actions-created suites. |
| `pr-commit-status.yml`      | `status`                                                     | Recomputes Status when an _external_ commit status (SlangPy, CLA, CodeRabbit) settles.                                       |
| `pr-review-fork-bridge.yml` | `pull_request_review`                                        | Stage 1 of the fork-review relay: unprivileged, no checkout, just completes so stage 2 can fire.                             |
| `pr-review-fork-apply.yml`  | `workflow_run` after the bridge                              | Stage 2: runs the board sync in the privileged base-repo context for fork-PR reviews.                                        |
| `pr-sweep-nightly.yml`      | nightly 07:00 UTC (+ manual)                                 | Sweep-mode backstop over every open PR.                                                                                      |
| `issue-add-labels.yml`      | `issues` opened (+ manual)                                   | Labels new issues `Dev Opened` when the author is in the `shader-slang/dev` team.                                            |
| `claude.yml`                | `issue_comment`, `issues`, `pull_request_review*` (+ manual) | The `@claude` assistant: responds to mentions on issues/PRs and to the `claude` issue label.                                 |
| `claude-ci-analysis.yml`    | `workflow_dispatch`                                          | Given a failed run ID and PR number, analyzes the failure, produces a fix, and pushes it to the PR branch.                   |

## 7. Auto-fix regeneration (slash commands)

`slash-command-dispatch.yml` listens on PR comments and turns an allow-listed
`/command` into a `repository_dispatch` that one of the `regenerate-*` workflows
consumes. Each regenerator regenerates the file and opens a follow-up PR
targeting your PR's head branch, then comments a link to it — so a failing check
from [group 1](#1-per-pr-and-merge-queue-gates) can be fixed by merging that PR
rather than by regenerating locally. Write permission on the repo is required to
invoke one.

| Workflow                     | Comment command           | Regenerates                                                     |
| ---------------------------- | ------------------------- | --------------------------------------------------------------- |
| `slash-command-dispatch.yml` | (the dispatcher itself)   | Relays `/format`, `/regenerate-toc`, `/regenerate-cmdline-ref`. |
| `regenerate-format.yml`      | `/format`                 | Runs `extras/formatting.sh` and commits the result.             |
| `regenerate-toc.yml`         | `/regenerate-toc`         | `docs/user-guide/toc.html`.                                     |
| `regenerate-cmdline-ref.yml` | `/regenerate-cmdline-ref` | `docs/command-line-slangc-reference.md`.                        |

## 8. Release and tag

| Workflow                          | Trigger                                             | Purpose                                                                                                                                                                                    |
| --------------------------------- | --------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `release.yml`                     | `push` tag `v20XX.N*` (+ manual)                    | Builds and publishes the release binaries for every supported os/platform and attaches them to the release.                                                                                |
| `release-linux-glibc-2-27.yml`    | `push` tag `v20XX.N*`, nightly 02:00 UTC (+ manual) | Extra Linux release build against glibc 2.27 (ubuntu18/gcc11) for older distros.                                                                                                           |
| `release-linux-glibc-2-28.yml`    | `push` tag `v20XX.N*`, nightly 02:00 UTC (+ manual) | Extra Linux release build against glibc 2.28.                                                                                                                                              |
| `container-publish-images.yml`    | `push`/`pull_request` on `docker/**` (+ manual)     | Builds and pushes the Linux CI container images to GHCR. PRs validate the tag/version contract only — they never build a Dockerfile, since that would run PR code on a self-hosted runner. |
| `perf-push-benchmark-results.yml` | `push` to master (+ manual)                         | Builds master on the benchmark runner and pushes MDL benchmark numbers to the results repo.                                                                                                |

See the `/slang-release-process` skill (`.claude/skills/slang-release-process/`)
for the human-side release checklist that drives these.

## 9. Manual and special-case

| Workflow                         | Trigger             | Purpose                                                                               |
| -------------------------------- | ------------------- | ------------------------------------------------------------------------------------- |
| `ci-retry.yml`                   | `workflow_dispatch` | Waits for a given run ID to finish, then reruns just its failed jobs.                 |
| `perf-compile-release-sweep.yml` | `workflow_dispatch` | Backfills the compile-performance history by sweeping past releases in a date window. |
| `check-spirv-tools.yml`          | `workflow_dispatch` | Placeholder for a future "SPIRV-Tools tip-of-tree" check; currently a no-op echo job. |

## 10. Composite actions

Shared step bundles under [`../actions/`](../actions/), referenced as
`uses: ./.github/actions/<name>`. They are not workflows and cannot be triggered
on their own.

| Action                | Purpose                                                                                                  |
| --------------------- | -------------------------------------------------------------------------------------------------------- |
| `common-setup`        | The setup shared by every build job: toolchain, submodules, LLVM, sccache, CMake configure.              |
| `common-test-setup`   | The setup shared by test jobs: fetch the build artifact and prepare the test environment.                |
| `setup-llvm-from-gcs` | Downloads the prebuilt LLVM from Google Cloud Storage, building it only on a cache miss.                 |
| `setup-sccache`       | Installs and configures sccache on Linux, macOS, and Windows.                                            |
| `setup-vulkan-icd`    | Overrides the NVIDIA Vulkan ICD to work around a libEGL crash on driver 580.x.                           |
| `format-setup`        | Installs the formatting tools (clang-format, gersemi, prettier, shfmt) for the format check/regenerator. |
| `check-disk-space`    | Fails a job early when free disk space is below a threshold.                                             |
| `claude-code-runner`  | Authentication, setup, execution, and result handling for the Claude-driven workflows.                   |

## 11. Other files in this directory

| File               | What it is                                                                                    |
| ------------------ | --------------------------------------------------------------------------------------------- |
| `pr-board-sync.md` | Design document for the PR board sync — start here before touching any `pr-*` workflow.       |
| `ci-examples.sh`   | Helper script (not a workflow) that runs all the examples in test mode; invoked from CI jobs. |
| `README.md`        | This file.                                                                                    |

Related configuration lives one level up in `.github/`: `actionlint.yaml`
(actionlint config), `cmake-options-matrix.json` (the `cmake-options.yml`
matrix), `scripts/` (JavaScript used by the board-sync workflows, unit-tested by
`check-workflow-scripts.yml`), and `pr-board-sync-templates/`.
