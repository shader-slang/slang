---
layout: user-guide
permalink: /user-guide/source-package-command-reference
---

Slang Package Command Reference
===============================

## Name

`slang package` — manage Slang source-package dependencies, local overrides, locked source trees,
and package build outputs.

`slang pkg` is an equivalent short form. The installed `slang-package` executable accepts the same
arguments.

## Synopsis

```text
slang package [--experimental] <command> [<arguments>]
slang pkg [--experimental] <command> [<arguments>]
slang-package [--experimental] <command> [<arguments>]
```

## Description

The package tool manages source dependencies described by `slang-package.json`. It resolves Git
release tags and local path dependencies into `slang-package-lock.json`, materializes locked Git
source under `deps/` by default, and records machine-local overrides in
`slang-package-overlay.json`.

Except for `init` and `help`, commands may be run from the package root or any ordinary
subdirectory. The tool searches parent directories for the nearest `slang-package.json`; a nested
package therefore forms its own workspace.

Package commands do not change Slang's `import` syntax. `fetch`, `update`, and local-registration
commands regenerate `slang-package-includes.txt` for compiler sessions that consume the
materialized graph. `build`
creates a flattened source bundle and documentation. Experimental build mode can additionally
create `.slang-module` files and native host executables.

## Global option

### `--experimental`

Enable experimental command behavior. This option must appear before the command:

```text
slang package --experimental build
slang package --experimental run --binary
```

It enables `.slang-module` and host-executable output during `build`, and the `--binary` form of
`run`. It does not change dependency resolution.

## Commands

The commands are:

- [`help`](#help) — show a command summary.
- [`init`](#init) — create a package skeleton.
- [`dependency`](#dependency) — add, pin, remove, or list direct dependencies.
- [`override`](#override) — register and control machine-local package trees.
- [`edit`](#edit) — make a materialized Git dependency editable in place.
- [`unedit`](#unedit) — return an edited dependency to package-tool ownership or adopt its commit.
- [`fetch`](#fetch) — reproduce and materialize the lock.
- [`update`](#update) — resolve dependencies and rewrite the lock.
- [`status`](#status) — report lock, checkout, and graph readiness.
- [`validate`](#validate) — check that a package is suitable for sharing.
- [`tree`](#tree) — print the selected dependency graph.
- [`why`](#why) — explain why a package is in the selected graph.
- [`build`](#build) — validate the graph and create package outputs.
- [`run`](#run) — run a configured host program from existing build output.
- [`docs`](#docs) — open or print the generated documentation index.
- [`test`](#test) — reserved; package testing is not implemented.

## `help`

### Synopsis

```text
slang package help
slang package -help
slang package --help
slang package --experimental help
```

### Description

Print a concise list of available commands. The experimental form also shows experimental build
and run behavior. There is currently no command-specific `help <command>` form.

## `init`

### Synopsis

```text
slang package init
```

### Description

Create a package skeleton in the current directory. Unlike other package commands, `init` does not
search parent directories for an existing package.

The directory name becomes the package name and must be a valid package name. The command fails if
`slang-package.json` already exists.

### Files and directories created

`init`:

- creates `src/`, `tests/`, `docs/`, `deps/`, and `build/`;
- writes `slang-package.json` with `src` as its export and `LICENSE` as its license file;
- sets the default workspace dependency and build directories to `deps/` and `build/`;
- records the installed Slang version as a minimum `tools.slang-toolchain` requirement when that
  version can be determined;
- writes a placeholder `LICENSE` if one does not already exist; and
- adds `.slang/`, `deps/`, `build/`, `slang-package-overlay.json`, and
  `slang-package-includes.txt` to `.gitignore`.

The generated license is intentionally a reminder, not a distributable license. Replace its
contents before running bare `validate`.

## `dependency`

### Description

Manage direct dependency declarations in the workspace `slang-package.json`. These subcommands do
not resolve the graph, rewrite the lock, or materialize checkouts. Run `status` to inspect the
result and `update` to resolve it.

Direct dependencies are kept in package-name order.

### `dependency add`

#### Synopsis

```text
slang package dependency add <name> --git <url> --version <range>
slang package dependency add <name> --git <url> --ref <ref> [--as <version>]
slang package dependency add <name> --path <path> --as <version>
```

#### Description

Add a direct dependency, or replace the existing direct declaration with the same name. Exactly
one of the three source forms must be used:

- **Git version range** selects a semantic-version release tag from the Git repository.
- **Git ref** pins a branch, tag, or commit. `--as` declares the exact semantic version that the
  selected source provides. When omitted, the resolver derives it from the nearest
  `vMAJOR.MINOR.PATCH` tag reachable from the resolved commit.
- **Path** uses a local directory in place and requires an exact version through `--as`.

`<name>` identifies the package in the graph and must match the selected package manifest.

#### Options

`--git <url>`
: Set the Git repository. The location cannot begin with `-`, contain whitespace or control
characters or quotes, or use Git's command-executing `ext::` transport.

`--version <range>`
: Select a compatible release tag. Versions are written without a `v` prefix. Whitespace joins
comparisons with AND and `||` joins alternatives, for example
`>=1.2.0 <2.0.0 !=1.4.0`.

`--ref <ref>`
: Select an opaque Git branch, tag, or commit instead of choosing a semantic-version release.

`--as <version>`
: Declare the exact `MAJOR.MINOR.PATCH` version provided by a Git ref or path dependency.

`--path <path>`
: Select a local package directory. Relative paths are resolved from the package that declares
them.

### `dependency pin`

#### Synopsis

```text
slang package dependency pin <name>
slang package dependency pin <name> --to <version>
slang package dependency pin <name> --commit
```

#### Description

Write a direct Git declaration from the package's current lock selection. An existing direct
declaration is replaced; a transitive package is promoted to a direct dependency. A lock file and
a Git-backed lock row are required.

Without an option, the command writes the lock's exact version as the manifest's Git version
constraint. `--to` writes another exact version. `--commit` writes the locked commit as `ref` and
the locked version as `as`, preventing a later update from following a moved tag.

The command changes only `slang-package.json`; it does not rewrite the current lock. An active
override remains registered.

#### Options

`--to <version>`
: Pin the declaration to the exact version supplied. It cannot be combined with `--commit`.
When the lock currently selects an override, this option is required because the local version
need not identify a published Git release.

`--commit`
: Pin the declaration to the exact locked commit. This is unavailable for a path-only or overlaid
lock row because such a row does not provide an independently selected Git commit.

### `dependency remove`

#### Synopsis

```text
slang package dependency remove <name>
```

#### Description

Remove a direct dependency from `slang-package.json`. The command fails if the manifest does not
declare `<name>`. It does not remove lock rows or checkouts; run `update` to resolve and
materialize the changed graph.

### `dependency list`

#### Synopsis

```text
slang package dependency list
```

#### Description

List direct dependencies from `slang-package.json`, including each Git version range, Git ref and
optional provided version, or path and provided version. It does not display transitive
dependencies; use `tree` for the selected graph.

## `override`

### Description

Manage local package substitutions in gitignored `slang-package-overlay.json`. Overrides are
machine-local development state, not publishable dependency declarations. An enabled override's
working-tree manifest and effective version participate in normal resolution.

Override commands do not copy or modify the registered directory. After changing the selected
local or published graph, run `update`.

### `override add`

#### Synopsis

```text
slang package override add <name> <path> [<version>]
```

#### Description

Register an existing local package directory as an enabled override. `<path>` may be absolute or
relative to the workspace, but must be on the same filesystem. The directory must contain a valid
`slang-package.json` whose package name is `<name>`.

The workspace must already have a lock file. The package itself does not need to be in that lock
when an explicit version is supplied.

The optional version is an exact `MAJOR.MINOR.PATCH` version used for solver compatibility. When
omitted, the command uses the package's current locked version. Supply it when the package is not
already in the lock.

Using the workspace checkout path, such as `deps/<name>`, updates the same in-place registration
created by `edit`; it does not create a second kind of local state.

### `override enable`

#### Synopsis

```text
slang package override enable <name>
```

#### Description

Enable a registered override. The registration is retained, and the next normal `update` selects
the local tree if it satisfies all incoming constraints.

### `override disable`

#### Synopsis

```text
slang package override disable <name>
```

#### Description

Disable a registered override without forgetting its path or provided version. The next `update`
selects the published graph. A disabled in-place checkout is subject to the normal ownership and
dirty-tree checks; update still refuses to discard local work unless `--clean` permits it.

### `override remove`

#### Synopsis

```text
slang package override remove <name>
```

#### Description

Remove an out-of-tree override registration. The command refuses to remove an in-place edit; use
`unedit` instead. It also refuses while the lock still selects the local path. Disable the
override and run `update` before removing that registration.

### `override list`

#### Synopsis

```text
slang package override list
```

#### Description

List every registered override with its enabled state, path, and effective version. Disabled
registrations are included.

## `edit`

### Synopsis

```text
slang package edit <name>
```

### Description

Turn the existing materialized Git checkout under `workspace.dependencies` (`deps/<name>` by
default) into an enabled in-place override. The
checkout stays at the same path, so current search paths remain usable, but `fetch` and `update`
stop treating the tree as package-tool-owned.

The package must be in the lock and already materialized. For a Git package, the checkout must
still have the origin recorded by the lock. Local file changes are allowed: preserving existing
work is a primary use of this command.

Path-only dependencies are already editable in place and do not need `edit`.

## `unedit`

### Synopsis

```text
slang package unedit <name>
slang package unedit <name> --clean [--yes]
slang package unedit <name> --adopt [--ref <ref>] [--as <version>] [--yes]
```

### Description

Remove the in-place override created by `edit`.

Plain `unedit` succeeds only when the lock is a Git-only pin and the checkout is clean at the
locked commit. It leaves the checkout in place and returns ownership to the package tool.

#### Options

`--clean`
: Discard uncommitted files, stashes, and commits that differ from the lock, restore the locked
commit from `.slang/cache`, and remove the edit registration. The command asks for confirmation
only when state must be discarded.

`--adopt`
: Keep the editable checkout's committed `HEAD`. The package must be a direct Git dependency, its
files must be committed, and it must have no stashes. The command rewrites both manifest and lock
as a Git pin and removes the edit registration.

`--ref <ref>`
: With `--adopt`, keep following a branch or tag that currently points at `HEAD`. A commit ID is
not accepted here; omit `--ref` to freeze `HEAD`.

`--as <version>`
: With `--adopt`, declare the exact semantic version provided by `HEAD`. If omitted, the command
uses a unique release tag at `HEAD`, or the nearest `vMAJOR.MINOR.PATCH` ancestor. It fails when
no such tag exists.

`--yes`
: Approve the destructive or manifest-changing operation without an interactive prompt. It
requires either `--clean` or `--adopt`.

`--clean` and `--adopt` cannot be combined. Adoption does not copy `HEAD` into `.slang/cache`;
push the commit to the package origin before expecting `validate`, `fetch` in a fresh workspace,
or other upstream checks to find it.

## `fetch`

### Synopsis

```text
slang package fetch [--clean] [--yes] [--skip-validate]
```

### Description

Reproduce the committed lock without selecting newer versions. The command refreshes each Git
repository under `.slang/cache` from its origin, verifies that every locked ref and commit is
available there, validates the locked graph, and materializes Git packages at their locked commits
under the configured dependency directory.

Path dependencies and enabled overrides remain in place. A clean tool-owned checkout already at
the correct origin and commit is left untouched. The command regenerates
`slang-package-includes.txt`.

When dependencies exist but the lock is absent, `fetch` runs `update` to create the first lock and
uses the same confirmation behavior. When neither a lock nor a dependency graph exists, use
`update` to create an empty lock.

Fetch installs the commit recorded in the lock even when its original tag has since moved. It
does not select newer releases or apply publisher retractions. Workspace exclusions are current
workspace policy and can make an existing lock stale.

### Options

`--clean`
: Permit replacement of a dirty, unowned, wrong-origin, or wrong-commit tool-owned checkout.
Destructive replacements are listed before confirmation.

`--yes`
: Approve required checkout or moved-ref changes without an interactive prompt. With no changes
requiring approval, it has no effect.

`--skip-validate`
: Skip source-layout checks and publishability checks for newly changed package trees after
materialization. The legal workspace-graph and upstream-cache checks still run. The command
prints a warning.

Without `--clean`, `fetch` stops before modifying any tree when a tool-owned checkout contains
local state. Use `edit` to preserve that work or `--clean` to discard it.

## `update`

### Synopsis

```text
slang package update [--ignore-overrides] [--clean] [--dry-run]
                     [--minimal] [--offline] [--yes] [--skip-validate]
```

### Description

Resolve all manifests reachable from the workspace, select compatible package versions, show the
proposed graph, materialize it, and rewrite `slang-package-lock.json`. This is the command that
takes newer compatible releases and applies publisher retractions.

Online update refreshes Git repositories only under `.slang/cache`; resolution and workspace
materialization then use those caches. Before changing `deps/` or the lock, update verifies graph
identity, dependency constraints, trusted path selection, workspace exclusions, toolchain
requirements, and cache availability.

After materialization, update checks publishability for new or changed Git and registered local
trees, and checks module layout and import-path uniqueness across the complete selected graph. The
new lock is written only after those checks succeed.

When the proposed lock differs, local checkout state would be discarded, or an existing named Git
ref in `deps/` would move, update asks for confirmation. Declining interactively changes nothing
and succeeds. A non-interactive invocation that requires confirmation fails unless `--yes` is
given.

### Options

`--ignore-overrides`
: Ignore enabled out-of-tree overrides for this solve without changing
`slang-package-overlay.json`. Enabled in-place overrides remain active so the command cannot
replace a user-owned checkout under the configured dependency directory.

`--clean`
: Permit replacement of dirty or otherwise mismatched tool-owned checkouts. The affected
checkouts are listed for confirmation. It cannot be combined with `--dry-run`.

`--dry-run`
: Resolve and validate the candidate graph and print the proposed lock changes without writing the
lock or dependency checkouts. An online dry run may refresh `.slang/cache`. It cannot prove that
unmaterialized remote source satisfies source-layout checks.

`--minimal`
: Use a compact one-line-per-package resolution report without detailed constraint rationale.

`--offline`
: Do not clone or fetch any package origin. Resolve and materialize exclusively from
`.slang/cache`. A missing cache, ref, or object is an error. Combine with `--dry-run` for a
network-free preview.

`--yes`
: Approve the proposed update and listed repository changes without an interactive prompt.

`--skip-validate`
: Skip post-materialization source-layout checks and publishability checks for changed package
trees. The legal graph and cache checks still run, and the command prints a warning.

## `status`

### Synopsis

```text
slang package status
```

### Description

Report whether the manifest and lock agree, required Git pins exist in the current cache,
checkouts are materialized and clean, local registrations agree with the lock, and the available
graph is buildable.

The first line reports the package name, lock state, package count, and one of `buildable`,
`incomplete`, or `not buildable`. Additional lines appear only for drift such as a missing or
stale lock, missing cache pins or checkouts, dirty checkouts, edits, enabled overrides, or source
layout errors. Suggested corrective commands are printed where appropriate.

Like `git status`, reportable drift is information and does not itself make the command fail.
`status` returns failure only when required root, lock, or overlay JSON cannot be read and parsed
well enough to produce a report.

The command does not contact remotes, modify package state, inspect `build/`, or look for newer
releases.

## `validate`

### Synopsis

```text
slang package validate
slang package validate <name>
slang package validate --all
```

### Description

Check that a package is suitable for sharing. Validation is stricter than buildability: local
development state can be valid input to `build` while preventing a package or lock from being
portable to another workspace. The command does not compile modules or run package tests.

There are four related sets of checks:

1. **Manifest and graph checks**
   - Every manifest uses the supported, closed JSON schema and has a matching package identity.
   - Every declared dependency selects one trusted lock row, all constraints and pinned refs still
     match, and every lock row is reachable.
   - Path dependencies exist and point to the selected versions.
   - Workspace exclusions do not reject a locked Git release.
   - All `tools.slang-toolchain` constraints in the selected graph accept the installed compiler.

2. **Upstream-cache checks**
   - Each Git cache is refreshed from its origin.
   - Every locked Git ref and locked commit exists in the cache. A moved ref may now resolve to a
     different commit without invalidating the commit identity already recorded by the lock.
   - A commit present only in `deps/<name>` does not pass. An adopted commit must be pushed to the
     origin so the cache can retrieve it.

3. **Source-layout checks**
   - The manifest exports at least one directory, every export exists, and every export remains
     inside its package.
   - Every exported `.slang` file is readable.
   - An export contains no more than 16,384 `.slang` files or 4,096 directories, and directory
     links do not escape the export.
   - A primary file starts with `module NAME;`, where `NAME` is its filename stem with hyphens
     changed to underscores.
   - A companion below the primary's same-named directory starts with `implementing NAME;`.
   - The declaration name matches the owning primary.
   - No two primary files in the checked graph export the same canonical import path, including
     collisions on case-insensitive filesystems.

4. **Publishability checks**
   - `license_files` lists at least one file.
   - Every listed license is a non-empty, readable file inside the package.
   - No listed license contains the placeholder written by `init`.
   - Every path dependency remains inside the package, so it will still exist when the package is
     shared independently.

The command forms apply those checks differently.

### Workspace validation

```text
slang package validate
```

Validate the workspace package as an application sharing gate. It applies source-layout and
publishability checks to the workspace, validates the materialized locked graph, and refreshes and
validates all locked Git cache entries.

To pass:

- replace the generated license placeholder;
- keep every exported source file in the required `module`/`implementing` layout;
- ensure every workspace path dependency stays within the workspace;
- have no enabled edit or override;
- when dependencies exist, have a current `slang-package-lock.json`;
- have no lock row that still requires a local override; and
- push every adopted Git commit to its origin so it can be fetched into `.slang/cache`.

A package with no dependencies does not need a lock. Disabled override registrations may remain,
but the selected lock itself must be portable.

Bare validation does not repeat license and source-layout checks for every unchanged transitive
package. Those packages still participate in graph legality and toolchain checks.

### Named-package validation

```text
slang package validate <name>
```

Validate one package tree named by the current lock. The tree may be an enabled override, a path
lock row, or a materialized Git checkout. The package's dependencies are checked against the
workspace lock; a nested lock inside that package is not used.

This form allows an enabled override, making it suitable for checking a library before publishing
a remote tag. It does not materialize missing packages or re-run the complete workspace graph
walk, including graph-wide toolchain selection and import-uniqueness checks. The package must
already be present in the lock.

### All locked packages

```text
slang package validate --all
```

Apply the named-package sharing check to every package in the lock and report package-tree
failures together. It refreshes and validates the relevant Git caches but does not materialize
trees.

`--all` cannot be combined with a package name. With no lock and no dependencies, it reports that
there are no locked packages to validate.

## `tree`

### Synopsis

```text
slang package tree
```

### Description

Print the current selected dependency graph from the workspace root. Each edge shows the selected
package version and the declaring requirement: a version range, Git ref, or path. Shared
subtrees are expanded once and marked `(*)` on later occurrences.

The command requires a current lock and loads manifests from active overrides, locked path trees,
or locked Git commits. It does not resolve newer versions, contact remotes, or modify state.

## `why`

### Synopsis

```text
slang package why <name>
```

### Description

Print every root-to-package path that currently requires `<name>`, including selected versions and
incoming requirements. The package must be present and reachable in the current lock.

This explains why a package is in the selected graph. It does not explain candidates that the
resolver rejected during an earlier `update`, and it does not contact remotes.

## `build`

### Synopsis

```text
slang package build [--skip-validate]
slang package --experimental build [--skip-validate]
```

### Description

Validate the materialized graph as buildable and create distribution output under the configured
workspace build directory (`build/` by default).

If a locked Git checkout is missing, build runs `fetch` first without `--clean`. If dependencies
exist without a lock, that fetch runs `update --yes` to create the first lock. An existing lock is
never rewritten by build.

A buildable graph requires legal lock identities and constraints, existing export directories,
correct `module` and `implementing` declarations, and unique primary import paths across the
closure. Build intentionally permits placeholder licenses, enabled overrides, and local path
dependencies because those do not prevent compilation.

### Stable output

A normal build:

- copies every exported `.slang` file to `build/bundle/source/`, preserving its
  export-relative import path, when `workspace.bundle.source` is enabled; and
- copies each package's Markdown files from `docs/` to `build/docs/<package-name>/` and writes
  `build/docs/index.md`.

`workspace.bundle.source` defaults to enabled. Existing source output is removed when it is
disabled.

### Experimental output

With the global `--experimental` option, build additionally:

- compiles each primary module to `build/bundle/modules/<import-path>.slang-module` when
  `workspace.bundle.modules` is enabled;
- writes `build/bundle/modules/provenance.json`;
- compiles configured workspace `build.host.executables` to `build/host/`; and
- copies the required `slang-rt` runtime beside those native executables and writes
  `build/host/EXPERIMENTAL.txt`.

The `.slang-module` format is unstable and tied to the producing compiler. Experimental host
output requires a matching exported workspace primary and a supported downstream C++ compiler.

### Option

`--skip-validate`
: Skip source declaration and import-uniqueness checks. The legal graph still runs, and export
files are still inventoried for bundle output. If build invokes fetch, the option is passed
through and also skips new-release publish checks. A warning is printed.

## `run`

### Synopsis

```text
slang package run [<name>] [<arguments>...]
slang package --experimental run --binary [<name>] [<arguments>...]
```

### Description

Run a host program configured by `build.host`. `run` consumes existing build output and does not
run `build` automatically.

If `<name>` matches an entry in `build.host.executables`, that executable is selected and the
remaining arguments are forwarded. Otherwise, the configured `build.host.default` is selected
and all arguments are forwarded. A sole configured executable becomes the default automatically;
manifests with multiple executables must name a default.

### Source mode

Normal `run` invokes the sibling `slangi` interpreter on
`build/bundle/source/<name>.slang`. The source bundle must be enabled and already built. Because
the interpreter searches from the input file's directory, the configured executable primary must
be at an export root.

### Binary mode

`--binary`
: Run the native artifact under `build/host/`. This option must immediately follow `run` and
requires the global `--experimental` option and a previous experimental build. Binary mode can
run an executable primary nested below the export root.

## `docs`

### Synopsis

```text
slang package docs
slang package docs --print
```

### Description

Use the documentation index from the last build. The command does not copy or regenerate
documentation; run `build` first.

Without options, open `build/docs/index.md` with the platform's registered Markdown application.

`--print`
: Print the canonical path to the generated index instead of opening it. This form is suitable for
scripts.

## `test`

### Synopsis

```text
slang package test
```

### Description

Reserved for a future package-owned testing model. The command currently fails with a
not-implemented diagnostic and does not invoke the internal `slang-test` harness. Because it is
reserved rather than usable, the concise `help` output does not list it.

## Confirmation

Commands ask for confirmation after describing the proposed mutation. A response of `y` or `yes`
(case-insensitive) approves it; the default response declines.

Declining in a terminal is a successful no-op. When standard input is not a terminal, a command
that needs approval fails rather than assuming consent. Use `--yes` on commands that provide it
for non-interactive operation.

Confirmation can cover:

- a new or changed lock during `update`;
- checkout state discarded by `--clean`;
- existing tags or origin-tracking branches in `deps/` that would move; and
- manifest and lock changes made by `unedit --adopt`.

## Files

`slang-package.json`
: Committed package intent: package identity, exports, licenses, dependency constraints,
retractions, workspace policy, toolchain requirements, and build settings.

`slang-package-lock.json`
: Committed exact selected graph. Git-pin rows record `git`, `ref`, `commit`, and `version`.
Override rows record `git`, `path`, and `version`. Path-only rows record `path` and `version`.

`slang-package-overlay.json`
: Gitignored machine-local edit and override registrations.

`slang-package-includes.txt`
: Gitignored generated list of absolute dependency export directories. `fetch`, `update`, and
applicable local-registration changes regenerate it.

`.slang/cache/`
: Workspace-local Git repositories refreshed from package origins. Online resolution updates this
cache; offline resolution requires it.

`deps/`
: Default materialization directory for locked Git checkouts. The path is configured by
`workspace.dependencies`.

`build/`
: Default output directory for source bundles, documentation, and experimental artifacts. The
path is configured by `workspace.build`.

## Exit status

Commands return zero on success and nonzero on errors. An interactively declined confirmation is
a successful no-op. `status` also returns zero for reportable drift such as an incomplete graph or
dirty checkout; it fails only when required package metadata cannot be read well enough to produce
the report.

## See also

- [Slang Source Packages](source-packages) — package and manifest concepts.
- [Using Source Packages](source-package-workflow) — an end-to-end workflow.
- [Growing an Application with Source Packages](source-package-command-use-cases) — detailed
  application journeys and behavioral contracts.
- [Writing Module Files, Import, and Include](module-files) — source-layout rules enforced by
  validation.
