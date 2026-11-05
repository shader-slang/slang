"""Verify the handoff from successful release measurements to BenchView export."""
import contextlib
import io
import json
from pathlib import Path
import sys
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import new_release_check


class CompletedReleaseTests(unittest.TestCase):
    """Only completed release batches may be handed to the nightly exporter."""

    def test_release_branch_requires_verified_ancestry(self):
        """A maintenance tag must not inherit an unrelated default release target."""
        with tempfile.TemporaryDirectory() as tmp:
            def git(*args):
                """Run isolated Git commands without changing the user's Git configuration."""
                return subprocess.check_output(
                    ["git", "-C", tmp, "-c", "user.name=Test",
                     "-c", "user.email=test@example.com", *args],
                    text=True, stderr=subprocess.DEVNULL).strip()

            git("init")
            git("commit", "--allow-empty", "-m", "base")
            git("update-ref", "refs/remotes/origin/master", "HEAD")
            git("checkout", "-b", "release/2026.20")
            git("commit", "--allow-empty", "-m", "maintenance")
            git("tag", "v2026.20")
            git("update-ref", "refs/remotes/origin/release/2026.20", "HEAD")
            revision = git("rev-parse", "HEAD")
            for target, expected in (("master", None), ("release/2026.20", "release/2026.20"),
                                     (revision, None), ("missing", None), (None, None)):
                with self.subTest(target=target), patch.object(
                        new_release_check.fetch_releases, "gh_get",
                        return_value={"published_at": "2026-10-17T04:00:00Z",
                                      "target_commitish": target}):
                    info = new_release_check.release_export_info(tmp, "v2026.20")
                    self.assertEqual(info["commit"], revision)
                    self.assertEqual(info.get("branch"), expected)

    def test_metadata_failure_preserves_tracked_history(self):
        """An unavailable release API cannot prevent measurement or undo the tracked results."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            completed = root / "completed.json"
            index = root / "index.json"
            index.write_text("[]")
            calls = []

            def run(cmd, **kwargs):
                """Simulate the successful tracking producer, including its index update."""
                calls.append(cmd)
                if "merge-index" in cmd:
                    index.write_text('[{"tag":"v2026.20"}]')
                return subprocess.CompletedProcess(cmd, 0)

            def unavailable(url):
                """Metadata is requested only after all four tracking steps succeeded."""
                self.assertEqual(len(calls), 4)
                raise OSError("release API unavailable")

            argv = ["new_release_check.py", "--results", tmp, "--repo", tmp,
                    "--completed", str(completed)]
            with patch.object(sys, "argv", argv), \
                 patch.object(new_release_check.fetch_releases, "window_tags",
                              return_value=[("2026-10-17", "v2026.20")]), \
                 patch.object(new_release_check.subprocess, "run", side_effect=run), \
                 patch.object(new_release_check.subprocess, "check_output", return_value="a" * 40), \
                 patch.object(new_release_check.fetch_releases, "gh_get", side_effect=unavailable), \
                 contextlib.redirect_stdout(io.StringIO()), \
                 contextlib.redirect_stderr(io.StringIO()) as warnings:
                new_release_check.main()
            self.assertEqual(json.loads(completed.read_text()), [])
            self.assertEqual(json.loads(index.read_text()), [{"tag": "v2026.20"}])
            self.assertIn("skipped BenchView metadata", warnings.getvalue())

    def test_completed_manifest(self):
        """No-new and failed sweeps clear stale manifests; successful sweeps list their tags."""
        for new, returncode in ((False, 0), (True, 0), (True, 1)):
            with self.subTest(new=new, returncode=returncode), tempfile.TemporaryDirectory() as tmp:
                root = Path(tmp)
                index = root / "index.json"
                index.write_text(json.dumps([] if new else [{"tag": "v2026.20"}]))
                completed = root / "completed.json"
                completed.write_text('[{"tag":"stale"}]')
                info = {"tag": "v2026.20", "commit": "b" * 40,
                        "commit_time": "2026-10-17T00:00:00Z",
                        "released_at": "2026-10-17T04:00:00Z"}
                argv = ["new_release_check.py", "--results", tmp, "--repo", tmp,
                        "--completed", str(completed)]
                with patch.object(sys, "argv", argv), \
                     patch.object(new_release_check.fetch_releases, "window_tags",
                                  return_value=[("2026-10-17", "v2026.20")]), \
                     patch.object(new_release_check, "release_export_info", return_value=info), \
                     patch.object(new_release_check.subprocess, "run") as run, \
                     contextlib.redirect_stdout(io.StringIO()):
                    run.return_value.returncode = returncode
                    if new and returncode:
                        with self.assertRaises(SystemExit):
                            new_release_check.main()
                    else:
                        new_release_check.main()
                result = json.loads(completed.read_text())
                if new and not returncode:
                    self.assertEqual(result[0]["tag"], "v2026.20")
                    self.assertEqual(result[0]["released_at"], info["released_at"])
                    self.assertIn("observed_at", result[0])
                    self.assertEqual(run.call_count, 4)
                else:
                    self.assertEqual(result, [])


if __name__ == "__main__":
    unittest.main()
