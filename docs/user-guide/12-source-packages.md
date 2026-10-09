---
layout: user-guide
permalink: /user-guide/source-packages
---

Slang Source Packages
=====================

The `slang package` command manages source dependencies stored in Git repositories. The short form
`slang pkg` accepts the same commands. Package management does not change Slang's `import` syntax;
its stable `bundle` command distributes source for the resolved graph. Experimental `build`
does that and can additionally emit `.slang-module` files and native executables. A command-order walkthrough
using the public `video-preview` demo is in
[Using Source Packages](source-package-workflow). Expected success and failure for each command,
used when changing the tool, is in
[Growing an Application with Source Packages](source-package-command-use-cases). The complete
syntax, options, side effects, and validation rules for every command are in the
[Slang Package Command Reference](source-package-command-reference).

A **package** is a directory with `slang-package.json`. Its name, exports, license files, and
dependencies apply wherever that package appears in a graph.

A **workspace** is the package whose `slang-package.json` starts resolution for a given solve. You
can run `slang package` from that directory or from an ordinary subdirectory (`src/`, `docs/`, and
so on). The command loads the workspace `slang-package.json` and `slang-package-lock.json` from the
nearest ancestor that contains `slang-package.json`. A nested
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
`slang package --experimental run` interprets `build.host.default` from the source bundle, or the only listed
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
    "output": "out",
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
dependency source is materialized, and `output` contains generated workspace output. Their defaults
are `deps/` and `out/`; `slang package init` writes those defaults explicitly. The same fields in
a dependency's manifest do not affect the enclosing workspace. `workspace.output` is a directory
name; it is not the top-level `build` object that configures compilation.

Distribution commands do not read extra workspace switches. `slang package bundle` always copies
exported source to `out/bundle/source/` and documentation to `out/docs/`. `slang package
--experimental build` writes those same trees and also compiles modules and host executables.
Two exported files that would occupy the same source-bundle name on a case-insensitive filesystem
are an error. A `workspace.bundle` object is rejected.

The optional top-level `build` object is how the package is compiled. Format version 1 only understands
`build.host`, which requests native host executables. `executables` lists output filenames without
directory separators; the package tool adds the platform executable suffix and writes each result
under `<workspace.output>/host`. Each name must match an exported workspace primary whose source
filename is `<name>.slang`. When more than one executable is listed, `default` names the artifact
`slang package --experimental run` selects if you do not pass an executable name.
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

Every package in the graph may declare the field; `update`, `fetch`, `status`, `bundle`, and
experimental `build` intersect those constraints against the installed compiler. `validate` checks the workspace
package's constraint while checking that package for sharing, and verifies the materialized lock
graph when it has dependencies. `slang package init` writes `>=` that installed version when it can
parse it. Slang compiler versions are calendar-based, not API levels, so a toolchain constraint
should be a lower bound such as `>=2026.8.0`. Do not cap it at a speculative future date. Use `!=`
to skip one known-bad compiler version without inventing an upper bound, for example
`>=2026.8.0 !=2026.9.0`.

Later schemas may add other system tools, or split Slang into separately versioned components if
distributions start shipping them independently. Unknown `tools` keys are errors today.

Ordinary dependency versions come from Git tags named `v` followed by one or more decimal
components, such as `v1.2.3` or `v2026.10.1.4`. Publishers must treat those tags as immutable.
Trailing zero components are not a different release: `1.2.3`, `1.2.3.0`, and `1.2.3.0.0` are the
same release, and a zero that is not trailing still counts, so `1.2.0.1` is newer than `1.2`.
Comparison is numeric from left to right on that canonical release, so `1.2.3 < 1.2.3.1 < 1.2.4`.
The canonical tag is `v` plus that spelling, with no leading zeros and no trailing zero components.
`v1.2.3` and `v1.2.0.1` are canonical. `v1.2` is the canonical tag for the release 1.2.0, so
`v1.2.0`, `v1.2.3.0`, and `v01.2.3` are ignored. The solver warns once for each ignored tag and
names the canonical tag. These are dotted release identifiers, not Semantic Version values, and a
component must fit in 32 bits. At most 32 components are accepted, counted before trailing zeros
are removed. A manifest may instead pin an opaque branch or
tag with `ref` and omit `as` to derive the solver identity from the nearest release tag on that
line, or write `as` to assign it explicitly. `schema_version` in `slang-package.json` is only the
file format version. A release row records the Git URL, canonical tag, exact version, and commit.
An edit row records the Git URL, branch, and the version, canonical tag, and commit of the release
that branch is representing. `edit <name> --advance` moves that version, tag, and commit together. `pinned`
is a boolean on the row and is
omitted when it is false.

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
strings), `update`, `fetch`, `status`, `validate`, `bundle`, and experimental `build` warn so those
ignored excludes are visible. Resolution skips excluded Git releases. Unlike a publisher retraction, adding an
exclusion changes the workspace's declared resolution intent, so
`fetch` rejects a lock that still selects an excluded release and asks for `slang package update`.

