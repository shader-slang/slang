# Contributing to the docs

This site documents `h-slang`'s heterogeneity features. Add explanations of the programming
model, short examples, and the execution guarantees readers need to use each primitive.

Describe the available features in short sentences, followed immediately by examples
and their expected results. Prefer this structure over step-by-step walkthroughs.

In prose, refer to the language flavor as `h-slang`, using inline code formatting.

Show expected printed results as comments beside the relevant statements in the main
snippet, rather than in a separate output code block. Prefer range views for buffer
transfers; keep explicit `copy` for partial host arrays and read-only destinations.
Tensor snippets should use the public Tensor API, without accessing its `buffer` field.

## Preview locally

Run the following from the root of the `slang` repository. Python 3.10 or newer is
recommended. A virtual environment keeps the documentation tools separate from compiler
and runtime dependencies.

On Windows, in PowerShell:

```powershell
python -m venv .temp/docs-venv
.\.temp\docs-venv\Scripts\python.exe -m pip install -r docs-requirements.txt
.\.temp\docs-venv\Scripts\python.exe -m mkdocs serve
```

On Linux or macOS:

```sh
python3 -m venv .temp/docs-venv
.temp/docs-venv/bin/python -m pip install -r docs-requirements.txt
.temp/docs-venv/bin/python -m mkdocs serve
```

Open <http://127.0.0.1:8000/>. The preview reloads when pages change; press `Ctrl+C` in
the terminal to stop it. If that port is occupied, add `--dev-addr 127.0.0.1:8001`.
The server listens only on the local machine.
Local previews and static builds use Material 9.6.23. After a dependency change, rerun
the install command above and restart the preview server.

### Preview browser examples

For repeated IntelliSense and slangvm runs, serve the built site with HTTP/1.1:

```sh
python -m mkdocs build --strict
python -m http.server 8000 --bind 127.0.0.1 --protocol HTTP/1.1 --directory build/heterogeneity-docs
```

Use the docs environment's Python and stop any existing preview on port 8000 first.
This keeps connections open between asset requests. On this Windows setup, MkDocs'
HTTP/1.0 development server intermittently left connection attempts pending, delaying
worker scripts and WASM downloads by minutes. HTTP/1.1 reduces connection churn, and
the latest nine cold/warm browser checks passed. If an existing port still stalls,
try another port such as 8001. This preview serves the last build; rebuild and refresh after edits.
The `--protocol` option requires Python 3.11 or newer.

## Build and check

Using the virtual environment's Python, run:

```sh
python -m mkdocs build --strict
```

For example, in PowerShell without activating the environment:

```powershell
.\.temp\docs-venv\Scripts\python.exe -m mkdocs build --strict
```

The output goes to `build/heterogeneity-docs/`, which is already ignored by Git.
Strict builds fail on warnings, including missing local links and anchors. External
links should also be checked when editing a page.

Check the rendered page at desktop and narrow widths, follow its links, and try search
before submitting a change. Building the site uses the packaged static assets. Running
code examples is a separate check against a matching compiler and runtime.

## Packaged browser runtime

The documentation can be built and served locally using the commands above.
This branch does not configure automatic deployment of the heterogeneity site.

For now, the complete browser runtime is checked in under `javascript/docs-runtime/`.
This includes both WASM modules, their matching JS wrappers, bundled standard-module
sources and precompiled `.slang-module` files,
Monaco and its workers/fonts, and license notices. `.gitattributes` preserves the exact
bytes of these generated files so checkout does not invalidate the manifest hashes.
The WASM and precompiled module binaries use Git LFS to keep large generated
assets outside ordinary Git blobs. Local checkouts need Git LFS; run `git lfs pull` if these files are
still pointers. Download LFS objects before building the site; the manifest check
rejects pointer files.

Refresh this temporary snapshot locally whenever the compiler, standard modules, or
editor changes. Use Emscripten 6.0.0 and TypeScript 5.9.3, matching the release workflow.
After configuring the Emscripten build with matching native generators:

```sh
cmake --build --preset emscripten --target slang-wasm slangvm-javascript
npm ci --prefix javascript/docs-editor
npm run build --prefix javascript/docs-editor
python extras/prepare-docs-runtime.py --snapshot
```

The snapshot command replaces only `javascript/docs-runtime/`. Commit that directory
along with the corresponding source changes. Its manifest records asset hashes without repository history or local build paths;
a docs-only commit can reuse it.
The package's `slang/standard-modules.json` lists the precompiled standard modules.
Staging and runtime validation require every listed binary and its source. Browser checks reject
standard-module source fallback during execution while verifying source-based IntelliSense.
The runner passes an explicit filesystem to `slangvm.init`: current editor sources first,
preloaded library bytes second, and `HTTPSlangFileSystem` for imports loaded on demand.
On the first press of ![Run](images/snippet-run.svg "Run"), the page preloads `slang/runtime.slang-module` and its implementation subtree
using that inventory. Subsequent Runs reuse the bytes, including through snapshots sent to
replacement workers. Unrelated standard modules remain on demand; slangd still loads sources.

The docs pass the shipped binary/source inventory as `HTTPSlangFileSystem`'s
`allowedPaths`, including in replacement workers. Unknown import candidates return
`null` synchronously without HTTP requests, including on the first Run. Other shipped
modules can still load on demand. Update the inventory when adding packaged files.
The docs also set `cacheNotFound: true` to remember HTTP 404/410 responses for listed
assets until page reload. Edited in-memory source takes precedence over the library.

Before committing a refreshed snapshot, select it explicitly and run the local checks
(PowerShell environment-variable syntax shown):

```powershell
$env:HSLANG_DOCS_RUNTIME_DIR = "javascript/docs-runtime"
$env:HSLANG_DOCS_REQUIRE_RUNTIME = "1"
python -m unittest discover -s extras/tests -p 'test_docs_*.py'
python -m mkdocs build --strict
node --test javascript/slangvm/tests/*.test.js
npm exec --prefix javascript/docs-editor -- playwright install chromium
npm run test:snippets --prefix javascript/docs-editor
```

Set `CHROME_PATH` to use an installed Chrome or Edge executable instead of installing
Chromium. Browser checks run every runnable example, including CPU-only execution,
GPU readback, and the canvas gradient, below `/preview/docs/` to exercise project URL
prefixes. They require a working WebGPU adapter and WebAssembly JSPI.

Select the committed snapshot with `HSLANG_DOCS_RUNTIME_DIR` and require
it with `HSLANG_DOCS_REQUIRE_RUNTIME=1` as shown above. Missing or changed assets fail the build.
Local previews prefer `build/heterogeneity-docs-runtime/` when present, otherwise use
the checked-in snapshot. Set `HSLANG_DOCS_RUNTIME_DIR` to select either explicitly.

The static output includes local JS/WASM, standard modules, workers, fonts,
search data, images, and license notices. Asset URLs resolve relative to the site.
Readers need a browser with WebAssembly JSPI; GPU examples also require WebGPU
and a secure context, such as HTTPS or localhost.

## Runnable snippets

The [first program](invoking.md), [host examples](host-functions.md),
[compute examples](grid-dispatch.md), and [SDF tutorial](sdf-path-tracer.md) have editors
with ![Run](images/snippet-run.svg "Run") and ![Reset](images/snippet-reset.svg "Reset") controls on hover or focus.
The page loads a minimal Monaco editor with Slang highlighting. Focusing it starts
IntelliSense in a separate worker; the first press of ![Run](images/snippet-run.svg "Run") fetches slangvm and its WASM
compiler. Text examples compile the current text in a fresh module worker and VM, with
output and diagnostics below the editor. Stop terminates the worker. Runs time out
after two minutes, and printed output is limited to 64 KiB of text.

