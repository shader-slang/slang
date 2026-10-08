"""Test the nightly alert triage: evidence for flagged counters, cross-workload
shifts judged against a night-to-night null, provenance, and commit-range ranking.

Fixtures are synthetic; nothing here touches the network or a git checkout.
"""
import json
import os
import tempfile
import unittest

import bench
import triage


def row(workload, size=64, sampling="interleaved", **timers):
    """A valid five-sample record; each keyword is a timer's five samples."""
    return {"workload": workload, "size": size, "ok": True, "samples": 5, "warmup": 1,
            "bucket": "test", "sampling_strategy": sampling, "timer_schema": "detailed",
            "timers": {name: bench.stats(vals) for name, vals in timers.items()}}


def around(center, spread=0.01):
    """Five samples centred on `center`, spread by +-`spread` (relative)."""
    return [center * (1 + spread * k) for k in (-1, -0.5, 0, 0.5, 1)]


def night(scale=1.0, workloads=8, timers=None, spread=0.01):
    """{workload: row} for one night; `scale` maps timer name to a multiplier."""
    timers = timers or {"readSerializedModuleAST": 20.0, "compileInner": 200.0}
    out = {}
    for i in range(workloads):
        wl = f"wl{i}"
        out[wl] = row(wl, **{t: around(v * (scale.get(t, 1.0) if isinstance(scale, dict) else scale),
                                       spread)
                             for t, v in timers.items()})
    return out


class ShiftTableTests(unittest.TestCase):
    def baseline(self, noisy_night=1.0):
        nights = {f"2026-01-0{i}-aaaa{i}": night() for i in range(1, 7)}
        if noisy_night != 1.0:
            nights["2026-01-03-aaaa3"] = night(noisy_night)
        return nights

    def shifted(self, table, timer):
        return next(t for t in table if t["timer"] == timer)

    def test_a_fixed_cost_in_one_timer_is_a_shift_and_its_neighbour_is_not(self):
        table = triage.shift_table(night({"readSerializedModuleAST": 1.08}), self.baseline())
        self.assertTrue(self.shifted(table, "readSerializedModuleAST")["significant"])
        self.assertFalse(self.shifted(table, "compileInner")["significant"])
        self.assertAlmostEqual(self.shifted(table, "readSerializedModuleAST")["median_ratio"], 1.08, 2)

    def test_movement_the_baseline_already_shows_is_not_a_shift(self):
        # One baseline night is uniformly 4% slow, so a 4.2% rise tonight is
        # inside ordinary night-to-night movement and must not be reported.
        table = triage.shift_table(night(1.042), self.baseline(noisy_night=1.04))
        self.assertFalse(self.shifted(table, "compileInner")["significant"])
        # Eight percent clears that same null.
        table = triage.shift_table(night(1.08), self.baseline(noisy_night=1.04))
        self.assertTrue(self.shifted(table, "compileInner")["significant"])

    def test_a_shift_confined_to_a_few_workloads_is_not_global(self):
        tonight = night()
        for wl in ("wl0", "wl1"):
            tonight[wl] = row(wl, readSerializedModuleAST=around(30.0), compileInner=around(200.0))
        table = triage.shift_table(tonight, self.baseline())
        self.assertFalse(self.shifted(table, "readSerializedModuleAST")["significant"])

    def test_tiny_timers_are_ignored(self):
        small = {"tinyTimer": 0.5}
        base = {f"2026-01-0{i}-aaaa{i}": night(timers=small) for i in range(1, 7)}
        self.assertEqual(triage.shift_table(night(1.5, timers=small), base), [])

    def test_a_baseline_night_with_other_provenance_is_not_compared(self):
        base = self.baseline()
        other = night()
        for wl in other:
            other[wl]["size"] = 128
        base["2026-01-03-aaaa3"] = other
        hist = []
        tonight = night({"compileInner": 1.5})
        for wl, cur in tonight.items():
            hist += [b[wl] for b in base.values() if triage.comparable(b[wl], cur)]
        self.assertEqual(len(hist), 5 * len(tonight))  # six nights, one excluded


