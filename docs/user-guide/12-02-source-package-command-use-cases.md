---
layout: user-guide
permalink: /user-guide/source-package-command-use-cases
---

Source Package Command Use Cases
================================

This chapter is a contract for `slang package` / `slang pkg` command behavior. Use it when changing
those commands: every case here should still hold, or this document should change in the same
change. Add a case when you introduce a new workflow, not only a regression test.

Manifest fields and validation rules are in [Slang Source Packages](source-packages). A
command-order walkthrough is in [Using Source Packages](source-package-workflow).

Each case names who is acting, which commands they run, what must succeed or fail, and what must
not happen. Side effects that a command must not perform are as important as the exit status.

## How to use these cases

When you change `tools/slang-package/` or the package unit tests, walk this list and mark which
cases the change touches. If a command starts succeeding or failing in a way this list does not
describe, stop and update the list before merging.

`slang package` flags use a double dash (`--dry-run`, `--skip-validate`). Other Slang tools use a
single dash (`-help`, `-target`).

## Shared invariants

These apply unless a case explicitly carves them out.

- **Fetch does not resolve.** Given a lock, `fetch` reproduces that graph. It does not take newer
  tags, apply retractions, or rewrite the lock.
- **Update is deliberate.** `update` is how a workspace takes newer compatible tags and rewrites
  the committed lock. CI should not run it.
- **One version per package name.** Dependents unify on a single selected version. Path, Git, and
  override rows for the same name must agree on that identity.
- **Tool-owned checkouts are replaceable; edited ones are not.** Fetch and update refuse to replace
  an unregistered checkout that has changed files, extra commits, or stashes unless `--clean` is
  passed. `edit` records that the tool no longer owns the tree.
- **Validation is graph-wide.** License, export, first-declaration (`module` / `implementing`), and
  import-uniqueness checks cover every reachable package, not only lock rows that changed.
- **`--skip-validate` is an escape hatch, not a CI flag.** It still checks lock identity against
  declared dependencies and materialized manifests. It skips licenses, first-declaration placement,
  and import uniqueness, and it prints a warning. `validate` has no skip flag.

## 1. CI reproduces a committed lock

**Who:** A builder or CI job on a clean clone that already has `slang-package-lock.json`.

**Commands:** `slang package fetch`, then `slang package build`. Not `update`.

**Must:**

- Fetch materializes the locked Git commits and path locations.
- Build compiles from that graph after full validation.
- A missing lock is an error that tells the user to run `update` once locally.

**Must not:**

- Fetch rewrite the lock or select a newer tag than the lock records.
- CI run `update` as part of the default build.

## 2. Developer takes newer compatible releases

**Who:** A developer whose constraints or upstream tags changed.

**Commands:** `slang package update --dry-run`, then `slang package update` after review.

**Must:**

- Dry-run print the selected graph (what moved, what stayed, and why) without writing the lock or
  replacing checkouts.
- A real update write the lock only after the selected graph materializes and passes full
  validation.
- Default report include rationale; `--minimal` keep one-line package lines, including unchanged
  packages, plus the summary count.

**Must not:**

- Combine `--dry-run` with `--clean`.
- Treat dry-run as a full rehearsal of source validation for unmaterialized remote candidates.
  Dry-run validates the workspace package and solver-read manifests; it cannot read remote source
  trees it did not check out. A clean dry-run may still be followed by a failing real update.

## 3. Init is not yet a valid package

**Who:** Someone who just ran `slang package init`.

**Commands:** `init`, then `validate` / `update` / `fetch` / `build`.

**Must:**

- `init` create the manifest, conventional directories, and a `LICENSE` file with placeholder
  text.
- `validate`, `update`, `fetch`, and `build` fail until that placeholder is replaced with a real
  license.
- After replacing the license and adding valid `module` sources, those commands can succeed.

**Must not:**

- Skip the placeholder check on the default (non-`--skip-validate`) path.

## 4. Update can repair a broken old graph

