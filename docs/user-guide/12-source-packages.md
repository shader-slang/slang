---
layout: user-guide
permalink: /user-guide/source-packages
---

Slang Source Packages
=====================

The `slang package` command manages source dependencies stored in Git repositories. The short form
`slang pkg` accepts the same commands. Package management does not change Slang's `import` syntax;
its stable build command distributes source for the resolved graph. Experimental build features
can additionally emit `.slang-module` files and native executables. A command-order walkthrough
using the public `video-preview` demo is in
[Using Source Packages](source-package-workflow). Expected success and failure for each command,
used when changing the tool, is in
[Growing an Application with Source Packages](source-package-command-use-cases).

A **package** is a directory with `slang-package.json`. Its name, exports, license files, and
dependencies apply wherever that package appears in a graph, including as a Git pin or a path
dependency.

A **workspace** is the package whose `slang-package.json` starts resolution for a given solve. You
can run `slang package` from that directory or from an ordinary subdirectory (`src/`, `docs/`, and
so on). The command loads the workspace `slang-package.json`, `slang-package-lock.json`, and
gitignored `slang-package-overlay.json` from the nearest ancestor that contains `slang-package.json`. A nested
package, such as a materialized dependency under `deps/` that has its own `slang-package.json`,
keeps that nearer root. `slang package init` still creates a package in the current directory. The
workspace owns `slang-package-lock.json` and generated state under `.slang/`. Nested packages'
lockfiles are not used for that solve. A module is a Slang language unit (`module NAME;`), not a
package-manager concept.

## Package layout

A package uses these conventional paths:

```text
slang-package.json
LICENSE
src/
tests/
docs/
```

`src/` contains importable Slang modules. Host executables are workspace primaries whose filename
stem matches a name in `build.host.executables`. For example, `src/video-preview.slang` is the source for
the `video-preview` executable and must declare `module video_preview;`.
`slang package run` interprets `build.host.default` from the source bundle, or the only listed
executable when `default` is omitted.

`tests/` and `docs/` are reserved for package tests and documentation. The initial package tool
creates these directories.

How to name those modules, write `import` and `__include`, and keep implementation files from
colliding across packages is described in
[Writing Module Files, Import, and Include](module-files).

The manifest declares the package and its source dependencies:

```json
{
  "schema_version": 1,
  "name": "my-shaders",
  "exports": ["src"],
  "license_files": ["LICENSE"],
  "workspace": {
    "dependencies": "deps",
    "build": "build",
    "excludes": [
      {
        "package": "noise",
        "version": "1.3.0",
        "reason": "Known regression in generated gradients"
      }
    ]
  },
  "build": {
    "host": {
      "executables": ["my-shaders"],
      "default": "my-shaders"
    }
  },
  "dependencies": {
    "noise": {
      "git": "https://github.com/example/slang-noise.git",
      "version": ">=1.2.0 <2.0.0"
    }
  },
  "tools": {
    "slang-toolchain": {
      "version": ">=2026.8.0"
    }
  },
  "retractions": [
    {
      "version": "1.1.0",
      "reason": "Published with an incomplete source export"
    }
  ]
}
```

`schema_version` is the file format identifier and is currently `1`. `name`, `exports`, and
`license_files` are also required. `dependencies`, `tools`, `workspace`, and `build` are optional.
Package manifests allow JSON comments.

`name` identifies the package throughout the dependency graph. `exports` lists relative source
roots whose primary module paths become importable. Every path in `license_files` must name a
non-empty license file inside the package repository. `slang package init` creates a file named
`LICENSE` containing placeholder text; keeping that filename is fine, but the file's placeholder
contents must be replaced before validation succeeds.

The `workspace` object is read only from the manifest that starts the solve. `dependencies` is where Git
dependency source is materialized, and `build` contains generated workspace output. Their defaults
are `deps/` and `build/`; `slang package init` writes those defaults explicitly. The same fields in
a dependency's manifest do not affect the enclosing workspace. `workspace.build` is a directory
name; it is not the top-level `build` object that configures compilation.