Each dependency entry has one of two shapes, matching `slang package dependency add`:

- `git` plus `version` selects the highest compatible dotted release tag.
- `git`, `ref`, and optional `as` selects an opaque branch, tag, or full 40-character commit ID.
  Omit `as` to derive the exact solver version from the nearest release tag
  reachable from that commit. Write `as` to claim a different identity.

A `path` field is rejected. Every dependency is a Git repository checked out at `deps/<name>`.

`git` may be a URL or a local Git repository path. A `version` is one or more clauses joined by
`||`. Each clause is a space-separated intersection of `>`, `>=`, `<`, `<=`, `!=`, `^`, and `~`
constraints, or a single exact version. `^` increments the leftmost non-zero component, or the
last component when every written component is zero, and drops the components after it. `^1.2.3`
means `>=1.2.3 <2`, `^0.2.3` means `>=0.2.3 <0.3`, `^0.0.3` means `>=0.0.3 <0.0.4`, `^0.0` means
`>=0 <0.1`, and `^0.0.0.4` means `>=0.0.0.4 <0.0.0.5`. `~` increments the second component when
at least two are written, and the only component otherwise: `~1.2.3` means `>=1.2.3 <1.3`,
`~1.2.3.4` means `>=1.2.3.4 <1.3`, and `~1` means `>=1 <2`. For example, `^1.2 !=1.5.0` accepts
later 1.x releases except 1.5.0, and `~1.2.3 || ^2` accepts either alternative. Dependents still
unify one version per package name: every incoming constraint must match that version. Both
`version` and `as` omit the release tag's `v` prefix. A bare version matches one release, so
`1.2.3` also matches `1.2.3.0`. A `^` or `~` bound still uses every component that was written,
including trailing zeros: `^0.0.0` means `>=0 <0.0.1`, which is narrower than `^0`. Because
`2`, `2.0`, and `2.0.0` are the same release, `<2.0.0` does not match `2` or `2.0`. `ref` is a
branch, tag, or full 40-character commit ID. A release row records that resolved commit. An edit
row records a branch instead.

One package name identifies one node in the graph. Requirements from multiple dependents must use
the same Git location. The resolver intersects their constraints and chooses the highest satisfying
canonical tag, unless the lock row is pinned or edited. A pin keeps the recorded version. An edit
keeps the version stored on the edit and reads that package's manifest from the checkout.

## Locking and fetching

`slang package update` resolves all manifests reachable from the workspace package, materializes
the resulting dependency set, and writes one `slang-package-lock.json` in the workspace root. The
lockfile is the definitive dependency graph. It starts with
`"schema_version": 1`, the same file-format identifier as `slang-package.json`.
Nested packages' lockfiles are not used for
that solve. When a lock exists, `slang package fetch` checks that it still satisfies every recorded
manifest and ensures every direct and transitive Git dependency is at its locked commit under
`workspace.dependencies` (`deps/` by default). Fetch installs that `commit`; it does not ask whether the
recorded release tag still points at it. A publisher who later moves `v1.2.0` does not break
fetch of a lock that already named a SHA. The next `update` is what sees the new tag identity and
may select it. A clean checkout already at the locked commit is left untouched; missing or
out-of-date checkouts are materialized as needed. An edited checkout is left on its recorded
branch. When dependencies exist
but a fresh checkout has no lock, fetch performs the initial solve, shows the same selection report
as update, confirms it, and writes the first lock. Later fetches reproduce that lock without
reselecting versions. CI that already has a committed lock should `fetch`, `bundle`, or experimental
`build`; those distribution commands fetch missing locked trees themselves and never rewrite that
lock. A first clone with no lock can start with `bundle`, which runs fetch and then update `--yes`.
Use an explicit `fetch` to materialize without distributing, to pass `--clean`, or to confirm a
first lock interactively.

