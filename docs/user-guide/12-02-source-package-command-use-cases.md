---
layout: user-guide
permalink: /user-guide/source-package-command-use-cases
---

Growing an Application with Source Packages
============================================

This chapter follows a person from an empty directory to an application made from several
packages. It says what you do by hand, what `slang package` does for you, and where the current
tool stops helping. Later sections follow the same application when an upstream package changes
its graph, and when you extract part of your own application into a package.

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
  `update`. The tool writes it; you review and commit it. A dependency's nested lock is not used
  when your workspace resolves that dependency.
- `slang-workspace.json` is machine-local state for `edit` and `override`. The tool writes it and
  `init` adds it to `.gitignore`. Do not commit it.

The **workspace** is the package from which you run the command. It owns the one lock for that
solve. This is not a multi-member workspace in the Cargo or npm sense: Slang currently has no
committed list of packages that are developed together.

## Journey 1: bootstrap one application

Assume you are starting an application named `image-viewer` and no package exists yet.

### Goal

Create a valid package, compile one module as a native executable, and make the first source
control commit.

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
`src`, names `LICENSE`, configures `deps` and `build`, and records the installed Slang version as
the minimum `tools.slang-toolchain` version when the version is available.

The generated package is deliberately incomplete. It does not add a `host` section. Replace the
placeholder text in `LICENSE`, then add the first module:

```slang
// src/image-viewer.slang
module image_viewer;

export __extern_cpp int main()
{
    return 0;
}
```

The filename uses a hyphen while the declaration uses the canonical underscore spelling. Add the
matching executable to the root-level `host` section in `slang-package.json`:

```json
"host": {
  "executables": ["image-viewer"],
  "default": "image-viewer"
}
```

Run the package-quality gate, build, and then run the artifact:

```sh
slang package validate
slang package build
slang package run
```

Finally, initialize source control if needed and commit:

```sh
git init
git add .
git commit
```

Commit source, the manifest, licenses, tests, and docs. Do not commit `.slang/`, `deps/`, `build/`,
or `slang-workspace.json`.

### Tool does

- `init` creates the manifest, conventional directories, license placeholder, and ignore rules.
- `validate` checks the closed manifest schema, license, export directories, module declarations,
  and installed toolchain. A package with no dependencies does not need a lock.
- `build` repeats full validation, emits source and module bundles under `build/bundle/`, compiles
  configured host executables, and collects Markdown under `build/docs/`.
- `run` executes the existing configured binary. It forwards trailing arguments but does not
  rebuild.

### Current gaps and pitfalls

- `init` does not ask whether you want an application or library, does not write a first source
  file, and leaves a license placeholder that makes the default `validate`, `update`, and `build`
  paths fail. Missing `init --app` and `init --lib` templates are the first onboarding gap.
- There is no package-level build script. Host executables need a supported C++ compiler and the
  sibling Slang tools available at runtime.
- `slang package test` is reserved but not implemented. The generated `tests/` directory is only a
  convention today.
- `run` does not build, and `docs` does not regenerate documentation. Run `build` first.
- An application that invokes `slangc` itself must consume the export paths written to
  `build/search-paths`; the package state is not injected into arbitrary compiler sessions.

### First checkpoint

At this point a new clone can reproduce the package without dependency resolution:

```sh
git clone <image-viewer-url>
cd image-viewer
slang package validate
slang package build
slang package run
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

There is no `slang package add` command. Edit `slang-package.json` by hand:

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

The manifest records the acceptable range. The lock records the exact tag, commit, effective
version, dependencies, and exports selected for this workspace.

### Tool does

- `update` clones resolver metadata under `.slang/cache/`, examines compatible Git tags, resolves
  the complete transitive graph, and selects one version per package name.
- A real update materializes Git source under `deps/NAME`, validates the selected graph, writes
  `slang-package-lock.json`, and regenerates `build/search-paths`.
- `status` checks that the root manifest, lock, local registrations, materialized manifests, and
  tool-owned Git checkouts agree. It diagnoses workspace state; it is not a replacement for
  source and license validation.
- `fetch` subsequently reproduces that lock without consulting newer tags, publisher retractions,
  or version selection:

  ```sh
  slang package fetch
  slang package build
  ```

  This is the normal clean-clone and CI path. CI should not run `update`.

From this point forward, the clean-clone flow has a lock and starts with fetch:

```sh
git clone <image-viewer-url>
cd image-viewer
slang package fetch
slang package build
slang package run
```

### Current gaps and pitfalls

- Dependency declaration, removal, and pinning are manual JSON edits. There is no `add`, `remove`,
  or package search command.
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

Replace the license placeholder. A library does not need `host.executables`, so do not add that
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

- `validate` checks that the library's files follow the package module rules before you tag it.
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

There are two workflows because “edit these source files” and “try a different package graph” are
different operations.

### Goal

Choose between patching source at the published identity and trialing an unpublished manifest or
version.

### Human does

To patch source without changing package metadata, make the materialized checkout
developer-owned:

```sh
slang package edit color-encoding
# edit deps/color-encoding/src/...
```

The checkout stays at `deps/color-encoding`; the lock keeps its published Git pin. Fetch and update
do not replace it while the edit is registered. When the checkout is back at its locked commit
with no changed files or stashes:

```sh
slang package unedit color-encoding
```

`unedit` refuses while work would be lost.

To trial an unpublished manifest or another repository, point the existing package name at a
directory:

```sh
slang package override color-encoding ../color-encoding 1.1.0
slang package update --from-local --dry-run
slang package update --from-local
```

`--from-local` is not “update one local package.” It re-solves the **entire graph** using every
registered override as the only candidate for its package name; packages without overrides still
come from Git. Registered edits remain published Git candidates, so their changed manifests do not
enter the solve.

If the local `color-encoding` manifest adds `color-math`, the full-graph local solve adds
`color-math` and its transitive requirements to the root lock. Every incoming constraint must
accept the override's exact `as` version.

Omit `AS` only when the name already has a lock row; the command reuses that version. An override
for a name absent from the lock requires `AS` and also needs a reachable dependency declaration in
the root or another resolved local manifest.

Restore a portable graph before sharing it:

```sh
slang package update
slang package unoverride color-encoding
```

A normal update restores published Git selections. `unoverride` refuses while the lock still
contains the local path row.

### Tool does

- For an edit, writes a registration to gitignored `slang-workspace.json`, protects the checkout
  from replacement, and keeps the published manifest and Git identity in the solve. Changing the
  edited manifest's dependencies or exports does **not** adopt that graph.
- For an override, records the name, path, and exact effective version in
  `slang-workspace.json`. It does not copy or modify the supplied directory.
- `edit`, `unedit`, `override`, and `unoverride` immediately regenerate `build/search-paths` to
  reflect the active registration. An override's locked export paths therefore become compiler
  inputs immediately, mapped to the local directory. If its manifest declares different exports,
  run `update --from-local` to adopt them.
- With `update --from-local`, records the original Git identity and local path in the definitive
  lock and resolves all current override manifests together.
- A local-path lock fails on another machine without the matching registration. This is
  intentional: a local graph must not silently masquerade as the portable published graph.

### Current gaps and pitfalls

- `--from-local` is easy to misread as a package-scoped update. It is a mode of the full-graph
  solver.
- An edit is not a way to trial manifest changes. Use an override, or publish a new tag and run
  normal `update`.
- If an edit is registered while you run `update --from-local`, its checkout HEAD must match a
  published release tag. An unpublished edit commit makes the local solve fail; use an override
  when the version or manifest identity differs.
- Overrides are path-only; there is no user-global Git-to-Git remapping policy.
- `slang-workspace.json` must not be committed. A team-wide source relationship belongs in
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
lock gains an exact row for it.

### Tool does

- Resolves manifests recursively from the selected candidate releases.
- Adds the new row to the one root lock and materializes it under `deps/color-math`.
- Validates module import uniqueness across both changed and unchanged packages.
- Explains the incoming constraint in the update report.

### Current gaps and pitfalls

- There is no `tree` or `why color-math` command after the update. The update report is currently
  the only explanation of why the name entered the lock.
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
slang package override color-encoding ../color-encoding 2.0.0
slang package override color-transfer ../color-transfer 1.0.0
slang package update --from-local --dry-run
slang package update --from-local
```