`workspace.bundle` selects optional distribution outputs under `build/bundle/`. `modules` (default
true) requests that experimental builds compile every primary to `build/bundle/modules/` and write
`provenance.json` beside those artifacts. Stable builds do not generate modules regardless of this
setting. `source` (default true) copies every exported `.slang` file into `build/bundle/source/`
using the same import-relative layout, so that directory is one compiler search path. Two files
that would occupy the same name on a case-insensitive filesystem are an error. Set either flag to
`false` to skip that output when its build mode is active.

The optional top-level `build` object is how the package is compiled. Format version 1 only understands
`build.host`, which requests native host executables. `executables` lists output filenames without
directory separators; the package tool adds the platform executable suffix and writes each result
under `workspace.build/host`. Each name must match an exported workspace primary whose source
filename is `<name>.slang`. When more than one executable is listed, `default` names the artifact
`slang package run` selects if you do not pass an executable name.
With a single executable, `default` may be omitted and that name is used. Dependency manifests may
declare this field, but only the workspace package controls executable generation. A top-level
`host` object is an error; nest it as `build.host`. Later artifact kinds belong as additional keys
under `build`, not as new top-level siblings of `workspace`.

The optional `tools` object declares required **system tools**: programs already installed on the
machine, not source packages and not lock rows. `slang package` checks that each named tool is
present and that its version satisfies the declared constraint. It does not fetch those tools into
`deps/` or record their versions in `slang-package-lock.json`.

Schema `1` accepts only `slang-toolchain`, which is the Slang compiler that provides
`slang-package`, regardless of how it was installed (a shader-slang/slang GitHub release, a Vulkan
SDK distribution, or another bundle). Its `version` uses the same constraint grammar as Git
dependencies (three-part versions, no `v` prefix). In the short term this is how a package states
the minimum compiler it needs to work correctly: a builtin such as `neural`, a standard-library
API, or a compiler fix that did not exist (or was broken) before that version. There is no
separate way to depend on those builtins, so the toolchain version stands in for them.

Every package in the graph may declare the field; `update`, `fetch`, `status`, and `build`
intersect those constraints against the installed compiler. `validate` checks the workspace
package's constraint while checking that package for sharing, and verifies the materialized lock
graph when it has dependencies. `slang package init` writes `>=` that installed version when it can
parse it. Slang compiler versions are calendar-based, not API levels, so a toolchain constraint
should be a lower bound such as `>=2026.8.0`. Do not cap it at a speculative future date. Use `!=`
to skip one known-bad compiler version without inventing an upper bound, for example
`>=2026.8.0 !=2026.9.0`.

Later schemas may add other system tools, or split Slang into separately versioned components if
distributions start shipping them independently. Unknown `tools` keys are errors today.

Ordinary dependency versions come from Git tags named `vMAJOR.MINOR.PATCH`, which package
publishers must treat as immutable. A manifest may instead pin an opaque branch or tag with `ref`
and assign its solver identity with `as`. `version` in `slang-package.json` is only the file
format version. For a Git package the lock records the resolved ref, exact semantic version, and
commit; a path package has no ref or commit to record, so its row carries the path and the `as`
version.

The optional top-level `retractions` array is publisher advice not to select releases matching a
version constraint. Each entry requires `version` and a non-empty `reason`. To retract a published
release, add the entry to the manifest and publish a new, higher release tag. During resolution,
the package tool reads retractions from the highest available release, even when that release is
outside the consumer's requested range, and skips matching candidates. A retraction does not
invalidate an existing lock: `fetch` remains reproducible, while the next `update` moves away from
the retracted release when another candidate satisfies the graph.

