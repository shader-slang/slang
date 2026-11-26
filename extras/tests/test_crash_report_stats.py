#!/usr/bin/env python3
"""Unit tests for crash-report-stats.py (offline; never calls gh)."""

import importlib.util
import os
import sys
import unittest

_EXTRAS = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
_spec = importlib.util.spec_from_file_location(
    "crs", os.path.join(_EXTRAS, "crash-report-stats.py")
)
crs = importlib.util.module_from_spec(_spec)
# A dataclass in the script looks its module up by name while the class is created, so the
# module has to be registered before it runs.
sys.modules[_spec.name] = crs
_spec.loader.exec_module(crs)

REPORT = (
    "==== Slang crash report (format 1) ====\n"
    "[report]\n"
    "kind:        internal-error\n"
    "signal-type: unexpected\n"
    "location:    slang-example.cpp:42\n"
    "slang:       {slang}\n"
    "host:        {host}\n"
    "front-end:   slangc\n"
    "signature:   {sig}\n"
    "\n"
    "[stack]\n"
    "#0  libslang-compiler.so+0x102dc\n"
    "[details]\n"
    "{details}\n"
)


def make_report(slang="2026.22.1", host="linux-aarch64", sig="ice-d1a8d0d9", details="x"):
    return REPORT.format(slang=slang, host=host, sig=sig, details=details)


def release(tag, published, assets=()):
    return {
        "tag_name": tag,
        "published_at": published,
        "draft": False,
        "prerelease": False,
        "assets": [{"name": n, "download_count": c} for n, c in assets],
    }


def issue(number, title="t", body="", created="2026-09-10T00:00:00Z", state="open"):
    return {
        "number": number,
        "title": title,
        "state": state,
        "created_at": created,
        "closed_at": None if state == "open" else "2026-09-12T00:00:00Z",
        "body": body,
        "labels": [{"name": "bug"}],
        "user": {"login": "someone"},
        "html_url": "https://github.com/x/y/issues/%d" % number,
    }


class ParseReportsTest(unittest.TestCase):
    def test_code_fence_and_crlf(self):
        body = "Crashed:\r\n```\r\n" + make_report().replace("\n", "\r\n") + "```\r\n"
        reports = crs.parse_reports(body)
        self.assertEqual(len(reports), 1)
        self.assertEqual(reports[0]["signature"], "ice-d1a8d0d9")
        self.assertEqual(reports[0]["slang"], "2026.22.1")
        self.assertEqual(reports[0]["location"], "slang-example.cpp:42")
        self.assertNotIn("\r", reports[0]["host"])

    def test_two_reports(self):
        body = make_report(sig="ice-1") + "\nand again\n" + make_report(sig="ice-2")
        sigs = [r["signature"] for r in crs.parse_reports(body)]
        self.assertEqual(sigs, ["ice-1", "ice-2"])

    def test_forged_line_in_details_is_ignored(self):
        body = make_report(details="signature: forged\nkind: forged\n[report]\nsignature: forged2")
        reports = crs.parse_reports(body)
        self.assertEqual(len(reports), 1)
        self.assertEqual(reports[0]["signature"], "ice-d1a8d0d9")
        self.assertEqual(reports[0]["kind"], "internal-error")

    def test_no_header_means_no_report(self):
        self.assertEqual(crs.parse_reports("[report]\nsignature: x\n"), [])
        self.assertEqual(crs.parse_reports(None), [])


class ClassifyTest(unittest.TestCase):
    def test_priority(self):
        self.assertEqual(crs.classify_title("Crash: assertion failed in internal error"), "crash")
        self.assertEqual(crs.classify_title("Internal error: assertion failed"), "internal-error")
        self.assertEqual(crs.classify_title("Assertion failure in lowering"), "assert")
        self.assertEqual(crs.classify_title("Segmentation fault in slangc"), "crash")

    def test_ice_whole_word(self):
        self.assertEqual(crs.classify_title("ICE when compiling"), "internal-error")
        self.assertIsNone(crs.classify_title("Device service price"))
        self.assertIsNone(crs.classify_title("Add a feature"))


