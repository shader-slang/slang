#!/usr/bin/env python3
"""Stage the browser runtime, optional Monaco/slangd assets, and mapped snippet imports."""

import argparse
import hashlib
import json
from pathlib import Path
import shutil
import uuid


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slangvm-dir", type=Path, default=repo / "build.em/Release/slangvm")
    parser.add_argument("--editor-dir", type=Path, default=repo / "build/docs-editor")
    parser.add_argument("--slang-wasm-dir", type=Path, default=repo / "build.em/Release/bin")
    parser.add_argument("--require-editor", action="store_true",
                        help="Fail unless the complete editor and language service are available (deployment builds).")
    parser.add_argument("--snapshot", action="store_true",
                        help="Replace the checked-in javascript/docs-runtime snapshot, including the editor.")
    args = parser.parse_args()
    package = args.slangvm_dir.resolve()
    mappings = json.loads((repo / "docs/heterogeneity/scripts/snippet-files.json").read_text(encoding="utf-8"))
    modules = set()
    for files in mappings.values():
        for url in files.values():
            if url.startswith("../assets/slangvm/"):
                name = url.removeprefix("../assets/slangvm/")
                if not (package / name).resolve().is_relative_to(package):
                    parser.error(f"Standard-module import escapes the slangvm package: {url}")
                modules.add(name)
    modules = sorted(modules)
    module_inventory = package / "slang/standard-modules.json"
    if not module_inventory.is_file():
        parser.error("Missing precompiled standard-module inventory; rebuild slangvm-javascript.")
    precompiled_modules = json.loads(module_inventory.read_text(encoding="utf-8"))
    if not precompiled_modules:
        parser.error("The precompiled standard-module inventory is empty; rebuild slangvm-javascript.")
    for name in precompiled_modules:
        path = (package / "slang" / name).resolve()
        if not name.endswith(".slang-module") or not path.is_relative_to(package / "slang"):
            parser.error(f"Invalid precompiled standard-module path: {name}")
    editor = args.editor_dir.resolve()
    slangd = args.slang_wasm_dir.resolve()
    include_editor = args.snapshot or args.require_editor or (editor / "editor.js").is_file()
    required = [package / name for name in (
        "wasm/slangvm-wasm.js", "wasm/slangvm-wasm.wasm", "LICENSE", "package.json"
    )] + [package / name for name in modules]
    required += [package / "slang" / name for name in precompiled_modules]
    required += [(package / "slang" / name).with_suffix(".slang") for name in precompiled_modules]
    # Include every wrapper/helper, even when no snippet explicitly imports it.
    for directory in ("src", "slang"):
        required += [package / path.relative_to(repo / "javascript/slangvm")
                     for path in (repo / "javascript/slangvm" / directory).rglob("*")
                     if path.is_file()]
    required += [package / "LICENSES" / path.name for path in (repo / "LICENSES").iterdir()
                 if path.is_file()]
    if include_editor:
        required += [editor / name for name in (
            "editor.js", "editor.css", "editor.worker.js", "language.worker.js",
            "MONACO-LICENSE.txt", "MONACO-NOTICES.txt", "SLANG-LICENSE.txt")]
        required += [slangd / name for name in ("slang-wasm.js", "slang-wasm.wasm")]
    missing = [str(path) for path in required if not path.is_file() or path.stat().st_size == 0]
    if not any((package / "third-party-notices").glob("*")):
        missing.append(str(package / "third-party-notices"))
    if missing:
        parser.error("Missing browser inputs (build slangvm-javascript, slang-wasm, and the editor):\n" + "\n".join(missing))
    if "settings.fileSystem" not in (package / "src/index.js").read_text(encoding="utf-8"):
        parser.error("The slangvm package is stale: rebuild slangvm-javascript for injectable filesystem support.")

    build = (repo / "build").resolve()
    build.mkdir(exist_ok=True)
    parent = (repo / "javascript").resolve() if args.snapshot else build
    destination = parent / ("docs-runtime" if args.snapshot else "heterogeneity-docs-runtime")
    # Only replace this script's dedicated generated directory, never caller-supplied inputs.
    if destination.resolve().parent != parent:
        parser.error("Refusing to replace a runtime staging directory outside its designated parent.")
    if any(source.is_relative_to(destination) for source in (package, editor, slangd)):
        parser.error("Runtime inputs must be outside the generated staging directory.")
    # Inherit build/ permissions. Python's private temporary directories can retain an
    # owner-only Windows ACL after rename, preventing a preview under another account
    # (for example, outside a build sandbox) from reading the staged assets.
    temporary = build / ("docs-runtime-" + uuid.uuid4().hex)
    temporary.mkdir()
    try:
        staged = temporary / "staged"
        runtime = staged / "assets/slangvm"
        runtime.mkdir(parents=True)
        for name in ("src", "wasm", "slang", "LICENSES", "third-party-notices"):
            source = package / name
            if source.is_dir():
                shutil.copytree(source, runtime / name)
        for name in ("LICENSE", "package.json"):
            shutil.copy2(package / name, runtime / name)
        if include_editor:
            shutil.copytree(editor, staged / "assets/editor")
            (staged / "assets/slangd").mkdir()
            for name in ("slang-wasm.js", "slang-wasm.wasm"):
                shutil.copy2(slangd / name, staged / "assets/slangd" / name)
            shutil.copy2(repo / "LICENSE", staged / "assets/slangd/LICENSE")
        manifest = {
            "wasm_sha256": hashlib.sha256((runtime / "wasm/slangvm-wasm.wasm").read_bytes()).hexdigest(),
            "standard_modules": modules,
            "precompiled_modules": ["assets/slangvm/slang/" + name for name in precompiled_modules],
            "editor": include_editor,
            # Validate the same bytes before MkDocs copies them into the publish artifact.
            "files": {path.relative_to(staged).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
                      for path in sorted(staged.rglob("*")) if path.is_file()},
        }
        (staged / "assets/runtime-manifest.json").write_text(
            json.dumps(manifest, indent=2) + "\n", encoding="utf-8", newline="\n")
        if destination.exists():
            shutil.rmtree(destination)
        staged.rename(destination)
    finally:
        shutil.rmtree(temporary)
    print(f"Staged runtime assets in {destination}")
    if args.snapshot:
        print("Validate with HSLANG_DOCS_RUNTIME_DIR=javascript/docs-runtime, then commit this snapshot with the source changes.")
    else:
        print("Run python -m mkdocs serve (restart an already running preview after staging).")


if __name__ == "__main__":
    main()
