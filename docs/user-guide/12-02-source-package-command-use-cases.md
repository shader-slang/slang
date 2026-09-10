---
layout: user-guide
permalink: /user-guide/source-package-command-use-cases
---

Growing an Application with Source Packages
============================================

This chapter presents user-centered use cases as journeys. It follows a person from an empty
directory to an application made from several packages, saying what you do by hand, what
`slang package` does for you, and where the current tool stops helping. Later journeys follow the
same application when an upstream package changes its graph and when you extract part of your own
application into a package.

Journeys 1 through 7 use only stable commands, so they describe what the tool supports today
without any opt-in. Binary module generation, host executable compilation, and binary `run` are
experimental, and everything about them is collected in Journey 8, the last journey, so it can be
read or ignored on its own. Source `run` is stable.

This is also a behavioral contract for the package tool. The maintainer appendix turns the
journeys into must/must-not checks for future command changes.

Manifest fields and validation rules are in [Slang Source Packages](source-packages). The
ready-made `video-preview` example is in [Using Source Packages](source-package-workflow).
Module file naming is in [Writing Module Files, Import, and Include](module-files).

## The three kinds of state

Before starting, distinguish the files that describe the same graph from different points of
view:

- `slang-package.json` is intent written by a package author. It declares the package's exported
  source, dependencies, licenses, and toolchain requirements. You edit and commit it.
- `slang-package-lock.json` is the exact graph selected for the workspace in which you ran
  `update`. The tool writes it; reviewing and committing a root application's lock is recommended
  for reproducibility but not enforced. A dependency's nested lock is not used when your workspace
  resolves that dependency. Portability is derived: a Git+path row requires local workspace state,
  while Git-only rows can be fetched elsewhere once their commits are reachable from the remote.
- `slang-package-overlay.json` is machine-local override state. The tool writes it and `init` adds it to
  `.gitignore`. `edit NAME` is shorthand for an override at `{workspace.dependencies}/NAME`. Do not commit
  this file.

`slang package help` groups commands under those three files, then build, in pipeline order:
manifest, overlay, lock. That is the file each command primarily writes or reports. Most commands
still _read_ the others. `unedit --adopt` drops the overlay and also writes the manifest and lock.

The **workspace** is the package from which you run the command. It owns the one lock for that
solve. This is not a multi-member workspace in the Cargo or npm sense: Slang currently has no
committed list of packages that are developed together.

## Journey 1: bootstrap one application

Assume you are starting an application named `image-viewer` and no package exists yet.

### Goal

Create a valid package, produce its distributable source bundle, and make the first source control
commit.

### Human does

Create the directory, enter it, and initialize the package:

```sh
mkdir image-viewer
cd image-viewer
slang package init
```

`init` derives the package name from the directory. It creates this starting tree:

```text
image-viewer/
├── .gitignore
├── LICENSE
├── slang-package.json
├── src/
├── tests/
├── docs/
├── deps/
└── build/
```

The last two directories are generated state and are ignored. The manifest initially exports
`src`, names `LICENSE`, configures `workspace.dependencies` (`deps/`) and `workspace.build`, and records the installed Slang version as
the minimum `tools.slang-toolchain` version when the version is available.

The generated package is deliberately incomplete. It does not add a `build.host` section. Replace the
placeholder text in `LICENSE`, then add the first module:

```slang
// src/image-viewer.slang
module image_viewer;

public float3 toneMap(float3 color)
{
    return color / (1.0f + color);
}
```

The filename uses a hyphen while the declaration uses the canonical underscore spelling.

Run the package-quality gate and then build:

```sh
slang package validate
slang package build
```

Finally, initialize source control if needed and commit:

```sh
git init
git add .
git commit
```

Commit source, the manifest, licenses, tests, and docs. Do not commit `.slang/`, `deps/`, `build/`,
`slang-package-overlay.json`, or `slang-package-includes.txt`.

### Tool does

- `init` creates the manifest, conventional directories, license placeholder, and ignore rules.
- Bare `validate` is the app sharing gate: closed manifest schema, license, export directories,
  module declarations, installed toolchain, and a portable lock with no active overrides.
  A package with no dependencies does not need a lock. `validate NAME` and `validate --all` apply
  the publishable-package rules to locked trees in this workspace.
- `build` checks that the available graph is legal and buildable, emits the source bundle under
  `build/bundle/source`, and collects Markdown under `build/docs/`. If a locked Git checkout is
  missing, it runs fetch first. If there is no
  lock and the manifest has dependencies, fetch runs update `--yes` so the first clone can build
  without a prompt. An existing lock is never rewritten. Build does not accept `--clean` or
  `--yes`.

### Current gaps and pitfalls

- `init` is intentionally scoped to the manifest, directories, ignore rules, and license reminder.
  It does not write a first source file, and its placeholder license makes the default `validate`,
  fail until you choose a real license. The placeholder does not prevent a local `build`.
- `slang package test` is reserved but not implemented. The generated `tests/` directory is only a
  convention today.
- `docs` does not regenerate documentation. Run `build` first.
- An application that compiles shaders or modules with `slangc` passes
  `-search-path-list slang-package-includes.txt`. That is not how this package tool produces a
  host executable. An API host loads the same paths with `slang_readSearchPathsFile` and assigns
  them to `SessionDesc.searchPaths`; package state is not injected into arbitrary compiler
  sessions.
- A `build.host` section has no effect on plain `build`; Journey 8 covers the explicit opt-in that
  produces its binary output.

### First checkpoint

At this point a new clone can reproduce the package without dependency resolution:

```sh
git clone <image-viewer-url>
cd image-viewer
slang package validate
slang package build
```

There is no lock yet because the package has no dependencies. Once dependencies are added, the
checked-in lock becomes part of this clone workflow.

## Journey 2: add the first published dependency

Assume another repository publishes a package named `color-encoding` with immutable release tags
such as `v1.0.0`.

### Goal

Use a compatible published version, understand what was selected, and check in enough information
for another machine to reproduce it.

### Human does

Add the direct dependency:

```sh
slang package dependency add color-encoding \
  --git https://example.com/color-encoding.git \
  --version ">=1.0.0 <2.0.0"
```

This atomically changes only `slang-package.json`; it does not resolve or materialize anything.
The resulting manifest edge is:

```json
"dependencies": {
  "color-encoding": {
    "git": "https://example.com/color-encoding.git",
    "version": ">=1.0.0 <2.0.0"
  }
}
```

Preview the solve:

```sh
slang package update --dry-run
```

The detailed report says which release would be added, which incoming constraint selected it, and
whether candidates were skipped by a publisher retraction or workspace exclusion. It does not
write the lock or replace checkouts. If you only need one line per package, while still seeing
unchanged packages, use:

```sh
slang package update --dry-run --minimal
```

If no solution exists, the failure report names the package that exhausted the search, lists every
incoming requirement with its owning package, and gives one reason for each candidate that was
considered. Reasons distinguish an incompatible version from a publisher retraction or workspace
exclusion. The final `Help:` line directs the workspace author back to the requirements and
exclusions that can change the result.

Apply the solve, then validate and build:

```sh
slang package update
slang package validate
slang package status
slang package build
```

Review and commit both files:

```sh
git diff -- slang-package.json slang-package-lock.json
git add slang-package.json slang-package-lock.json
git commit
```

The manifest records the acceptable range. The lock records selection identity only: the exact
solver `version`, plus Git `ref` and `commit`, or an overlay `path`. Declared exports and
dependencies are reloaded from the selected tree.

To freeze that selection as committed intent, pin the Git edge without copying lock identity into
the overlay:

```sh
slang package dependency pin color-encoding
```