The root-only `workspace.excludes` array is committed consumer policy. Each entry names a
`package`, a `version` constraint, and a non-empty `reason`. Dependency manifests' workspace
settings, including exclusions, are ignored for the solve. If a reachable package lists
`workspace.excludes` entries that this workspace does not copy (same `package` and `version`
strings), `update`, `fetch`, `status`, `validate`, and `build` warn so those ignored excludes are
visible. Resolution skips excluded Git releases. Unlike a publisher retraction, adding an
exclusion changes the workspace's declared resolution intent, so
`fetch` rejects a lock that still selects an excluded release and asks for `slang package update`.
Path dependencies and local overrides carry an effective version for solver compatibility, but
workspace exclusions apply only to remote Git selections.

Each dependency entry has one of three shapes, matching `slang package dependency add`:

- `git` plus `version` selects the highest compatible `vMAJOR.MINOR.PATCH` release tag.
- `path` plus `as` uses one relative tree as the exact semantic version named by `as`.
- `git`, `ref`, and `as` pins an opaque branch, tag, or full 40-character commit ID and uses `as`
  as its exact solver version.

`git` may be a URL or a local Git repository path. A `version` is one or more clauses joined by
`||`. Each clause is a space-separated intersection of `>`, `>=`, `<`, `<=`, and `!=` comparisons,
or a single exact version. For example, `>=1.2.0 !=1.3.0` skips 1.3.0, and
`>=1.0.0 <1.3.0 || >=1.3.1 <2.0.0` accepts either interval. Dependents still unify one version per
package name: every incoming constraint must match that version. Both `version` and `as` omit the
release tag's `v` prefix. `ref` is a branch, tag, or full 40-character commit ID; the lock always
records the exact commit.

A dependency `path` must be relative to the manifest that declares it and must be paired with an
exact `as` version. The target directory must contain its own `slang-package.json`, and its package
name must match the dependency key.

For example, a package can check in another package under `vendor/noise`:

```json
{
  "name": "my-shaders",
  "exports": ["src"],
  "license_files": ["LICENSE"],
  "dependencies": {
    "noise": {
      "path": "vendor/noise",
      "as": "1.4.0"
    }
  }
}
```

The resolver reads every reachable path package manifest and includes all of its transitive
dependencies in the workspace lock. Path packages are used in place and are not copied under
`deps/`; Git packages are fetched there.

One package name identifies one node in the graph. Git requirements from multiple dependents must
use the same Git location, and the resolver intersects their constraints and chooses the highest
satisfying tag. Path requirements for one name must resolve to the same canonical directory and
claim the same `as` version or resolution fails. A path requirement wins over the Git source for
the same name, but its `as` version must satisfy every Git version constraint and pinned `as`
identity. The tool warns that the Git source was shadowed. The path package's transitive
dependencies are still resolved normally.

A path in the workspace package or another local package may use `..` to leave the package that
declares it, which supports sibling packages in a larger checkout. Such a path is valid for a
local build, and `update`, `fetch`, and `status` warn that the declaring package is not independently
portable. `slang package validate` rejects the path when it checks that package for sharing. A path
inside a Git release must remain in that release's checkout. A missing target is always an error.

## Locking and fetching

`slang package update` resolves all manifests reachable from the workspace package, materializes
the resulting dependency set, and writes one `slang-package-lock.json` in the workspace root. The
lockfile is the definitive dependency graph and records both Git and path packages. It starts with
`"schema_version": 1`, the same file-format identifier as `slang-package.json`.
Nested packages' lockfiles are not used for
that solve. When a lock exists, `slang package fetch` checks that it still satisfies every recorded
manifest and ensures every direct and transitive Git dependency is at its locked commit under
`workspace.dependencies` (`deps/` by default). Fetch installs that `commit`; it does not ask whether the
recorded release tag still points at it. A publisher who later moves `v1.2.0` does not break
fetch of a lock that already named a SHA. The next `update` is what sees the new tag identity and
may select it. A clean checkout already at the locked commit is left untouched; missing or
out-of-date checkouts are materialized as needed. Path dependencies remain at their
locked relative locations. When dependencies exist
but a fresh checkout has no lock, fetch performs the initial solve, shows the same selection report
as update, confirms it, and writes the first lock. Later fetches reproduce that lock without
reselecting versions. CI that already has a committed lock should `fetch` or `build`; `build`
fetches missing locked trees itself and never rewrites that lock. A first clone with no lock can
start with `build`, which runs fetch and then update `--yes`. Use an explicit `fetch` to
materialize without building, to pass `--clean`, or to confirm a first lock interactively.

