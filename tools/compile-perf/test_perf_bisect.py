"""Test perf_bisect's target selection, time budget and attribution procedure."""
import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest.mock import patch

import bench
import perf_bisect
import trend

LIMITS = {"rel": 1.10, "warn_rel": 1.05, "abs": 2.0}


def record(value, workload="minimal", counter="compileInner"):
    """A valid five-sample record for one workload with one measured timer."""
    return {"workload": workload, "size": 0, "ok": True, "samples": 5, "warmup": 1,
            "sampling_strategy": "interleaved", "timer_schema": "detailed",
            "timers": {counter: bench.stats([value] * 5)}}


class TargetSelectionTests(unittest.TestCase):
    """select_targets over a temporary results tree."""

    def setUp(self):
        self.root = Path(tempfile.mkdtemp(prefix="perf_bisect_targets_"))
        self.addCleanup(shutil.rmtree, self.root)

    def night(self, label, commit, value):
        d = self.root / label
        d.mkdir()
        (d / "results.json").write_text(json.dumps([record(value)]))
        (d / "meta.json").write_text(json.dumps({"commit": commit}))
        return d

    def alert(self, first, rerun, baseline_nights, head="headsha000",
              workload="minimal", counter="compileInner"):
        """Write the alerting night with an archived confirmation, then select."""
        d = self.root / "2026-10-10-head"
        d.mkdir()
        (d / "results.json").write_text(json.dumps([record(first, workload, counter)]))
        (d / "meta.json").write_text(json.dumps({"commit": head}))
        tier = trend.classify_metric(first / 100.0, first - 100.0, LIMITS["rel"],
                                     LIMITS["warn_rel"], LIMITS["abs"])
        change = {"workload": workload, "counter": counter,
                  "baseline": 100.0, "value": first}
        plan = {"label": "2026-10-10-head", "runner": "r", "thresholds": LIMITS,
                "baseline_labels": [label for label, _, _ in baseline_nights],
                "notes": [], "judged_count": 1,
                "regressions": [change] if tier == "error" else [],
                "warnings": [change] if tier == "warning" else []}
        (d / "confirmation.json").write_text(json.dumps(
            {"plan": plan, "repeated": [record(rerun, workload, counter)],
             "status": "complete"}))
        for label, commit, value in baseline_nights:
            self.night(label, commit, value)
        return d

    def select(self, night, **kw):
        return perf_bisect.select_targets(night, self.root, **kw)

    def test_newest_flat_night_on_another_commit_is_the_first_base(self):
        d = self.alert(120, 118, [("2026-10-07-a", "aaaa", 100), ("2026-10-08-b", "bbbb", 101),
                                  ("2026-10-09-c", "cccc", 99)])
        t = self.select(d)
        self.assertTrue(t["proceed"])
        self.assertEqual([b["commit"] for b in t["bases"]], ["cccc", "bbbb", "aaaa"])
        self.assertEqual(t["head"], "headsha000")
        self.assertEqual(t["changes"][0]["tier"], "regression")

    def test_a_night_that_already_carries_the_step_is_not_a_base(self):
        # 10-09 ran a different commit but was already slow: choosing it would
        # make the endpoints agree, so the base must come from before the step.
        d = self.alert(120, 118, [("2026-10-07-a", "aaaa", 100), ("2026-10-08-b", "bbbb", 119),
                                  ("2026-10-09-c", "cccc", 121)])
        self.assertEqual([b["commit"] for b in self.select(d)["bases"]], ["aaaa"])

    def test_every_baseline_night_on_head_means_nothing_to_bisect(self):
        d = self.alert(120, 118, [("2026-10-08-x", "headsha000", 100),
                                  ("2026-10-09-y", "headsha", 100)])
        t = self.select(d)
        self.assertFalse(t["proceed"])
        self.assertIn("same commit as head", t["reason"])

    def test_a_short_sha_prefix_of_head_is_the_same_commit(self):
        self.assertTrue(perf_bisect.same_commit("headsha000", "headsha"))
        self.assertTrue(perf_bisect.same_commit("HEADSHA", "headsha000"))
        self.assertFalse(perf_bisect.same_commit("headsha000", "headsha001"))
        self.assertFalse(perf_bisect.same_commit("", "headsha"))

    def test_no_flat_night_means_nothing_to_bisect(self):
        d = self.alert(120, 118, [("2026-10-09-c", "cccc", 119)])
        t = self.select(d)
        self.assertFalse(t["proceed"])
        self.assertIn("flat", t["reason"])

    def test_a_confirmed_warning_needs_the_warning_tier(self):
        d = self.alert(107, 106, [("2026-10-09-c", "cccc", 100)])
        self.assertFalse(self.select(d)["proceed"])
        t = self.select(d, min_tier="warning")
        self.assertTrue(t["proceed"])
        self.assertEqual(t["changes"][0]["tier"], "warning")

    def test_a_cleared_rerun_has_no_targets(self):
        d = self.alert(120, 101, [("2026-10-09-c", "cccc", 100)])
        t = self.select(d, min_tier="warning")
        self.assertFalse(t["proceed"])
        self.assertIn("no confirmed change", t["reason"])

    def test_known_flaky_counters_are_not_targets(self):
        d = self.alert(144, 140, [("2026-10-09-c", "cccc", 100)],
                       workload="api_module_graph", counter="apiGetCode")
        t = self.select(d)
        self.assertEqual(t["changes"], [])
        self.assertFalse(t["proceed"])
        self.assertIn("known-flaky", t["reason"])

    def test_nights_on_one_commit_give_one_base(self):
        d = self.alert(120, 118, [("2026-10-07-a", "aaaa", 100), ("2026-10-08-b", "bbbb", 100),
                                  ("2026-10-09-b", "bbbb", 100)])
        self.assertEqual([b["label"] for b in self.select(d)["bases"]],
                         ["2026-10-09-b", "2026-10-07-a"])