Default pin writes `git` plus the lock's exact `version`. `--to 1.0.0` writes that exact version
instead. `--commit` writes `git`, `ref` (the locked SHA), and `as` (the lock version). Pin does not
run `update`; inspect `status` and update afterward, the same as `dependency add`. A later
compatible tag no longer satisfies a default pin. Pinning a transitive package promotes it to a
direct edge using the lock's Git URL. Pin leaves an active overlay registered. Once the lock row
selects that overlay, pass `--to` explicitly because its effective version need not be published
in Git. `--commit` requires a Git-only lock row because an overlay row has no locked SHA; use
`unedit --adopt` when the intent is to retain the working-tree commit. Path dependencies and
path-only lock rows are already pins.

After pin, inspect `status` and run `update` if the written version differs from the lock, then
commit the manifest (and lock, if update rewrote it).

### Tool does

- `update` refreshes origin data under `.slang/cache/`, examines compatible cached Git tags,
  resolves the complete transitive graph, and selects one version per package name.
- A real update prints that exact selection and asks before applying it, unless the solve selects
  exactly what the committed lock already records. In a terminal, declining the prompt leaves the
  workspace untouched and succeeds; it is a decision, not a command failure. Without a terminal the
  command does not prompt: it fails and tells you to re-run with `--yes`.
- A real update stages Git objects and refs from `.slang/cache` into `deps/NAME`, publish-checks new or changed Git
  packages and changed local registrations, checks closure-wide buildability, writes
  `slang-package-lock.json`, and regenerates `slang-package-includes.txt`.
- `status` prints one line when the workspace is current. When something is dirty, it lists
  missing checkouts, dirty or diverged pins, enabled overrides, and source
  problems and missing cached upstream pins, without inspecting `build/` or contacting remotes. A missing lock or pin is
  `incomplete`; a present graph that fails the source check is `not buildable`. Reportable
  drift does not make status fail.
- `fetch` subsequently reproduces that lock. It checks out each recorded `commit` and does not
  consult newer tags, publisher retractions, or version selection. It does refresh the cache and
  stages cached refs into `deps/`; moving an existing tag or origin-tracking branch requires the
  command's consolidated destructive confirmation:

  ```sh
  slang package fetch
  slang package build
  ```

  With an existing lock, fetch also regenerates `slang-package-includes.txt` without re-solving or
  rewriting that lock. This is the non-destructive way to restore a missing or stale include list,
  although fetch may still restore tool-owned checkouts to their locked commits. Do not pass
  `--clean` unless local checkout state should be discarded. With no lock, fetch delegates to
  `update --yes` and writes the initial lock.

  This is the normal clean-clone and CI path. With a committed lock, `build` fetches any missing
  locked trees first and does not rewrite that lock. With no lock, `build` runs fetch, which runs
  update `--yes` to write the first lock. `fetch` remains the command for prefetch, `--clean`, and
  CI that wants materialization without building. CI that already has a committed lock should not
  run `update`.

From this point forward, a clone can start with build:

```sh
git clone <image-viewer-url>
cd image-viewer
slang package build
```

### Current gaps and pitfalls

- Dependency add/remove cover range, `ref`+`as`, and path+`as` edges. `dependency pin` freezes a
  Git lock selection as an exact version or SHA; there is no package registry search.
- Update always solves the entire graph. There is no package-scoped update.
- `--dry-run` can inspect remote manifests but does not materialize remote source. A preview can
  succeed and the real update can later fail source or module-layout validation.
- Do not hand-edit the lock. Re-run `update` after resolving manifest changes or lock merge
  conflicts.
- Version strings in manifests omit the release tag's `v` prefix.

## Journey 3: author and publish the first library

Now suppose the color code does not exist yet. You want to author `color-encoding` and consume it
from `image-viewer`.

### Goal

Create a reusable package, validate it independently, publish a Git release, and add it to the
application.

### Human does

Create a separate repository and initialize it:

```sh
mkdir color-encoding
cd color-encoding
slang package init
```

Replace the license placeholder. A library does not need `build.host.executables`, so do not add that
optional section. Add its primary:

```slang
// src/color/encoding.slang
module encoding;

public float3 decodeColor(float3 encoded)
{
    return encoded;
}
```

The import path is `color.encoding`, from the file's path below `src`. The declaration uses only
the simple filename stem, `encoding`.

Validate the package:

```sh
slang package validate
```

There is currently no `package`, `pack`, `publish`, or version-bump command. Create the Git release
manually:

```sh
git init
git add .
git commit
git tag v1.0.0
git remote add origin <color-encoding-url>
git push origin HEAD --tags
```

Back in `image-viewer`, add the Git dependency to its manifest, run `update --dry-run`, then
`update`. Import it from the application:

```slang
import color.encoding;
```

### Tool does

- Bare `validate` in the library's own repository checks that the library's files follow the
  package module rules before you tag it. In a consuming app, `validate NAME` checks that same
  library against the app's lock.
- The consumer's resolver reads `slang-package.json` from release tags and uses the tagged commit
  as the immutable source identity.
- The consumer's root lock includes the library and all of its transitive dependencies. A lock
  committed inside `color-encoding` is useful when developing that repository itself, but it does
  not constrain the graph selected by `image-viewer`.

### Current gaps and pitfalls

- Publication is entirely a Git workflow. The tool does not inspect which files would ship, check
  that a tag is new and immutable, bump a version, push, sign, or provide a registry.
- A tag is expected to be immutable. Moving it can make a previously selected tag disagree with
  its locked commit.
- There is no package test command or publish-time build gate. Run `validate` and any project tests
  manually before tagging.
- The package name is a graph identity, not an import prefix. Moving source can preserve
  `import color.encoding` even if the package repository is renamed, as long as its export-relative
  path stays `color/encoding.slang`.

## Journey 4: work on a consumed package locally

Edits and overrides use one mechanism. `edit` is the convenient in-place form; `override add`
also supports a different local directory.

### Goal

Patch a dependency in place, trial an unpublished local graph, then either discard the overlay or
adopt a committed fix as manifest intent.

### Human does

To patch source without changing package metadata, make the materialized checkout
developer-owned:

```sh
slang package edit color-encoding
# edit deps/color-encoding/src/...
```

The checkout stays at `deps/color-encoding`. `edit` registers an enabled override there, using the
version from the current lock. Fetch and update do not replace that user-owned tree. Compiler
search paths already named that directory, so claiming it does not require `update` to keep
building against the checkout. New export directories in its working-tree manifest are picked up
when the registration is written. New dependency edges still need `update`, because they change
which packages the lock must contain. The next `update` reads the working-tree manifest and writes
a Git+path lock row; those edges participate in the full solve and are not copied into the lock.

`edit` also accepts a checkout that already has local changes. That is the recommended recovery
when you modified `deps/color-encoding` first and only then discovered that fetch and update
refuse to run: registering the edit adopts the work as-is rather than making you choose between
losing it and hand-copying it elsewhere.

When you are ready to hand the checkout back:

```sh
slang package unedit color-encoding
```

Default `unedit` removes the registration only when the lock is already a Git pin, the tree is
clean, and `HEAD` is that exact commit. To discard local state, use `--clean`; it restores the
locked commit and asks for confirmation unless you also pass `--yes`. If update has already
written a local-path row, disable the override and update the published graph before unediting.

To keep a committed fix, make the pin authoritative in the manifest:

```sh
git -C deps/color-encoding commit -am "Fix conversion"
slang package unedit color-encoding --adopt --as 1.1.1
```

`--adopt` replaces the direct dependency's version range with a Git pin at `HEAD`, converts the
lock row back to a Git pin, and removes the overlay. Without `--ref`, a unique release tag at
`HEAD` becomes both `ref` and `as`; otherwise `ref` is the commit and `as` is derived from the
nearest ancestor release tag. `--adopt --ref BRANCH` writes that branch as `ref` (it must point at
`HEAD`) and still locks the SHA. Pass `--as VERSION` to override the derived identity. The commit
or tag must be reachable from the configured remote before another machine can fetch it.