Every lock row records selection identity only: an exact `version`, plus `git`/`ref`/`commit` for a
Git pin or `path` for a path or overlay row. Declared `exports` and `dependencies` are not copied
into the lock. Commands reload them from an active overlay's working-tree manifest, or from the
manifest of the locked version (Git at `commit`, or the path directory). `status` uses that live
graph, including overlays, to check whether the current lock still satisfies every pin that cannot
change. It does not look for newer Git tags; that is `update`.

Dependency checkout paths are stable. A pin stays at `deps/NAME` while it is tool-owned, locally
overridden in place, and returned to tool ownership. Fetch and update refuse to replace an
unregistered checkout with
changed files, extra commits, or stashes. Pass `--clean` explicitly to permit replacement.

That refusal happens first, before any other work: both commands inspect every checkout the
current lock owns up front, and update stops before resolving rather than after printing a plan it
cannot apply. The error names each checkout and its drift in the same terms `status` uses, and the
ways forward are to commit or discard the changes, run `slang package edit NAME` to keep working in
that checkout, or re-run with `--clean`.

Run `slang package update` deliberately when manifest constraints or upstream releases change.
`slang package update --dry-run` prints the selected graph (what moved, what stayed, and why)
without writing the lock or replacing checkouts. `--ignore-overrides` ignores out-of-tree
overrides for that solve; it does not change `slang-package-overlay.json` or replace in-place overrides.
`--minimal` keeps one-line package changes and the summary count. The installed Slang
toolchain is omitted unless its constraint fails. Resolver Git clones
under `.slang/cache/` may still be populated so the tool can inspect available tags. A real update
prints that report and asks before applying the exact graph it just resolved, unless that graph
already matches the committed lock. In a terminal, declining that prompt leaves the workspace
unchanged and still succeeds. Without a terminal the command does not prompt: it fails and tells
you to re-run with `--yes`. Pass `--yes` for a non-interactive invocation. Reproducing a committed lock uses
`slang package fetch` or `slang package build`; an inconsistent existing lock is an error.

The tool invokes the `git` executable from the system path. Existing Git credential and SSH
configuration therefore applies without separate package-tool authentication. Git locations
cannot begin with `-`, use Git's command-executing `ext::` transport, or contain whitespace or
control characters.

After fetching, `slang-package-includes.txt` lists each package export directory, one per line
(for example `deps/color-encoding/src`), not the package root. The workspace `src/` is omitted
because a compiler input file already names that primary. A host that compiles shaders or
modules itself passes the file to `slangc`:

```sh
slangc -search-path-list slang-package-includes.txt app/tonemap.slang -target spirv -o tonemap.spv
```

That is the same as a separate `-I` for each listed directory. `slangi` does not read the list; it
searches only next to its input file, which is why `slang package run` interprets the flattened
source bundle instead. Library callers can load the same file with `slang_readSearchPathsFile`,
assign the returned array and count to `slang::SessionDesc::searchPaths` and `searchPathCount`,
and keep its `outAllocation` alive until `createSession` returns. It is a derived, gitignored
file; fetch or update regenerates it. Each line is an absolute path, so the listed directories
remain valid when the compiler is invoked from a subdirectory. Package commands do not inject
these paths into compiler sessions automatically.

## Validating packages

Package validation has three layers:

- The **legal graph** checks closed JSON schemas, dependency and lock identities, trusted path
  selections, materialized manifests, and toolchain constraints. Commands never skip this layer.
- A **buildable workspace** additionally requires every export in the materialized closure to
  exist, every source file to use the required `module` or `implementing` declaration, and every
  primary import path to be unique across the graph. Local overrides and escaping path
  dependencies remain valid build inputs.