The runnable name is also the file's name: `<!-- runnable: render-sphere -->` creates
`render-sphere.slang`. Names must be unique on a page; examples that draw an image use a
`render-` prefix. Another cell on that page can use `import "render-sphere";` (quoted,
because of the hyphen) to access its public declarations. Each press of ![Run](images/snippet-run.svg "Run") compiles a snapshot of
the current text in all cells, so imported cells need not be run first. Only the
selected cell's `main` executes. IntelliSense also sees edits to imported cells.
Use **Ctrl+click** (**⌘+click** on macOS), **F12**, or **Go to Definition** in the
context menu to follow imports and symbols to their definitions in cells on the
same page. Navigation opens enclosing deep dives and code folds and focuses the target
line. It uses the current edited sources, including quoted module imports.
After an edit, completed results show a reminder to run again; execution is never
automatic. Pressing ![Reset](images/snippet-reset.svg "Reset") restores only the selected cell.

Shared support files use `<!-- module: name -->` before their Slang code fence.
They participate in the same editable source snapshot and IntelliSense, with Reset
but no Run button or entry point. Add an empty entry in `scripts/snippet-files.json`
when the module has no external assets. A hyphenated filename is imported with a
quoted name, for example `import "sphere-tracer";`.

Put supporting explanations and module cells inside a collapsed `??? example "Deep Dive: ..."`
section, indenting both the marker and code fence by four spaces. Modules remain
available to other cells while collapsed. Keep parameters readers should change as
`static const` globals above functions, and leave the example's short `main` visible.
The SDF tutorial shares its frame loop, camera, PRNG, sampling helpers, generic SDF
tracer, BRDF contract, path tracer, and progressive renderer this way. Its source
extraction helper is `extras/sdf-tutorial-sources.cjs`, including indented modules.

To initially collapse a section, add a trailing `//!FOLD` comment to its opening brace
or to the declaration immediately before that brace:

```slang
void helper() //!FOLD
{
    print("This body starts folded.");
}
```

The snippet script removes the marker before editing, IntelliSense, or execution. Readers
can unfold normally; pressing ![Reset](images/snippet-reset.svg "Reset") restores the original source and default folds. Markers without
a matching foldable section are ignored. Matching text inside strings or block comments
is preserved. This works locally in Monaco without waiting for slangd.
For Allman-style blocks, the folded view keeps the declaration followed by an ellipsis;
both brace lines are hidden. Monaco folds whole lines, so a brace written on the same
line as the declaration stays visible. Unfolding restores the original formatting.

Canvas examples run in a fresh VM on the main thread, using the package's DOM window support.
Their loading phase times out after two minutes; drawing continues until `main` returns or
Stop closes the supplied window. The runner keeps a snapshot at each presentation before closing the VM,
so a completed example retains its image without keeping the VM alive. Resizing completed or
stopped output clears it to white. Canvas loops must
yield through `processWindowEvents`; a CPU loop that never yields can block the page and Stop.

Prepare the optional runtime assets before serving the site:

