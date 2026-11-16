"""Markdown cell references become validated anchors without changing code examples."""

import importlib.util
from pathlib import Path
from types import SimpleNamespace
import unittest

import markdown
from mkdocs.exceptions import PluginError


REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location("docs_cell_links_hook", REPO / "extras/heterogeneity_docs.py")
hook = importlib.util.module_from_spec(spec)
spec.loader.exec_module(hook)


def render(source):
    html = markdown.markdown(source, extensions=["admonition", "pymdownx.details",
        "pymdownx.highlight", "pymdownx.superfences"], extension_configs={
            "pymdownx.highlight": {"css_class": "codehilite", "guess_lang": False,
                "extend_pygments_lang": [{"name": "slang", "lang": "hlsl"}]}})
    page = SimpleNamespace(file=SimpleNamespace(src_uri="fixture.md"), present_anchor_ids=set())
    return hook.on_page_content(html, page, None), page.present_anchor_ids


class CellLinkTests(unittest.TestCase):
    cell = "\n\n<!-- module: sphere -->\n\n```slang\npublic float radius = 1;\n```\n"

    def test_inline_and_reference_links_have_real_anchors_and_metadata(self):
        html, anchors = render("[Radius](cell:sphere.slang#radius)\n[Module][sphere]\n\n"
                               "[sphere]: cell:sphere.slang\n" + self.cell)
        for target in ("cell-sphere.slang", "cell-sphere.slang--radius"):
            self.assertIn('href="#' + target + '"', html)
            self.assertIn('id="' + target + '"', html)
            self.assertIn(target, anchors)
        self.assertIn('data-cell-target="sphere.slang"', html)
        self.assertIn('data-cell-symbol="radius"', html)
        self.assertIn('data-cell-link=""', html)
        self.assertLess(html.index('id="cell-sphere.slang"'), html.index('<!-- module: sphere -->'))

    def test_nested_deep_dive_and_qualified_symbols(self):
        nested = '\n\n??? example "Deep Dive"\n' + "\n".join("    " + line for line in self.cell.splitlines())
        html, _ = render('[Method](cell:sphere.slang#Shape.distance)' + nested)
        self.assertIn('data-cell-symbol="Shape.distance"', html)
        self.assertIn('<details', html)

    def test_missing_cells_and_malformed_links_fail_the_build(self):
        for url in ("cell:missing.slang#radius", "cell:../sphere.slang#radius",
                    "cell://sphere.slang#radius", "cell:sphere.slang?x=1", "cell:sphere.slang#Shape..distance"):
            with self.subTest(url=url), self.assertRaises(PluginError):
                render(f"[Broken]({url})" + self.cell)

    def test_examples_are_untouched_and_cannot_define_cells(self):
        source = '`[Example](cell:missing.slang#radius)`\n\n````markdown\n' + self.cell + '\n````\n'
        html, anchors = render(source)
        self.assertIn('[Example](cell:missing.slang#radius)', html)
        self.assertFalse(anchors)
        with self.assertRaisesRegex(PluginError, "Missing cell"):
            render('[Broken](cell:sphere.slang)' + source)

    def test_duplicate_cells_and_reserved_anchor_collisions_fail(self):
        with self.assertRaisesRegex(PluginError, "Duplicate cell"):
            render(self.cell + self.cell)
        with self.assertRaisesRegex(PluginError, "Reserved cell anchor"):
            render('<span id="cell-sphere.slang"></span>\n' + self.cell)

    def test_marker_must_precede_a_highlighted_block(self):
        with self.assertRaisesRegex(PluginError, "Missing cell"):
            render('[Broken](cell:sphere.slang)' + self.cell.replace('```slang', 'Intervening paragraph.\n\n```slang'))

    def test_unrelated_html_and_repeated_links_are_preserved(self):
        html, _ = render('[Site](other.md#topic)\n[One](cell:sphere.slang#radius)\n'
                         '[Two](cell:sphere.slang#radius)' + self.cell)
        self.assertIn('href="other.md#topic"', html)
        self.assertEqual(html.count('id="cell-sphere.slang--radius"'), 1)


if __name__ == "__main__":
    unittest.main()