To assign another effective version to the in-place tree, re-add that same override with `as`:

```sh
slang package override add color-encoding deps/color-encoding 1.1.0
slang package update --dry-run
slang package update
```

A sibling clone is the same mechanism with a different path. Remove the in-place registration
first because one package name cannot have two local trees, then `update` so the lock selects
that directory:

```sh
slang package override add color-encoding ../color-encoding 1.1.0
```

Enabled overrides automatically participate in the **entire graph** solve as the only candidate
for their package names; packages without enabled overrides still come from Git.

If the local `color-encoding` manifest adds `color-math`, the full-graph local solve adds
`color-math` and its transitive requirements to the root lock. Every incoming constraint must
accept the override's exact `as` version.

Omit `AS` only when the name already has a lock row; the command reuses that version. An override
for a name absent from the lock requires `AS` and also needs a reachable dependency declaration in
the root or another resolved local manifest.

Restore a portable graph before sharing it:

```sh
slang package override disable color-encoding
slang package update
slang package override remove color-encoding
```

Disabling retains the local path and version while update restores published Git selections.
Removal refuses while the lock still contains the local path row. Re-enable later to return to the
same local tree without re-entering its configuration.

### Tool does

- Records every local substitution under `overrides` in `slang-package-overlay.json`. `edit NAME`
  creates the special case whose path is `{workspace.dependencies}/NAME`; there is no second edit
  representation.
- Does not copy or modify the supplied directory. Re-adding the in-place path updates that same
  registration's effective version.
- Local registration changes regenerate `slang-package-includes.txt` from the active tree. An in-place
  `edit` already sits at `{workspace.dependencies}/NAME`, so those paths keep working without `update`; a
  newly declared export directory on that working tree is included immediately. An out-of-tree
  override is recorded immediately, but `update` is what writes its path into the lock and
  resolves any new dependency edges. Disabling a lock-adopted override needs `update` before
  published Git paths are regenerated.
- A plain update records each enabled override's original Git identity and local path in the
  definitive lock and resolves all enabled override manifests together.
- A local-path lock fails on another machine without the matching registration. This is
  intentional: a local graph must not silently masquerade as the portable published graph.

### Current gaps and pitfalls

- `unedit --adopt` applies only to a direct Git dependency because that is the manifest edge it can
  pin. `dependency pin NAME` can promote a transitive Git package to a direct exact-version edge
  without adopting a working tree. A local-only fix that is not published still needs adopt or an
  upstream release.
- An adopted commit with no `vMAJOR.MINOR.PATCH` tag in its history requires explicit `--as`;
  Git object identity does not determine a semantic version. An ancestor release tag is enough.
- Overrides are path-only; there is no user-global Git-to-Git remapping policy.
- `slang-package-overlay.json` and `slang-package-includes.txt` must not be committed. A team-wide
  source relationship belongs in
  `slang-package.json` as a path or Git dependency.
- A manifest path dependency cannot be overridden by the current command.

## Journey 5: a consumed package adds a dependency

Months later, `color-encoding` version 1.1.0 starts using a new package named `color-math`.
`image-viewer` still directly depends only on `color-encoding`.

### Goal

Understand how an upstream graph can grow without changing the application's manifest, and decide
when to adopt it.

### Human does

#### Publisher

The `color-encoding` author adds the dependency to that package's manifest:

```json
"dependencies": {
  "color-math": {
    "git": "https://example.com/color-math.git",
    "version": ">=1.0.0 <2.0.0"
  }
}
```

The author validates and tests the package, commits the manifest, and tags a new
`color-encoding` release. If the new dependency is an implementation detail compatible with the
published interface, this may be a compatible release. If it changes the package contract, the
publisher should choose the corresponding breaking release range.

#### Consumer

Nothing happens merely because a new tag exists:

```sh
slang package fetch
```

Fetch reproduces the old lock. It does not discover `color-math`.

When the consumer deliberately previews an update:

```sh
slang package update --dry-run
```

the report should show `color-encoding` moving to 1.1.0 and `color-math` being added because the
new `color-encoding` manifest requires it. After review:

```sh
slang package update
git diff -- slang-package-lock.json
git add slang-package-lock.json
git commit
```

The consumer's own `slang-package.json` is unchanged. The new package is transitive, but the root
lock gains an exact row for it. To make that selection a committed direct requirement:

```sh
slang package dependency pin color-math
slang package status
slang package update
```

### Tool does

- Resolves manifests recursively from the selected candidate releases.
- Adds the new row to the one root lock and materializes it under `deps/color-math`.
- Validates module import uniqueness across both changed and unchanged packages.
- Explains the incoming constraint in the update report.

### Current gaps and pitfalls

- `slang package tree` shows the selected graph, and `slang package why color-math` prints every
  root-to-package path and incoming requirement. These commands explain current graph presence,
  not the historical candidates rejected during the solve; keep the update report when that
  history matters. Like `status`, they read existing workspace and cache state without contacting
  remotes.
- There is no package-scoped update. Previewing or taking the new `color-encoding` release may
  move other compatible packages in the same solve.
- `--minimal` preserves the added/changed/unchanged list but intentionally drops the incoming
  constraint explanation.
- The dependency package's own lock is ignored. Only the manifest edge and the consumer's root
  solve determine the selected `color-math` version.

## Journey 6: a consumed package splits in two

Next, the publisher moves `src/color/transfer.slang` out of `color-encoding` into a new package
named `color-transfer`.

### Goal

Adopt a package split without accidentally changing import paths or creating duplicate modules.

### Human does

#### Publisher

Create and validate `color-transfer`, then move the primary and its companions while preserving
their paths below the export root:

```text
color-transfer/
└── src/
    └── color/
        ├── transfer.slang
        └── transfer/
            └── lookup.slang
```

The primary still declares `module transfer;`, and users can still write:

```slang
import color.transfer;
```

The new `color-encoding` manifest depends on `color-transfer`. Its new release must stop exporting
the old `color/transfer.slang`; otherwise both packages export the same import and graph validation
fails. The publisher tags `color-transfer` first, then tags the new `color-encoding` release that
depends on it.

A split is not automatically SemVer-compatible. If consumers were promised that
`color-encoding` alone provided a particular public surface, moving that surface may require a
breaking release even when the Slang import path is preserved. The package tool validates graph
shape; it does not decide API compatibility.

#### Consumer

If `image-viewer` reaches `color-transfer` through `color-encoding`, its manifest can remain
unchanged:

```sh
slang package update --dry-run
slang package update
git add slang-package-lock.json
git commit
```

The lock gains `color-transfer`, and all packages in the graph put its `src` directory on the
build search path. Existing `import color.transfer;` statements can continue to work if the
publisher preserved the import path and transitive dependency.

The application needs a direct manifest dependency on `color-transfer` when it wants that
relationship to be explicit, or when the new `color-encoding` release no longer retains the
transitive edge. That manifest edit is manual today.

### Try the split before either package is tagged

The application already has a lock for `color-encoding`. Add the future `color-transfer` Git
identity to the local `color-encoding` manifest, even if that remote has no usable tag yet. Then
register both local trees:

```sh
slang package override add color-encoding ../color-encoding 2.0.0
slang package override add color-transfer ../color-transfer 1.0.0
slang package update --dry-run
slang package update
```

The solve is still whole-graph. The first override's local manifest introduces the second package;
the second override supplies its exact local candidate. This is the current way to test a
multi-package upstream change before publication.

After publishing both tags, restore the portable graph:

```sh
slang package override disable color-encoding
slang package override disable color-transfer
slang package update
slang package override remove color-encoding
slang package override remove color-transfer
```

### Tool does

- Uses transitive manifest edges, so consumers do not need to copy every upstream dependency into
  their root manifest.
- Detects exact and case-insensitive duplicate import paths across the old and extracted package.
- Enforces one selected version per package name. A new package name that already exists elsewhere
  in the consumer graph must satisfy all incoming requirements for that same identity.