class BudgetTests(unittest.TestCase):
    def at(self, hhmm):
        h, m = map(int, hhmm.split(":"))
        return datetime.datetime(2026, 10, 18, h, m, tzinfo=datetime.timezone.utc)

    def test_budget_ends_before_the_next_nightly(self):
        cases = {"05:32": 360, "22:00": 360, "00:30": 220, "02:30": 100, "03:05": 0,
                 "04:59": 0}
        for start, minutes in cases.items():
            with self.subTest(start=start):
                self.assertEqual(perf_bisect.budget_minutes(self.at(start), (5, 0)), minutes)

    def test_the_cron_is_read_from_the_nightly_workflow(self):
        nightly = (Path(__file__).resolve().parents[2] / ".github" / "workflows"
                   / "nightly-mdl-perf-test.yml")
        if not nightly.exists():
            self.skipTest("nightly workflow is not in this checkout")
        self.assertEqual(perf_bisect.nightly_start(nightly), (5, 0))


class FakeLab:
    """Stand-in for SuiteLab: each commit has one fixed median per counter."""

    def __init__(self, medians, pattern=(1.0,), fail_build=(), fail_measure=()):
        self.medians, self.pattern = medians, list(pattern)
        self.fail_build, self.fail_measure = set(fail_build), set(fail_measure)
        self.built, self.calls = [], 0

    def build(self, sha, timeout=None):
        self.built.append(sha)
        if sha in self.fail_build:
            raise perf_bisect.BuildError(f"{sha}: build failed")
        return sha

    def measure(self, sha):
        if sha in self.fail_measure:
            raise perf_bisect.MeasurementError(f"{sha}: workload failed")
        # Host noise is a multiplier per measurement call, cycled. Batches order
        # their slots A B, then B A, so the pattern decides which slot is hit.
        scale = self.pattern[self.calls % len(self.pattern)]
        self.calls += 1
        return {("minimal", "compileInner"): [self.medians[sha] * scale] * 5}


def targets(bases=("c0",), head="c8", max_builds=12, deadline=None):
    return {"source_run": 1, "label": "L", "head": head,
            "bases": [{"label": f"n-{b}", "commit": b} for b in bases],
            "changes": [{"tier": "regression", "workload": "minimal",
                         "counter": "compileInner", "baseline": 100.0, "value": 120.0}],
            "thresholds": LIMITS, "max_builds": max_builds, "deadline_utc": deadline}