class PackageTest(unittest.TestCase):
    def test_packages(self):
        f = crs.package_of_asset
        self.assertEqual(f("slang-2026.22.1-linux-x86_64.tar.gz"), "linux-x86_64")
        self.assertEqual(f("slang-2026.22.1-linux-aarch64-glibc-2.27.zip"), "linux-aarch64-glibc-2.27")
        self.assertEqual(f("slang-2026.22.1-windows-x86_64.zip"), "windows-x86_64")
        self.assertEqual(f("slang-2026.22.1-macos-aarch64.zip"), "macos-aarch64")
        self.assertEqual(f("slang-wasm.tar.gz"), "wasm")
        self.assertIsNone(f("source.zip"))

    def test_binary_asset_filter(self):
        self.assertFalse(crs.is_binary_asset("slang-1-linux-x86_64-debug-info.zip"))
        self.assertFalse(crs.is_binary_asset("slang-macos-dist-x86_64.zip"))
        self.assertTrue(crs.is_binary_asset("slang-1-macos-x86_64.zip"))


class AttributionTest(unittest.TestCase):
    def setUp(self):
        self.releases = [
            crs.normalize_release(release("v2026.20", "2026-08-01T00:00:00Z", [("slang-linux-x86_64.zip", 100)])),
            crs.normalize_release(release("v2026.22.1", "2026-09-01T00:00:00Z", [("slang-linux-x86_64.zip", 200)])),
        ]

    def attribute(self, **kwargs):
        return crs.attribute_issue(crs.analyze_issue(crs.normalize_issue(issue(1, **kwargs))), self.releases)

    def test_report_uses_slang_field_not_date(self):
        # Created after v2026.22.1, but the report says v2026.20.
        self.assertEqual(self.attribute(body=make_report(slang="2026.20"), created="2026-09-15T00:00:00Z"),
                         (["v2026.20"], "report"))
        # A v prefix in the field is tolerated.
        self.assertEqual(self.attribute(body=make_report(slang="v2026.20"))[0], ["v2026.20"])

    def test_keyword_uses_creation_date(self):
        self.assertEqual(self.attribute(title="crash", created="2026-08-15T00:00:00Z"), (["v2026.20"], "date"))
        self.assertEqual(self.attribute(title="crash", created="2026-09-01T00:00:00Z"), (["v2026.22.1"], "date"))
        self.assertEqual(self.attribute(title="crash", created="2026-07-01T00:00:00Z"), ([], "date"))

    def test_build_stats_counts_by_mode(self):
        issues = [
            crs.normalize_issue(issue(1, body=make_report(slang="2026.20"), created="2026-09-15T00:00:00Z")),
            crs.normalize_issue(issue(2, title="segfault", created="2026-09-15T00:00:00Z")),
            crs.normalize_issue(issue(3, title="docs question")),
        ]
        stats = crs.build_stats(self.releases, issues, 12)
        rows = {r["tag"]: r for r in stats["releases"]}
        self.assertEqual((rows["v2026.20"]["report_issues"], rows["v2026.20"]["keyword_issues"]), (1, 0))
        self.assertEqual((rows["v2026.22.1"]["report_issues"], rows["v2026.22.1"]["keyword_issues"]), (0, 1))
        self.assertEqual(stats["totals"]["crash_issues"], 2)
        self.assertEqual(stats["totals"]["with_report"], 1)
        self.assertEqual(stats["signatures"][0]["first_seen"], "v2026.20")
        self.assertEqual(stats["hosts"], [{"host": "linux-aarch64", "issues": 1}])


class ZeroDownloadTest(unittest.TestCase):
    def test_no_division_by_zero(self):
        self.assertIsNone(crs.issues_per_10k(3, 0))
        self.assertAlmostEqual(crs.issues_per_10k(3, 30000), 1.0)
        releases = [crs.normalize_release(release("v1", "2026-08-01T00:00:00Z"))]
        issues = [crs.normalize_issue(issue(1, title="crash", created="2026-08-02T00:00:00Z"))]
        stats = crs.build_stats(releases, issues, 12)
        self.assertEqual(stats["releases"][0]["downloads"], 0)
        self.assertIsNone(stats["releases"][0]["per_10k"])
        self.assertIn("n/a", crs.format_text(stats, "x/y", "2026-08-01"))


if __name__ == "__main__":
    unittest.main()