- A **publishable package** is one package whose source tree is buildable, whose license files are
  present and no longer contain the generated placeholder, and whose path dependencies stay
  inside that package.

Bare `slang package validate` is the **app** sharing check. It applies the publishable-package
rules to the workspace package, rejects active overrides and a lock that requires local
override state, and checks that the materialized lock graph is legal. It does not repeat license
or source-layout checks for unchanged transitive dependencies.

`slang package validate NAME` is the **library** sharing check in this workspace: the same
publishable-package rules on that locked tree (an enabled override, path lock row, or
`deps/NAME`), with Git and path edges checked against this workspace lock rather than a nested
lock under `NAME`. `slang package validate --all` runs that library check on every locked
package's tree. Neither named form materializes packages or walks the legal graph again.

`build` requires a legal, buildable workspace. It deliberately permits the generated license
placeholder, overrides, and local path dependencies because those do not prevent
compilation. If a tool-owned Git checkout is missing, or if there is no lock and the manifest
has dependencies, build runs `fetch` first (without `--clean`). A missing lock makes fetch run
`update --yes` so a first clone can build without a prompt. An existing lock is never rewritten.
Each of those hand-offs prints why the inner command is running. A dirty checkout that would
require `--clean` still fails; run `slang package fetch --clean` yourself.

`fetch` and `update` always verify the legal graph from selected manifests **before** they clear
search paths or materialize `deps/`. A Git pin without an active local path is read at its locked
commit from whichever repository already has that revision: an existing `deps/NAME` checkout when
it is already the locked commit, so a current workspace needs no network, otherwise `.slang/cache`.
Either way the committed manifest is read, not the working-tree file, so a dirty checkout cannot
change what the graph check sees. After materialization, they apply the publishable-package checks
to each Git package whose checkout was newly created or changed, and to each changed local
registration, then check source layout and import uniqueness across the complete selected graph.
The closure check includes unchanged packages because a new module can
collide with one already selected. `update --dry-run` runs that legal-graph check and still cannot
claim source-layout success, because it does not materialize remote trees.

Pass `--skip-validate` on `fetch`, `update`, or `build` only as an escape hatch. It skips
source-layout and new-release publish checks **after** materialize. The legal graph still runs
first. The command prints a warning. `slang package validate` has no skip flag.

`slang package status` prints one header line when the workspace is current, for example
`Package 'video-preview': lock current, 3 packages, buildable.` Extra lines appear only when
something is dirty. The header says `incomplete` when the lock or Git pins are missing, and
`not buildable` only after those trees are present and the source check fails. Dirty checkouts and
enabled overrides are listed by name. Disabled out-of-tree overrides are silent; a disabled
in-place override remains visible because its `deps/NAME` checkout may still contain local work.
Like
`git status`, reportable drift does not make the command fail. Status returns nonzero only when
required root manifest, existing lock, or overlay JSON cannot be read and parsed well enough to
produce a report. It does not inspect `build/`, modify package state, or contact remotes.

Use `slang package dependency add` and `dependency remove` to edit direct manifest edges, and
`dependency list` to inspect them. Add accepts exactly one source shape:
`--git URL --version RANGE`, `--git URL --ref REF --as VERSION`, or
`--path PATH --as VERSION`. `dependency pin NAME` rewrites a Git edge from the current lock:
default writes that lock's exact `version`, `--to MAJOR.MINOR.PATCH` writes a different exact
version, and `--commit` writes `ref` plus `as` from the locked SHA. A transitive Git package is
promoted to a direct edge. Pinning leaves an active overlay registered. When the lock row already
selects that overlay, pass `--to` explicitly because its effective version need not be a published
Git version; `--commit` requires a Git-only lock row because an overlay row has no locked SHA. Path
dependencies are already pins. These commands change only
`slang-package.json`; inspect `status` and run `update` afterward. `slang package tree` prints
the selected lock graph, while
`slang package why NAME` prints every current root-to-package path and incoming requirement. Why
explains the graph that is locked now, not candidates rejected during an earlier solve. Unlike
`status`, `tree` and `why` may populate `.slang/cache` when a locked Git revision is not already
present under `deps/`.