- Records only the consumer workspace's selected graph. Nested locks from either publisher do not
  participate.

### Current gaps and pitfalls

- There is no migration or package-split command and no API compatibility check.
- `tree` and `why` expose the durable selected graph, but rejected-candidate rationale exists only
  in the update report.
- Dry-run reads candidate manifests but cannot validate the unmaterialized packages' source trees.
- If real update materializes the split and then source validation fails, the previous lock stays
  unchanged but some dependency directories may already have moved.
- Publisher retractions and workspace exclusions are asymmetric: a new retraction does not break
  the consumer's old lock on fetch, while a newly committed root exclusion makes that lock stale
  and fetch rejects it.

## Journey 7: split your application into packages

Return to `image-viewer`. Its `src/color/math.slang` has become useful enough to extract.

### Goal

Move code behind a package boundary without unnecessarily rewriting Slang imports, then develop
that package as a sidecar until it has a Git remote.

### Human does

Start from the import path you want to preserve:

```slang
import color.math;
```

The current primary lives at:

```text
image-viewer/src/color/math.slang
```

Its companion files, if any, live below `src/color/math/` and begin with
`implementing math;`. The extraction should keep `color/math.slang` and `color/math/...` below
the new package's export root. Package boundaries do not change Slang import syntax.

Before moving files, identify:

- which modules belong together;
- whether other application modules use internal symbols that must become `public`;
- which dependencies the extracted package itself needs;
- whether the package should remain a vendored tree in this repository or become an independently
  released sidecar.

The tool does not analyze or perform this split. There is no `extract` command.

#### Extract: sidecar package

Create the package in a sibling directory (recommended):

```sh
mkdir -p ../color-math
cd ../color-math
slang package init
```

Replace its license placeholder, move the source, and initialize git in that sidecar:

```text
color-math/
├── LICENSE
├── slang-package.json
└── src/
    └── color/
        ├── math.slang
        └── math/
            └── approximation.slang
```

Delete the old `image-viewer/src/color/math.slang` and companion directory after the move. Keeping
both copies would export `color.math` twice and make root graph validation fail.

Back in `image-viewer`, declare the eventual Git identity and point the solver at the sidecar so
the application uses **one** tree:

```sh
slang package dependency add color-math --git https://example.com/color-math.git --version ">=1.0.0 <2.0.0"
slang package override add color-math ../color-math 1.0.0
slang package update --dry-run
slang package update
```

Do not `dependency add --git ../color-math` (or any other path to that same tree) without an
override. Fetch would clone that git onto `deps/color-math`, a second copy of the files you are
already editing.

`deps/color-math` is gitignored. Use it only as a laptop override (`override add color-math
deps/color-math 1.0.0`, the same representation created by `edit`). A sidecar there is not in the
application repository until that inner git has a remote.

`slang package validate color-math` and root `build` use this workspace's lock pins. They do not
materialize a nested graph under the sidecar. When the sidecar origin exists, push tags, keep
`git` pointed at that origin, then restore a portable lock:

```sh
slang package override disable color-math
slang package update
slang package override remove color-math
```

#### Vendoring: path dependency, not extract

A path dependency vendors a tree that lives in this repository. That is committed co-development,
not extract:

```sh
mkdir -p packages/color-math
cd packages/color-math
slang package init
```

In the root `image-viewer/slang-package.json`, add:

```json
"color-math": {
  "path": "packages/color-math",
  "as": "1.0.0"
}
```

Then `update` from the root. The path package stays under `packages/color-math`; it is not copied
to `deps/`. Clone of the application repository reproduces it without `slang-package-overlay.json`.

#### Publish first, then consume

Create the package in its own Git repository, validate it, tag `v1.0.0`, and push it as in
Journey 3. Add `git` plus `version` to the application's manifest and run normal `update`.
This avoids machine-local lock state but requires the package to be publishable before the
application can consume it.

### Tool does

- `init` scaffolds each package. `validate NAME` in the application workspace checks that
  sidecar against this workspace lock; bare `validate` in the sidecar's own repository is the
  library sharing gate there.
- Root `update` resolves the path, override, or Git identity into one application graph.
- Graph validation catches duplicate imports if the old file was not removed from the root export,
  and catches case-only collisions on case-insensitive filesystems.
- `build` copies source from both root and dependencies and preserves export-relative import paths
  in the generated bundle.

### Current gaps and pitfalls

- There is no `extract`, `new --lib`, `workspace add`, or source-move command. You create the
  manifest, license, directories, public API boundary, and dependency edge manually.
- There is no committed multi-member workspace model. The root treats a vendored child as an
  ordinary path dependency; commands run in the child start a separate solve.
- A sibling `../color-math` path dependency is allowed but warns because cloning only the root
  repository will not reproduce it. Use a child path for committed vendoring or an override for
  sidecar extract.
- Path dependencies require a manually chosen exact `as` version even when they have never been
  published.
- A manifest path dependency cannot later be locally overridden with `override`; change the
  manifest relationship or publish a Git identity first.
- One package name maps to one selected version. Extraction cannot introduce another unrelated
  package with the same name already present in the graph.
- Moving a module can expose accidental dependency direction or visibility problems. The package
  tool detects graph and layout errors, not architectural cycles in your intended API.

## Journey 8: build binary artifacts (experimental)

Everything before this point uses stable commands. `.slang-module` generation, host executable
compilation, and binary `run` are experimental, so they are separated here: the journeys above
stay valid whether or not these features ship in their current form. Source `run` is stable.

### Goal

Generate unstable `.slang-module` binaries or compile a package module into a native executable,
accepting that the artifacts, command spelling, and features may change.

### Human does

Starting from the `image-viewer` package of Journey 1, opt in to module generation:

```sh
slang package --experimental build
```

When `workspace.bundle.modules` is enabled, this writes `.slang-module` files under
`build/bundle/modules`. The command emits a warning every time because their binary format is not
stable. The adjacent `provenance.json` records that the format is experimental and unstable,
along with the compiler version, source commit, tracked-source dirty state, and path.

To build a host executable too, give the entry module a C++-visible entry point:

```slang
// src/image-viewer.slang
module image_viewer;

export __extern_cpp int main()
{
    return 0;
}
```

Declare the executable in the `build.host` section of `slang-package.json`:

```json
"build": {
  "host": {
    "executables": ["image-viewer"],
    "default": "image-viewer"
  }
}
```

Plain `slang package build` still produces the stable source bundle and skips host and module
binaries. The stable run path interprets that source bundle:

```sh
slang package build
slang package run
```

Opt in to binary outputs and native execution with the global flag, which must appear before the
subcommand, and the run-specific `--binary` option:

```sh
slang package --experimental build
slang package --experimental run --binary
```

Both run modes accept an optional executable name and forward every remaining argument verbatim,
with no `--` separator:

```sh
slang package run image-viewer --input frame.exr
slang package --experimental run --binary image-viewer --input frame.exr
```

The leading value is treated as an executable name only when it matches a configured one, so an
application flag in that position is still forwarded.

### Tool does

- `build` performs the same validation, source-bundle, and documentation work as the stable path.
  When enabled in the manifest, it additionally compiles `.slang-module` files and emits a warning
  about their unstable binary format.
- Module provenance records `experimental: true`, `format_stability: "unstable"`, and the
  compiler source commit and dirty state so copied artifacts retain their compatibility boundary.
- Host executables and runtime libraries are written under `build/host`, which contains
  `EXPERIMENTAL.txt` even when copied separately from the rest of the build tree.
- `build` without `--experimental` produces source and documentation only, regardless of module or
  host settings, and removes stale module and host directories from an earlier experimental build.
- `run` interprets the already-built source copy selected by the optional name, otherwise
  `build.host.default` or the only configured executable.