class CounterHintTests(unittest.TestCase):
    def evidence(self, history, original, rerun=None, key_flaky=None, counter="c"):
        base_rows = [row("w", **{counter: h}) for h in history]
        return triage.counter_evidence("w", counter, row("w", **{counter: original}),
                                       row("w", **{counter: rerun}) if rerun else None,
                                       base_rows, key_flaky or {})

    def stable(self, n=6):
        return [around(20.0, 0.01)] * n

    def test_a_slower_median_with_unchanged_best_samples_is_tail_only(self):
        ev = self.evidence(self.stable(), [20.0, 20.1, 24.0, 25.0, 26.0])
        self.assertEqual(ev["hint"], "tail-only")

    def test_every_sample_slower_is_min_shifted(self):
        ev = self.evidence(self.stable(), around(22.0), around(22.2))
        self.assertEqual(ev["hint"], "min-shifted")

    def test_history_that_already_contains_slow_samples_is_bimodal(self):
        history = [[20.0, 20.1, 20.2, 31.0, 31.5]] * 3 + [around(20.0)] * 3
        ev = self.evidence(history, [31.0, 31.2, 31.5, 31.7, 32.0], [31.7, 31.8, 32.0, 32.5, 33.0])
        self.assertEqual(ev["hint"], "bimodal-history")
        self.assertIn("3 of 6", ev["hint_detail"])

    def test_the_registry_wins_and_carries_its_reason(self):
        ev = self.evidence(self.stable(), around(22.0), key_flaky={"w|c": "known bimodal"})
        self.assertEqual((ev["hint"], ev["hint_detail"]), ("known-flaky", "known bimodal"))

    def test_too_little_history_is_stated_not_guessed(self):
        ev = self.evidence(self.stable(2), around(22.0))
        self.assertEqual(ev["hint"], "insufficient-history")


class BucketSummaryTests(unittest.TestCase):
    def rows(self, inner_scale, bucket="real_world"):
        out = {}
        for wl in ("a", "b"):
            r = row(wl, compileInner=around(100.0 * inner_scale), frontEndExecute=around(30.0 * inner_scale),
                    SemanticChecking=around(25.0 * inner_scale))
            r["bucket"] = bucket
            r["wall_ms"] = bench.stats(around(120.0 * inner_scale))
            out[wl] = r
        return out

    def test_whole_compile_ratio_and_phase_share_for_real_workloads(self):
        base = {f"2026-01-0{i}-x{i}": self.rows(1.0) for i in range(1, 7)}
        buckets, detail = triage.bucket_summary(self.rows(1.05), base)
        self.assertEqual([b["bucket"] for b in buckets], ["real_world"])
        self.assertAlmostEqual(buckets[0]["compileInner_median_ratio"], 1.05, 3)
        self.assertAlmostEqual(buckets[0]["wall_median_ratio"], 1.05, 3)
        self.assertAlmostEqual(detail[0]["SemanticChecking_share_of_compile"], 0.25, 3)
        self.assertAlmostEqual(detail[0]["frontEndExecute_share_of_compile"], 0.30, 3)

    def test_other_buckets_are_summarized_but_not_listed_as_real_world(self):
        base = {f"2026-01-0{i}-x{i}": self.rows(1.0, "sema") for i in range(1, 7)}
        buckets, detail = triage.bucket_summary(self.rows(1.0, "sema"), base)
        self.assertEqual([b["bucket"] for b in buckets], ["sema"])
        self.assertEqual(detail, [])