The solve is still whole-graph. The first override's local manifest introduces the second package;
the second override supplies its exact local candidate. This is the current way to test a
multi-package upstream change before publication.

After publishing both tags, restore the portable graph:

```sh
slang package update
slang package unoverride color-encoding
slang package unoverride color-transfer
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
- There is no durable dependency-tree view after resolution. The user has to inspect the update
  report or lock to understand the new graph.
- Dry-run reads candidate manifests but cannot validate the unmaterialized packages' source trees.
- If real update materializes the split and then source validation fails, the previous lock stays
  unchanged but some dependency directories may already have moved.
- Publisher retractions and workspace exclusions are asymmetric: a new retraction does not break
  the consumer's old lock on fetch, while a newly committed root exclusion makes that lock stale
  and fetch rejects it.

## Journey 7: split your application into packages

Return to `image-viewer`. Its `src/color/math.slang` has become useful enough to extract.

### Goal

Move code behind a package boundary without unnecessarily rewriting Slang imports, then choose
whether the new package is committed with the application, developed in a sibling repository, or
published independently.

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
- whether the package should be a permanent part of this repository or an independently released
  project.

The tool does not analyze or perform this split.

#### Option A: commit a child package with the application

Create a package below the repository:

```sh
mkdir -p packages/color-math
cd packages/color-math
slang package init
```

Replace its license placeholder, do not add application-only `host` settings, and move the source:

```text
packages/color-math/
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

Validate it from its own directory:

```sh
slang package validate
```

In the root `image-viewer/slang-package.json`, add a committed path dependency:

```json
"color-math": {
  "path": "packages/color-math",
  "as": "1.0.0"
}
```

Then return to the root workspace:

```sh
slang package update --dry-run
slang package update
slang package build
```

The path package stays under `packages/color-math`; it is not copied to `deps/`. Commit the child
package, root manifest, and root lock together. Only the root lock controls the application's
solve; the child can have its own lock when developed independently, but that lock is ignored by
the root.

This is the most reproducible current form of a repository-local split. It is explicit in the
published root manifest and works on another machine without `slang-workspace.json`.

#### Option B: develop the extracted package in a sibling repository

Suppose the intended published identity is
`https://example.com/color-math.git`, but no release exists yet. Add the intended portable
dependency to the root manifest:

```json
"color-math": {
  "git": "https://example.com/color-math.git",
  "version": ">=1.0.0 <2.0.0"
}
```

Register the sibling directory with an explicit version:

```sh
slang package override color-math ../color-math 1.0.0
slang package update --from-local --dry-run
slang package update --from-local
```

The override lets the solver use the local manifest without a release tag. The resulting lock is
machine-local and requires the matching gitignored workspace registration. Do not use this lock
as the portable state for CI. A normal update cannot satisfy the Git edge until a compatible
release tag exists.

After publishing `v1.0.0`, run a normal update to replace the local row with the Git tag, then
remove the registration:

```sh
slang package update
slang package unoverride color-math
git add slang-package.json slang-package-lock.json
git commit
```

#### Option C: publish first, then consume

Create the package in its own Git repository, validate it, tag `v1.0.0`, and push it as in
Journey 3. Add `git` plus `version` to the application's manifest and run normal `update`.
This avoids machine-local lock state but requires the package to be publishable before the
application can consume it.

### Tool does

- `init` scaffolds each package, and `validate` checks each package independently.
- Root `update` resolves the path, override, or Git identity into one application graph.
- Graph validation catches duplicate imports if the old file was not removed from the root export,
  and catches case-only collisions on case-insensitive filesystems.
- `build` compiles primaries from both root and dependencies and preserves export-relative import
  paths in the generated bundle.

### Current gaps and pitfalls

- There is no `extract`, `new --lib`, `workspace add`, or source-move command. You create the
  manifest, license, directories, public API boundary, and dependency edge manually.
- There is no committed multi-member workspace model. The root treats the child as an ordinary
  path dependency; commands run in the child start a separate solve.
- A sibling `../color-math` path dependency is allowed but warns because cloning only the root
  repository will not reproduce it. Use a child path for committed co-development or a local
  override for machine-local work.
- Path dependencies require a manually chosen exact `as` version even when they have never been
  published.
- A manifest path dependency cannot later be locally overridden with `override`; change the
  manifest relationship or publish a Git identity first.