- `--experimental run --binary` executes the corresponding already-built native artifact.
- Neither mode builds or resolves packages.
- `slang package --experimental help` documents the binary build and run options.

### Current gaps and pitfalls

- There is no package-level build script. Host executables need a supported C++ compiler and the
  sibling Slang tools available at runtime, and the tool does not check for the C++ compiler as
  part of the toolchain constraint.
- Because `run` never builds, a stale source copy or native artifact runs silently after a source
  edit. Run the corresponding build first.
- If the selected output does not exist, `run` reports the missing path and build command instead
  of attempting a build.
- A stable build intentionally does not diagnose missing host toolchains or invalid executable
  entry points because it does not attempt those outputs.

## Lessons from established package workflows

The journeys above expose gaps that other ecosystems have already named. Slang is Git-first and
source-oriented, so their exact commands are not the design, but their human workflows are useful
checks.

### Cargo: scaffold, add, and commit a shared workspace

Cargo distinguishes application and library scaffolds with
[`cargo new --bin` and `cargo new --lib`](https://doc.rust-lang.org/cargo/guide/creating-a-new-project.html).
It edits dependency intent with
[`cargo add`](https://doc.rust-lang.org/cargo/commands/cargo-add.html), and its
[workspaces](https://doc.rust-lang.org/cargo/reference/workspaces.html) give committed members one
root lock and shared configuration.

That suggests three workflow checks for Slang:

- `init` should leave manifest structure useful without pretending to choose application source,
  library source, or a license for the user.
- `dependency add` and `dependency pin` should edit manifest intent without assuming a package
  registry; update remains the separate operation that selects or re-checks a pin.
- A future committed member list should model packages that are always developed and checked in
  together, with one root solve.

Cargo also separates graph inspection from updating:
[`cargo tree`](https://doc.rust-lang.org/cargo/commands/cargo-tree.html), especially its inverted
`-i` view, can inspect who depends on a package after the update. Slang's `tree` and `why NAME`
provide the corresponding selected-graph inspection; the update report additionally records why
candidates were rejected.

### Go: keep local composition separate from published intent

Go starts a module with
[`go mod init`](https://go.dev/doc/modules/managing-dependencies) and composes local modules using
[`go work use`](https://go.dev/doc/tutorial/workspaces). The
[module reference](https://go.dev/ref/mod) generally advises against committing `go.work` unless
the modules are always developed together, because a local overlay can make tests differ from
what downstream users build.

That is the reason to keep Slang's current `slang-package-overlay.json` gitignored. It is closer to a
Go local workspace or replacement overlay than to a committed Cargo workspace. If Slang adds
committed package members, they should be a different concept and file.

Unlike Go, Slang package identity is not an import prefix. A future workspace command should not
couple the Git/package name to export-relative Slang module names.

Go also provides
[`go mod why`](https://go.dev/ref/mod#go-mod-why). The consumed-package-growth journeys need the
same durable question: “why is `color-math` in my graph?” Go answers from imported packages;
`slang package why color-math` answers from manifest and lock edges and prints every dependency
path.

### npm: initialize and connect a child in one workflow

npm can create a member with
[`npm init -w`](https://docs.npmjs.com/cli/v12/using-npm/workspaces/) and add dependencies for a
specific member with workspace-aware install commands. The root
[`package-lock.json`](https://docs.npmjs.com/cli/v12/configuring-npm/package-lock-json) records the
installed application graph; nested locks are not the consuming root's graph.

The useful lesson is not npm's node-module hoisting. It is that “create this child and attach it to
this application” can be one human operation. Slang's current extraction journey needs separate
directory creation, `init`, source moves, manifest edits, an exact `as`, and root update.

[`npm explain`](https://docs.npmjs.com/cli/v12/commands/npm-explain) is another precedent for a
post-update explanation command.

### Gradle: distinguish one build from substituting another build

Gradle's `include` subprojects are the closer analogy for a committed child path package. Its
[composite builds](https://docs.gradle.org/current/userguide/composite_builds.html) are explicitly
independent builds rather than subprojects, and
[`includeBuild` for a local fork](https://docs.gradle.org/current/userguide/how_to_use_local_forks.html)
substitutes a local build for the same published coordinates. That second relationship is closer
to a gitignored Slang override. Slang approximates these two needs with manifest path dependencies
and local overrides, but it has no first-class committed member list.

Gradle's
[`dependencyInsight`](https://docs.gradle.org/current/userguide/viewing_debugging_dependencies.html)
also shows why update reports alone are insufficient: people need to inspect selection reasons
without performing another update.

### Peer pitfalls to retain in Slang's design

- **Do not leak a local overlay into the published graph.** `slang-package-overlay.json` should stay
  local; committed path or Git edges belong in the manifest.
- **Do not confuse a package's own lock with what consumers select.** The solve root owns the
  definitive graph, as with root locks in Cargo and npm.
- **Do not test only the overlay in CI.** CI should fetch the portable committed lock, not depend
  on local overrides.
- **Do not make users hand-merge a generated lock.** The recovery should be to resolve manifest
  intent and regenerate it.
- **Show what would ship.** Cargo and npm provide package/pack previews before publication.
  Slang's `exports` and license files define similar content, but there is no preview command.
- **Retraction is not deletion.** Existing locks remain reproducible; deliberate update consults
  publisher advice. This matches the useful property of Cargo yanks and Go retractions even though
  Slang's Git implementation differs.

### What not to copy

Slang does not need npm-style hoisting, Gradle's opt-in locking model, or a registry-first
publishing workflow to fix the journeys above. Remaining improvements include committed
multi-package composition, package-content preview, testing, and transactional updates. Unlike
Cargo's introductory loop, `run` should not be read as build-and-run; it deliberately executes
only an existing source copy or native artifact.

## How flags change the journeys

The safe default is to omit flags. This section explains when a person should depart from that
default and what remains invariant.

### `update --dry-run`

**Use it when:** you want to preview a first lock, a dependency update, an upstream graph change,
or a local-override solve.

**It changes:** the resolver still reads candidate manifests and prints the same detailed or
minimal selection report, but it does not write `slang-package-lock.json`, replace dependency
checkouts, or regenerate materialized state. Resolver caches under `.slang/cache/` may still be
populated. The preview lists existing dependency refs that a real apply would move.

**It does not prove:** that remote source passes license and module-layout validation. Those trees
are not materialized during the preview.

**Combinations:** use it with `--minimal`, `--ignore-overrides`, or `--offline`. `--dry-run --clean`
is rejected because there is no checkout replacement to authorize.

### `update --minimal`

**Use it when:** automation or an experienced user needs a compact list rather than selection
rationale.

**It changes:** report formatting only. Added, removed, upgraded, downgraded, replaced, and
unchanged package lines remain, followed by summary counts. A detailed report already includes
that information plus rationale, so it does not repeat the one-line list.

**It does not change:** resolution, validation, materialization, or lock output. It is valid on
both dry-run and real update.

### `update --offline`

**Use it when:** `.slang/cache` is already populated and you need to re-resolve or rematerialize
without contacting package Git remotes.

**It changes:** `update` lists tags, resolves refs, reads cached manifests, and materializes
checkouts from the local cache. It does not fetch or clone the package URL. A missing cache, an unknown ref, or a commit that is not a local object
fails and tells you to re-run without `--offline`.

**It does not change:** the cache-to-`deps/` staging path. Online update uses that same path after
refreshing the cache from origin. Fetch and validate also refresh caches; status, tree, why, and
docs remain local readers. Offline mode is update-only in the current command surface, so fetch
still fails when it cannot refresh an origin even if its cache is otherwise warm.

**Combinations:** use it with `--yes`, `--dry-run`, `--minimal`, `--ignore-overrides`, or
`--skip-validate`. `--dry-run --offline` still writes neither lock nor checkouts.

### `update --yes`

**Use it when:** a non-interactive caller has already decided to apply the report.

Without this flag, a real update resolves once, prints the exact selected graph, and defaults to
“no” at its confirmation prompt. It then materializes that same in-memory lock without refreshing
remote selection a second time. `--dry-run` remains an advisory preview across invocations; a later
update may see newer remote state.

The prompt only appears when there is a decision to make: a solve that would change the committed
lock, a `--clean` run that would discard local checkout state, or cache staging that would move an
existing tag or origin-tracking branch in `deps/`. All affected repositories are listed before
that one prompt. Re-running `update` on a graph that already matches the lock and has only additive
cache state exits without asking. Materialization still restores missing checkouts and advances
out-of-date ones. Answering “no” prints that nothing was applied and exits
successfully, so a declined update does not fail a script that treats a non-zero exit as a broken
workspace.

### Enabled overrides and `update --ignore-overrides`

**Use enabled overrides when:** unpublished local trees should participate in every plain update.
The resolver uses each enabled override as the only candidate for that package name.
Non-overridden and disabled packages still resolve from Git. The resulting lock records local
paths and requires the same `slang-package-overlay.json`.

**Use `--ignore-overrides` when:** this command should write the published Git graph without
disabling registrations. In-place overrides created by `edit` stay active because they own their
`deps/NAME` checkouts; out-of-tree overrides are ignored for this solve. An in-place override that
drops out of the published graph remains parked, and the next plain update can reattach it.

**They do not mean:** “update only this package” or “use every nearby repository.”

Use `override enable` and `override disable` for persistent per-package switches.

### `fetch --clean` and `update --clean`

**Use it when:** the tool is about to replace a checkout it owns, but that tree has changed files,
extra commits, or stashes that you intentionally want to discard.

**It changes:** dirty-checkout protection for replacement. It is destructive authorization, not
dependency selection.

**It does not change:** an enabled local override. Local package registrations remain protected by
their own workflow.

**Combination:** `update --dry-run --clean` is rejected. Fetch may combine `--clean` with
`--skip-validate`. When fetch would actually discard local checkout state, it lists every affected
package and asks once. Pass `--yes` only when that destruction was pre-approved. Build does not
accept `--clean`.

### `fetch --yes`

**Use it when:** fetch has no lock and would run update, `--clean` would discard checkout state, or
cache staging would move existing named refs, and there is no interactive terminal.

**It does not change:** an existing lock. Build does not take `--yes`; a missing lock makes the
nested update run as `update --yes` so a first clone can `slang package build` without a prompt.

### `fetch --skip-validate`, `update --skip-validate`, and `build --skip-validate`

**Use it when:** a validation bug or temporarily invalid source tree blocks an investigation and
you accept that later compilation may fail. It is a workaround, not the CI path.

**It skips:** new-release license and portability checks, the first `module` / `implementing`
declaration rules, and graph-wide import uniqueness.

**It keeps:** the legal graph (identities, trusted edges, toolchain, exclusions) before any
`deps/` change, plus dirty-checkout protection and the rest of the command's normal side effects
other than source-layout and publish checks. It always prints a warning.

**Combinations:** `update --dry-run --skip-validate` still runs the legal graph from cache and
still cannot inspect remote source layout. `build --skip-validate` passes the flag through to
fetch when build has to materialize missing locked trees. `validate` intentionally has no skip
flag.

### `override add NAME PATH [AS]`

`AS` is a positional exact version, not a global flag. Omit it only when `NAME` already has a lock
row whose version the local tree represents. Supply it for a newly introduced name or when the
local tree represents another version. The value must satisfy every incoming constraint when you
run `update`.

### `unedit NAME --clean` and `unedit NAME --adopt [--ref REF] [--as VERSION]`

Use plain `unedit` only when the checkout is clean at the Git commit in the lock. `--clean`
authorizes restoring that commit and discarding local files, commits, and stashes. Both reject a
Git+path lock row; disable the override and update first when the local graph was already solved.

Use `--adopt` to keep a committed fix. It changes a direct Git dependency in
`slang-package.json` to `ref` plus `as`, writes `HEAD` as the lock commit, and removes the local
override. A unique semantic-version tag at `HEAD` supplies both `ref` and `as` when `--ref` is
omitted. An untagged `HEAD` uses its object ID as `ref` and derives `as` from the nearest ancestor
release tag, or requires `--as VERSION` when none exists. `--adopt --ref BRANCH` records that
branch as `ref` while still locking `HEAD`. After the overlay is removed, the
committed tree's declared graph must still resolve onto that Git pin (the package name must match,
and live edges must still select the lock). Extra export paths alone are not a mismatch. Both
destructive clean and adopt require confirmation; pass `--yes` for automation. Adopt does not copy
the commit from `deps/NAME` to `.slang/cache`. Push it to origin before `validate`, or before
another workspace can fetch it.

### `--experimental`

**Use it when:** you need `.slang-module` binaries or host executables. Journey 8 covers both
workflows.

**It changes:** `build` also compiles enabled `.slang-module` output and configured host
executables, and `run --binary` becomes available. The flag is global and must appear before the
subcommand. Source `run` stays available without it. Every other journey in this chapter is
unaffected by it.

**It does not change:** validation, resolution, bundle output, or documentation collection.

### Help spellings and commands without flags

`slang package help`, `-help`, and `--help` print stable package help, including source `run`.
Commands are grouped by the file they primarily write or report: the manifest
(`slang-package.json`), the overlay (`slang-package-overlay.json`), the lock
(`slang-package-lock.json`), then build. `unedit --adopt` is listed under overlay because it
drops that registration; it also writes the manifest and lock. Stable help says
`--experimental` enables experimental options; `slang package --experimental help` lists
`run --binary` and the extra `build` outputs. `init`, `status`, `tree`, and `edit` accept no
additional arguments; `validate` accepts an optional package name or `--all`; `build` accepts
only `--skip-validate` (not `--clean` or `--yes`); `unedit` accepts `--clean`, or `--adopt`
with optional `--ref` and `--as`, plus `--yes`; and `docs` accepts `--print`. `test` is present
but returns a not-implemented error.

## Gaps, tensions, and intentional asymmetries

These behaviors can look contradictory from a user's point of view. Some preserve an important
invariant; others are unfinished workflow.

### Init creates an invalid package

`init` creates useful structure and a license reminder, but the placeholder makes immediate
validation fail. This is an onboarding gap, not a desired invariant. App/library templates should
produce a source scaffold and make the remaining human obligation obvious; whether a generated
license can ever be valid requires an explicit license choice.

### Status reports state; validate gates sharing

`status` prints one line when the lock, checkouts, and graph are current, and lists only the
dirty items otherwise. It also checks that the existing cache contains every locked Git ref and
commit, without refreshing it. Missing pins are `incomplete`; a present graph that fails the source
check is `not buildable`. Like `git status`, reportable drift still returns success; malformed
required JSON is an error because no reliable report can be produced. Bare `validate` applies the
license, source-layout, and path-portability rules to the workspace package as the app sharing
gate. `validate NAME` and `validate --all` apply those publishable-package rules to locked
package trees in this workspace.

### Fetch ignores retractions but honors root excludes

A publisher retraction is new advice about a release that an existing lock may continue to
reproduce. A release tag that later points at a different commit is the same kind of advice:
fetch still installs the SHA the lock recorded. It separately discloses and asks before changing
the existing tag ref under `deps/`; the next `update` is what can select the new identity for the
lock. A root `workspace.excludes` entry is current committed intent for this workspace, so a
lock selecting that version is stale. The asymmetry is intentional: publisher advice does not
retroactively break reproducibility, while the workspace's own changed policy does.

### Dry-run is not a complete rehearsal

Dry-run promises no lock or checkout mutation. It does run the legal-graph check against selected
manifests in `.slang/cache` and local trees, and reports named refs that applying the cache would
move. It still cannot validate remote source layout, so a
dry-run report can be correct and the real update can still fail after materialization.

### Failed update is not fully transactional

A graph that is illegal is rejected before search paths are cleared or `deps/` is rewritten. A
content-only failure happens after materialize: the previous lock remains, search paths may be
empty, and some `deps/` directories may already contain the candidate graph. With an old lock, run
`slang package fetch` to restore its checkouts and search paths. A failed first update can leave
partial checkouts and no lock.

### Edit and override solve different problems

`edit` and `override add` write the same kind of registration in `slang-package-overlay.json`. The
difference is which directory it names. `edit` claims the checkout that is already at
`{workspace.dependencies}/NAME`, so ownership changes without moving compiler inputs; fetch and update
stop replacing that tree immediately. `override add` with another path records a sidecar tree;
`update` is what writes that path into the lock and resolves the overlay's dependency graph.
`--ignore-overrides` still leaves in-place edits active so it cannot replace `deps/NAME`.

### Local solve writes a definitive but non-portable lock

An enabled-override update writes the normal root lock even though that lock requires gitignored
local registrations. Other machines fail rather than silently using another graph, which is safe,
but the user must disable overrides and update before sharing. A future tool could distinguish or
label a local trial lock more visibly.

### A nested lock does not protect a package's consumers

Each package repository may lock its own development workspace. A consuming root ignores that lock
and re-solves from manifests. This is intentional root-lock ownership, but it surprises authors
who expect their library lock to pin downstream applications. Upstream splits and added
dependencies therefore appear only in each consumer's root lock.

### Run, docs, and test do less than their names may imply

`run` does not build, `docs` opens `build/docs/index.md` (or prints the path with `--print`), and
`test` is
unimplemented. Narrow side effects make commands predictable, but missing `build --run`,
documentation generation, and package testing leave common loops manual.

### One package name has one version

The resolver unifies all incoming constraints on one package name. This keeps module and lock
identity simple, but prevents side-by-side incompatible majors under the same name. An upstream
split that reuses an existing graph name must unify with it or choose a distinct package name.

### Skip-validate is safe only within its stated boundary

The flag lets a user materialize or build around source validation, while preserving manifest and
lock identity checks. That is a deliberate workaround boundary. It does not make an invalid graph
publishable or suitable for CI, and it cannot guarantee build success.

## Prioritized gaps

The journeys suggest the following order. This is a product backlog, not a promise that every item
belongs in the package tool.

### Correctness and recovery

1. Stage candidate checkouts and validate before replacing `deps/` or writing the lock, so update
   is transactional from the user's point of view.
2. Give a failed first update an explicit recovery command and remove partial tool-owned state.
3. Add tests that cover upstream packages adding dependencies and splitting modules, including
   unchanged-package import collisions.

### Onboarding and ordinary dependency work

1. Decide whether optional source scaffolding belongs in `init` without conflating manifest
   initialization with application/library policy.
2. Add package registry search if Slang later gains a registry.
3. Implement a package testing contract for the generated `tests/` directory.
4. Add a package-content listing to complement the sharing checks performed by `validate`.

### Multi-package development

1. Define a committed member model, distinct from gitignored `slang-package-overlay.json`, for packages
   that are always developed and checked in together.
2. Add a command that initializes and attaches a child package while preserving import paths.
3. Make local trial state visibly different from a portable lock, or provide an explicit command
   that verifies a lock is portable before commit.
4. Consider package-scoped update after defining how it interacts with one-version-per-name
   unification and transitive graph changes.

### Publication and distribution

1. Add a release validation/pack step before adding push or registry behavior.
2. Define immutable-tag diagnostics, signing, and provenance expectations.
3. Defer registry/search and user-global Git remapping until the Git-first workflow is complete.

## Maintainer appendix: command contracts

Use these checks when changing `tools/slang-package/`. A behavior change should either preserve
them or update this chapter and its regression tests in the same change.

### Bootstrap contract

- `init` creates the manifest, conventional directories, placeholder license, and ignore entries.
- Default `validate` rejects the workspace license placeholder. Build allows it, while update and
  fetch reject it when it appears in a new or changed Git checkout or a changed local registration
  (an edit or enabled override). A vendored path package is not publish-checked by update or fetch,
  because consuming an in-repo tree through a path dependency is not a claim that the tree could be
  published. `validate NAME` and `validate --all` apply the publishable checks to any locked tree,
  including a path row, without requiring a portable workspace.
- A dependency-free valid package can validate and build without a lock.

### Resolve and reproduce contract

- `update` is the command that reselects versions and writes a graph lock. Fetch with an
  existing lock does not reselect. Fetch with dependencies and no lock delegates to update
  (prompt, or `--yes`). Build never rewrites an existing lock; with no lock it delegates to
  fetch, which delegates to update `--yes`.
- `update --dry-run` writes neither lock nor dependency checkouts.
- A real update reports one selected in-memory graph, confirms it when it differs from the
  committed lock, and applies that exact graph. A declined confirmation applies nothing and is not
  an error.
- Fetch with an existing lock selects nothing and does not rewrite that lock. Fetch with
  dependencies and no lock announces that it is running update, then performs the confirmed
  initial solve and writes the first lock. Build with no lock announces fetch, then that
  nested fetch announces update `--yes`.
- A real update writes the lock only after the candidate graph validates.
- Every reachable dependency has one exact lock row; Git rows include ref and commit.
- Path packages remain in place; Git packages materialize under the configured deps directory.
- Materialization leaves a tool-owned, clean Git checkout untouched when its origin and `HEAD`
  already match the lock. Missing, dirty, unowned, or out-of-date checkouts still follow the normal
  repair or safety path.
- Fetch, validate, update, build, status, and local-registration changes reject a path-only lock
  row when the corresponding manifest edge requires Git. This prevents a lock edit from
  redirecting a published dependency to arbitrary local source.
- Publisher retractions affect update selection but not an existing fetched lock.
- Root workspace exclusions affect update and make a conflicting existing lock stale for fetch.

### Graph-evolution contract

- A new transitive dependency can add a root-lock row without changing the consumer's manifest.
- A package split can preserve a Slang import only when the new graph exports that import exactly
  once.
- Nested package locks never constrain the consuming root.
- Legal-graph and buildability checks consider the complete reachable graph, including unchanged
  rows. Publishability is checked for the workspace on bare `validate`, for one locked tree on
  `validate NAME`, for every locked tree on `validate --all`, and for changed Git or local
  selections on `fetch` and `update`.

### Local-development contract

- `edit` creates an enabled override at `{workspace.dependencies}/NAME`, using the locked version, and
  prevents replacement of that checkout.
- `edit` accepts a checkout that already holds local changes, and requires only that the directory
  is still the Git repository the lock names. Refusing a dirty tree would withhold the one command
  that preserves the work in exactly the state that needs it.
- Default `unedit` requires a Git-only lock row, no uncommitted files or stashes, and `HEAD` equal
  to the locked commit; `unedit --clean` restores that commit before removing the registration.
- `unedit --adopt` pins a direct Git dependency and the lock to committed `HEAD`, requiring
  explicit `--as` unless one semantic-version tag identifies it.
- `dependency pin` writes Git intent from the current lock without rewriting the lock and does not
  drop an overlay. Default uses a Git-only lock's exact version. An overlay lock row requires
  explicit `--to`; `--commit` is the locked SHA and also requires a Git-only row.
- An in-place override's working-tree manifest enters the solve.
- `override` records a machine-local path and exact effective version. Re-adding
  `{workspace.dependencies}/NAME` updates the same registration; a different path requires `unedit` first.
- Enabled overrides participate in plain whole-graph update; disabled overrides retain
  configuration while published resolution is active.
- `update --ignore-overrides` ignores out-of-tree overrides for this command only. In-place
  overrides remain active so it cannot replace a checkout registered through `edit`. An in-place
  package that drops out of the graph stays registered and on disk so a later update can restore it.
- A local-path lock fails on another machine without matching `slang-package-overlay.json`.
- Disable an override and update to restore published selection before removing it.
- `validate NAME` certifies a locked library tree in this workspace, including an enabled override.
  Bare `validate` still rejects the app while any edit or override is enabled.
- Local-registration changes regenerate `slang-package-includes.txt` from the active tree. In-place `edit`
  does not need `update` to keep compiling `deps/NAME`. Out-of-tree overrides need `update` to
  enter the lock and to activate new dependency edges. Disabling a lock-adopted override requires
  update first.
- Dirty, unregistered Git checkouts are not replaced without `--clean`; enabled overrides remain
  protected. Fetch and update check every checkout the current lock owns before they do anything
  else, so update stops before resolving rather than after reporting a plan it cannot apply. The
  refusal names each checkout and its drift using the same facts `status` prints, and offers the
  same three ways forward: commit or discard, `edit` the checkout, or re-run with `--clean`.

### Validation contract

- Fetch and update always enforce the legal graph **before** materialize, then publish-check
  changed Git and local selections and enforce source layout and import uniqueness across the
  closure after materialize. `--skip-validate` skips only that second stage. Build never rewrites
  an existing lock. If a tool-owned Git checkout is missing, build invokes fetch (without
  `--clean`). If the lock itself is missing and the manifest has dependencies, that fetch runs
  update `--yes` so a first clone can build without a prompt. Each hand-off prints why the inner
  command is running. Bare `validate` checks workspace publishability and lock portability.
  `validate NAME` and `validate --all` check locked trees against this workspace lock.
- `--skip-validate` exists only on fetch, update, and build; it warns and keeps lock, manifest,
  closure, toolchain, export, and dirty-checkout checks.
- `status` diagnoses lock, registration, checkout, and cached upstream state without mutation or
  remote access. A current workspace is one header line; observations appear only when something is dirty. Missing
  pins are `incomplete`; a present graph that fails the source check is `not buildable`. It never
  inspects `build/` and is not the package-quality gate. Reportable drift returns success;
  unreadable or malformed required JSON returns failure.

### Output and side-effect contract

- Detailed update output explains what moved, what stayed, and the incoming constraints.
- `--minimal` retains one-line changes, unchanged packages, and summary counts. Detailed reports
  include rationale only, then the summary. The installed toolchain is not listed on success.
- Dependency add/remove changes only the manifest; tree and why explain the current lock graph
  from existing workspace and cache state without contacting remotes.
- Materialization prints per-package source/checkout progress. A failure explains that the prior
  lock remains authoritative and how to recover potentially partial derived state.
- `docs` opens `build/docs/index.md` with the registered application. `--print` writes the path
  instead. It does not regenerate documentation.
- `test` reports that package testing is not implemented.
- A command failure must not claim that an update, fetch, or build succeeded.

### Experimental binary-artifact contract

Keep this separable from the contracts above, so the stable journeys hold whether or not binary
artifacts ship in their current form.

- `.slang-module` and host executable builds require the global `--experimental` flag before the
  subcommand.
- Stable build distributes source and removes stale module and host output.
- Every module build warns that the binary format is unstable. Module provenance records the
  experimental status, compiler source commit, and tracked-source dirty state.
- Host output lives under `build/host` with an `EXPERIMENTAL.txt` marker.
- `build` without the flag still emits source and docs, skips binary outputs, and removes stale
  module and host directories.
- Stable `run` interprets the bundled source; `--experimental run --binary` executes an existing
  native artifact.
- `--experimental help` adds the binary build and run options.
- Neither run mode silently builds or resolves.

## Executable test anchors

Start with these unit tests when changing a journey:

- Bootstrap and license: `PackageToolInit`, `PackageValidateStructureAndLicense`.
- Fetch and initial lock: `PackageToolFetchRequiresLock`,
  `PackageToolDependencyCommandsAndInitialFetch`,
  `PackageToolFetchInstallsLockedCommitAfterMovedTag`.
- Update preview, confirmation, and report: `PackageToolUpdateDryRun`,
  `PackageToolUpdateRequiresConfirmation`, `PackageToolUpdateSkipsConfirmationWhenLockIsUnchanged`,
  `PackageGitSkipsAlreadyMaterializedRevision`, `PackageResolveReportFormat`.
- Module layout and uniqueness: `PackageCommandsValidateDependencyModuleLayout`,
  `PackageToolUpdateRejectsBundleCaseConflict`, `PackageValidateRejectsFlattenedModuleAlias`.
- Local overrides and enable state: `PackageToolLocalOverrideUpdatesDefinitiveLock`,
  `PackageToolUpdateIgnoresOverrides`, `PackageToolIgnoreOverridesParksEditedDependency`,
  `PackageToolEditAdoptsLocalTree`, `PackageToolEditIsInPlaceOverride`,
  `PackageToolIgnoreOverridesKeepsInPlaceOverride`,
  `PackageToolDisabledInPlaceOverrideProtectsDirtyTree`,
  `PackageToolUneditAdoptsCommitPin`, `PackageToolUneditAdoptRejectsManifestDrift`,
  `PackageToolUneditAdoptsVersionTag`,
  `PackageToolUneditRejectsConflictingOptions`, `PackageToolRejectsLegacyWorkspaceEdits`,
  `PackageLocalRegistryJSON`.
- Path dependencies: `PackageToolPathDependencies`,
  `PackageToolFetchRejectsPathLockForGitDependency`, `PackageToolRejectsPathIntoSlangState`,
  `PackageResolverPathShadowsSelectedGit`, `PackageResolverPathPackageGitTransitive`,
  `PackageResolverDropsConstraintNotesFromShadowedGitPackage`.
- Exclusions and retractions: `PackageResolverAppliesWorkspaceExclusions`,
  `PackageToolFetchRejectsWorkspaceExclusion`, `PackageResolverUsesLatestReleaseRetractions`,
  `PackageResolverBacktracksPastRetractionAndExclude`,
  `PackageResolverUnsatisfiableAfterRetractionAndExclude`.
- Command failure formatting: `PackageToolFailureTranscripts`,
  `PackageResolverReportsNoPublishedCandidates`,
  `PackageResolveFailureIndentsMultiLineCandidateReason`,
  `PackageResolverUnsatisfiableAfterRetractionAndExclude`.
- Toolchain selection: `PackageToolSlangToolchain`, `PackageResolverSlangToolchain`.
- Dependency editing and graph inspection: `PackageToolDependencyCommandsAndInitialFetch`,
  `PackageResolverPinnedRefWithoutVersionConstraint`,
  `PackageToolRefAndAsDependencyWithoutVersionResolves`, `PackageToolDependencyPinFromLock`,
  `PackageToolDependencyPinPromotesTransitiveAndKeepsOverlay`.
- Stable source build and experimental binary artifacts: `PackageToolBuild`, `PackageToolRun`,
  `PackageToolExecutableRequiresWorkspaceSource`,
  `PackageToolBuildFetchesMissingLockedCheckouts`,
  `PackageToolBuildCreatesFirstLockViaFetch`,
  `PackageToolBuildRejectsCleanAndYes`.

The upstream-add and upstream-split journeys do not yet have end-to-end command tests named after
them. Add those anchors when the next resolver or command-lifecycle change touches those cases.

## Review checklist for future command changes

Before merging a package-command change:

1. Name the human journey it changes.
2. List which of manifest, lock, workspace registration, checkouts, caches, search paths, and build
   outputs it reads or writes.
3. State how every accepted flag changes that journey, including combinations.
4. Check clean clone, dirty checkout, local override, dry-run, validation failure, and CI
   reproduction behavior.
5. Explain recovery after every failure that can occur after a side effect.
6. Run the relevant anchors above and add a journey-level regression when the behavior is new.