**Who:** A workspace whose current checkout or locked release has invalid source, and a newer
release or local path that is valid.

**Commands:** `slang package update` (without first requiring the old graph to validate).

**Must:**

- Validate the workspace package before solving.
- Not require the previous dependency trees to pass module-layout or license checks.
- Validate the newly selected graph after materialization, then write the lock only if that graph
  is valid.

**Must not:**

- Trap the user on a bad old release by pre-validating the previous closure.

## 5. Invalid module layout cannot land in a successful graph

**Who:** A workspace or dependency whose companion file starts with `module` instead of
`implementing`, or whose primaries collide on import path (including case-only collisions).

**Commands:** `update`, `fetch`, `build`, `validate`.

**Must:**

- Default `update` fail and leave `slang-package-lock.json` unwritten when the selected graph is
  invalid.
- Default `fetch` fail after materialization when a locked package's layout is invalid.
- Default `build` fail before compilation.
- `validate` fail with the same class of error.

**Must not:**

- Report a successful fetch or update for a graph that `validate` would reject.
- Check only changed lock rows. An unchanged package can still collide with a newly selected
  module.

## 6. Skip-validate is a workaround, not a policy change

**Who:** A developer blocked by a validation bug or a temporarily invalid tree who still needs to
lock or fetch.

**Commands:** `update --skip-validate`, `fetch --skip-validate`, `build --skip-validate`.

**Must:**

- Still fail on a missing lock (`fetch`), lock/manifest identity mismatches, and unreadable
  materialized manifests.
- Skip license placeholder, first-declaration, and import-uniqueness checks.
- Print a warning that validation was skipped.
- Still collect an export inventory for `build` so compilation can be attempted.

**Must not:**

- Exist on `validate`.
- Be the documented CI path.
- Skip Git dirty-checkout protection or `--clean` rules.

## 7. Status diagnoses; it does not mutate

**Who:** A developer asking whether the workspace matches the lock.

**Commands:** `slang package status`.

**Must:**

- Check the root manifest against the lock, registered edits and overrides, materialized
  manifests, and that tool-owned Git checkouts are at locked commits without changed files or
  stashes.
- Name a corrective command when something is wrong (`fetch`, `update`, `update --from-local`,
  `edit`, or `--clean`).
- Fail if the lock exists but packages are not materialized yet.

**Must not:**

- Fetch, update, write files, or contact remotes.
- Be treated as a substitute for `validate`. Status can report that materialized manifests match
  the lock while module layout would still fail `fetch` or `build`.

## 8. Dirty tool-owned checkouts are not silently replaced

**Who:** A developer who edited files under `deps/NAME` without `edit`.

**Commands:** `fetch`, `update`.

**Must:**

- Refuse to replace that checkout.
- Succeed if `--clean` is passed and replacement is intended.

**Must not:**

- Combine `--clean` with `update --dry-run`.

## 9. Edit keeps a checkout out of tool ownership

**Who:** A developer iterating on a dependency in place.

**Commands:** `edit NAME`, then `fetch` / `update`, then `unedit NAME`.

**Must:**

- Leave the checkout at `deps/NAME` (or the configured deps directory).
- Record the edit in gitignored `slang-workspace.json`.
- Leave an edited checkout untouched by later fetch and update.
- Refuse `unedit` while the checkout has changed files, extra commits, or stashes.

**Must not:**

- Move the checkout to a new path for the edit.

## 10. Overrides are local and must be solved explicitly

**Who:** A developer pointing a package name at a local directory.

**Commands:** `override`, `update --from-local`, `unoverride`, then a normal `update` to restore a
Git pin.

**Must:**

- Require `update --from-local` when local manifests should become the lock's source of truth.
- Keep `slang-workspace.json` required while override or edit state exists.
- Restore a portable Git pin with a normal `update` before removing the override.

**Must not:**

- Treat `fetch` as the command that adopts a new local manifest graph.

## 11. Path dependencies stay put; Git dependencies materialize under deps