def line(n, step_at, before=100.0, after=120.0):
    """Commits c0..cn where c<step_at> and later carry the step."""
    shas = [f"c{i}" for i in range(n + 1)]
    return shas, {s: (after if i >= step_at else before) for i, s in enumerate(shas)}


class BisectorTests(unittest.TestCase):
    def setUp(self):
        self.out = Path(tempfile.mkdtemp(prefix="perf_bisect_run_"))
        self.addCleanup(shutil.rmtree, self.out)

    def bisect(self, lab, shas, t=None, **kw):
        t = t or targets(head=shas[-1])
        commits = lambda base, head: shas[shas.index(base) + 1:shas.index(head) + 1]
        return perf_bisect.Bisector(t, lab, commits, self.out, aa_rounds=2, ab_rounds=2,
                                    **kw).run()

    def saved(self):
        return json.loads((self.out / "bisect.json").read_text())

    def test_a_single_step_is_named_and_verified_against_its_parent(self):
        shas, medians = line(8, step_at=5)
        state = self.bisect(FakeLab(medians), shas)
        self.assertEqual(state["status"], "culprit")
        self.assertEqual((state["culprit"], state["parent"]), ("c5", "c4"))
        self.assertEqual(self.saved()["culprit"], "c5")
        self.assertLessEqual(state["builds_used"], 2 + 4)

    def test_a_step_on_the_first_commit_after_base_verifies_against_base(self):
        shas, medians = line(4, step_at=1)
        state = self.bisect(FakeLab(medians), shas)
        self.assertEqual((state["status"], state["culprit"], state["parent"]),
                         ("culprit", "c1", "c0"))

    def test_a_noisy_host_stops_before_any_attribution(self):
        shas, medians = line(4, step_at=2)
        # A A2 | A2 A: A is measured 3% slow in both rounds, A2 never.
        state = self.bisect(FakeLab(medians, pattern=(1.03, 1.0, 1.0, 1.03)), shas)
        self.assertEqual(state["status"], "unclear")
        self.assertIn("A/A spread", state["reason"])

    def test_endpoints_that_do_not_step_try_one_more_base(self):
        shas, medians = line(4, step_at=2)
        medians["cx"] = 120.0
        t = targets(bases=("cx", "c0"), head="c4")
        state = self.bisect(FakeLab(medians), shas, t=t)
        self.assertEqual(state["status"], "culprit")
        self.assertEqual(state["base"], "c0")

    def test_a_base_that_does_not_build_falls_back_to_the_next(self):
        shas, medians = line(4, step_at=2)
        medians["cx"] = 100.0
        t = targets(bases=("cx", "c0"), head="c4")
        state = self.bisect(FakeLab(medians, fail_build={"cx"}), shas, t=t)
        self.assertEqual((state["status"], state["base"]), ("culprit", "c0"))

    def test_no_targets_means_nothing_is_built(self):
        lab = FakeLab({})
        t = targets()
        t["changes"] = []
        state = self.bisect(lab, ["c0", "c8"], t=t)
        self.assertEqual(state["status"], "unclear")
        self.assertEqual(lab.built, [])

    def test_no_step_between_endpoints_is_not_reproduced(self):
        shas, medians = line(4, step_at=99)
        state = self.bisect(FakeLab(medians), shas)
        self.assertEqual(state["status"], "not-reproduced")

    def test_a_gradual_change_stops_with_an_interval(self):
        shas = [f"c{i}" for i in range(5)]
        medians = {"c0": 100.0, "c1": 100.0, "c2": 110.0, "c3": 120.0, "c4": 120.0}
        state = self.bisect(FakeLab(medians), shas)
        self.assertEqual(state["status"], "interval")
        self.assertIsNone(state["culprit"])
        self.assertIn("ambiguous", state["reason"])

    def test_a_probe_that_does_not_build_is_skipped_for_its_neighbour(self):
        shas, medians = line(8, step_at=5)
        state = self.bisect(FakeLab(medians, fail_build={"c4"}), shas)
        self.assertIn({"commit": "c4", "verdict": "skipped",
                       "detail": "c4: build failed"}, state["probes"])
        # c4 is the only commit before the step that could be its parent.
        self.assertIn(state["status"], ("culprit", "interval"))
        if state["status"] == "culprit":
            self.assertEqual(state["culprit"], "c5")

    def test_the_build_budget_ends_the_search_with_an_interval(self):
        shas, medians = line(16, step_at=9)
        state = self.bisect(FakeLab(medians), shas, t=targets(head="c16", max_builds=3))
        self.assertEqual(state["status"], "interval")
        self.assertIn("build budget", state["reason"])
        self.assertEqual(state["builds_used"], 3)
        self.assertTrue(self.saved()["complete"])

    def test_the_deadline_ends_the_search_with_an_interval(self):
        shas, medians = line(16, step_at=9)
        now = [1000.0]

        def clock():
            now[0] += 60.0
            return now[0]

        deadline = datetime.datetime.fromtimestamp(1000.0 + 40 * 60,
                                                   datetime.timezone.utc).isoformat()
        state = self.bisect(FakeLab(medians), shas, t=targets(head="c16", deadline=deadline),
                            clock=clock)
        self.assertIn(state["status"], ("interval", "unclear"))
        self.assertIn("time budget", state["reason"])

    def test_a_deadline_inside_the_last_round_names_no_culprit(self):
        shas, medians = line(1, step_at=1)
        now = [0.0]
        lab = FakeLab(medians)
        real = lab.measure

        def measure(sha):
            now[0] += 10.0
            return real(sha)

        lab.measure = measure
        # A/A and endpoints take 80 s; verification would end at 120 s.
        deadline = datetime.datetime.fromtimestamp(115.0, datetime.timezone.utc).isoformat()
        verified = []
        real_verify = perf_bisect.Bisector._verify

        def spy(bisector, *args):
            verified.append(args)
            return real_verify(bisector, *args)

        with patch.object(perf_bisect.Bisector, "_verify", spy):
            state = self.bisect(lab, shas, t=targets(head="c1", deadline=deadline),
                                clock=lambda: now[0])
        self.assertEqual(len(verified), 1)
        self.assertEqual(state["status"], "interval")
        self.assertIsNone(state["culprit"])
        self.assertIn("time budget", state["reason"])
        self.assertLessEqual(now[0], 125.0)

    def test_a_killed_run_still_leaves_a_report(self):
        shas, medians = line(8, step_at=5)

        class Killed(Exception):
            pass

        lab = FakeLab(medians)
        real = lab.measure

        def measure(sha):
            if sha == "c4":
                raise Killed()
            return real(sha)

        lab.measure = measure
        with self.assertRaises(Killed):
            self.bisect(lab, shas)
        saved = self.saved()
        self.assertFalse(saved["complete"])
        self.assertEqual(saved["interval"]["good"], "c0")
        self.assertIn("stopped before completion", (self.out / "bisect.md").read_text())

    def test_the_report_carries_shas_and_numbers_only(self):
        shas = [f"{i:07x}0" for i in range(5)]
        medians = {s: (120.0 if i >= 2 else 100.0) for i, s in enumerate(shas)}
        state = self.bisect(FakeLab(medians), shas, t=targets(bases=(shas[0],), head=shas[-1]))
        text = perf_bisect.render(state)
        self.assertIn(f"Culprit `{shas[2]}`", text)
        self.assertNotIn(state["reason"], text)

    def test_injected_text_never_reaches_the_report(self):
        evil = "@someone [link](https://x.invalid) <img> | ` **bold**"
        state = {"status": evil, "complete": "yes", "reason": evil, "head": evil,
                 "base": "abcdef0", "culprit": evil, "parent": evil, "interval": {
                     "good": evil, "bad": "abcdef1", "commits": evil},
                 "aa_spread_pct": evil, "builds_used": evil, "max_builds": 12,
                 "deadline_utc": evil,
                 "counters": [{"workload": evil, "counter": evil, "followed": evil,
                               "endpoints": {"base": evil, "head": 1.0}}],
                 "probes": [{"commit": evil, "verdict": evil, "detail": evil}]}
        text = perf_bisect.render(state)
        for fragment in ("@", "](", "<img>", "**bold**", "https://"):
            self.assertNotIn(fragment, text)
        self.assertIn("`abcdef0`", text)