class CommitMappingTests(unittest.TestCase):
    def test_paths_map_to_components(self):
        for path, expected in (
                ("source/slang/hlsl.meta.slang", "builtin-module"),
                ("source/slang/slang-check-overload.cpp", "semantic-checking"),
                ("source/slang/slang-ast-type.cpp", "semantic-checking"),
                ("source/slang/slang-parser.cpp", "parse"),
                ("source/slang/slang-lower-to-ir.cpp", "ast-to-ir"),
                ("source/slang/slang-ir-link.cpp", "ir-and-emit"),
                ("source/slang/slang-serialize-ast.cpp", "serialization"),
                ("docs/user-guide/intro.md", None),
                ("tests/language-feature/x.slang", None),
                (".github/workflows/ci.yml", None)):
            self.assertEqual(triage.component_of(path)[0], expected, path)

    def commit(self, sha, pr, *files):
        return {"sha": sha, "pr": pr, "author": "a", "subject": f"s ({'#'}{pr})",
                "files": [{"path": p, "additions": a, "deletions": d} for p, a, d in files]}

    def test_only_files_that_can_move_the_story_timers_score(self):
        commits = [
            self.commit("c1", 1, ("source/slang/slang-check-decl.cpp", 100, 50),
                        ("tests/a.slang", 500, 0)),
            self.commit("c2", 2, ("source/slang/hlsl.meta.slang", 30, 0)),
            self.commit("c3", 3, ("docs/a.md", 900, 0)),
            self.commit("c4", 4, ("source/slang/slang-ir-link.cpp", 40, 0))]
        ranked = triage.score_commits(commits, {"SemanticChecking"})
        self.assertEqual([(r["pr"], r["relevant_lines"]) for r in ranked], [(1, 150), (2, 30)])
        self.assertEqual([r["pr"] for r in triage.score_commits(commits, {"loadBuiltinModule"})], [2])
        self.assertEqual(triage.score_commits(commits, {"generateOutput"})[0]["pr"], 4)

    def test_pr_number_is_read_from_a_squash_subject(self):
        self.assertEqual(triage.PR_RE.search("Fix a thing (#13232)").group(1), "13232")
        self.assertIsNone(triage.PR_RE.search("Revert (#1) of a thing"))


class RangeTests(unittest.TestCase):
    def points(self, *spec):
        return {label: {"label": label, "date": label[:10], "commit": commit}
                for label, commit in spec}

    def test_the_base_is_the_newest_night_built_from_another_commit(self):
        pts = self.points(("2026-01-01-a", "aaaa"), ("2026-01-02-b", "bbbb"),
                          ("2026-01-03-b", "bbbb"), ("2026-01-04-c", "cccc"))
        base = triage.resolve_range(pts, pts["2026-01-04-c"], list(pts)[:3])
        self.assertEqual(base["commit"], "bbbb")
        # A short label suffix matches a full SHA recorded in meta.
        pts["2026-01-04-c"]["commit"] = "bbbb0000ffff"
        self.assertEqual(triage.resolve_range(pts, pts["2026-01-04-c"], list(pts)[:3])["commit"], "aaaa")

    def test_a_night_built_from_the_same_commit_has_no_range(self):
        pts = self.points(("2026-01-01-a", "aaaa"), ("2026-01-02-a", "aaaa"), ("2026-01-03-a", "aaaa"))
        self.assertIsNone(triage.resolve_range(pts, pts["2026-01-03-a"], list(pts)[:2]))