**Who:** A workspace that mixes `path` + `as` with Git `version` or `ref` + `as`.

**Commands:** `update`, `fetch`.

**Must:**

- Keep path packages at their locked relative locations.
- Check Git packages out under `{workspace.deps}/NAME` (default `deps/`).
- Record an exact `version` on every lock row. Git rows also record `ref` and `commit`.

**Must not:**

- Copy a path dependency into `deps/` as if it were a Git pin.

## 12. Workspace excludes and publisher retractions

**Who:** A root that lists `workspace.excludes`, or a publisher that lists `retractions`.

**Commands:** `update` to select; `fetch` to reproduce.

**Must:**

- `update` skip excluded or retracted versions when choosing a new graph.
- `fetch` keep reproducing an existing lock even after a publisher adds a retraction, until
  someone runs `update`.
- `fetch` reject a lock that still selects a version the *workspace* now excludes, because that
  lock is stale relative to declared intent.

**Must not:**

- Put a personal exclude in `slang-workspace.json`. Excludes live on the workspace manifest.

## 13. Toolchain constraint is local, not fetched

**Who:** A package that declares `tools.slang-toolchain.version`.

**Commands:** `update`, `fetch`, `validate`, `build`.

**Must:**

- Intersect workspace and reachable-package toolchain constraints against the installed compiler.
- Fail when the installed compiler is outside that intersection.

**Must not:**

- Fetch a compiler, or put the toolchain in the lock as a package row. The lock may mention the
  installed toolchain in reports; it is not a Git dependency.

## 14. Build consumes a valid graph; run does not rebuild

**Who:** A developer producing bundle modules, host executables, or docs.

**Commands:** `build`, then `run [name] [args...]`, then `docs`.

**Must:**

- `build` run full graph validation first (unless `--skip-validate`).
- Compile workspace `host.executables` only from workspace primaries, not from a dependency's
  `host` section.
- `run` execute the already-built binary and fail with a build reminder if it is missing.
- `docs` print the generated documentation directory and not copy or regenerate files.

**Must not:**

- `run` silently rebuild.
- `docs` mutate the tree.

## 15. Failed update is not yet transactional

**Who:** Anyone whose real `update` materializes a candidate and then fails validation.

**Must (current behavior):**

- Leave the previous lock unwritten / unchanged.
- Surface the validation error.

**Known gap:**

- Dependency directories may already have been replaced. If a previous lock exists, `fetch`
  restores it. A first `update` that fails can leave a partial tree and no lock. Staging checkouts
  and writing them only after validation is a separate change; do not pretend it already works.

## Mapping to tests

These unit tests are the current executable anchors. They do not replace the cases above; they
are where a change should usually land a regression.

| Case | Tests to start from |
| --- | --- |
| 1 CI fetch | `PackageToolFetchRequiresLock` |
| 2 Update / dry-run / report | `PackageToolUpdateDryRun`, `PackageResolveReportFormat` |
| 3 Init license | `PackageToolInit`, `PackageValidateStructureAndLicense` |
| 5 Layout and uniqueness | `PackageCommandsValidateDependencyModuleLayout`, `PackageToolUpdateRejectsBundleCaseConflict`, `PackageValidateRejectsFlattenedModuleAlias` |
| 6 Skip-validate | `PackageCommandsValidateDependencyModuleLayout` |
| 7 Status | `PackageToolUpdateDryRun` (status after a valid graph) |
| 10 Overrides | `PackageToolLocalOverrideUpdatesDefinitiveLock` |
| 11 Path dependencies | `PackageToolPathDependencies` |
| 12 Excludes / retractions | `PackageResolverAppliesWorkspaceExclusions`, `PackageToolFetchRejectsWorkspaceExclusion` |
| 13 Toolchain | `PackageToolSlangToolchain`, `PackageResolverSlangToolchain` |
| 14 Build / run | `PackageToolBuild`, `PackageToolRun`, `PackageToolExecutableRequiresWorkspaceSource` |