Each `.slang` file that is not below a module's companion directory is a primary module file. Its
first declaration must be `module NAME;`, where `NAME` matches the filename stem with hyphens
replaced by underscores. Namespace directories do not form part of the declaration. For example,
`src/acme/noise.slang` declares `module noise;`, and `src/acme/image-noise.slang` declares
`module image_noise;`. Every `.slang` file below `src/acme/noise/` belongs to that module and must
instead begin with `implementing noise;`. From the primary, `__include` those files with a path
relative to the primary (for example `__include "noise/hash";`), as shown in
[Writing Module Files, Import, and Include](module-files).

## Creating and editing packages

`slang package init` creates `slang-package.json` and the conventional directories in the current
directory. It writes `tools.slang-toolchain` as `>=` the installed compiler version when that
version can be parsed. It adds `.slang/`, `deps/`, `build/`, `slang-package-overlay.json`, and
`slang-package-includes.txt` to
`.gitignore`. `slang package help` lists commands under the manifest, overlay, lock, and build.
`.slang/cache/` contains resolver Git repositories used to inspect release manifests. Fetched
source remains visible under `deps/`; generated files go under `build/`.

`slang package edit NAME` marks the existing `{workspace.dependencies}/NAME` checkout (by default
`deps/NAME`) as editable without moving it. Under the covers this is an enabled override at that
path, using the current locked version. Gitignored `slang-package-overlay.json` records that the package
tool no longer owns the tree. Fetch and update do not replace it. Search paths already point at
that directory, so compiling against it does not wait on `update`. `update` reads the working-tree
manifest and writes a Git+path lock row when the overlay graph must enter the lock. The checkout
may already have local changes when `edit` runs; it only has to still be the Git repository the
lock names.

Plain `unedit NAME` requires a Git-only lock row and a clean checkout at that locked commit.
`unedit NAME --clean` discards local state and restores the locked commit. Use
`unedit NAME --adopt [--as VERSION]` instead to pin a committed `HEAD` in the direct dependency's
manifest and lock, replacing its version range with canonical `ref` plus `as` intent. A unique
`vMAJOR.MINOR.PATCH` tag at `HEAD` supplies the version automatically; an untagged commit requires
`--as`. Clean and adopt ask for confirmation unless `--yes` is passed.

For example, the generated local-state file may contain:

```json
{
  "schema_version": 1,
  "overrides": {
    "noise": {
      "path": "deps/noise",
      "as": "1.4.0",
      "enabled": true
    },
    "shared": {
      "path": "../shared",
      "as": "2.3.0",
      "enabled": true
    }
  }
}
```

Use the package commands to change this file; its schema is tool-owned and may evolve.

`slang package override add NAME PATH [AS]` uses an existing local package directory instead. `AS`
is an exact semantic version for solver compatibility. When it is omitted, the command uses the
version in the package's current lock row. If `NAME` is already edited and `PATH` is that
workspace checkout, the command updates the same in-place registration. `override enable`,
`override disable`, `override remove`, and `override list` retain or inspect the same registration.
A disabled override
keeps its path and version but plain update selects published Git. An override does not copy or
modify the supplied directory.

A registered local manifest must agree with the lock. Enabled overrides, including the in-place
form created by `edit`, automatically participate in plain
`slang package update`. `update --ignore-overrides` writes the published Git graph for this command
only for out-of-tree overrides; in-place overrides remain active so the command cannot replace
their user-owned checkouts. An in-place checkout that is absent from that graph stays on disk and
registered so a later plain update can restore it without losing work. An override records its
original
Git location and its effective path and requires the matching registration in
`slang-package-overlay.json`; it therefore fails explicitly on another machine or in CI. The override's
effective version must satisfy every incoming constraint, and all of its transitive dependencies
are resolved. Disable enabled overrides and run `slang package update` before removing their
registrations or committing a portable published resolution.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.

