"""Resolve repository links and include the browser runtime in the docs site."""

import hashlib
import json
import os
from pathlib import Path
from urllib.parse import quote, unquote, urlsplit, urlunsplit

from markdown.extensions import Extension
from markdown.treeprocessors import Treeprocessor
from mkdocs.exceptions import PluginError
from mkdocs.structure.files import File


class RepositoryLinks(Extension):
    """Resolve links outside docs_dir against the configured repository source URL."""

    def __init__(self, config, page):
        super().__init__()
        self.repo = Path(config.config_file_path).resolve().parent
        self.docs = Path(config.docs_dir).resolve()
        self.page = Path(page.file.abs_src_path).resolve()
        self.source_url = config.extra["repository_source_url"].rstrip("/") + "/"

    def extendMarkdown(self, md):
        links = self

        class ResolveLinks(Treeprocessor):
            def run(self, root):
                for element in root.iter("a"):
                    url = urlsplit(element.get("href", ""))
                    if url.scheme or url.netloc or not url.path.startswith("../"):
                        continue
                    target = (links.page.parent / unquote(url.path)).resolve()
                    if target.is_relative_to(links.docs):
                        continue  # MkDocs handles links to other published pages and assets.
                    if not target.is_relative_to(links.repo) or not target.is_file():
                        raise PluginError(f"Missing repository file linked from {links.page.name}: {url.path}")
                    path = quote(target.relative_to(links.repo).as_posix(), safe="/")
                    base = urlsplit(links.source_url + path)
                    element.set("href", urlunsplit((base.scheme, base.netloc, base.path, url.query, url.fragment)))

        # Parsed links include reference-style links, but exclude examples inside code blocks.
        # Resolve them before MkDocs validates paths against its smaller docs_dir (priority 0).
        md.treeprocessors.register(ResolveLinks(md), "repository_links", 10)


def on_page_markdown(markdown, page, config, **kwargs):
    # Each page has its own source directory. Replace the previous page's extension on rebuilds.
    config.markdown_extensions = [
        extension for extension in config.markdown_extensions if not isinstance(extension, RepositoryLinks)
    ]
    config.markdown_extensions.append(RepositoryLinks(config, page))
    return markdown


class RuntimeAsset(File):
    # License/notices Markdown in a package must be served verbatim, not added as docs pages.
    def is_documentation_page(self):
        return False


def on_files(files, config):
    repo = Path(config.config_file_path).resolve().parent
    runtime_dir = os.environ.get("HSLANG_DOCS_RUNTIME_DIR")
    root = repo / "build/heterogeneity-docs-runtime"
    if runtime_dir:
        root = (repo / runtime_dir).resolve()
    elif not (root / "assets/runtime-manifest.json").is_file():
        root = repo / "javascript/docs-runtime"
    if os.environ.get("HSLANG_DOCS_REQUIRE_RUNTIME") == "1":
        manifest_path = root / "assets/runtime-manifest.json"
        if not manifest_path.is_file():
            raise PluginError(
                "Deployment requires browser assets. Refresh javascript/docs-runtime with "
                "extras/prepare-docs-runtime.py --snapshot, or select a local runtime build."
            )
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        if not manifest.get("editor") or not manifest.get("files"):
            raise PluginError("Deployment requires a complete runtime manifest, including the editor and language service.")
        required = {
            "assets/slangvm/src/index.js", "assets/slangvm/wasm/slangvm-wasm.js",
            "assets/slangvm/wasm/slangvm-wasm.wasm", "assets/slangd/slang-wasm.js",
            "assets/slangd/slang-wasm.wasm", "assets/editor/editor.js", "assets/editor/editor.css",
            "assets/editor/editor.worker.js", "assets/editor/language.worker.js",
        }
        mappings = json.loads((repo / "docs/heterogeneity/scripts/snippet-files.json").read_text(encoding="utf-8"))
        required.update(url.removeprefix("../") for mapping in mappings.values() for url in mapping.values()
                        if url.startswith("../assets/"))
        inventory_path = root / "assets/slangvm/slang/standard-modules.json"
        if not inventory_path.is_file():
            raise PluginError("Missing precompiled standard-module inventory; refresh the runtime snapshot.")
        precompiled = json.loads(inventory_path.read_text(encoding="utf-8"))
        binaries = {"assets/slangvm/slang/" + name for name in precompiled}
        if not binaries or binaries != set(manifest.get("precompiled_modules", [])):
            raise PluginError("Runtime snapshot has an incomplete precompiled standard-module inventory.")
        required.add("assets/slangvm/slang/standard-modules.json")
        required.update(binaries)
        required.update(Path(name).with_suffix(".slang").as_posix() for name in binaries)
        missing = required - manifest["files"].keys()
        if missing:
            raise PluginError("Runtime snapshot is incomplete; rebuild it locally: " + ", ".join(sorted(missing)))
        for name, digest in manifest["files"].items():
            path = (root / name).resolve()
            if not path.is_relative_to(root) or not path.is_file():
                raise PluginError(f"Missing runtime asset: {name}")
            if hashlib.sha256(path.read_bytes()).hexdigest() != digest:
                raise PluginError(f"Runtime asset changed after staging: {name}")
    if (root / "assets/runtime-manifest.json").is_file():
        for path in sorted(root.rglob("*")):
            if path.is_file():
                files.append(RuntimeAsset(path.relative_to(root).as_posix(), str(root), config.site_dir, False))
    return files