- One package name maps to one selected version. Extraction cannot introduce another unrelated
  package with the same name already present in the graph.
- Moving a module can expose accidental dependency direction or visibility problems. The package
  tool detects graph and layout errors, not architectural cycles in your intended API.

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

That suggests three concrete Slang improvements:

- `init --app` and `init --lib` should leave a valid, useful starting package.
- `add` should edit the manifest and show the selected pin instead of requiring JSON editing. The
  Slang form would take a Git URL and version range, not assume a package registry.
- A future committed member list should model packages that are always developed and checked in
  together, with one root solve.

Cargo also separates graph inspection from updating:
[`cargo tree`](https://doc.rust-lang.org/cargo/commands/cargo-tree.html), especially its inverted
`-i` view, can inspect who depends on a package after the update. Slang currently explains a
dependency only while printing the update report.

### Go: keep local composition separate from published intent

Go starts a module with
[`go mod init`](https://go.dev/doc/modules/managing-dependencies) and composes local modules using
[`go work use`](https://go.dev/doc/tutorial/workspaces). The
[module reference](https://go.dev/ref/mod) generally advises against committing `go.work` unless
the modules are always developed together, because a local overlay can make tests differ from
what downstream users build.

That is the reason to keep Slang's current `slang-workspace.json` gitignored. It is closer to a
Go local workspace or replacement overlay than to a committed Cargo workspace. If Slang adds
committed package members, they should be a different concept and file.

Unlike Go, Slang package identity is not an import prefix. A future workspace command should not
couple the Git/package name to export-relative Slang module names.

Go also provides
[`go mod why`](https://go.dev/ref/mod#go-mod-why). The consumed-package-growth journeys need the
same durable question: “why is `color-math` in my graph?” Go answers from imported packages;
Slang would answer from manifest and lock edges. A full dependency path is more useful after the
fact than a lock row alone.

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

- **Do not leak a local overlay into the published graph.** `slang-workspace.json` should stay
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
publishing workflow to fix the journeys above. The immediate improvements are scaffolding,
manifest editing, committed multi-package composition, graph explanation, package-content preview,
testing, and transactional updates. Unlike Cargo's introductory loop, `slang package run` should
not be read as build-and-run; it deliberately executes only an existing artifact.

## How flags change the journeys

The safe default is to omit flags. This section explains when a person should depart from that
default and what remains invariant.

### `update --dry-run`

**Use it when:** you want to preview a first lock, a dependency update, an upstream graph change,
or a local-override solve.

**It changes:** the resolver still reads candidate manifests and prints the same detailed or
minimal selection report, but it does not write `slang-package-lock.json`, replace dependency
checkouts, or regenerate materialized state. Resolver caches under `.slang/cache/` may still be
populated.

**It does not prove:** that remote source passes license and module-layout validation. Those trees
are not materialized during the preview.

**Combinations:** use it with `--minimal` or `--from-local`. `--dry-run --clean` is rejected because
there is no checkout replacement to authorize.

### `update --minimal`

**Use it when:** automation or an experienced user needs a compact list rather than selection
rationale.

**It changes:** report formatting only. Added, removed, upgraded, downgraded, replaced, and
unchanged package lines remain, followed by summary counts.

**It does not change:** resolution, validation, materialization, or lock output. It is valid on
both dry-run and real update.

### `update --from-local`

**Use it when:** one or more registered overrides have unpublished manifest changes that should
participate in a trial solve.

**It changes:** the full-graph resolver uses every registered override as the candidate for that
package name. Non-overridden packages still resolve from Git. The resulting lock records local
paths and requires the same `slang-workspace.json`.

**It does not mean:** “update only this package,” “use every nearby repository,” or “adopt the
changed manifest from an in-place edit.” Edits retain published Git candidates.

**Precondition:** at least one local package registration must exist. In normal use, register an
override first. Combine with `--dry-run` before writing a local lock.

### `fetch --clean` and `update --clean`

**Use it when:** the tool is about to replace a checkout it owns, but that tree has changed files,
extra commits, or stashes that you intentionally want to discard.

**It changes:** dirty-checkout protection for replacement. It is destructive authorization, not
dependency selection.

**It does not change:** an explicitly registered edit or override into a tool-owned checkout.
Local package registrations remain protected by their own workflow.

**Combination:** `update --dry-run --clean` is rejected. Fetch may combine `--clean` with
`--skip-validate`.

### `fetch --skip-validate`, `update --skip-validate`, and `build --skip-validate`

**Use it when:** a validation bug or temporarily invalid source tree blocks an investigation and
you accept that later compilation may fail. It is a workaround, not the CI path.

**It skips:** license content, the first `module` / `implementing` declaration rules, and
graph-wide import uniqueness.

**It keeps:** manifest parsing, lock identity, dependency closure, materialized manifest checks,
toolchain constraints, export-directory traversal, dirty-checkout protection, and the rest of the
command's normal side effects. It always prints a warning.

**Combinations:** `update --dry-run --skip-validate` skips workspace source validation but still
cannot inspect remote source that dry-run did not materialize. `validate` intentionally has no
skip flag.

### `override NAME PATH [AS]`

`AS` is a positional exact version, not a global flag. Omit it only when `NAME` already has a lock
row whose version the local tree represents. Supply it for a newly introduced name or when the
local tree represents another version. The value must satisfy every incoming constraint when you
run `update --from-local`.

### `run [NAME] [ARGS...]`

The optional leading name selects a configured host executable; otherwise `host.default` (or the
only configured executable) is used. Remaining values are forwarded to the existing artifact.
This selection does not trigger build or package resolution.

### Help spellings and commands without flags

`slang package help`, `-help`, and `--help` print package help. `init`, `validate`, `status`,
`edit`, `unedit`, `unoverride`, and `docs` otherwise accept only their documented positional
arguments. `test` is present but returns a not-implemented error.

## Gaps, tensions, and intentional asymmetries

These behaviors can look contradictory from a user's point of view. Some preserve an important
invariant; others are unfinished workflow.

### Init creates an invalid package

`init` creates useful structure and a license reminder, but the placeholder makes immediate
validation fail. This is an onboarding gap, not a desired invariant. App/library templates should
produce a source scaffold and make the remaining human obligation obvious; whether a generated
license can ever be valid requires an explicit license choice.

### Status can pass when validate fails

`status` checks whether materialized manifests and Git checkouts match the lock. It does not inspect
license content or module declarations. This distinction is intentional, but the command names do
not make the boundary obvious. Use `status` to diagnose workspace state and `validate` as the
package-quality gate.

### Fetch ignores retractions but honors root excludes

A publisher retraction is new advice about a release that an existing lock may continue to
reproduce. A root `workspace.excludes` entry is current committed intent for this workspace, so a
lock selecting that version is stale. The asymmetry is intentional: publisher advice does not
retroactively break reproducibility, while the workspace's own changed policy does.

### Dry-run is not a complete rehearsal

Dry-run promises no lock or checkout mutation, which means it cannot validate remote source trees.
Its report can be correct and the real update can still fail after materialization. A future
staging design could materialize candidates outside `deps/`, validate them, and still preserve the
no-visible-mutation promise.

### Failed update is not transactional

The new lock is written only after validation, but `build/search-paths` is cleared and
materialization happens first. A failed update leaves the previous lock unchanged while search
paths are empty and some `deps/` directories may already contain the candidate graph. With an old
lock, run `slang package fetch` to restore its checkouts and search paths. A failed first update
can leave partial checkouts and no lock. This is a correctness and recovery gap.

### Edit and override solve different problems

`edit` protects the current checkout but ignores its changed manifest during resolution.
`override` redirects a package identity, and `update --from-local` adopts all override manifests.
This is a principled distinction, but the current terminology and two-step override workflow are
hard to discover.

### Local solve writes a definitive but non-portable lock

`update --from-local` writes the normal root lock even though that lock requires gitignored local
registrations. Other machines fail rather than silently using another graph, which is safe, but
the user must remember to run normal `update` before sharing. A future tool could distinguish or
label a local trial lock more visibly.

### A nested lock does not protect a package's consumers

Each package repository may lock its own development workspace. A consuming root ignores that lock
and re-solves from manifests. This is intentional root-lock ownership, but it surprises authors
who expect their library lock to pin downstream applications. Upstream splits and added
dependencies therefore appear only in each consumer's root lock.

### Run, docs, and test do less than their names may imply

`run` does not build, `docs` only prints the generated location, and `test` is unimplemented.
Narrow side effects make commands predictable, but missing `build --run`, documentation generation,
and package testing leave common loops manual.

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

1. Add `init --app` and `init --lib` scaffolds with a first source file and clear license step.
2. Add `add`, `remove`, and pin/ref forms that edit manifest intent and preview the resulting
   selection.
3. Add `tree` and `why NAME` so update explanations remain available after the update.
4. Implement a package testing contract for the generated `tests/` directory.
5. Add a package-content preview analogous to pack/publish dry-run.

### Multi-package development

1. Define a committed member model, distinct from gitignored `slang-workspace.json`, for packages
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
- Default `validate`, `update`, and `build` reject the license placeholder.
- A dependency-free valid package can validate and build without a lock.
- `run` executes an existing artifact and never silently builds it.

### Resolve and reproduce contract

- `update` is the only normal command that reselects versions and writes a graph lock.
- `update --dry-run` writes neither lock nor dependency checkouts.
- `fetch` requires a lock, selects nothing, and does not rewrite that lock.
- A real update writes the lock only after the candidate graph validates.
- Every reachable dependency has one exact lock row; Git rows include ref and commit.
- Path packages remain in place; Git packages materialize under the configured deps directory.
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
- Selection and validation consider the complete reachable graph, including unchanged rows.

### Local-development contract

- `edit` keeps the published Git identity and prevents replacement of `deps/NAME`.
- An edited manifest does not enter the solve.
- `override` records a machine-local path and exact effective version.
- `update --from-local` resolves the complete graph using all overrides, not one package.
- A registered edit used during `update --from-local` must have HEAD at a published release tag.
- A local-path lock fails on another machine without matching `slang-workspace.json`.
- Normal `update` restores published selections before `unoverride`.
- `edit`, `unedit`, `override`, and `unoverride` regenerate `build/search-paths` immediately to
  reflect the active local registrations.
- Dirty, unregistered Git checkouts are not replaced without `--clean`; registered edits remain
  protected.

### Validation contract

- Default fetch, update, build, and validate enforce licenses, exports, module-header placement,
  toolchain constraints, and graph-wide import uniqueness.
- `--skip-validate` exists only on fetch, update, and build; it warns and keeps lock, manifest,
  closure, toolchain, export, and dirty-checkout checks.
- `status` diagnoses lock, registration, and checkout state without mutation or remote access. It
  is not the package-quality gate.

### Output and side-effect contract

- Detailed update output explains what moved, what stayed, and the incoming constraints.
- `--minimal` retains one-line changes, unchanged packages, and summary counts.
- `docs` prints the generated documentation location but does not regenerate it.
- `test` reports that package testing is not implemented.
- A command failure must not claim that an update, fetch, or build succeeded.

## Executable test anchors

Start with these unit tests when changing a journey:

- Bootstrap and license: `PackageToolInit`, `PackageValidateStructureAndLicense`.
- Fetch and required lock: `PackageToolFetchRequiresLock`.
- Update preview and report: `PackageToolUpdateDryRun`, `PackageResolveReportFormat`.
- Module layout and uniqueness: `PackageCommandsValidateDependencyModuleLayout`,
  `PackageToolUpdateRejectsBundleCaseConflict`, `PackageValidateRejectsFlattenedModuleAlias`.
- Local overrides: `PackageToolLocalOverrideUpdatesDefinitiveLock`.
- Path dependencies: `PackageToolPathDependencies`,
  `PackageToolFetchRejectsPathLockForGitDependency`, `PackageToolRejectsPathIntoSlangState`,
  `PackageResolverPathShadowsSelectedGit`, `PackageResolverPathPackageGitTransitive`.
- Exclusions and retractions: `PackageResolverAppliesWorkspaceExclusions`,
  `PackageToolFetchRejectsWorkspaceExclusion`, `PackageResolverUsesLatestReleaseRetractions`.
- Toolchain selection: `PackageToolSlangToolchain`, `PackageResolverSlangToolchain`.
- Build and run: `PackageToolBuild`, `PackageToolRun`,
  `PackageToolExecutableRequiresWorkspaceSource`.

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