`slang package build` validates the materialized package graph. Its stable distribution output is
source: when `workspace.bundle.source` is enabled, it copies every exported `.slang` file into
`build/bundle/source/` at the same import-relative paths. The resulting tree is a single search
path: `src/acme/noise.slang` and its companion `src/acme/noise/helper.slang` become
`build/bundle/source/acme/noise.slang` and `build/bundle/source/acme/noise/helper.slang`. A
case-insensitive name collision across packages is an error.

With `--experimental`, build also honors `workspace.bundle.modules`. It compiles every primary
module in the workspace and its resolved dependencies to a front-end `.slang-module` under
`workspace.build/bundle/modules`, preserving its import path. For example, an exported
`src/acme/noise.slang` becomes `build/bundle/modules/acme/noise.slang-module`, whether that source
belongs to the workspace or a dependency. Companion files included by that primary are compiled
into the same artifact and do not produce separate files.

The `.slang-module` binary format is unstable and has no compatibility guarantee. Every build that
generates one prints a warning. `build/bundle/modules/provenance.json` records
`experimental: true`, `format_stability: "unstable"`, and the compiler's name, version, exact
source commit when available, tracked-source `dirty` state, and path. Consumers must require the
same Slang toolchain that produced the files. The module tree can be used as a source-free search
path only with that constraint; it is not a stable distribution format.

When `build.host.executables` is present, `slang package --experimental build` also compiles each matching
workspace primary with the host executable target and writes `build/host/<executable-name>` (plus
`.exe` on Windows). The `main` function in that file must use the native ABI, such as
`export __extern_cpp int main()`, and a supported downstream C++ compiler must be available. Build
copies the matching `slang-rt` shared library beside the executables so the artifacts can locate
their runtime support. It also writes `build/host/EXPERIMENTAL.txt`, so the status remains visible
when the host directory is copied independently. Configuring a host executable without a matching
workspace `.slang` primary is an error.

The same command copies every `.md` file below each materialized package's `docs/` directory to
`build/docs/<package-name>/`, preserving paths below `docs/`. Namespacing the output by package
keeps files such as `docs/README.md` from different packages distinct. Other file types are not
copied. Build also writes `build/docs/index.md`: the workspace dependency tree, then an
alphabetized list of copied Markdown files per package. Every package in the graph is listed;
only packages that contributed Markdown link from the tree into that file list.

`slang package run` uses sibling `slangi` to interpret an existing primary in
`build/bundle/source` and forwards trailing arguments. With no executable name, it selects
`build.host.default`. It does not build first and fails with instructions when the manifest does
not configure a host executable or `slang package build` has not produced the source bundle.
Because `slangi` derives its only source search path from the input filename, this stable mode
requires the configured primary to be at an export root, and therefore at the root of the
flattened source bundle. Nested executable primaries require binary mode.

`slang package --experimental run --binary` instead executes the selected native artifact under
`build/host`. The global `--experimental` option must appear immediately after `slang package`,
and `--binary` must immediately follow `run`. This mode requires output from
`slang package --experimental build`; without `--binary`, experimental `run` still uses `slangi`
and the source bundle.

`slang package test` is reserved and currently reports that it is not implemented. It does not
invoke `slang-test`. Package testing will get a dedicated model; `slang-test` remains an internal
compiler harness and is not part of the package command surface.

`slang package docs` opens `build/docs/index.md` with the application registered for Markdown
on this machine. `--print` writes that path instead of launching, which is what scripts should
use. The command does not copy or regenerate documentation; run `slang package build` for that,
or `slang package --experimental build` when the manifest configures host executables.

## Possible future enhancements

The initial workspace layout deliberately keeps resolver clones in `.slang/cache/` and compile
inputs in the workspace. Future versions may add a user-global immutable cache with copy-on-edit,
let compiler sessions consume workspace metadata without `slang-package-includes.txt`, and share immutable
dependency trees between workspaces. Git-to-Git replacement is also deferred until Slang has a
global user remapping policy or package-index integration; current overrides intentionally replace
a dependency with a local path only.