class SuiteLabTests(unittest.TestCase):
    def test_paths_are_resolved_before_the_driver_builds(self):
        work = Path(tempfile.mkdtemp(prefix="perf_bisect_lab_"))
        self.addCleanup(shutil.rmtree, work)
        seen = []

        def driver(out_dir):
            seen.append(out_dir)
            return os.path.join(out_dir, "api-driver")

        cwd = os.getcwd()
        os.chdir(work)
        self.addCleanup(os.chdir, cwd)
        with patch.object(bench, "build_api_driver", side_effect=driver):
            lab = perf_bisect.SuiteLab("probe", "snap", "work",
                                       [("api_session_create", "apiCreateSession")])
        self.assertTrue(os.path.isabs(seen[0]))
        self.assertTrue(os.path.isabs(lab.driver))


class CliTests(unittest.TestCase):
    def test_a_setup_failure_still_writes_a_report(self):
        root = Path(tempfile.mkdtemp(prefix="perf_bisect_cli_"))
        self.addCleanup(shutil.rmtree, root)
        t = targets()
        t["changes"][0]["workload"] = "api_session_create"
        t["changes"][0]["counter"] = "apiCreateSession"
        (root / "targets.json").write_text(json.dumps(t))
        with patch.object(bench, "build_api_driver", return_value=None):
            perf_bisect.main(["run", "--targets", str(root / "targets.json"),
                              "--probe", str(root / "probe"), "--snap", str(root / "snap"),
                              "--work", str(root / "work"), "--out", str(root / "out")])
        state = json.loads((root / "out" / "bisect.json").read_text())
        self.assertEqual((state["status"], state["complete"]), ("unclear", True))
        self.assertIn("setup failed", state["reason"])
        self.assertTrue((root / "out" / "bisect.md").exists())