Every lock row records selection identity only. A release row stores `git`, `ref`, `version`, and
`commit`. An edit row stores `git`, `branch`, `ref`, `version`, and `commit`. The tag and commit
are the release the branch is representing, and `fetch` does not check that commit out. `pinned`
is written only
when the boolean is set. Declared `exports` and `dependencies` are not copied into the lock.
Commands reload them from an edited checkout's working tree, or from the manifest at the locked
commit. `status` uses that live graph to check whether the current lock still satisfies every
requirement that cannot change. It does not look for newer Git tags; that is `update`.

Dependency checkouts stay at `deps/NAME`. Each Git URL has its own permanent repository under
`.slang/repositories/`. The directory name is the URL's repository name, such as `noise`, plus a
short hash of the exact URL, so two URLs that both end in `noise.git` do not share a directory.
`deps/NAME` is a worktree of the repository for the URL the lock resolved. Fetch and update do not
replace an edited checkout, and they refuse to replace a tool-owned checkout that has changed
files, extra commits, or stashes. Pass `--clean` explicitly to permit discarding uncommitted files
in that worktree. `--clean` does not move an edit. Branches, tags, commits, and stashes stay in
the permanent repository when the worktree is removed, including when a remap switches the package
to a different URL. A checkout that is a separate Git repository, rather than one of these
worktrees, is still deleted only after any commits or tags that are not on a remote are listed and
confirmed.

That refusal happens first, before any other work: both commands inspect every checkout the
current lock owns up front, and update stops before resolving rather than after printing a plan it
cannot apply. The error names each checkout and its drift in the same terms `status` uses, and the
ways forward are to commit or discard the changes, run `slang package edit NAME` to keep working in
that checkout, or re-run with `--clean`.

Run `slang package update` deliberately when manifest constraints or upstream releases change.
`slang package update --dry-run` prints the selected graph (what moved, what stayed, and why)
without writing the lock or replacing checkouts. `update` does not fetch, merge, reset, or
otherwise change an edited checkout, and it does not move that row's version or pin. It reads the
tree as it exists and solves the transitive dependencies declared by that manifest. When `HEAD` is
not on the recorded branch, `update` reports the mismatch and leaves the checkout alone.
`--minimal` keeps one-line package changes and the summary count. `--offline` resolves and
materializes from `.slang/repositories` only: it does not fetch or clone the package URL. A missing
repository, ref, or object fails and asks you to re-run without `--offline`. Online update first
refreshes those repositories from their origins, then checks out a worktree under `deps/`. The installed Slang toolchain is omitted unless its constraint fails. A real update
prints that report and asks before applying the exact graph it just resolved, unless that graph
already matches the committed lock. In a terminal, declining that prompt leaves the workspace
unchanged and still succeeds. Without a terminal the command does not prompt: it fails and tells
you to re-run with `--yes`. Pass `--yes` for a non-interactive invocation. Reproducing a committed lock uses
`slang package fetch` or `slang package bundle`; an inconsistent existing lock is an error.

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
searches only next to its input file, which is why `slang package --experimental run` interprets the flattened
source bundle instead. Library callers can load the same file with `slang_readSearchPathsFile`,
assign the returned array and count to `slang::SessionDesc::searchPaths` and `searchPathCount`,
and keep its `outAllocation` alive until `createSession` returns. It is a derived, gitignored
file; fetch or update regenerates it. Each line is an absolute path, so the listed directories
remain valid when the compiler is invoked from a subdirectory. Package commands do not inject
these paths into compiler sessions automatically.

## Validating packages

Package validation has three layers:

- The **workspace graph** checks closed JSON schemas, dependency and lock identities, manifests
  available in `deps/` or `.slang/repositories`, and toolchain constraints. Commands never skip this layer.
- The **upstream cache** check refreshes `.slang/repositories` when network access is allowed and verifies
  that every locked Git ref and commit is represented there. A commit that exists only in
  `deps/NAME` does not pass until it reaches the origin.
