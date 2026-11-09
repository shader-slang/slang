"""Repository links must work in source Markdown and the smaller published site."""

import importlib.util
from pathlib import Path
import tempfile
from types import SimpleNamespace
import unittest

import markdown
from mkdocs.exceptions import PluginError


REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("docs_links_hook", REPO / "extras/heterogeneity_docs.py")
hook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hook)


class DocsLinkTests(unittest.TestCase):
    def setUp(self):
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.repo = Path(self.temporary.name).resolve()
        self.docs = self.repo / "docs/heterogeneity"
        self.docs.mkdir(parents=True)
        (self.repo / "source").mkdir()
        (self.repo / "source/VM notes.md").write_text("# Build and package\n", encoding="utf-8")
        (self.repo / "README.md").write_text("# Project\n", encoding="utf-8")
        self.config = SimpleNamespace(
            config_file_path=str(self.repo / "mkdocs.yml"), docs_dir=str(self.docs),
            extra={"repository_source_url": "https://gitlab.example/team/slang/-/blob/h-slang/"},
            markdown_extensions=["fenced_code"],
        )

    def render(self, source, page="installation.md"):
        page = SimpleNamespace(file=SimpleNamespace(abs_src_path=str(self.docs / page)))
        source = hook.on_page_markdown(source, page, self.config)
        return markdown.markdown(source, extensions=self.config.markdown_extensions)

    def test_file_links_preserve_query_and_anchor_on_both_hosts(self):
        for base in ("https://gitlab.example/team/slang/-/blob/h-slang/",
                     "https://github.com/team/slang/blob/release-v1"):
            with self.subTest(base=base):
                self.config.extra["repository_source_url"] = base
                html = self.render("[Build](../../source/VM%20notes.md?plain=1#build-and-package)")
                self.assertIn(base.rstrip("/") + "/source/VM%20notes.md?plain=1#build-and-package", html)

    def test_reference_links_and_nested_pages_use_their_source_directory(self):
        (self.docs / "nested").mkdir()
        self.render("[Project](../../README.md)")
        html = self.render("[Project][repo]\n\n[repo]: ../../../README.md", "nested/page.md")
        self.assertIn('href="https://gitlab.example/team/slang/-/blob/h-slang/README.md"', html)
        self.assertEqual(sum(isinstance(ext, hook.RepositoryLinks)
                             for ext in self.config.markdown_extensions), 1)

    def test_site_links_external_links_and_code_examples_are_unchanged(self):
        (self.docs / "nested").mkdir()
        html = self.render("""[Site](../index.md#intro)
[External](https://example.com/docs)
[Anchor](#details)
[Protocol relative](//example.com/docs)
`[Inline example](../../../missing.md)`

```markdown
[Code example](../../../missing.md)
```
""", "nested/page.md")
        for link in ("../index.md#intro", "https://example.com/docs", "#details", "//example.com/docs"):
            self.assertIn(f'href="{link}"', html)
        self.assertIn("[Inline example](../../../missing.md)", html)
        self.assertIn("[Code example](../../../missing.md)", html)

    def test_missing_files_and_paths_outside_repository_fail(self):
        for path in ("../../missing.md", "../../../outside.md"):
            with self.subTest(path=path):
                with self.assertRaisesRegex(PluginError, "Missing repository file"):
                    self.render(f"[Broken]({path})")


if __name__ == "__main__":
    unittest.main()