class BundleTests(unittest.TestCase):
    """The whole path on a small results tree, with a fake commit source."""

    LABELS = [f"2026-01-0{i}-sha{i}" for i in range(1, 8)]

    def write_tree(self, root, candidate=True, confirmation=True, same_commit=False):
        for i, label in enumerate(self.LABELS):
            d = os.path.join(root, "daily", label)
            os.makedirs(d)
            last = i == len(self.LABELS) - 1
            scale = {"readSerializedModuleAST": 1.08} if last else {}
            rows = list(night(scale).values())
            commit = "sha1" if same_commit else f"sha{i + 1}"
            self.dump(d, "results.json", rows)
            self.dump(d, "meta.json", {"date": label[:10], "commit": commit,
                                       "runner": "r", "commit_time": ""})
            if last and confirmation:
                orig = rows[0]["timers"]["readSerializedModuleAST"]
                change = {"workload": "wl0", "counter": "readSerializedModuleAST",
                          "baseline": 20.0, "value": orig["median"]}
                plan = {"label": label, "runner": "r", "notes": [], "judged_count": 1,
                        "thresholds": {"rel": 1.10, "warn_rel": 1.05, "abs": 2.0},
                        "baseline_labels": self.LABELS[:-1],
                        "regressions": [], "warnings": [change] if candidate else []}
                # 22.4 ms clears the alert's 2 ms absolute floor over the 20 ms baseline.
                rerun = [row("wl0", readSerializedModuleAST=around(22.4))]
                self.dump(d, "confirmation.json", {"label": label, "status": "complete",
                                                   "repeated": rerun, "plan": plan})

    @staticmethod
    def dump(directory, name, data):
        with open(os.path.join(directory, name), "w", encoding="utf-8") as fh:
            json.dump(data, fh)

    @staticmethod
    def fake_commits(base, head, repo_dir, gh):
        return ([{"sha": "a" * 40, "author": "x", "date": "d", "subject": "Docs (#1)",
                  "files": [{"path": "docs/a.md", "additions": 9, "deletions": 0}]},
                 {"sha": "b" * 40, "author": "y", "date": "d", "subject": "Add builtins (#2)",
                  "files": [{"path": "source/slang/core.meta.slang", "additions": 80, "deletions": 1}]},
                 {"sha": "c" * 40, "author": "z", "date": "d", "subject": "Tweak ir (#3)",
                  "files": [{"path": "source/slang/slang-ir-link.cpp", "additions": 5, "deletions": 5}]}],
                [])

    def bundle(self, **kw):
        with tempfile.TemporaryDirectory() as root:
            self.write_tree(root, **{k: v for k, v in kw.items() if k in
                                     ("candidate", "confirmation", "same_commit")})
            return triage.build_bundle(root, label=self.LABELS[-1], fetch=self.fake_commits)

    def test_a_confirmed_shift_is_attributed_to_the_builtin_change(self):
        b = self.bundle()
        self.assertTrue(b["has_candidates"])
        self.assertEqual([(f["counter"], f["outcome"]) for f in b["flagged"]],
                         [("readSerializedModuleAST", "warning")])
        self.assertEqual([t["timer"] for t in b["global_shifts"]], ["readSerializedModuleAST"])
        self.assertEqual([c["pr"] for c in b["candidates"]], [2])
        self.assertEqual(b["commit_range"]["base_commit"], "sha6")
        # The base and the candidate are both build points; so is the head.
        self.assertEqual(b["bisect_points"], ["sha6", "b" * 40, "c" * 40])

    def test_a_night_without_candidates_says_so_and_stops(self):
        b = self.bundle(candidate=False)
        self.assertFalse(b["has_candidates"])
        self.assertEqual(b["flagged"], [])

    def test_a_night_without_confirmation_says_so_and_stops(self):
        b = self.bundle(confirmation=False)
        self.assertFalse(b["has_candidates"])
        self.assertIn("no confirmation.json", b["notes"][0])

    def test_a_night_built_from_the_baselines_commit_is_a_measurement_difference(self):
        b = self.bundle(same_commit=True)
        self.assertTrue(b["commit_range"]["same_commit"])
        self.assertEqual(b["commits"], [])
        self.assertTrue(any("measurement variation" in n for n in b["notes"]))

    def test_markdown_renders_each_section(self):
        md = triage.render_markdown(self.bundle())
        for text in ("## Flagged counters", "readSerializedModuleAST", "## Commit range",
                     "#2", "Suggested build points", "weak signal"):
            self.assertIn(text, md)

    def test_pick_point_by_label_commit_and_default(self):
        pts = [{"label": "2026-01-01-a", "date": "2026-01-01", "commit": "aaaa1111"},
               {"label": "2026-01-02-b", "date": "2026-01-02", "commit": "bbbb2222"}]
        self.assertEqual(triage.pick_point(pts)["label"], "2026-01-02-b")
        self.assertEqual(triage.pick_point(pts, commit="aaaa")["label"], "2026-01-01-a")
        self.assertEqual(triage.pick_point(pts, label="2026-01-01-a")["commit"], "aaaa1111")
        with self.assertRaises(triage.TriageError):
            triage.pick_point(pts, label="nope")
        with self.assertRaises(triage.TriageError):
            triage.pick_point(pts, commit="cccc")


if __name__ == "__main__":
    unittest.main()
