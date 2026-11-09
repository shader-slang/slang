"""Regression tests for incomplete or stale Pages runtime artifacts."""

import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
from types import SimpleNamespace
import unittest
from unittest.mock import patch

from mkdocs.exceptions import PluginError


REPO = Path(__file__).resolve().parents[2]


class DocsRuntimeTests(unittest.TestCase):
    def test_snapshot_does_not_record_private_git_metadata(self):
        self.write(".git/HEAD", "ref: refs/heads/private-development\n")
        result = self.stage("--snapshot")
        self.assertEqual(result.returncode, 0, result.stderr)
        manifest = json.loads((self.repo / "javascript/docs-runtime/assets/runtime-manifest.json").read_text())
        self.assertNotIn("source_revision", manifest)
        self.assertNotIn("source_dirty", manifest)

    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.repo = Path(self.temporary.name).resolve()
        self.write("extras/prepare-docs-runtime.py", (REPO / "extras/prepare-docs-runtime.py").read_text())
        self.write("LICENSE")
        self.write("LICENSES/Apache-2.0.txt")
        for name in ("src/index.js", "src/browser-host.js", "src/http-file-system.js"):
            self.write("javascript/slangvm/" + name)
            self.write("build.em/Release/slangvm/" + name, "settings.fileSystem")
        for name in ("wasm/slangvm-wasm.js", "wasm/slangvm-wasm.wasm", "LICENSE", "package.json",
                     "LICENSES/Apache-2.0.txt", "third-party-notices/emdawnwebgpu-LICENSE",
                     "slang/slang/runtime.slang"):
            self.write("build.em/Release/slangvm/" + name)
        self.write("build.em/Release/slangvm/slang/standard-modules.json", json.dumps([
            "slang/runtime.slang-module", "slang/tensor.slang-module"
        ]))
        for name in ("runtime.slang-module", "tensor.slang-module", "tensor.slang"):
            self.write("build.em/Release/slangvm/slang/slang/" + name)
        for name in ("editor.js", "editor.css", "editor.worker.js", "language.worker.js",
                     "MONACO-LICENSE.txt", "MONACO-NOTICES.txt", "SLANG-LICENSE.txt", "font.ttf"):
            self.write("build/docs-editor/" + name)
        for name in ("slang-wasm.js", "slang-wasm.wasm"):
            self.write("build.em/Release/bin/" + name)
        self.write("docs/heterogeneity/scripts/snippet-files.json", json.dumps({
            "example": {"/slang/runtime.slang": "../assets/slangvm/slang/slang/runtime.slang"}
        }))
        spec = importlib.util.spec_from_file_location("docs_hook", REPO / "extras/heterogeneity_docs.py")
        self.hook = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(self.hook)
        self.config = SimpleNamespace(config_file_path=str(self.repo / "mkdocs.yml"),
                                      site_dir=str(self.repo / "site"))
        self.staged = self.repo / "build/heterogeneity-docs-runtime"
        self.environment = patch.dict(os.environ, {"HSLANG_DOCS_REQUIRE_RUNTIME": "1", "HSLANG_DOCS_RUNTIME_DIR": ""})
        self.environment.start()
        self.addCleanup(self.environment.stop)

    def write(self, name, text="fixture"):
        path = self.repo / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def stage(self, *arguments):
        return subprocess.run([sys.executable, str(self.repo / "extras/prepare-docs-runtime.py"), *arguments],
                              capture_output=True, text=True)

    def test_deployment_requires_runtime_but_local_docs_do_not(self):
        with self.assertRaisesRegex(PluginError, "Deployment requires browser assets"):
            self.hook.on_files([], self.config)
        with patch.dict(os.environ, {"HSLANG_DOCS_REQUIRE_RUNTIME": "0"}):
            self.assertEqual(self.hook.on_files([], self.config), [])

    def test_complete_package_includes_workers_fonts_imports_and_notices(self):
        result = self.stage("--require-editor")
        self.assertEqual(result.returncode, 0, result.stderr)
        files = {file.src_uri for file in self.hook.on_files([], self.config)}
        self.assertTrue({"assets/editor/font.ttf", "assets/editor/language.worker.js",
                         "assets/slangd/slang-wasm.wasm", "assets/slangvm/slang/slang/runtime.slang",
                         "assets/slangvm/slang/slang/runtime.slang-module",
                         "assets/slangvm/slang/slang/tensor.slang-module",
                         "assets/slangvm/third-party-notices/emdawnwebgpu-LICENSE"}.issubset(files))

    def test_missing_editor_fails_without_replacing_previous_staging(self):
        result = self.stage("--require-editor")
        self.assertEqual(result.returncode, 0, result.stderr)
        manifest = (self.staged / "assets/runtime-manifest.json").read_bytes()
        (self.repo / "build/docs-editor/editor.js").unlink()
        result = self.stage("--require-editor")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("editor.js", result.stderr)
        self.assertEqual((self.staged / "assets/runtime-manifest.json").read_bytes(), manifest)

    def test_missing_import_or_wrapper_fails(self):
        for name in ("slang/slang/runtime.slang", "src/browser-host.js",
                     "slang/slang/runtime.slang-module", "slang/slang/tensor.slang-module",
                     "slang/slang/tensor.slang"):
            with self.subTest(name=name):
                path = self.repo / "build.em/Release/slangvm" / name
                contents = path.read_bytes()
                path.unlink()
                result = self.stage("--require-editor")
                self.assertNotEqual(result.returncode, 0)
                self.assertIn(path.name, result.stderr)
                path.write_bytes(contents)

    def test_editor_remains_optional_for_local_staging(self):
        shutil.rmtree(self.repo / "build/docs-editor")
        result = self.stage()
        self.assertEqual(result.returncode, 0, result.stderr)
        with self.assertRaisesRegex(PluginError, "complete runtime manifest"):
            self.hook.on_files([], self.config)

    def test_source_only_package_cannot_replace_snapshot(self):
        result = self.stage("--snapshot")
        self.assertEqual(result.returncode, 0, result.stderr)
        manifest = self.repo / "javascript/docs-runtime/assets/runtime-manifest.json"
        previous = manifest.read_bytes()
        (self.repo / "build.em/Release/slangvm/slang/standard-modules.json").unlink()
        result = self.stage("--snapshot")
        self.assertNotEqual(result.returncode, 0)
        self.assertIn("precompiled standard-module inventory", result.stderr)
        self.assertEqual(manifest.read_bytes(), previous)

    def test_deployment_requires_binaries_even_without_a_documented_import(self):
        result = self.stage("--snapshot")
        self.assertEqual(result.returncode, 0, result.stderr)
        root = self.repo / "javascript/docs-runtime"
        name = "assets/slangvm/slang/slang/tensor.slang-module"
        (root / name).unlink()
        manifest = root / "assets/runtime-manifest.json"
        contents = json.loads(manifest.read_text())
        del contents["files"][name]
        manifest.write_text(json.dumps(contents))
        with self.assertRaisesRegex(PluginError, "snapshot is incomplete"):
            self.hook.on_files([], self.config)

    def test_deployment_rejects_changed_or_missing_assets(self):
        result = self.stage("--require-editor")
        self.assertEqual(result.returncode, 0, result.stderr)
        asset = self.staged / "assets/slangvm/wasm/slangvm-wasm.wasm"
        asset.write_text("corrupted")
        with self.assertRaisesRegex(PluginError, "changed after staging"):
            self.hook.on_files([], self.config)
        asset.unlink()
        with self.assertRaisesRegex(PluginError, "Missing runtime asset"):
            self.hook.on_files([], self.config)

    def test_snapshot_is_used_without_a_local_build_and_can_be_selected_explicitly(self):
        result = self.stage("--snapshot")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertFalse(self.staged.exists())
        self.assertTrue(self.hook.on_files([], self.config))
        # CI must use the committed snapshot even if an unrelated local staging directory exists.
        self.write("build/heterogeneity-docs-runtime/assets/runtime-manifest.json", "{}")
        with patch.dict(os.environ, {"HSLANG_DOCS_RUNTIME_DIR": "javascript/docs-runtime"}):
            self.assertTrue(self.hook.on_files([], self.config))

    def test_new_doc_import_must_be_in_the_snapshot(self):
        result = self.stage("--snapshot")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.write("docs/heterogeneity/scripts/snippet-files.json", json.dumps({
            "new-example": {"/slang/new.slang": "../assets/slangvm/slang/slang/new.slang"}
        }))
        with self.assertRaisesRegex(PluginError, "snapshot is incomplete"):
            self.hook.on_files([], self.config)


if __name__ == "__main__":
    unittest.main()