class FirstParentTests(unittest.TestCase):
    """first_parent_commits against a real throwaway repository."""

    def setUp(self):
        if shutil.which("git") is None:
            self.skipTest("git is not available")
        self.repo = Path(tempfile.mkdtemp(prefix="perf_bisect_git_"))
        self.addCleanup(shutil.rmtree, self.repo, True)
        self.git("init", "-q")

    def git(self, *args):
        env = dict(os.environ, GIT_AUTHOR_NAME="t", GIT_AUTHOR_EMAIL="t@t",
                   GIT_COMMITTER_NAME="t", GIT_COMMITTER_EMAIL="t@t")
        return subprocess.run(["git", "-C", str(self.repo), *args], check=True, env=env,
                              stdout=subprocess.PIPE, text=True).stdout.strip()

    def commit(self, msg):
        self.git("commit", "-q", "--allow-empty", "-m", msg)
        return self.git("rev-parse", "HEAD")

    def test_a_merged_branch_counts_as_its_merge_commit(self):
        base = self.commit("base")
        trunk = self.git("rev-parse", "--abbrev-ref", "HEAD")
        self.git("checkout", "-q", "-b", "side")
        self.commit("side work")
        self.git("checkout", "-q", trunk)
        first = self.commit("first")
        self.git("merge", "-q", "--no-ff", "-m", "merge side", "side")
        merge = self.git("rev-parse", "HEAD")
        self.assertEqual(perf_bisect.first_parent_commits(self.repo, base, merge),
                         [first, merge])

    def test_base_must_be_an_ancestor_of_head(self):
        a = self.commit("a")
        b = self.commit("b")
        with self.assertRaisesRegex(ValueError, "not an ancestor"):
            perf_bisect.first_parent_commits(self.repo, b, a)


if __name__ == "__main__":
    unittest.main()
