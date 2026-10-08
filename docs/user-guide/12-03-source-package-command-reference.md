---
layout: user-guide
permalink: /user-guide/source-package-command-reference
---

Slang Package Command Reference
===============================

## Name

`slang package` — manage Slang source-package dependencies, locked source trees, and generated
bundle output.

`slang pkg` is an equivalent short form. The installed `slang-package` executable accepts the same
arguments.

## Synopsis

```text
slang package <command> [<arguments>]
slang pkg <command> [<arguments>]
slang-package <command> [<arguments>]
```

## Description

The package tool manages source dependencies described by `slang-package.json`. It resolves Git
release tags into `slang-package-lock.json` and materializes each dependency under `deps/` by
default. Every dependency is a Git repository. There is no path dependency and no overlay.

Except for `init` and `help`, commands may be run from the package root or any ordinary
subdirectory. The tool searches parent directories for the nearest `slang-package.json`; a nested
package therefore forms its own workspace.

Package commands do not change Slang's `import` syntax. `fetch` and `update` regenerate
`slang-package-includes.txt` for compiler sessions that consume the
materialized graph. `bundle` creates a flattened source bundle and documentation.

## Commands

The commands are:

- [`help`](#help) — show a command summary.
- [`init`](#init) — create a package skeleton.
- [`dependency`](#dependency) — add, remove, or list direct dependencies.
- [`pin`](#pin) — hold a solved release, or hold the current edit without moving it.
- [`unpin`](#unpin) — clear a pin. An edit stays on its branch.
- [`edit`](#edit) — check out a branch of a solved dependency.
- [`unedit`](#unedit) — end an edit on a release tag.
- [`fetch`](#fetch) — reproduce and materialize the lock.
- [`update`](#update) — resolve dependencies and rewrite the lock.
- [`status`](#status) — report lock, checkout, and graph readiness.
- [`validate`](#validate) — check that a package is suitable for sharing.
- [`tree`](#tree) — print the selected dependency graph.
- [`why`](#why) — explain why a package is in the selected graph.
- [`bundle`](#bundle) — validate the graph and create the source distribution.
- [`docs`](#docs) — open or print the generated documentation index.
- [`test`](#test) — reserved; package testing is not implemented.

## `help`

### Synopsis

```text
slang package help
slang package -help
slang package --help
```

### Description

Print a concise list of available commands. There is currently no command-specific
`help <command>` form.

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

- creates `src/`, `tests/`, `docs/`, `deps/`, and `out/`;
- writes `slang-package.json` with `src` as its export and `LICENSE` as its license file;
- sets the default workspace dependency and output directories to `deps/` and `out/`;
- records the installed Slang version as a minimum `tools.slang-toolchain` requirement when that
  version can be determined;
- writes a placeholder `LICENSE` if one does not already exist; and
- adds `.slang/`, `deps/`, `out/`, `slang-package-overlay.json`, and
  `slang-package-includes.txt` to `.gitignore`. The overlay name stays ignored so an old file is
  not committed. A file that still contains entries is an error.

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
slang package dependency add <name> [--git <url>] --version <range>
slang package dependency add <name> [--git <url>] --ref <ref> [--as <version>]
```

#### Description

Add a direct dependency, or replace the existing direct declaration with the same name. Exactly
one of the two source forms must be used:

- **Git version range** selects a dotted release tag from the Git repository.
- **Git ref** selects a branch, tag, or commit. `--as` declares the exact version that the
  selected source provides. When omitted, the resolver derives it from the nearest
  release tag reachable from the resolved commit.

`--path` is rejected. A dependency is a Git repository checked out at `deps/<name>`.

When `--git` is omitted, the command reads `SLANG_PACKAGE_INDEX`. That variable is a local path
or an `http`/`https` URL of a package index. A relative path is resolved from the current
directory. The index is JSON:

```json
{
    "schema_version": 1,
    "packages": {
        "noise": "https://example.com/noise.git"
    }
}
```

`schema_version` must be the integer `1`. `packages` maps a package name to one Git URL. The
command writes that URL into `slang-package.json` as if `--git` had been passed. `--version` or
`--ref` is still required. An explicit `--git` is used as given and the variable is not read. If
the variable is unset or empty, omitting `--git` fails. A variable that is set but names an index
that cannot be read fails, and so does a name the index does not list. `fetch` and `update` do
not read `SLANG_PACKAGE_INDEX`. Remapping an existing graph uses [`--remap-urls`](#update).

`<name>` identifies the package in the graph and must match the selected package manifest.

#### Options

`--git <url>`
: Set the Git repository. The location cannot begin with `-`, contain whitespace or control
characters or quotes, or use Git's command-executing `ext::` transport.

`--version <range>`
: Select a compatible release tag. A version is one or more decimal components with no `v`
prefix, for example `1.4` or `1.4.0.2`. Trailing zero components are the same release, so
`1.4.0` and `1.4` match the canonical tag `v1.4`, while `1.4.0.2` is the different release
`v1.4.0.2`. Whitespace joins comparisons with AND and `||` joins alternatives, for example
`>=1.2.0 <2.0.0 !=1.4.0`. `^1.2.3` means `>=1.2.3 <2`, `^0.0.0.4` means
`>=0.0.0.4 <0.0.0.5`, and `~1.2.3` means `>=1.2.3 <1.3`. A `^` or `~` bound uses the components
as written, so `^0.0.0` is narrower than `^0`. A bare version matches that one release.

`--ref <ref>`
: Select an opaque Git branch, tag, or commit instead of choosing a semantic-version release.

`--as <version>`
: Declare the exact dotted version provided by a Git ref, such as `1.4.0` or `1.4.0.2`.

`dependency pin` has been removed. Use [`pin`](#pin) to hold a solved version in the lock.

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

List direct dependencies from `slang-package.json`, including each Git version range or Git ref and
optional provided version. It does not display transitive
dependencies; use `tree` for the selected graph.

## `pin`

### Synopsis

```text
slang package pin <name> <version>
slang package pin <name>
```

### Description

Set the pin boolean on a lock row. The boolean lives in `slang-package-lock.json`, not in
`slang-package.json`. The manifest range is left as it was. `update` will not move a pinned
version. `edit` and `unedit` leave the boolean as they found it. `edit <name> --advance` and an ending of
`unedit` can move the version and leave the boolean set.

`<version>` is an exact dotted version, and it is required when the dependency is not edited. The
command resolves the canonical tag for that release, such as `v1.2` for `1.2` or `1.2.0`, stores
that version and the tag's commit, and sets the boolean. The version has to satisfy every incoming
constraint. When it does not, or when no canonical tag exists, `pin` fails and leaves the row
unchanged.

While the dependency is edited, `pin <name>` sets the boolean and leaves the version and the
branch alone. Passing `<version>` in that state fails.

## `unpin`

### Synopsis

```text
slang package unpin <name>
```

### Description

Clear the pin boolean. This works during an edit. The branch stays checked out. After `unedit`,
the next `update` may select a different tag.

## `edit`

### Synopsis

```text
slang package edit <name> --branch <branch>
slang package edit <name> --branch <branch> --create
slang package edit <name> --advance
```

### Description

`edit` applies to a dependency that is already resolved. The branch name is required. Without
`--create`, the branch must already exist and the command checks it out in `deps/<name>`. With
`--create`, a missing branch is created from the commit the dependency is resolved to. `--create`
does not reset a branch that already exists.

The lock row keeps the version, tag, and commit it already had, and records the branch beside
them. The pin boolean is unchanged. Editing a pinned dependency starts the branch at the pinned
commit. Editing a dependency that is not pinned does not set the boolean. `fetch` does not check
out the represented commit.

`edit <name> --advance` looks for canonical release tags on one line of history: from the branch
tip back to the commit of the release the row is representing. At a merge, a parent that does not
contain that commit is skipped. When more than one parent contains it, the walk takes the first
parent.

Consider a branch created at the `v1.2` commit, then updated by merging `main`:

```text
v1.2 -- fix -- M (tip)
         \    /
          v1.4
```

`M` lists `fix` as its first parent and the `v1.4` commit as its second. Both parents contain the
`v1.2` commit, so the ancestor test does not separate them. Parent order does: the walk takes
`fix` and never visits `v1.4`. A tag created on `fix` after `v1.2` is the one `--advance` can
adopt. The same merge performed the other way around, with `main` checked out, would record
`v1.4` as the first parent, and the walk would see `v1.4` on its way back to `v1.2`. The result
follows whichever line was current at the merge.

One `edit <name> --advance` replaces the version, the tag, and the commit with the
greatest canonical tag on that walk that is newer than the stored version and that still satisfies
incoming constraints. When `v1.3` and `v1.4` are both on the line, the row becomes `1.4`. The
command does not stop at the next tag and wait for another advance. The branch stays, and the
checkout does not move. `--advance` cannot be combined with `--branch` or `--create`. The package
name is the first argument, so a dependency named `advance` is edited with
`edit advance --branch <branch>`. When no such tag exists, or the walk never visits that commit,
the command says so and leaves the row unchanged. `update` does not run this, and it does not change
an edited checkout.

## `unedit`

### Synopsis

```text
slang package unedit <name> [--restore | --advance | --tag <version>] [--clean]
```

### Description

End an edit on a release row: a version and a commit. Declining a prompt leaves the edit. There
is no `--yes`. An ending that needs approval asks, and a run without a terminal fails and leaves
the edit. `--advance`, `--restore`, and `--tag` select one ending and cannot be combined. None of
them changes the pin boolean.

The command looks for a canonical release tag on the same line of history `edit <name> --advance` uses.
The candidate is the greatest such tag that is strictly newer than the represented release and
that still satisfies the dependency constraints. A tag that arrived only through a merge of some
other line is not a candidate.

When that newer tag is the commit that is checked out, the lock switches to that version and
commit. The checkout does not move, and there is no prompt.

When the newer tag is behind the checked-out commit, the command tells how many commits would
leave the workspace and asks whether to check out the tag. Approving writes that version and that
tag's commit. Declining does not fall through to the older release.

When a newer tag exists and none of them satisfy the constraints, the command says which tag was
rejected and why, then offers only the release the edit is already representing.

When no newer satisfying tag exists, the only ending is that same release. That requires approval
even when the checkout is already on that commit.

`--advance` selects the newer tag and fails when no newer tag on that line satisfies the
constraints. It asks only when the checkout would move.

`--restore` checks out the version and commit the edit is representing. After `edit <name> --advance`,
that is the commit just advanced to. A newer legal tag is named and then left unused. This always
asks.

`--tag <version>` creates a local annotated tag on the checked-out commit, then selects it the
way `--advance` does without moving `HEAD`. `<version>` is an exact dotted version. The tag name
is `v` plus the canonical spelling, so `1.4.0` creates `v1.4`. The command fails before creating
the tag when the release already has a tag, the version is not strictly greater than the version
stored on the edit, the version is not strictly greater than every canonical tag already on the
edit line, or the version does not satisfy the constraints. The tag is not pushed.

`--clean` discards uncommitted files, untracked files, and stashes so `HEAD` may move. It does
not approve dropping commits that follow a tag. Apply it only for an ending that would move
`HEAD`. A dirty checkout blocks that move until the changes are committed or `--clean` is passed.

## `fetch`

### Synopsis

```text
slang package fetch [--clean] [--yes] [--skip-validate] [--remap-urls <url>]
```

### Description

Reproduce the committed lock without selecting newer versions. The command refreshes each Git
repository under `.slang/cache` from its origin, verifies that every locked ref and commit is
available there, validates the locked graph, and materializes Git packages at their locked commits
under the configured dependency directory.

An edited checkout is left on its recorded branch. A clean tool-owned checkout already at
the correct origin and commit is left untouched. The command regenerates
`slang-package-includes.txt`.

When dependencies exist but the lock is absent, `fetch` runs `update` to create the first lock and
uses the same confirmation behavior. When neither a lock nor a dependency graph exists, use
`update` to create an empty lock.

`fetch` installs the Git URLs recorded in the lock and does not read a remap index. With an
existing lock, `--remap-urls <url>` re-resolves instead, using that index for every listed package
name and saving the URL in the lock. `<url>` must be `http` or `https`. A later `update` keeps
using the index stored in the lock. `fetch` cannot clear that policy; use `update --no-remap`.

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

`--remap-urls <url>`
: Re-resolve and record `<url>` as the lock's remap index. The same rules as
[`update --remap-urls`](#update) apply. Passing the option twice, or omitting `<url>`, is an error.

Without `--clean`, `fetch` stops before modifying any tree when a tool-owned checkout contains
local state. Use `edit` to preserve that work or `--clean` to discard it.

## `update`

### Synopsis

```text
slang package update [--clean] [--dry-run]
                     [--minimal] [--offline] [--yes] [--skip-validate]
                     [--remap-urls <url> | --no-remap]
```

### Description

Resolve all manifests reachable from the workspace, select compatible package versions, show the
proposed graph, materialize it, and rewrite `slang-package-lock.json`. This is the command that
takes newer compatible releases and applies publisher retractions.

Online update refreshes Git repositories only under `.slang/cache`; resolution and workspace
materialization then use those caches. Before changing `deps/` or the lock, update verifies graph
identity, dependency constraints, workspace exclusions, toolchain
requirements, and cache availability.

`update` does not fetch, merge, reset, or otherwise change an edited checkout, and it does not
move that row's version or pin. It reads the tree as it exists and solves the transitive
dependencies declared by that manifest. When `HEAD` is not on the recorded branch, `update`
reports the mismatch and leaves the checkout alone.

After materialization, update checks publishability for new or changed Git trees, and checks
module layout and import-path uniqueness across the complete selected graph. The
new lock is written only after those checks succeed.

When the proposed lock differs, a package Git URL would change, a Git repository would be deleted,
local checkout state would be discarded, or an existing named Git ref in `deps/` would move,
update asks for confirmation. Declining interactively changes nothing and succeeds. A
non-interactive invocation that requires confirmation fails unless `--yes` is given.

A Git repository is not deleted while it has commits or tags that are not on one of its remotes.
The command lists those commits and tags with the confirmation. `--yes` approves that deletion
after the list is printed. If the remotes cannot be contacted, the repository is left in place.

When the lock records a remap index, `update` reads that index and resolves every listed package
name from its Git URL. Names the index does not list keep the manifest URL. The manifest itself is
left unchanged. If the index cannot be read, the command fails and the lock stays.

`--remap-urls <url>` sets or replaces that index. `<url>` must be `http` or `https`. It is not
taken from `SLANG_PACKAGE_INDEX`. The command lists each package whose Git URL would change, then
one confirmation covers that list together with any other update changes. `--yes` approves it.
Declining writes nothing. After approval the lock stores both `<url>` and the resolved Git URLs.

`--no-remap` clears the index and re-resolves from the manifest Git URLs. It lists packages whose
URLs would change back and uses the same confirmation. `--no-remap` and `--remap-urls` together
are an error.

An edited package whose URL would change stops the command before the prompt. Unedit it first.
A pin keeps its version. The canonical tag for that version must exist on the repository the
resolve is using; if it does not, the solve fails and the lock stays. The checkout of an edited
package is not moved onto another URL.

### Options

`--ignore-overrides` has been removed.

`--clean`
: Permit replacement of dirty or otherwise mismatched tool-owned checkouts. The affected
checkouts are listed for confirmation. Commits and tags that are not on a remote are listed
before that repository is deleted. It cannot be combined with `--dry-run`.

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

`--remap-urls <url>`
: Resolve listed packages from the Git URLs in the package index at `<url>`, and record `<url>` in
the lock. `<url>` must be `http` or `https`. URL changes are listed and confirmed with the rest of
the update. Passing the option twice, or omitting `<url>`, is an error. It cannot be combined
with `--no-remap`.

`--no-remap`
: Drop the lock's remap index and resolve from the manifest Git URLs. URL changes are listed and
confirmed with the rest of the update.

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
stale lock, missing cache pins or checkouts, dirty checkouts, edits, or source layout errors.
Suggested corrective commands are printed where appropriate.

Like `git status`, reportable drift is information and does not itself make the command fail.
`status` returns failure only when the required root manifest or an existing lock cannot be read
and parsed well enough to produce a report, and when `slang-package-overlay.json` still contains
entries.

The command does not contact remotes, modify package state, inspect `out/`, or look for newer
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
development state can be valid input to `bundle` while preventing a package or lock from being
portable to another workspace. The command does not compile modules or run package tests.

There are four related sets of checks:

1. **Manifest and graph checks**
   - Every manifest uses the supported, closed JSON schema and has a matching package identity.
   - Every declared dependency selects one trusted lock row, all constraints and pinned refs still
     match, and every lock row is reachable.
   - Workspace exclusions do not reject a locked Git release.
   - All `tools.slang-toolchain` constraints in the selected graph accept the installed compiler.

2. **Upstream-cache checks**
   - Each Git cache is refreshed from its origin.
   - Every locked Git ref and locked commit exists in the cache. A moved ref may now resolve to a
     different commit without invalidating the commit identity already recorded by the lock.
   - A commit present only in `deps/<name>` does not pass. Push it to the origin so the cache can
     retrieve it.

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
- have no edited dependency;
- when dependencies exist, have a current `slang-package-lock.json`; and
- push every locked Git commit to its origin so it can be fetched into `.slang/cache`.

A package with no dependencies does not need a lock. A pinned release row can be committed. An
edited lock cannot. A non-empty `slang-package-overlay.json` is an error.

Bare validation does not repeat license and source-layout checks for every unchanged transitive
package. Those packages still participate in graph legality and toolchain checks.

### Named-package validation

```text
slang package validate <name>
```

Validate one package tree named by the current lock. The tree is the materialized Git checkout.
The package's dependencies are checked against the workspace lock; a nested lock inside that
package is not used.

This form is suitable for checking a library before publishing a remote tag. It does not
materialize missing packages or re-run the complete workspace graph
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
package version and the declaring requirement: a version range or Git ref. Shared
subtrees are expanded once and marked `(*)` on later occurrences.

The command requires a current lock and loads manifests from edited working trees or locked Git
commits. It does not resolve newer versions, contact remotes, or modify state.

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

## `bundle`

### Synopsis

```text
slang package bundle [--skip-validate]
```

### Description

Validate the materialized graph as buildable and create the source distribution under the
configured workspace output directory (`out/` by default).

If a locked Git checkout is missing, bundle runs `fetch` first without `--clean`. If dependencies
exist without a lock, that fetch runs `update --yes` to create the first lock. An existing lock is
never rewritten by bundle.

A buildable graph requires legal lock identities and constraints, existing export directories,
correct `module` and `implementing` declarations, and unique primary import paths across the
closure. Bundle permits a placeholder license because that does not prevent compilation.

### Output

A bundle:

- copies every exported `.slang` file to `out/bundle/source/`, preserving its export-relative
  import path; and
- copies each package's Markdown files from `docs/` to `out/docs/<package-name>/` and writes
  `out/docs/index.md`.

### Option

`--skip-validate`
: Skip source declaration and import-uniqueness checks. The legal graph still runs, and export
files are still inventoried for bundle output. If bundle invokes fetch, the option is passed
through and also skips new-release publish checks. A warning is printed.

## `docs`

### Synopsis

```text
slang package docs
slang package docs --print
```

### Description

Use the documentation index from the last `bundle`. The command does not copy or regenerate
documentation; run `bundle` first.

Without options, open `out/docs/index.md` with the platform's registered Markdown application.

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
- a package Git URL changed by a remap index, or restored by `update --no-remap`;
- checkout state discarded by `--clean`;
- commits or tags that are not on a remote, before the Git repository that holds them is deleted;
- existing tags or origin-tracking branches in `deps/` that would move; and
- an `unedit` ending that would leave the branch or move `HEAD`. `unedit` has no `--yes`.

## Files

`slang-package.json`
: Committed package intent: package identity, exports, licenses, dependency constraints,
retractions, workspace policy, toolchain requirements, and build settings. Dependency Git URLs
stay as published. A remap does not rewrite them.

`slang-package-lock.json`
: Committed exact selected graph. Optional `remap_index` is the `http` or `https` package index
`update` reads on later resolves. A release row records `git`, `ref`, `commit`, and `version`.
An edit row records `git`, `branch`, `ref`, `version`, and `commit`. The tag and commit are the
release the branch is representing, and that row is not something to commit. `pinned` is recorded
only when the boolean is set. A `path` field is
rejected.

`slang-package-overlay.json`
: No longer used. `init` still lists it in `.gitignore`. A file that contains entries is an error.

`slang-package-includes.txt`
: Gitignored generated list of absolute dependency export directories. `fetch` and `update`
regenerate it.

`.slang/cache/`
: Workspace-local Git repositories refreshed from package origins. Online resolution updates this
cache; offline resolution requires it.

`deps/`
: Default materialization directory for locked Git checkouts. The path is configured by
`workspace.dependencies`.

`out/`
: Default output directory for source bundles and documentation. The path is configured by
`workspace.output`. `bundle` refreshes `out/docs`.

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

## Experimental

Unstable package behavior. These commands and options are separate from the stable reference
above. They require `--experimental` immediately before the command. The option does not change
stable commands or dependency resolution.

### Synopsis

```text
slang package --experimental <command> [<arguments>]
slang pkg --experimental <command> [<arguments>]
slang-package --experimental <command> [<arguments>]
```

### `help`

```text
slang package --experimental help
```

Print the stable command list, and also `build` and `run`.

### `build`

```text
slang package --experimental build [--skip-validate]
```

Create the built distribution. `.slang-module` files and host executables are unstable. `build`
performs the same validation, source copy, and documentation refresh as `bundle`, then adds binary
targets. `docs` can open the index this command writes. A later `bundle` removes
`out/bundle/modules` and `out/host`.

If a locked Git checkout is missing, build runs `fetch` first without `--clean`. If dependencies
exist without a lock, that fetch runs `update --yes` to create the first lock. An existing lock is
never rewritten by build.

In addition to the source bundle and `out/docs` written by `bundle`, build:

- compiles each primary module to `out/bundle/modules/<import-path>.slang-module`;
- writes `out/bundle/modules/provenance.json`;
- compiles configured workspace `build.host.executables` to `out/host/` when that list is present;
  and
- copies the required `slang-rt` runtime beside those native executables and writes
  `out/host/EXPERIMENTAL.txt`.

The `.slang-module` format is unstable and tied to the producing compiler. Host output requires a
matching exported workspace primary and a supported downstream C++ compiler. When no host
executables are configured, build removes `out/host`.

`--skip-validate`
: The same escape hatch as on `bundle`. The option is also passed through when build invokes
fetch.

### `run`

```text
slang package --experimental run [<name>] [<arguments>...]
slang package --experimental run --binary [<name>] [<arguments>...]
```

Run a host program configured by `build.host`. `run` consumes existing output and does not bundle
or build first. Without `--experimental` the command is rejected.

If `<name>` matches an entry in `build.host.executables`, that executable is selected and the
remaining arguments are forwarded. Otherwise, the configured `build.host.default` is selected and
all arguments are forwarded. A sole configured executable becomes the default automatically;
manifests with multiple executables must name a default.

Without `--binary`, `run` invokes the sibling `slangi` interpreter on
`out/bundle/source/<name>.slang`. The source bundle must already exist. Because the interpreter
searches from the input file's directory, the configured executable primary must be at an export
root.

`--binary` must immediately follow `run`. It executes the native artifact under `out/host/` from
the last `build`, including an executable primary nested below the export root.