- A **buildable workspace** additionally requires every export in the materialized closure to
  exist, every source file to use the required `module` or `implementing` declaration, and every
  primary import path to be unique across the graph.
- A **publishable package** is one package whose source tree is buildable and whose license files
  are present and no longer contain the generated placeholder.

Bare `slang package validate` is the **app** sharing check. It applies the publishable-package
rules to the workspace package, rejects a lock that still has an edited dependency, and checks
that the materialized lock graph is legal. A pinned release row can be committed. It does not
repeat license or source-layout checks for unchanged transitive dependencies. It refreshes and
validates the upstream cache for every Git lock row.

`slang package validate NAME` is the **library** sharing check in this workspace: the same
publishable-package rules on that locked checkout, with edges checked against this workspace lock
rather than a nested lock under `NAME`. `slang package validate --all` runs that library check on
every locked package's tree. Neither named form materializes packages or walks the legal graph
again. Both refresh and validate the relevant upstream cache repositories.

`bundle` and experimental `build` require a legal, buildable workspace. They permit the generated
license placeholder because it does not prevent compilation. If a tool-owned Git checkout is
missing, or if there is no lock and the
manifest has dependencies, the command runs `fetch` first (without `--clean`). A missing lock
makes fetch run `update --yes` so a first clone can bundle without a prompt. An existing lock is
never rewritten.
Each of those hand-offs prints why the inner command is running. A dirty checkout that would
require `--clean` still fails; run `slang package fetch --clean` yourself.

`fetch` and `update` always verify the workspace graph from selected manifests **before** they
clear search paths or materialize `deps/`. A release is read at its locked commit from `deps/NAME`
when that repository contains the commit, otherwise from `.slang/repositories`. The committed manifest is
read, not the working-tree file, so a dirty release checkout cannot change what the graph check
sees. An edit is read from the working tree. Online commands fetch origin data only into
`.slang/repositories`. Published branches are stored as `refs/remotes/origin/*` and published tags
as `refs/slang-cache/tags/*`. Fetch does not update or delete `refs/heads/*` or `refs/tags/*`, so a
branch or tag created in the worktree stays put. `deps/NAME` is a worktree of that repository and
already sees the published refs. After materialization, they apply the publishable-package checks
to each Git package whose checkout was newly created or changed, then check source layout and
import uniqueness across the complete selected graph.
The closure check includes unchanged packages because a new module can
collide with one already selected. `update --dry-run` runs that legal-graph check and still cannot
claim source-layout success, because it does not materialize remote trees.

Pass `--skip-validate` on `fetch`, `update`, `bundle`, or experimental `build` only as an escape hatch. It skips
source-layout and new-release publish checks **after** materialize. The legal graph still runs
first. The command prints a warning. `slang package validate` has no skip flag.

`slang package status` prints one header line when the workspace is current, for example
`Package 'video-preview': lock current, 3 packages, buildable.` Extra lines appear only when
something is dirty. The header says `incomplete` when the lock or Git pins are missing, and
`not buildable` only after those trees are present and the source check fails. Dirty checkouts and
edits are listed by name. Like `git status`, reportable drift does not make the command fail.
Status returns nonzero only when the required root manifest or an existing lock cannot be read and
parsed well enough to produce a report, and when `slang-package-overlay.json` still contains
entries. That file is no longer used. It also reports when a locked ref or commit is absent from
the existing cache.
It does not inspect `out/`, modify package state, or contact remotes.

