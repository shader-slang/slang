#!/usr/bin/env python3
"""Attribute a confirmed nightly compile-perf change to one master commit.

The nightly (nightly-mdl-perf-test.yml) confirms a change against a frozen
baseline but does not say which commit caused it. claude-perf-bisect.yml uses
this module to find out:

    targets   on the notification host: read the night's archived
              confirmation and its baseline nights, then choose the counters to
              follow and a presumed-good base night.
    deadline  on the perf host: size the time budget so the bisect ends before
              the next nightly starts, and record it in targets.json.
    run       on the perf host: build and measure commits between base and
              head, rewriting bisect.json and bisect.md after every step.
    report    print bisect.md again from bisect.json.
    triage    in the nightly's analyze job: say from the archived samples
              alone whether a confirmed change looks real or like noise, as one
              line for the existing Slack message.

The measurement rules live here rather than in a prompt so that two runs on the
same data follow the same rules, and so that test_perf_bisect.py can check
them. The defaults follow the slang-perf-analyst policy in
slang-coworkers/nanoclaw#1980: 20 A/A rounds and 12 A/B rounds of five samples
each, a 2% A/A ceiling, and a 3% minimum reproduced effect.

Nothing in the suite imports this module. It only reads the suite, so a defect
here cannot cost the nightly a data point.

Every output carries commit SHAs and numbers only, never commit subjects or
messages. Squash-merge messages contain PR bodies, which are untrusted text,
and bisect.md is published in a public job summary.

targets.json holds:
    source_run, label     the nightly run and the night it registered
    head                  the commit that night built
    bases                 [{label, commit}], newest first: baseline nights on a
                          different commit that are flat on every target counter
    changes               [{tier, workload, counter, baseline, value}]: the
                          confirmed changes, where baseline is the frozen
                          trailing median and value the rerun's median
    thresholds            the nightly's {rel, warn_rel, abs}
    min_tier, max_builds  the dispatch inputs
    proceed, reason       whether there is anything to bisect, and why not
    deadline_utc          added by `deadline`
"""
import argparse
import datetime
import json
import math
import os
from pathlib import Path
import re
import shutil
import statistics
import subprocess
import sys
import tempfile
import time

import bench
import confirm
from lib import analyze, corpus, manifest
import trend

# The A/A and A/B round counts, the 2% A/A ceiling and the 3% minimum effect are
# the slang-perf-analyst policy in nanoclaw#1980 (PERF_POLICY.json).
AA_ROUNDS = 20
AB_ROUNDS = 12
SAMPLES = 5
AA_MAX_SPREAD = 0.02
MIN_EFFECT = 0.03
# A single A/A draw understates the spread that later batches show, so the noise
# level never drops below 1%: a quiet host still gets a 1% flat band.
NOISE_FLOOR = 0.01

# Counters whose nightly samples are bimodal, so a confirmed change on them is
# not by itself evidence of a code change. api_module_graph/apiGetCode ran at
# about 21 ms with isolated samples near 31 ms on 4 of the 7 nights from
# 2026-09-30 to 2026-10-06 (PERF_POLICY.json in nanoclaw#1980).
KNOWN_FLAKY = {("api_module_graph", "apiGetCode")}

CMAKE_FLAGS = [
    # The nightly's flags (nightly-mdl-perf-test.yml, step "Build Slang (tip of tree)"), so probe
    # timings compare with the nightly's own.
    "-DSLANG_SLANG_LLVM_FLAVOR=DISABLE",
    "-DCMAKE_COMPILE_WARNING_AS_ERROR=false",
    "-DSLANG_ENABLE_RELEASE_LTO=ON",
    "-DSLANG_STANDARD_MODULE_DEVELOP_BUILD=OFF",
]
# slangc alone is not enough: a default-optimization SPIR-V compile loads
# slang-glslang to run spirv-opt, and bench.py treats its absence as benign, so
# a probe without it would silently skip time the nightly measured.
BUILD_TARGETS = ["slangc", "slang-glslang"]


class BuildError(Exception):
    """A commit did not build, or its build lacks a binary the suite needs."""


class MeasurementError(Exception):
    """A binary ran, but some target counter has no complete valid samples."""


class OutOfBudget(Exception):
    """The build budget or the time budget does not allow the next step."""


def same_commit(a, b):
    """Return whether two recorded commits name the same revision.

    Daily meta stores a full SHA while older points carry the label's short
    SHA (see analyze.daily_labels), so commits compare as prefixes, never by
    equality.
    """
    a, b = (a or "").lower(), (b or "").lower()
    return bool(a) and bool(b) and (a.startswith(b) or b.startswith(a))