1. Build the native generators and the browser package following
   [slangvm's build instructions](../../javascript/slangvm/README.md#build-and-package).
   With the Emscripten build configured, run `cmake --build --preset emscripten-slangvm`.
2. Build the language service from the same checkout with `cmake --build --preset emscripten`.
   This uses the standard WASM filesystem and does not require WebGPU or JSPI.
3. Build the editor bundle (Node.js and npm required):

   ```sh
   npm ci --prefix javascript/docs-editor
   npm run build --prefix javascript/docs-editor
   ```

4. From the Slang repository root, use the docs environment's Python:

   ```sh
   python extras/prepare-docs-runtime.py
   python -m mkdocs serve
   ```

The staging script defaults to `build.em/Release/slangvm`; use `--slangvm-dir` for
another matching build. The editor defaults to `build/docs-editor` and slangd to
`build.em/Release/bin`; override them with `--editor-dir` and `--slang-wasm-dir`.
It stages the browser package, its bundled standard modules, and licenses in
`build/heterogeneity-docs-runtime/`. The MkDocs hook includes these as
static assets in the site output. Rebuild and stage again when the runtime or standard
modules change, then restart the preview. Local staging stays out of Git; the temporary
browser runtime snapshot is committed separately as described above.

The site can still build without generated assets: editing falls back to plain text,
and pressing ![Run](images/snippet-run.svg "Run") reports a loading error if slangvm is absent. If only the language
service fails, Monaco keeps highlighting and editing and shows the error below it.
WebGPU needs HTTPS or localhost, and slangvm also needs WebAssembly JSPI.
Compilation and execution happen in the browser. Standard modules come from the
slangvm package, independently of application source URLs. Native-only APIs remain
unavailable in the browser. The first program
uses the CPU-only backend and does not require WebGPU.

To enable another self-contained example, put `<!-- runnable: example-name -->`
on its own line immediately before its fenced `slang` block. Supply `[host] void main()`
and use browser-compatible operations. For CPU-only programs, use
`<!-- runnable: example-name backend=none -->`; the default backend is WebGPU.
Add an entry with that name to `scripts/snippet-files.json`. Use an empty mapping
(`{}`) when the example imports only standard modules or other cells. The language
worker loads standard-module sources from the packaged `standard-modules.json`
inventory, including transitive dependencies. To supply additional files, map
absolute virtual filesystem paths to URLs:

```json
{
  "example-name": {
    "/helpers.slang": "../examples/helpers.slang"
  }
}
```

URLs resolve relative to the mapping file. Include any additional transitive imports. Each snippet's
language worker fetches these files in parallel, creates directories with
`FS.mkdirTree()`, writes files with `FS.writeFile()`, and then creates slangd.
Editor changes use document notifications. Unmapped imports produce normal compiler
diagnostics; files are never fetched on demand by slangd. Each snippet has its own
WASM instance and filesystem. Editable cells override mapped files with the same path.
On Run, mapped files are also loaded as bytes into the execution filesystem. This
supports both helper modules and binary assets such as the SDF tutorial's EXR environment map.
Include required asset mappings in each runnable cell that uses them, including
through imports; importing another cell does not inherit that cell's mappings.
The staging script validates mappings into the packaged standard modules. Other URLs
must be served separately, and application imports must also be reachable through the
VM's filesystem when running the example.

For canvas output, use `<!-- runnable: example-name canvas=16:9 -->` and
`[host] void main(Window w)`. The ratio controls the display's height; its width fits the
container. Backing dimensions use the device pixel ratio, capped at 2. The runner sizes the
canvas before `main` and through slangvm's `beforeFrame` callback before each
`processWindowEvents` call resumes, so resize events and pointer positions use the same size.

Import `slang.runtime` for the window and texture helpers.
Use `createSurface`, `acquireSurfaceTexture`, `blitTexture2D`, and `presentSurface`
for one-shot output. Continuous examples should poll input, handle window resize events,
and yield through `processWindowEvents`, as shown in [grid dispatch](grid-dispatch.md).
The examples do not depend on a separate viewer library.

## Add a feature page

1. Add a Markdown file beneath `docs/heterogeneity/`.
2. Add its title and path to `nav` in the root `mkdocs.yml`.
3. Introduce the primitive's purpose before its signature. Explain where code executes,
   what values cross the host/device boundary, and when work completes.
4. Use small examples with enough context to understand their inputs and outputs.
   Keep examples self-contained so readers can run them using the bundled modules.
5. Describe limitations and link to related feature pages using relative Markdown links.
6. Build with `--strict` and review the local preview.

For files elsewhere in this repository, use a relative path through the repository
root, such as `[build setup](../building.md)` or
`[VM build notes](../../source/slangvm/README.md)`. These links follow the checked-out
branch when viewed in GitLab or GitHub and also work in local Markdown viewers.
Link to a directory's README explicitly. Keep clone commands and links to other
repositories as full URLs.

MkDocs publishes only `docs/heterogeneity/`, so the docs hook checks these file paths
and expands links outside that directory using `extra.repository_source_url` in
`mkdocs.yml`. To build against another host or ref, set `HSLANG_DOCS_SOURCE_URL` to
its file-view base URL (including `blob/<ref>/` on GitHub or `-/blob/<ref>/` on GitLab).
Links between published pages remain relative site links. Anchors and query strings
are preserved; link examples inside code blocks are not rewritten.

Keep the landing page to the project description, experimental warning, and getting-started
links. Put new topics in **Intro**, **Programming Model Guide**, or **Tutorials**. Prefer runnable snippets
and their output; reserve longer explanations for **Rough edges** and **Behind the scenes**.

## Example outputs

`images/relax_inn_seaview_suite_1k.exr` is the 1024 × 512 linear HDR environment
[Relax Inn Seaview Suite](https://polyhaven.com/a/relax_inn_seaview_suite) from
Poly Haven: photography by Dario Barresi, processing by Jenelle van Heerden,
licensed [CC0](https://polyhaven.com/license). The original is preserved. The tutorial
samples `images/relax_inn_seaview_suite_front_1k.exr`, a copy shifted left by 256 pixels
with horizontal wrapping (a 90° change in longitude). This places the window roughly
45° from the camera-facing direction, toward +X. The shift preserves every HDR value
without interpolation, and the new EXR retains the original float channels and lossless
PIZ compression. Regenerate it with `python extras/rotate-sdf-environment.py`
(requires the `OpenEXR` and `numpy` Python packages).

`images/sdf-environment.png` is only a tone-mapped, sRGB-encoded preview of the rotated map.
Regenerate that preview with `node extras/render-sdf-environment-preview.cjs` after
staging or building the docs runtime. It uses the tutorial's current `displayColor`
helper and the docs editor's Playwright dependency. Set `CHROME_PATH` to select an
installed Chrome or Edge. The script runs headlessly and serves files on loopback only.
Pass `--ggx-comparison` to regenerate `images/sdf-ggx-roughness.png`. That mode uses
the tutorial's GGX sampler to render roughness 0.1, 0.35, and 0.7 at 4,096 samples
per pixel, with the same environment and display transform in each panel.

The SDF tutorial's vector diagrams are generated by
`python extras/generate-sdf-diagrams.py`. Edit that script and regenerate the SVGs
together. Chapter 1 uses `images/sdf-signed-distance.svg`, `images/sdf-sphere-tracing.svg`, `images/sdf-camera.svg`,
`images/sdf-normal-light.svg`, and `images/sdf-smooth-union.svg`; Chapter 2 uses
`images/sdf-mirror-vectors.svg`, `images/sdf-hemisphere-vectors.svg`, and
`images/sdf-ray-offset.svg`. Chapter 3 adds `sdf-microfacets.svg` and `sdf-half-vector.svg`.
The script shares colours, arrowheads, and typography.
Tracing circles use the exact sphere distance, and the union contours come from
the same distance formulas as the tutorial. The SVGs include titles
and descriptions; also supply descriptive alt text when embedding them in Markdown.
Use simple symbols in the drawing and explain them with KaTeX equations in the
surrounding prose. SVG images do not run the page's math renderer.

The images in `images/` are D3D12 renders:

- `triangles.png`: the complete program in [Launch draw calls](draw-calls.md).
- `silhouette.png`: the first four blocks in [Launch ray-tracing pipelines](ray-tracing.md).
- `ao-bunny.png`: an ambient-occlusion bunny render illustrating the secondary-ray pattern.

The triangle examples write their output directly to PNG files.

The bunny image was rendered offscreen at 1024 × 768 for 64 frames, with 32 visibility
samples per surface pixel per frame. Its accumulated linear RGB was converted to sRGB
before PNG encoding. It is an illustrative output; the abbreviated code on the page
does not reproduce the complete image.

## Code blocks and styling

Use fenced `slang` blocks for `h-slang` snippets. The Markdown configuration maps
`slang` to Pygments' HLSL lexer for build-time highlighting and fold-marker detection;
`h-slang`-specific keywords may appear as ordinary identifiers. The runnable editor
uses Slang independently of this highlighting alias. Use `powershell`, `sh`, or `yaml`
for other languages.
The generated site uses system fonts and serves its search, styles, scripts, and
editor fonts locally; Material's Google Fonts loading is disabled.

### Links into code cells

Use `cell:` destinations in ordinary Markdown links to refer to a runnable or module
cell on the same page. Include its `.slang` filename and, optionally, a definition:

```markdown
Use [`sphereTrace`](cell:sphere-tracer.slang#sphereTrace) to find a hit.
See [`distance`](cell:checker-scene.slang#SphereCheckerScene.distance).
Open the complete [sphere tracer](cell:sphere-tracer.slang).
```

The filename must match the cell's `runnable:` or `module:` marker. Qualify members
with their containing type or namespace, separated by dots. Reference-style Markdown
links work too. Link names are case-sensitive; use declaration names without generic
arguments or parameter lists. Overloaded declarations offer a choice of definitions.

Following a link opens its Deep Dive, unfolds the definition, and selects it in the
editor without running the program. Symbol lookup uses the current edited source;
moving a definition does not break its link. If the reader renames or removes it,
navigation opens the cell and explains that the definition was not found. If the
editor or language service cannot load, navigation still opens the code cell.

The docs build validates cell filenames and generates ordinary fragment links, such
as `#cell-sphere-tracer.slang--sphereTrace`, so the resulting URLs can be shared and
used with browser Back/Forward. Symbol names are resolved when followed, rather than
compiled during the docs build. These links currently target cells on the same page.

### Math notation

Write LaTeX math inline with `$...$`, or on its own line between `$$` delimiters:

```text
The error falls as $1 / \sqrt{N}$ for $N$ samples.

$$
% label: Running average
\mu_{n+1} = \mu_n + \frac{X_{n+1} - \mu_n}{n + 1}.
$$
```

The error falls as $1 / \sqrt{N}$ for $N$ samples.

$$
% label: Running average
\mu_{n+1} = \mu_n + \frac{X_{n+1} - \mu_n}{n + 1}.
$$

Name each display equation with a `% label:` line inside its block, so readers can
tell what it is at a glance. `scripts/math.js` shows the name as a caption under the
equation and numbers it after the enclosing section, such as `Eq 1.4(a): Translated
distance field`. To number an equation without a caption, write `% number` instead;
the number then appears just after the centred equation. Without the script, KaTeX
treats either line as a comment.

Use blank lines around display blocks. Use backticks for literal code and escape a
literal dollar sign as `\$` when needed. Math delimiters inside code blocks and
editable Slang cells are left alone. Explain new symbols next to the equation;
keep the corresponding code expression when it helps a beginner connect the two.

`pymdownx.arithmatex` preserves the expressions during Markdown conversion, and
`scripts/math.js` renders only those marked expressions with KaTeX. This supports
[LaTeX math notation](https://katex.org/docs/supported), including fractions,
vectors, sums, integrals, and aligned equations; it is not a full LaTeX document engine.
Rendered math includes MathML for accessibility. Long display equations scroll
horizontally within their own block on narrow screens.

KaTeX 0.19.1, its fonts, and its license are checked in under `vendor/katex/`, so
viewing or building the docs does not require a CDN or an npm install. To reproduce
the bundled files from the pinned archive, run from the repository root:

```sh
npm pack katex@0.19.1 --ignore-scripts --pack-destination .temp
python extras/prepare-docs-math.py .temp/katex-0.19.1.tgz
```

The script checks the archive's SHA-512 integrity before copying files. To upgrade,
update the version and integrity in the script together, regenerate the assets, and
check the rendered equations. The Markdown configuration follows
[Material's math integration](https://squidfunk.github.io/mkdocs-material/reference/math/).

### Site styling

The site uses Material with a muted green palette and dark green navigation. Material
color variables and layout adjustments live in `docs/heterogeneity/stylesheets/extra.css`;
the runnable editor styles in `stylesheets/snippets.css` target Material's `.md-typeset`
content area. The Pygments stylesheet is checked in as
`stylesheets/code.css`; after changing its style or the pinned Pygments version,
regenerate it with the virtual environment's Python:

```sh
python -c "from pathlib import Path; from pygments.formatters import HtmlFormatter; Path('docs/heterogeneity/stylesheets/code.css').write_text(HtmlFormatter(style='friendly').get_style_defs('.codehilite') + '\n', encoding='utf-8')"
```