Use `slang package dependency add` and `dependency remove` to change direct manifest edges, and
`dependency list` to inspect them. Add accepts `--git URL --version RANGE` or
`--git URL --ref REF [--as VERSION]`. When `--git` is omitted and `SLANG_PACKAGE_INDEX` names a
package index, the Git URL is copied from that index into the manifest. The variable is a local
path, resolved from the current directory when it is relative, or an `http`/`https` URL. The
index is a JSON object with `"schema_version": 1` and a `packages` map from package name to Git
URL. `--version` or `--ref` is still required, and an explicit `--git` is not replaced.
`fetch` and `update` do not read that variable. `update --remap-urls URL` records an `http` or
`https` package index in the lock and resolves listed packages from it, leaving manifest URLs
unchanged. A later `update` reuses the index stored in the lock. `update --no-remap` clears it
and resolves from the manifest URLs again. `fetch --remap-urls URL` sets the same policy;
`fetch` without the option installs the locked URLs and does not read the index. These add and
remove commands change only `slang-package.json` until
`update` also rewrites the lock; inspect
`status` and run `update` afterward. `slang package pin NAME VERSION` holds a solved release at
that exact version, and `slang package unpin NAME` clears that hold. The boolean lives in the lock,
not the manifest. `slang package tree` prints
the selected lock graph, while
`slang package why NAME` prints every current root-to-package path and incoming requirement. Why
explains the graph that is locked now, not candidates rejected during an earlier solve. Unlike
`status`, `tree`, and `why` read local workspace and cache state without contacting remotes.

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
version can be parsed. It adds `.slang/`, `deps/`, `out/`, `slang-package-overlay.json`, and
`slang-package-includes.txt` to `.gitignore`. The overlay filename remains ignored so an old file
is not committed. A file that still contains entries is an error. `slang package help` lists
commands under the manifest, lock, and bundle.
`.slang/repositories/` contains one permanent Git repository for each exact package URL that has
been resolved. The directory name starts with the URL's repository name, for example `noise-`
followed by eight hexadecimal characters of the URL's SHA-1. Resolution reads those repositories.
`deps/NAME` is a worktree of the repository for the URL selected for that package, so switching
the URL detaches that worktree and attaches the other repository without deleting either one.
Fetched source remains visible there; generated files go under `out/`.

`slang package edit NAME --branch BRANCH` checks out `BRANCH` in the already-resolved
`deps/NAME` checkout. `--create` creates a missing branch from the commit the dependency is
resolved to and does not reset a branch that already exists. The lock keeps the version, tag, and
commit it already had, and records the branch beside them. The pin boolean is unchanged.
`edit NAME --advance` moves that version, tag, and commit, in one step, to the greatest canonical
tag on the line of history from the branch tip back to the commit the row is representing. A tag
between the represented release and that latest tag is not a separate step. The tag must still
satisfy incoming constraints. The checkout does not move. `--advance` cannot be combined with
`--branch` or `--create`. The package name stays the first argument, so a dependency named
`advance` is edited with `edit advance --branch BRANCH`.

`slang package unedit NAME` ends the edit on a release tag. `--advance`, `--restore`, and
`--tag VERSION` select one ending and cannot be combined. There is no `--yes`. Declining a prompt
leaves the edit. A run without a terminal that needs approval fails and leaves the edit. None of
the endings changes the pin boolean.

Plain `unedit` selects the greatest canonical tag on that same line that is strictly newer than
the represented release and that still satisfies incoming constraints. When that tag is the
checked-out commit, the lock switches to it without a prompt and without moving `HEAD`. When the
tag is behind `HEAD`, the command says how many commits would leave the workspace and asks whether
to check out the tag. Declining does not fall through to the represented release. When no newer
tag satisfies the constraints, the only ending offered is the release the edit is already
representing, and that still asks. `--advance` requires a newer legal tag. `--restore` returns to
that same represented release, including after `edit NAME --advance`, and always asks.
`--tag VERSION` creates a local annotated tag, `v` plus the canonical spelling, on `HEAD` and then
selects it. The tag is not pushed. The command fails before creating the tag when the release
already has a tag, the version is not strictly greater than the stored version and every canonical
tag already on the edit line, or the version does not satisfy the constraints.

`--clean` discards uncommitted files, untracked files, and stashes so `HEAD` may move. It does not
approve dropping commits that follow a tag. A dirty checkout blocks any ending that would move
`HEAD` until those changes are committed or `--clean` is passed.

`slang package pin NAME VERSION` sets the lock boolean and stores that exact release when the
dependency is not edited. The version must satisfy incoming constraints, and a canonical tag must
exist. `pin NAME` while the dependency is edited sets the boolean only. `unpin NAME` clears the
boolean and works during an edit. The branch stays. `update` will not move a pinned version.
`edit NAME --advance` and `unedit` can, and they leave the boolean set.