def on_master_check(repo, ref="origin/master"):
    """Return a predicate: is a commit a full SHA that is an ancestor of `ref` in `repo`?

    Nights record full SHAs, so anything shorter, or anything not on master
    (a dispatch pointed at another branch's night, say), is refused rather than
    built on the perf host.
    """
    def check(commit):
        if not (isinstance(commit, str) and re.fullmatch(r"[0-9a-f]{40}", commit)):
            return False
        return subprocess.run(["git", "-C", str(repo), "merge-base", "--is-ancestor", commit, ref],
                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
    return check


def night_values(night_dir):
    """Map (workload, counter) to the median a night's results.json recorded."""
    out = {}
    runs = analyze.read_json(Path(night_dir) / "results.json")
    for r in analyze.canonical_runs(runs):
        if not r.get("ok"):
            continue
        for counter, stat in (r.get("timers") or {}).items():
            median = (stat or {}).get("median")
            if isinstance(median, (int, float)) and math.isfinite(median):
                out[(r["workload"], counter)] = median
    return out


def is_flat_night(values, change, limits):
    """Return whether a night's median sits inside the nightly's thresholds.

    The comparison uses the same frozen baseline median and the same
    classify_metric bands that raised the alert, so "flat" means "this night
    would not have alerted on this counter".
    """
    value = values.get((change["workload"], change["counter"]))
    if value is None:
        return False
    base = change["baseline"]
    floor = trend.abs_floor_for(change["counter"], limits["abs"])
    return trend.classify_metric(value / base, value - base, limits["rel"],
                                 limits["warn_rel"], floor) is None


def select_targets(night_dir, nights_root, min_tier="regression", max_builds=12,
                   source_run=None, on_master=None):
    """Choose what to bisect for one confirmed night, as a targets.json dict.

    The archive must already have passed `confirm.py report`, which checks that
    it belongs to its workflow attempt and to these original results. This
    function only derives the confirmed changes from it again.

    The base is the newest baseline night that ran a different commit from head
    and is flat on every target counter. Consider this timeline: a step lands
    on 10-07, the 10-07 and 10-08 nights alert, 10-09 confirms nothing, and
    10-10 alerts again. For the 10-10 alert, the newest night on a different
    commit is 10-09, which already carries the step. Picking it would make the
    endpoints agree and end the bisect as not-reproduced. Requiring a flat
    night skips 10-09 and the other slow nights, and lands before the step.
    Every base is still only presumed good: `run` measures the endpoints before
    it bisects.

    When `on_master` is given, head and every base must satisfy it (see
    on_master_check), so the perf host only ever builds merged master code.
    """
    night_dir = Path(night_dir)
    archive = analyze.read_json(night_dir / "confirmation.json")
    plan = archive["plan"]
    errors, warnings, _ = confirm.confirmed_changes(
        plan, analyze.read_json(night_dir / "results.json"), archive["repeated"])
    tiers = [("regression", errors)]
    if min_tier == "warning":
        tiers.append(("warning", warnings))
    confirmed = [(tier, c) for tier, found in tiers for c in found]
    changes = [{"tier": tier, "workload": c.workload, "counter": c.counter,
                "baseline": c.baseline, "value": c.value}
               for tier, c in confirmed if (c.workload, c.counter) not in KNOWN_FLAKY]
    head = analyze.read_json(night_dir / "meta.json")["commit"]
    limits = plan["thresholds"]

    bases, other_commit_seen = [], False
    # baseline_labels is the trailing window in series order, oldest first
    # (trend.save_candidates), so reversing it walks back from the newest night.
    for label in reversed(plan["baseline_labels"]):
        night = Path(nights_root) / label
        if not (night / "meta.json").exists() or not (night / "results.json").exists():
            continue
        commit = analyze.read_json(night / "meta.json").get("commit") or label.split("-")[-1]
        if same_commit(commit, head):
            continue
        other_commit_seen = True
        # Several nights can build one commit (weekends); one base per commit.
        if any(same_commit(commit, b["commit"]) for b in bases):
            continue
        if on_master is not None and not on_master(commit):
            continue
        values = night_values(night)
        if changes and all(is_flat_night(values, ch, limits) for ch in changes):
            bases.append({"label": label, "commit": commit})

    if on_master is not None and not on_master(head):
        reason = "head is not a master commit"
    elif not changes:
        reason = ("only known-flaky counters changed" if confirmed
                  else "no confirmed change at the requested tier")
    elif not other_commit_seen:
        reason = "every baseline night ran the same commit as head"
    elif not bases:
        reason = "no baseline night on another commit is flat on the target counters"
    else:
        reason = ""
    return {"source_run": source_run, "label": plan["label"], "head": head,
            "bases": bases, "changes": changes, "thresholds": limits,
            "min_tier": min_tier, "max_builds": max_builds,
            "proceed": not reason, "reason": reason}


def nightly_start(workflow_path):
    """Return the nightly's daily cron start, (hour, minute) in UTC.

    The value is read from the nightly workflow itself, so the budget cannot
    drift from the schedule it has to stay clear of.
    """
    text = Path(workflow_path).read_text(encoding="utf-8")
    m = re.search(r'cron:\s*"(\d{1,2})\s+(\d{1,2})\s+\*\s+\*\s+\*"', text)
    if not m:
        raise ValueError(f"{workflow_path}: no daily 'M H * * *' cron schedule")
    return int(m.group(2)), int(m.group(1))


def budget_minutes(now, start, margin=30, wrapup=20, cap=360, minimum=90):
    """Return how many minutes a bisect starting at `now` may run, or 0 to skip it.

    The run ends `margin` minutes before the next nightly start and leaves
    `wrapup` minutes for the summary, upload and cleanup. `cap` bounds a wide
    interval so one regression cannot hold the pool all day. A budget under
    `minimum` cannot fit even a short bisect, so it is reported as 0. For
    example, a start at 00:30 UTC before a 05:00 nightly gets
    270 - 30 - 20 = 220 minutes.
    """
    nxt = now.replace(hour=start[0], minute=start[1], second=0, microsecond=0)
    if nxt <= now:
        nxt += datetime.timedelta(days=1)
    budget = min(int((nxt - now).total_seconds() // 60) - margin - wrapup, cap)
    return budget if budget >= minimum else 0


def first_parent_commits(repo, base, head):
    """Return the first-parent commits after `base` up to `head`, oldest first.

    Master is squash-merged, so each first-parent commit is one merged PR. A
    non-squash merge appears as its merge commit and is attributed as a whole.
    Only SHAs are read: commit text never enters this module.
    """
    def git(*args):
        return subprocess.run(["git", "-C", str(repo), *args], stdout=subprocess.PIPE,
                              stderr=subprocess.DEVNULL, text=True)

    if git("merge-base", "--is-ancestor", base, head).returncode != 0:
        raise ValueError(f"base {base[:10]} is not an ancestor of head {head[:10]}")
    listed = git("rev-list", "--first-parent", "--reverse", f"{base}..{head}")
    if listed.returncode != 0:
        raise ValueError("git rev-list failed")
    commits = listed.stdout.split()
    if not commits or not same_commit(commits[-1], head):
        raise ValueError(f"no first-parent commits between {base[:10]} and {head[:10]}")
    return commits


class SuiteLab:
    """Build commits in a probe worktree and measure them with the suite.

    Every commit builds in one worktree with the nightly's CMake flags, and is
    measured by this checkout's bench.py on one corpus prepared once, so all
    probes see byte-identical inputs. Building and measuring run the historical
    commit's source and build scripts by design; that code is what the bisect
    measures, so it runs here with no credentials in the environment.
    """

    def __init__(self, probe, snap, work, keys, samples=SAMPLES, warmup=1):
        # Absolute paths: build_api_driver uses the work directory both as its
        # cwd and as the output prefix, which a relative path breaks.
        self.probe, self.snap, self.work = (Path(p).resolve() for p in (probe, snap, work))
        self.keys, self.samples, self.warmup = list(keys), samples, warmup
        self.cases = [(manifest.BY_NAME[w], manifest.BY_NAME[w].default_size)
                      for w in sorted({w for w, _ in self.keys})]
        self.corpus_root = self.work / "prepared-corpus"
        for spec, size in self.cases:
            corpus.materialize(spec, size, str(self.corpus_root / corpus.dir_name(spec, size)))
        (self.work / "logs").mkdir(parents=True, exist_ok=True)
        self.driver = None
        if any(spec.mode == "api" for spec, _ in self.cases):
            self.driver = bench.build_api_driver(str(self.work))
            if not self.driver:
                raise MeasurementError("the api-driver did not build")
        self.configured = False

    def _run(self, cmd, log, timeout):
        # Output goes to a log file, not stdout: git and the build tools can
        # echo commit text, and stdout is what the workflow shows.
        with open(log, "a", encoding="utf-8") as fh:
            try:
                return subprocess.run(cmd, cwd=self.probe, stdout=fh, stderr=subprocess.STDOUT,
                                      timeout=timeout).returncode
            except subprocess.TimeoutExpired:
                raise OutOfBudget(f"the time budget ran out during '{cmd[0]} {cmd[1]}'")

    def build(self, sha, timeout=None):
        """Build `sha` within `timeout` seconds and return its snapshotted slangc.

        The trusted workflow step initializes every submodule once. A probe
        commit can still move a submodule pin, so each build syncs the pins it
        records; that fetches only when a pin moved.
        """
        log = self.work / "logs" / f"build-{sha[:10]}.log"
        deadline = None if timeout is None else time.time() + timeout
        steps = [["git", "checkout", "--quiet", "--detach", "--force", sha],
                 ["git", "submodule", "update", "--quiet", "--recursive", "--force"]]
        if not self.configured:
            steps.append(["cmake", "--preset", "default", "--fresh", *CMAKE_FLAGS])
        steps.append(["cmake", "--build", "--preset", "release", "--target", *BUILD_TARGETS])
        for cmd in steps:
            left = None if deadline is None else max(1.0, deadline - time.time())
            code = self._run(cmd, log, left)
            if code != 0:
                raise BuildError(f"{sha[:10]}: '{cmd[0]} {cmd[1]}' exited {code}")
        self.configured = True
        out = self.probe / "build" / "Release"
        exe = "slangc.exe" if sys.platform == "win32" else "slangc"
        if not (out / "bin" / exe).exists():
            raise BuildError(f"{sha[:10]}: slangc missing after the build")
        if not any(out.glob("*/*slang-glslang*")):
            raise BuildError(f"{sha[:10]}: slang-glslang missing after the build")
        dest = self.snap / sha
        shutil.rmtree(dest, ignore_errors=True)
        for sub in ("bin", "lib"):
            if (out / sub).is_dir():
                shutil.copytree(out / sub, dest / sub)
        return str(dest / "bin" / exe)

    def measure(self, slangc):
        """Run the target workloads once and return {(workload, counter): samples}."""
        api = None
        if self.driver:
            library = bench.find_libslang(slangc)
            if not library:
                raise MeasurementError("no slang library beside the probe's slangc")
            api = {"driver": self.driver, "libslang": library}
        with tempfile.TemporaryDirectory(prefix="perf_bisect_out_") as out:
            records = bench.run_workloads(slangc, self.cases, self.samples, self.warmup,
                                          str(self.corpus_root), out, api=api, prepared=True)
        runs = {r["workload"]: r for r in analyze.canonical_runs(records)}
        result = {}
        for workload, counter in self.keys:
            run = runs.get(workload)
            if not run or not run.get("ok"):
                raise MeasurementError(f"{workload}: the workload failed")
            values = ((run.get("timers") or {}).get(counter) or {}).get("samples") or []
            if len(values) != self.samples or not all(
                    isinstance(v, (int, float)) and math.isfinite(v) and v > 0 for v in values):
                raise MeasurementError(f"{workload}/{counter}: incomplete samples")
            result[(workload, counter)] = list(values)
        return result


class Bisector:
    """Run the attribution procedure and keep bisect.json current while it runs.

    The procedure, in order:
      1. A/A: measure head against itself. If the spread exceeds 2%, this host
         cannot resolve the change and the status is `unclear`.
      2. Endpoints: measure base against head. Only counters that step from
         base to head are followed. If none step, the next base is tried once,
         then the status is `not-reproduced`.
      3. Search: binary search over the first-parent commits between them. A
         probe is good when it is flat with base and steps to head, bad when it
         steps from base and is flat with head, and anything else stops the
         search with an interval.
      4. Verify: measure base, parent and culprit together. The culprit is named
         only if the parent is flat with base and the step reproduces.

    Every comparison uses one noise level, the A/A spread floored at
    NOISE_FLOOR. Two medians are flat when they differ by at most the noise. A
    counter steps when it rises by at least max(MIN_EFFECT, twice the noise)
    and by at least the nightly's absolute floor, so no comparison can be both.
    """

    def __init__(self, targets, lab, commits, out_dir, clock=time.time,
                 aa_rounds=AA_ROUNDS, ab_rounds=AB_ROUNDS):
        self.t, self.lab, self.commits = targets, lab, commits
        self.out, self.clock = Path(out_dir), clock
        self.aa_rounds, self.ab_rounds = aa_rounds, ab_rounds
        self.keys = [(c["workload"], c["counter"]) for c in targets["changes"]]
        self.floor = {k: trend.abs_floor_for(k[1], targets["thresholds"]["abs"])
                      for k in self.keys}
        deadline = targets.get("deadline_utc")
        self.deadline = (datetime.datetime.fromisoformat(deadline).timestamp()
                         if deadline else math.inf)
        self.binaries = {}
        self.last_cost = 0.0
        self.noise = NOISE_FLOOR
        self.kept = []
        self.state = {
            "status": "unclear", "complete": False, "reason": "started",
            "source_run": targets.get("source_run"), "label": targets.get("label"),
            "head": targets["head"], "base": None, "culprit": None, "parent": None,
            "interval": None, "aa_spread_pct": None, "noise_pct": None,
            "counters": [{"workload": w, "counter": c, "followed": False}
                         for w, c in self.keys],
            "probes": [], "builds_used": 0, "max_builds": targets["max_builds"],
            "deadline_utc": deadline,
        }

    # -- persistence -------------------------------------------------------

    def save(self):
        """Write bisect.json and bisect.md so a kill still leaves a valid report."""
        self.out.mkdir(parents=True, exist_ok=True)
        for name, text in (("bisect.json", json.dumps(self.state, indent=2)),
                           ("bisect.md", render(self.state))):
            tmp = self.out / (name + ".tmp")
            with analyze.open_output(str(tmp)) as fh:
                fh.write(text)
            os.replace(tmp, self.out / name)

    def finish(self, status, reason):
        self.state.update(status=status, reason=reason, complete=True)
        self.save()
        return self.state

    # -- primitives --------------------------------------------------------

    def _check_time(self):
        if self.clock() + self.last_cost > self.deadline:
            raise OutOfBudget("the time budget does not allow another step")

    def _build(self, sha):
        if sha in self.binaries:
            return self.binaries[sha]
        if self.state["builds_used"] >= self.state["max_builds"]:
            raise OutOfBudget(f"the build budget ({self.state['max_builds']}) is used up")
        self._check_time()
        start = self.clock()
        self.state["builds_used"] += 1
        left = None if math.isinf(self.deadline) else self.deadline - self.clock()
        try:
            self.binaries[sha] = self.lab.build(sha, timeout=left)
        finally:
            self.last_cost = max(self.last_cost, self.clock() - start)
            self.save()
        return self.binaries[sha]

    def _batch(self, slots, rounds):
        """Measure named binaries in alternating order and return pooled medians.

        Round order alternates (A B C, then C B A) so slow drift during a batch
        lands on every slot alike.
        """
        pooled = {name: {k: [] for k in self.keys} for name, _ in slots}
        start = self.clock()
        # A measurement in flight is not interrupted here; the workflow step's
        # timeout bounds it. These checks keep a late result from counting.
        for r in range(rounds):
            for name, sha in (slots if r % 2 == 0 else list(reversed(slots))):
                if self.clock() >= self.deadline:
                    raise OutOfBudget("the time budget ran out during a measurement")
                measured = self.lab.measure(self.binaries[sha])
                if self.clock() >= self.deadline:
                    raise OutOfBudget("the time budget ran out during a measurement")
                for key, values in measured.items():
                    if key in pooled[name]:
                        pooled[name][key].extend(values)
        self.last_cost = max(self.last_cost, self.clock() - start)
        return {name: {k: statistics.median(v) for k, v in by.items()}
                for name, by in pooled.items()}

    def flat(self, x, y):
        """Return whether y differs from x by at most the noise, in either direction."""
        return abs(y / x - 1) <= self.noise

    def steps(self, x, y, key):
        """Return whether y rose above x by max(MIN_EFFECT, 2 * noise) and the counter's absolute floor."""
        return y / x - 1 >= max(MIN_EFFECT, 2 * self.noise) and y - x >= self.floor[key]

    def _counter(self, key):
        return next(c for c in self.state["counters"]
                    if (c["workload"], c["counter"]) == key)

    # -- procedure ---------------------------------------------------------

    def run(self):
        """Run the whole procedure and return the final state."""
        head = self.t["head"]
        self.save()
        if not self.keys or not self.t["bases"]:
            return self.finish("unclear", "targets.json has nothing to bisect")
        try:
            self._build(head)
        except BuildError as e:
            return self.finish("unclear", f"head did not build: {e}")
        except OutOfBudget as e:
            return self.finish("unclear", str(e))
        # Once the search has bounds, any later stop still reports them.
        try:
            return self._run_from_head(head)
        except (OutOfBudget, MeasurementError) as e:
            return self.finish("interval" if self.state["interval"] else "unclear", str(e))

    def _run_from_head(self, head):
        aa = self._batch([("a", head), ("a2", head)], self.aa_rounds)
        spread = max(abs(aa["a2"][k] / aa["a"][k] - 1) for k in self.keys)
        self.noise = max(spread, NOISE_FLOOR)
        self.state.update(aa_spread_pct=round(100 * spread, 3),
                          noise_pct=round(100 * self.noise, 3))
        self.save()
        if spread > AA_MAX_SPREAD:
            return self.finish("unclear", f"the A/A spread {100 * spread:.2f}% exceeds "
                               f"{100 * AA_MAX_SPREAD:.0f}%; this host cannot resolve the change")

        base, why = None, []
        for candidate in self.t["bases"][:2]:
            sha = candidate["commit"]
            try:
                self._build(sha)
            except BuildError as e:
                why.append(f"base did not build: {e}")
                continue
            ends = self._batch([("base", sha), ("head", head)], self.ab_rounds)
            for key in self.keys:
                self._counter(key)["endpoints"] = {"base": ends["base"][key],
                                                   "head": ends["head"][key]}
            self.kept = [k for k in self.keys if self.steps(ends["base"][k], ends["head"][k], k)]
            self.state["base"] = sha
            self.save()
            why.append(f"no target counter steps from {sha[:10]} to head")
            if self.kept:
                base = sha
                break
        if base is None:
            return self.finish("not-reproduced" if self.state["base"] else "unclear",
                               "; ".join(why))
        for key in self.kept:
            self._counter(key)["followed"] = True

        try:
            seq = [base] + self.commits(base, head)
        except ValueError as e:
            return self.finish("unclear", str(e))
        lo, hi = 0, len(seq) - 1
        self._interval(seq, lo, hi)
        failed = set()
        while hi - lo > 1:
            open_ = [i for i in range(lo + 1, hi) if seq[i] not in failed]
            if not open_:
                return self.finish("interval", "no commit between the bounds built and measured")
            mid = (lo + hi) // 2
            i = min(open_, key=lambda j: (abs(j - mid), j))
            sha = seq[i]
            try:
                self._build(sha)
                verdict = self._probe(base, sha, head)
            except (BuildError, MeasurementError) as e:
                failed.add(sha)
                self.state["probes"].append({"commit": sha, "verdict": "skipped",
                                             "detail": str(e)})
                self.save()
                continue
            self.state["probes"].append({"commit": sha, "verdict": verdict})
            if verdict == "good":
                lo = i
            elif verdict == "bad":
                hi = i
            else:
                self._interval(seq, lo, hi)
                return self.finish("interval", f"probe {sha[:10]} was {verdict}; the change "
                                   "may span several commits")
            self._interval(seq, lo, hi)
        return self._verify(base, seq[lo], seq[hi])

    def _interval(self, seq, lo, hi):
        self.state.update(status="interval", interval={
            "good": seq[lo], "bad": seq[hi], "commits": hi - lo})
        self.save()

    def _probe(self, base, sha, head):
        """Classify one probe as good, bad, ambiguous, or drift.

        A probe whose batch is not good or bad gets one more batch, so at most
        two. "drift" means the endpoints themselves did not step in that batch,
        which leaves nothing to place the probe against.
        """
        verdict = "ambiguous"
        for _ in range(2):
            m = self._batch([("base", base), ("probe", sha), ("head", head)], self.ab_rounds)
            b, p, h = m["base"], m["probe"], m["head"]
            if not all(self.steps(b[k], h[k], k) for k in self.kept):
                verdict = "drift"
                continue
            if all(self.flat(b[k], p[k]) and self.steps(p[k], h[k], k) for k in self.kept):
                return "good"
            if all(self.steps(b[k], p[k], k) and self.flat(p[k], h[k]) for k in self.kept):
                return "bad"
            verdict = "ambiguous"
        return verdict

    def _verify(self, base, parent, culprit):
        slots = [("base", base), ("culprit", culprit)]
        if parent != base:
            slots.insert(1, ("parent", parent))
        v = self._batch(slots, self.ab_rounds)
        # With no parent slot the culprit directly follows base, so base is the parent.
        q = v.get("parent", v["base"])
        for key in self.kept:
            self._counter(key)["verify"] = {"base": v["base"][key], "parent": q[key],
                                            "culprit": v["culprit"][key]}
        self.state.update(parent=parent)
        if all(self.flat(v["base"][k], q[k]) and self.steps(q[k], v["culprit"][k], k)
               for k in self.kept):
            self.state["culprit"] = culprit
            return self.finish("culprit", "the step reproduces from the parent, which is "
                               "flat with base")
        return self.finish("interval", "the parent-to-culprit step did not reproduce")


# A change counts as real when the fastest sample moves: a slow tail alone is
# what host noise looks like, while a code change also slows the best run.
# MIN_SHIFT is 3%: replayed on the 8 archived quiet nights before 2026-09-30
# (1,728 primary timers), the rule below fires on 2.3% of them (16.9% if it
# compared with the lowest nightly minimum instead of their median).
# TAIL_RATIO and TAIL_NIGHTS are the bimodal-history rule of nanoclaw#1980.
MIN_SHIFT = 1.03
TAIL_RATIO = 1.25
TAIL_NIGHTS = 0.30
MIN_HISTORY = 3


def triage(results_dir, label):
    """Judge each confirmed change of one night as real, noise or unclear.

    The rules use only what the nightly has already stored: the night's two
    batches and its trailing baseline nights on the same runner. Per counter,
    in order:
      known-flaky       a counter listed in KNOWN_FLAKY: skipped, and left
                        out of the night's verdict.
      runner-changed    no baseline night ran on tonight's runner: unclear.
      thin-history      fewer than MIN_HISTORY such nights: unclear.
      same-commit-flat  the newest such night built head's commit and was flat
                        on this counter, so the same code measured fast a day
                        earlier: noise.
      bimodal-history   at least TAIL_NIGHTS of the nights had a sample
                        TAIL_RATIO above their median, reaching within 5% of
                        tonight's fastest sample: noise.
      min-shifted       tonight's fastest sample (either batch) is MIN_SHIFT
                        above the median of the nights' fastest samples and
                        clears the absolute floor: real. The median, not the
                        lowest of those minima, because one unusually fast
                        night would otherwise make a quiet night look shifted.
      below-floor       the fastest sample moved by MIN_SHIFT but by less than
                        the absolute floor, which a short counter cannot reach:
                        unclear, not noise.
      sibling-shifted   only the tail moved, but another counter of the same
                        workload is min-shifted: real.
      tail-only         only the tail moved: noise.
    The night's verdict comes from its confirmed regressions, or from its
    warnings when no regression remains once known-flaky counters are left
    out: real if any counter is real, noise if all are noise, unclear
    otherwise, and "none" if nothing is left to judge.
    """
    night = Path(results_dir) / "daily" / label
    archive = analyze.read_json(night / "confirmation.json")
    plan = archive["plan"]
    original = analyze.read_json(night / "results.json")
    errors, warnings, _ = confirm.confirmed_changes(plan, original, archive["repeated"])
    meta = analyze.read_json(night / "meta.json")
    head, runner = meta.get("commit", ""), meta.get("runner", "")
    limits = plan["thresholds"]

    history = []
    for hist_label in plan["baseline_labels"]:
        hist = Path(results_dir) / "daily" / hist_label
        if not (hist / "meta.json").exists() or not (hist / "results.json").exists():
            continue
        hist_meta = analyze.read_json(hist / "meta.json")
        if runner and hist_meta.get("runner") == runner:
            runs = {r["workload"]: r for r in analyze.canonical_runs(
                analyze.read_json(hist / "results.json")) if r.get("ok")}
            history.append({"label": hist_label, "commit": hist_meta.get("commit", ""),
                            "runs": runs})
    tonight = [{r["workload"]: r for r in analyze.canonical_runs(batch)}
               for batch in (original, archive["repeated"])]

    def stat(runs, workload, counter):
        return ((runs.get(workload) or {}).get("timers") or {}).get(counter) or {}

    rows = []
    for tier, change in [("regression", c) for c in errors] + [("warning", c) for c in warnings]:
        wl, ct = change.workload, change.counter
        row = {"tier": tier, "workload": wl, "counter": ct, "min_ratio": None}
        stats = [stat(h["runs"], wl, ct) for h in history]
        stats = [x for x in stats if all(isinstance(x.get(k), (int, float))
                                         for k in ("min", "max", "median"))]
        best = min(stat(b, wl, ct).get("min", math.inf) for b in tonight)
        if (wl, ct) in KNOWN_FLAKY:
            row["rule"] = "known-flaky"
        elif not history:
            row["rule"] = "runner-changed"
        elif len(stats) < MIN_HISTORY or not math.isfinite(best):
            row["rule"] = "thin-history"
        else:
            hist_best = statistics.median(x["min"] for x in stats)
            row["min_ratio"] = round(best / hist_best, 4)
            floor = trend.abs_floor_for(ct, limits["abs"])
            newest = history[-1]
            tails = [x for x in stats if x["max"] >= TAIL_RATIO * x["median"]]
            change_dict = {"workload": wl, "counter": ct, "baseline": change.baseline}
            if same_commit(newest["commit"], head) and is_flat_night(
                    {(wl, ct): stat(newest["runs"], wl, ct).get("median")}, change_dict, limits):
                row["rule"] = "same-commit-flat"
            elif (len(tails) >= TAIL_NIGHTS * len(stats)
                  and max(x["max"] for x in tails) >= 0.95 * best):
                row["rule"] = "bimodal-history"
            elif best >= MIN_SHIFT * hist_best and best - hist_best >= floor:
                row["rule"] = "min-shifted"
            elif best >= MIN_SHIFT * hist_best:
                row["rule"] = "below-floor"
            else:
                row["rule"] = "tail-only"
        rows.append(row)
    shifted = {r["workload"] for r in rows if r["rule"] == "min-shifted"}
    for r in rows:
        if r["rule"] == "tail-only" and r["workload"] in shifted:
            r["rule"] = "sibling-shifted"
        r["verdict"] = TRIAGE_RULES[r["rule"]]
    # The night's verdict follows the tier that raised the alert: regressions
    # when there are any, so a red night is judged by the counters that made it
    # red; warnings only on a night that confirmed nothing worse. Within that
    # set the night is real if ANY counter is real, because one real step is
    # enough to need a bisect; noise only if every counter is noise; else unclear.
    judged = [r for r in rows if r["rule"] != "known-flaky"]
    lead = [r for r in judged if r["tier"] == "regression"] or judged
    verdicts = {r["verdict"] for r in lead}
    verdict = ("none" if not lead else "real" if "real" in verdicts
               else "noise" if verdicts == {"noise"} else "unclear")
    return {"label": label, "head": head,
            "previous": history[-1]["commit"] if history else None,
            "history_nights": len(history), "verdict": verdict, "counters": rows}


TRIAGE_RULES = {"known-flaky": "skipped", "runner-changed": "unclear",
                "thin-history": "unclear", "same-commit-flat": "noise",
                "bimodal-history": "noise", "min-shifted": "real",
                "sibling-shifted": "real", "below-floor": "unclear", "tail-only": "noise"}


def triage_line(result):
    """Return the one Slack line for a triage result, or "" when there is nothing to say.

    The line leads with the tier the verdict was taken from and names the other
    tier after it, so it reads against the alert above it. Known-flaky counters
    are skipped rather than counted as noise, and are reported separately.
    Counters that do not look real are counted by rule in parentheses so a
    noise verdict says why. For the 2026-10-16 night, with 4 confirmed
    regressions and 12 confirmed warnings, one of them known-flaky: "Real or
    noise: real. 4 of 4 confirmed regressions look real; 11 of 11 confirmed
    warnings do too (1 known-flaky counter skipped); compared with
    the last 7 nights; f2b163c082..e49310561e."
    The line is assembled from fixed words, rule names, counts and validated
    SHAs only, the same contract as render().
    """
    if result.get("verdict") not in ("real", "noise", "unclear"):
        return ""
    rows = [r for r in result.get("counters") or [] if r.get("rule") in TRIAGE_RULES
            and r.get("tier") in ("regression", "warning")]

    def part(tier):
        group = [r for r in rows if r["tier"] == tier]
        judged = [r for r in group if r["rule"] != "known-flaky"]
        real = sum(1 for r in judged if TRIAGE_RULES[r["rule"]] == "real")
        other = {}
        for r in judged:
            if TRIAGE_RULES[r["rule"]] != "real":
                other[r["rule"]] = other.get(r["rule"], 0) + 1
        notes = [f"{rule} {n}" for rule, n in sorted(other.items())]
        flaky = len(group) - len(judged)
        if flaky:
            notes.append(f"{flaky} known-flaky counter{'s' if flaky > 1 else ''} skipped")
        return group, real, len(judged), (f" ({'; '.join(notes)})" if notes else "")

    lead_tier = ("regression" if any(r["tier"] == "regression" and r["rule"] != "known-flaky"
                                     for r in rows) else "warning")
    other_tier = "warning" if lead_tier == "regression" else "regression"
    _, real, judged, notes = part(lead_tier)
    text = (f"Real or noise: {result['verdict']}. {real} of {judged} confirmed "
            f"{lead_tier}s look real{notes}")
    group, o_real, o_judged, o_notes = part(other_tier)
    if o_judged:
        text += f"; {o_real} of {o_judged} confirmed {other_tier}s do too{o_notes}"
    elif group:
        n = len(group)
        text += f"; {n} known-flaky {other_tier}{'' if n == 1 else 's'} skipped"
    nights = result.get("history_nights")
    if isinstance(nights, int) and not isinstance(nights, bool):
        compared = ("compared with the last night" if nights == 1
                    else f"compared with the last {nights} nights")
    else:
        compared = "compared with recent nights"

    def sha(s):
        return s[:10] if isinstance(s, str) and SHA_RE.fullmatch(s) else "?"

    return (f"{text}; {compared}; "
            f"{sha(result.get('previous'))}..{sha(result.get('head'))}.")


SHA_RE = re.compile(r"[0-9a-f]{7,40}")
NAME_RE = re.compile(r"[A-Za-z0-9_]{1,64}")
STATUSES = ("unclear", "not-reproduced", "interval", "culprit")
VERDICTS = ("good", "bad", "ambiguous", "drift", "skipped")


def render(state):
    """Return bisect.md for a state, built from validated SHAs, names and numbers.

    bisect.md is published in a public job summary, and a state file could in
    principle be edited between steps, so every field is checked against a
    closed shape before it is written: SHAs must be lowercase hex, workload and
    counter names identifiers, statuses and verdicts members of fixed sets, and
    measurements finite numbers. Anything else renders as "?". The free-text
    `reason` and probe `detail` fields stay in bisect.json and never reach the
    summary.
    """
    def sha(s):
        return f"`{s[:10]}`" if isinstance(s, str) and SHA_RE.fullmatch(s) else "?"

    def name(s):
        return s if isinstance(s, str) and NAME_RE.fullmatch(s) else "?"

    def num(x):
        return isinstance(x, (int, float)) and not isinstance(x, bool) and math.isfinite(x)

    def qty(counter, x):
        return analyze.fmt_qty(counter, x) if num(x) else "-"

    status = state.get("status") if state.get("status") in STATUSES else "?"
    done = "" if state.get("complete") is True else " (stopped before completion)"
    spread = state.get("aa_spread_pct")
    builds = [state.get(k) if num(state.get(k)) else "?" for k in ("builds_used", "max_builds")]
    deadline = state.get("deadline_utc")
    deadline = (deadline if isinstance(deadline, str)
                and re.fullmatch(r"[0-9T:.+\-]{10,40}", deadline) else "none")
    lines = [f"**Status:** {status}{done}", "",
             f"Head {sha(state.get('head'))} · base {sha(state.get('base'))} · builds "
             f"{builds[0]}/{builds[1]} · A/A spread "
             f"{f'{spread:.2f}%' if num(spread) else 'n/a'} · deadline {deadline}"]
    iv = state.get("interval") if isinstance(state.get("interval"), dict) else {}
    if status == "culprit":
        lines.append(f"Culprit {sha(state.get('culprit'))}, parent {sha(state.get('parent'))}")
    elif iv:
        count = iv.get("commits") if num(iv.get("commits")) else "?"
        lines.append(f"Interval {sha(iv.get('good'))}..{sha(iv.get('bad'))} "
                     f"({count} first-parent commit(s))")
    lines += ["", "| workload | counter | followed | base | head | parent | culprit |",
              "|---|---|---|---|---|---|---|"]
    for c in state.get("counters") or []:
        counter = name(c.get("counter"))
        ends = c.get("endpoints") if isinstance(c.get("endpoints"), dict) else {}
        ver = c.get("verify") if isinstance(c.get("verify"), dict) else {}
        cells = [qty(counter, x) for x in (ends.get("base"), ends.get("head"),
                                           ver.get("parent"), ver.get("culprit"))]
        lines.append(f"| {name(c.get('workload'))} | {counter} | "
                     f"{'yes' if c.get('followed') is True else 'no'} | " + " | ".join(cells) + " |")
    probes = state.get("probes") or []
    if probes:
        lines += ["", "| probe | verdict |", "|---|---|"]
        lines += [f"| {sha(p.get('commit'))} | "
                  f"{p.get('verdict') if p.get('verdict') in VERDICTS else '?'} |" for p in probes]
    return "\n".join(lines) + "\n"


def github_output(**values):
    """Print key=value lines and append them to $GITHUB_OUTPUT when it is set."""
    text = "".join(f"{k}={v}\n" for k, v in values.items())
    sys.stdout.write(text)
    path = os.environ.get("GITHUB_OUTPUT")
    if path:
        with analyze.open_output(path, "a") as fh:
            fh.write(text)


def builds_arg(value):
    n = int(value)
    # 40 covers a 2^30-commit range plus fallbacks; it bounds runner time.
    if not 3 <= n <= 40:
        raise argparse.ArgumentTypeError("max builds must be 3..40 (head, base and a probe)")
    return n


def main(argv=None):
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="command", required=True)
    t = sub.add_parser("targets", help="choose counters and a base for one night")
    t.add_argument("--night", required=True, type=Path, help="daily/<label>/ of the night")
    t.add_argument("--nights-root", required=True, type=Path,
                   help="directory holding the baseline nights' <label>/ folders")
    t.add_argument("--min-tier", choices=("regression", "warning"), default="regression")
    t.add_argument("--max-builds", type=builds_arg, default=12)
    t.add_argument("--source-run", default=None)
    t.add_argument("--master-repo", type=Path, default=None,
                   help="a clone with master's history: head and bases must be on --master-ref")
    t.add_argument("--master-ref", default="origin/master")
    t.add_argument("--out", required=True, type=Path)
    d = sub.add_parser("deadline", help="size the time budget and record it in targets.json")
    d.add_argument("--targets", required=True, type=Path)
    d.add_argument("--nightly-workflow", required=True, type=Path)
    d.add_argument("--now", default=None, help="ISO time (tests); default is the current UTC time")
    r = sub.add_parser("run", help="build and measure; keep bisect.json current")
    r.add_argument("--targets", required=True, type=Path)
    r.add_argument("--probe", required=True, type=Path, help="git worktree to build in")
    r.add_argument("--snap", required=True, type=Path, help="where built binaries are kept")
    r.add_argument("--work", required=True, type=Path, help="corpus, driver and build logs")
    r.add_argument("--out", required=True, type=Path, help="bisect.json and bisect.md")
    r.add_argument("--aa-rounds", type=int, default=AA_ROUNDS)
    r.add_argument("--ab-rounds", type=int, default=AB_ROUNDS)
    r.add_argument("--samples", type=int, default=SAMPLES)
    p = sub.add_parser("report", help="print bisect.md from bisect.json")
    p.add_argument("--out", required=True, type=Path)
    g = sub.add_parser("triage", help="judge a confirmed night real or noise from its samples")
    g.add_argument("--results", required=True, type=Path, help="the results repo checkout")
    g.add_argument("--label", required=True)
    g.add_argument("--out", type=Path, default=None, help="where to write triage.json")
    args = ap.parse_args(argv)

    if args.command == "targets":
        targets = select_targets(args.night, args.nights_root, args.min_tier,
                                 args.max_builds, args.source_run,
                                 on_master_check(args.master_repo, args.master_ref)
                                 if args.master_repo else None)
        with analyze.open_output(str(args.out)) as fh:
            json.dump(targets, fh, indent=2)
        github_output(proceed="true" if targets["proceed"] else "false",
                      head=targets["head"],
                      base=targets["bases"][0]["commit"] if targets["bases"] else "",
                      reason=targets["reason"])
        n, b = len(targets["changes"]), len(targets["bases"])
        trend.write_step_summary(
            f"### Bisect targets\n\n{n} target counter{'' if n == 1 else 's'}; "
            f"{b} candidate base night{'' if b == 1 else 's'}; head `{targets['head'][:10]}`. "
            + ("Bisecting." if targets["proceed"] else f"Not bisecting: {targets['reason']}."))
    elif args.command == "deadline":
        now = (datetime.datetime.fromisoformat(args.now) if args.now
               else datetime.datetime.now(datetime.timezone.utc))
        budget = budget_minutes(now, nightly_start(args.nightly_workflow))
        targets = analyze.read_json(args.targets)
        targets["deadline_utc"] = ((now + datetime.timedelta(minutes=budget)).isoformat()
                                   if budget else None)
        with analyze.open_output(str(args.targets)) as fh:
            json.dump(targets, fh, indent=2)
        github_output(budget=budget)
        if not budget:
            trend.write_step_summary("Not bisecting: too little time before the next nightly. "
                                     "Re-dispatch with the run id after it finishes.")
    elif args.command == "run":
        targets = analyze.read_json(args.targets)
        runner = Bisector(targets, None, lambda b, h: first_parent_commits(args.probe, b, h),
                          args.out, aa_rounds=args.aa_rounds, ab_rounds=args.ab_rounds)
        # Saved before setup, so a failed corpus or driver build still leaves a report.
        runner.save()
        try:
            runner.lab = SuiteLab(args.probe, args.snap, args.work,
                                  [(c["workload"], c["counter"]) for c in targets["changes"]],
                                  samples=args.samples)
        except (MeasurementError, OSError, ValueError) as e:
            state = runner.finish("unclear", f"setup failed: {type(e).__name__}: {e}")
        else:
            state = runner.run()
        print(f"perf_bisect: {state['status']}")
    elif args.command == "triage":
        result = triage(args.results, args.label)
        if args.out:
            with analyze.open_output(str(args.out)) as fh:
                json.dump(result, fh, indent=2)
        line = triage_line(result)
        github_output(verdict=result["verdict"], line=line)
        if line:
            trend.write_step_summary(f"### Real or noise\n\n{line}")
    else:
        sys.stdout.write(render(analyze.read_json(args.out / "bisect.json")))


if __name__ == "__main__":
    main()