A non-empty `slang-package-overlay.json` is an error. There is no override command and no path
dependency. An edit changes the branch of `deps/NAME`. It does not change where the package lives.
An edited lock is not something to commit. Run `unedit` first. A pinned release row can be
committed.

Fetched package trees contain source only. Compilation output must be written outside these trees
because the same source commit can be compiled against different resolved dependency graphs.

`slang package bundle` validates the materialized package graph and writes the source distribution.
It copies every exported `.slang` file into `out/bundle/source/` at the same import-relative paths.
The resulting tree is a single search path: `src/acme/noise.slang` and its companion
`src/acme/noise/helper.slang` become `out/bundle/source/acme/noise.slang` and
`out/bundle/source/acme/noise/helper.slang`. A case-insensitive name collision across packages is
an error.

`slang package --experimental build` is that same distribution plus binary targets. The global
option is required because the binary format is unstable. Build compiles every primary module in
the workspace and its resolved dependencies to a front-end `.slang-module` under
`<workspace.output>/bundle/modules`, preserving its import path. For example, an exported
`src/acme/noise.slang` becomes `out/bundle/modules/acme/noise.slang-module`, whether that source
belongs to the workspace or a dependency. Companion files included by that primary are compiled
into the same artifact and do not produce separate files. A later `bundle` removes that module
directory and any host output.

The `.slang-module` binary format is unstable and has no compatibility guarantee. Every build that
generates one prints a warning. `out/bundle/modules/provenance.json` records
`experimental: true`, `format_stability: "unstable"`, and the compiler's name, version, exact
source commit when available, tracked-source `dirty` state, and path. Consumers must require the
same Slang toolchain that produced the files. The module tree can be used as a source-free search
path only with that constraint; it is not a stable distribution format.

When `build.host.executables` is present, experimental `build` also compiles each matching
workspace primary with the host executable target and writes `out/host/<executable-name>` (plus
`.exe` on Windows). The `main` function in that file must use the native ABI, such as
`export __extern_cpp int main()`, and a supported downstream C++ compiler must be available. The command
copies the matching `slang-rt` shared library beside the executables so the artifacts can locate
their runtime support. It also writes `out/host/EXPERIMENTAL.txt`, so the status remains visible
when the host directory is copied independently. Configuring a host executable without a matching
workspace `.slang` primary is an error. `bundle` does not compile host executables.

Both commands copy every `.md` file below each materialized package's `docs/` directory to
`out/docs/<package-name>/`, preserving paths below `docs/`. Namespacing the output by package
keeps files such as `docs/README.md` from different packages distinct. Other file types are not
copied. They also write `out/docs/index.md`: the workspace dependency tree, then an alphabetized
list of copied Markdown files per package. Every package in the graph is listed; only packages
that contributed Markdown link from the tree into that file list.

`slang package --experimental run` uses sibling `slangi` to interpret an existing primary in
`out/bundle/source` and forwards trailing arguments. The command is rejected without the global
`--experimental` option. With no executable name, it selects `build.host.default`. It does not
bundle first and fails with instructions when the manifest does not configure a host executable
or `slang package bundle` has not produced the source bundle. Because `slangi` derives its only
source search path from the input filename, this mode requires the configured primary to be at an
export root, and therefore at the root of the flattened source bundle. Nested executable primaries
require binary mode.

`slang package --experimental run --binary` instead executes the selected native artifact under
`out/host`. `--binary` must immediately follow `run`. This mode requires output from
`slang package --experimental build`.

`slang package test` is reserved and currently reports that it is not implemented. It does not
invoke `slang-test`. Package testing will get a dedicated model; `slang-test` remains an internal
compiler harness and is not part of the package command surface.

`slang package docs` opens `out/docs/index.md` with the application registered for Markdown
on this machine. `--print` writes that path instead of launching, which is what scripts should
use. The command does not copy or regenerate documentation; run `slang package bundle` or
`slang package --experimental build` for that.

## Possible future enhancements

The workspace keeps one permanent Git repository per package URL under `.slang/repositories/` and
checks out the selected commit as a worktree under `deps/`. Future versions may add a user-global
immutable cache with copy-on-edit, let compiler sessions consume workspace metadata without
`slang-package-includes.txt`, and share immutable dependency trees between workspaces. An edit
changes the branch of the
checkout at `deps/<name>`. It does not point that package at another directory.
