#!/usr/bin/env python3
"""Evidence bundle for a nightly compile-perf alert.

When the nightly confirms a regression, someone has to answer the same
questions every time: is it real or noise, which counters moved and across how
many workloads, and which commits between last night's build and tonight's could
have caused it. This script answers the measurable parts deterministically and
writes them as one JSON bundle (plus a Markdown rendering), so a person -- or the
analysis agent described in AGENT-ANALYSIS.md -- starts from numbers instead of
redoing the arithmetic by hand.

It consumes what the nightly already produced and never re-derives the alert:

  daily/<label>/results.json       tonight's original sweep
  daily/<label>/confirmation.json  the frozen baseline, the flagged candidates
                                   and the rerun (written by confirm.py)
  daily/<label>/meta.json          commit, runner

and reuses confirm.confirmed_changes and analyze.canonical_runs, so "flagged",
"confirmed" and "comparable" mean exactly what they mean in the alert.

What the bundle contains, per flagged counter:
  - the baseline's per-night medians/mins/maxes next to tonight's original and
    rerun samples, so a median that moved only because of slow outliers is
    visible as such (`tail-only`, `bimodal-history`);
  - a coarse hint -- a description of the evidence, not a verdict.
Across the whole sweep:
  - a per-timer shift table (median ratio over every comparable workload, with
    a leave-one-night-out null so ordinary night-to-night movement is not
    reported as a shift). One timer moving ~8% in nearly every workload while
    its neighbours stay flat is the signature of a fixed per-compile cost, as
    opposed to a defect in one workload's code path.
Across the commit range between the last differing baseline commit and tonight's:
  - each commit's author, PR number and changed files, mapped to the compiler
    components (and so the timers) those files can affect, ranked by changed
    lines. This is a heuristic for choosing what to bisect or read first; it
    does not establish causation.

Only the standard library is used (CI enforces this for the suite). Commit data
comes from a local git checkout when it has both commits, otherwise from the
GitHub REST API (GITHUB_TOKEN / GH_TOKEN optional).

    python3 triage.py --results <perf-results> --out-json bundle.json --out-md bundle.md
    python3 triage.py --results <dir> --commit <sha> --repo-dir <slang checkout>
"""
import argparse
import json
import os
import re
import statistics
import subprocess
import sys
import urllib.error
import urllib.request

import confirm
from lib import analyze

HERE = os.path.dirname(os.path.abspath(__file__))
SCHEMA_VERSION = 1

# Provenance a baseline night must share with tonight's row before the two are
# compared. Same rule as trend.py: a missing field is never evidence of equality.
PROVENANCE_FIELDS = ("size", "timer_schema", "sampling_strategy")

# A timer's baseline median must reach this before the shift table uses it. A
# 0.05 ms timer moving 20% is quantization, not a shift.
SHIFT_FLOOR_MS = 5.0
SHIFT_MIN_WORKLOADS = 5
SHIFT_MIN_RATIO = 1.02
# A shift must clear the biggest ordinary night-to-night movement by this much.
SHIFT_NULL_MARGIN = 0.005
OVER_RATIO = 1.03

# Heuristic knobs for the per-counter hint.
TAIL_NIGHT_RATIO = 1.25  # a night's max over its own median marks a slow tail
TAIL_FRACTION_BIMODAL = 0.3
MIN_SHIFT_RATIO = 1.03
TAIL_REACHED = 0.95

MAX_COMMITS = 80
MAX_FILES_PER_COMMIT = 12
MAX_DIFF_BYTES = 80_000
MAX_BODY_BYTES = 12_000

# Map changed paths to the compiler component, and so to the timers, they can
# move. First match wins, so the specific rules come before the broad ones.
# `None` marks paths that cannot change compile time at all.
_FRONT = {"frontEndExecute", "compileInner"}
_BACK = {"linkAndOptimizeIR", "generateOutput", "compileInner"}
COMPONENT_RULES = [
    (r"^(docs|tests|\.github|examples|external|extras|\.claude)/|\.md$", None, set()),
    (r"^source/slang/.*\.meta\.slang$|^source/standard-modules/", "builtin-module",
     {"loadBuiltinModule", "readSerializedModuleAST", "readSerializedModuleIR",
      "SemanticChecking"} | _FRONT),
    (r"^source/slang/(slang-serialize|slang-fossil)", "serialization",
     {"loadBuiltinModule", "readSerializedModuleAST", "readSerializedModuleIR"} | _FRONT),
    (r"^source/slang/slang-(parser|lexer|preprocessor|token)", "parse",
     {"parseTranslationUnit"} | _FRONT),
    (r"^source/slang/slang-(check|ast|lookup|syntax|name|visitor)", "semantic-checking",
     {"SemanticChecking"} | _FRONT),
    (r"^source/slang/slang-lower-to-ir", "ast-to-ir",
     {"generateIRForTranslationUnit"} | _FRONT),
    (r"^source/slang/slang-(ir|emit|legalize|spirv|glsl|hlsl|metal|wgsl|cuda)", "ir-and-emit",
     {"simplifyIR", "specializeModule", "legalizeResourceTypes"} | _BACK),
    (r"^source/(core|compiler-core)/", "core-library", _FRONT | _BACK),
    (r"^(source|include|tools)/", "other-compiler-source", _FRONT | _BACK),
]
_COMPILED_RULES = [(re.compile(p), c, t) for p, c, t in COMPONENT_RULES]


class TriageError(Exception):
    """A condition the caller should report, not a bug."""


# --------------------------------------------------------------------- loading

def load_rows(results_dir, label):
    """{workload: canonical ok row} for `label`'s original sweep."""
    path = analyze.results_path(results_dir, label)
    return {r["workload"]: r for r in analyze.canonical_runs(analyze.read_json(path))
            if r.get("ok") and r.get("timers")}


def daily_points(results_dir):
    return analyze.daily_labels(results_dir)


def pick_point(points, label=None, commit=None):
    """The daily point to analyze: by label, by commit (newest match), or newest."""
    if label:
        for p in points:
            if p["label"] == label:
                return p
        raise TriageError(f"no daily point labelled {label!r}")
    if commit:
        hits = [p for p in points if p["commit"] and (p["commit"].startswith(commit)
                                                      or commit.startswith(p["commit"]))]
        if not hits:
            raise TriageError(f"no daily point for commit {commit}")
        return sorted(hits, key=lambda p: (p["date"], p["label"]))[-1]
    if not points:
        raise TriageError("no daily points in the results directory")
    return sorted(points, key=lambda p: (p["date"], p["label"]))[-1]


def comparable(a, b):
    return all(a.get(f) is not None and a.get(f) == b.get(f) for f in PROVENANCE_FIELDS)


def stat(row, counter):
    st = (row.get("timers") or {}).get(counter)
    return st if st and st.get("samples") else None


def load_known_flaky(path):
    if path and os.path.exists(path):
        return analyze.read_json(path)
    return {}


# ------------------------------------------------------------ flagged counters

def _summ(st):
    return {"median": st["median"], "min": st["min"], "max": st["max"],
            "stdev": st.get("stdev"), "samples": st["samples"]}


def counter_evidence(workload, counter, cur_row, rerun_row, base_rows, known_flaky):
    """Evidence for one flagged counter, and a coarse hint about what it shows."""
    hist = [stat(r, counter) for r in base_rows if comparable(r, cur_row)]
    hist = [h for h in hist if h]
    cur, rer = stat(cur_row, counter), (stat(rerun_row, counter) if rerun_row else None)
    ev = {"workload": workload, "counter": counter,
          "original": _summ(cur) if cur else None,
          "rerun": _summ(rer) if rer else None,
          "history": {"nights": len(hist),
                      "medians": [h["median"] for h in hist],
                      "mins": [h["min"] for h in hist],
                      "maxes": [h["max"] for h in hist]},
          "hint": "insufficient-history", "hint_detail": ""}
    if len(hist) < 3 or not cur:
        return ev
    base_med = statistics.median(ev["history"]["medians"])
    base_min = statistics.median(ev["history"]["mins"])
    tail_nights = sum(1 for h in hist if h["median"] > 0 and h["max"] / h["median"] > TAIL_NIGHT_RATIO)
    tail_fraction = tail_nights / len(hist)
    ev["baseline_median"], ev["baseline_min"] = base_med, base_min
    ev["tail_fraction"] = round(tail_fraction, 2)
    ev["ratios"] = {
        "original_median": cur["median"] / base_med,
        "original_min": cur["min"] / base_min,
        "rerun_median": rer["median"] / base_med if rer else None,
        "rerun_min": rer["min"] / base_min if rer else None}
    best_min = min(x["min"] for x in (cur, rer) if x)
    worst_min = max(x["min"] for x in (cur, rer) if x)
    tail_reached = max(ev["history"]["maxes"]) >= TAIL_REACHED * worst_min
    shifted = best_min / base_min >= MIN_SHIFT_RATIO
    key = f"{workload}|{counter}"
    if key in known_flaky:
        ev["hint"], ev["hint_detail"] = "known-flaky", known_flaky[key]
    elif tail_fraction >= TAIL_FRACTION_BIMODAL and tail_reached:
        ev["hint"] = "bimodal-history"
        ev["hint_detail"] = (f"{tail_nights} of {len(hist)} baseline nights already contain "
                             f"samples as slow as tonight's fastest")
    elif shifted:
        ev["hint"] = "min-shifted"
        ev["hint_detail"] = "even the fastest sample in each batch is slower than the baseline's"
    else:
        ev["hint"] = "tail-only"
        ev["hint_detail"] = "the median rose but the fastest samples did not"
    return ev


def flagged_counters(plan, original_rows, repeated_rows, base_by_label, known_flaky):
    """One record per candidate in the frozen plan, with the alert's own outcome."""
    outcome, notes = {}, []
    try:
        errors, warnings, cleared = confirm.confirmed_changes(plan, original_rows, repeated_rows)
        for tier, items in (("error", errors), ("warning", warnings), ("cleared", cleared)):
            for ch in items:
                outcome[(ch.workload, ch.counter)] = tier
    except ValueError as exc:
        notes.append(f"confirmation could not be evaluated: {exc}")
    cur = {r["workload"]: r for r in analyze.canonical_runs(original_rows) if r.get("ok")}
    rer = {r["workload"]: r for r in analyze.canonical_runs(repeated_rows) if r.get("ok")}
    out = []
    for tier, key in (("error", "regressions"), ("warning", "warnings")):
        for row in plan.get(key, []):
            wl, counter = row["workload"], row["counter"]
            if wl not in cur:
                notes.append(f"{wl}/{counter}: workload missing from the original sweep")
                continue
            base_rows = [b[wl] for b in base_by_label.values() if wl in b]
            ev = counter_evidence(wl, counter, cur[wl], rer.get(wl), base_rows, known_flaky)
            ev.update({"initial_tier": tier,
                       "outcome": outcome.get((wl, counter), "cannot-evaluate"),
                       "plan_baseline": row.get("baseline"), "plan_value": row.get("value")})
            out.append(ev)
    return out, notes


# --------------------------------------------------------------- shift analysis

def _timer_stat(cur_rows, base_rows_list, timer):
    """Median over workloads of tonight/baseline, using medians and mins."""
    meds, mins, over = [], [], 0
    for wl, cur in cur_rows.items():
        c = stat(cur, timer)
        if not c:
            continue
        hist = [stat(b[wl], timer) for b in base_rows_list
                if wl in b and comparable(b[wl], cur)]
        hist = [h for h in hist if h]
        if len(hist) < 4:
            continue
        bm = statistics.median(h["median"] for h in hist)
        if bm < SHIFT_FLOOR_MS:
            continue
        r = c["median"] / bm
        meds.append(r)
        mins.append(c["min"] / statistics.median(h["min"] for h in hist))
        over += r > OVER_RATIO
    if len(meds) < SHIFT_MIN_WORKLOADS:
        return None
    return {"n": len(meds), "median_ratio": statistics.median(meds),
            "min_ratio": statistics.median(mins), "share_over": over / len(meds)}


def shift_table(cur_rows, base_by_label):
    """Per-timer shift across workloads, each judged against its own null.

    The null for a timer is the same statistic computed for every baseline night
    against the other baseline nights. Tonight's statistic is only called a shift
    when it clears the largest of those by SHIFT_NULL_MARGIN: a night that is
    uniformly 3% slow for reasons unrelated to the code shows up in the null too.
    """
    labels = sorted(base_by_label)
    timers = sorted({t for r in cur_rows.values() for t in r["timers"]})
    table = []
    for timer in timers:
        tonight = _timer_stat(cur_rows, list(base_by_label.values()), timer)
        if not tonight:
            continue
        null = []
        for held_out in labels:
            others = [base_by_label[l] for l in labels if l != held_out]
            s = _timer_stat(base_by_label[held_out], others, timer)
            if s:
                null.append(s["median_ratio"])
        tonight["null_max"] = max(null) if null else None
        tonight["null_nights"] = len(null)
        tonight["significant"] = bool(
            null and tonight["median_ratio"] >= SHIFT_MIN_RATIO
            and tonight["median_ratio"] > max(null) + SHIFT_NULL_MARGIN
            and tonight["share_over"] >= 0.5)
        tonight["timer"] = timer
        table.append(tonight)
    table.sort(key=lambda t: t["median_ratio"], reverse=True)
    return table


# ---------------------------------------------------------------- whole-compile impact

REAL_WORLD_BUCKETS = ("real_world",)


def bucket_summary(cur_rows, base_by_label):
    """Whole-compile effect per bucket, and the realistic workloads in detail.

    A regression in one phase matters to users as a share of the whole compile, so
    this reports compileInner and wall time ratios per bucket and, for the
    real-world bucket, how much of each compile the front end and semantic checking
    are. The caller multiplies a phase's shift by its share; the bundle supplies both.
    """
    def ratio(cur, base_rows, getter):
        hist = [getter(b) for b in base_rows if comparable(b, cur)]
        hist = [h for h in hist if h]
        c = getter(cur)
        if len(hist) < 4 or not c:
            return None
        base = statistics.median(hist)
        return c / base if base > 0 else None

    inner = lambda r: (stat(r, "compileInner") or {}).get("median")
    wall = lambda r: (r.get("wall_ms") or {}).get("median")
    buckets = {}
    for wl, cur in cur_rows.items():
        base_rows = [b[wl] for b in base_by_label.values() if wl in b]
        entry = buckets.setdefault(cur.get("bucket", "?"), {"inner": [], "wall": []})
        for key, getter in (("inner", inner), ("wall", wall)):
            r = ratio(cur, base_rows, getter)
            if r is not None:
                entry[key].append((r, wl))
    out = []
    for name, e in sorted(buckets.items()):
        item = {"bucket": name, "workloads": max(len(e["inner"]), len(e["wall"]))}
        for key, label in (("inner", "compileInner"), ("wall", "wall")):
            vals = sorted(e[key], reverse=True)
            item[f"{label}_median_ratio"] = statistics.median(v for v, _ in vals) if vals else None
            item[f"{label}_worst"] = {"ratio": vals[0][0], "workload": vals[0][1]} if vals else None
        out.append(item)
    detail = []
    for wl, cur in sorted(cur_rows.items()):
        if cur.get("bucket") not in REAL_WORLD_BUCKETS:
            continue
        base_rows = [b[wl] for b in base_by_label.values() if wl in b]
        total = inner(cur)
        entry = {"workload": wl, "compileInner": {"ratio": ratio(cur, base_rows, inner)},
                 "wall": {"ratio": ratio(cur, base_rows, wall)}}
        for t in ("frontEndExecute", "SemanticChecking"):
            st = stat(cur, t)
            entry[f"{t}_share_of_compile"] = st["median"] / total if st and total else None
            entry[f"{t}_ratio"] = ratio(cur, base_rows, lambda r, t=t: (stat(r, t) or {}).get("median"))
        detail.append(entry)
    return out, detail


# ---------------------------------------------------------------- commit range

def prefix_equal(a, b):
    return bool(a and b and (a.startswith(b) or b.startswith(a)))


def resolve_range(points_by_label, current, baseline_labels):
    """(base_point, same_commit) -- the newest baseline night built from a
    different commit than tonight's, or None when every baseline night shares it."""
    cands = [points_by_label[l] for l in baseline_labels if l in points_by_label]
    cands.sort(key=lambda p: (p["date"], p["label"]), reverse=True)
    for p in cands:
        if not prefix_equal(p["commit"], current["commit"]):
            return p
    return None


def component_of(path):
    for rx, comp, timers in _COMPILED_RULES:
        if rx.search(path):
            return comp, timers
    return None, set()


PR_RE = re.compile(r"\(#(\d+)\)\s*$")


def score_commits(commits, story_timers):
    """Rank commits by changed lines in files that can move the story timers."""
    ranked = []
    for c in commits:
        by_comp, total, why = {}, 0, []
        for f in c["files"]:
            comp, timers = component_of(f["path"])
            if not comp or not (timers & story_timers):
                continue
            n = f["additions"] + f["deletions"]
            by_comp[comp] = by_comp.get(comp, 0) + n
            total += n
            why.append(f["path"])
        c["relevant_lines"], c["components"] = total, by_comp
        if total:
            ranked.append({"sha": c["sha"], "pr": c["pr"], "author": c["author"],
                           "subject": c["subject"], "relevant_lines": total,
                           "components": by_comp, "files": why[:5]})
    ranked.sort(key=lambda r: r["relevant_lines"], reverse=True)
    return ranked


def git_commits(repo_dir, base, head):
    def run(*args):
        return subprocess.run(["git", "-C", repo_dir, *args], capture_output=True, text=True,
                              check=True, encoding="utf-8", errors="replace").stdout
    for sha in (base, head):
        subprocess.run(["git", "-C", repo_dir, "cat-file", "-e", f"{sha}^{{commit}}"],
                       capture_output=True, check=True)
    log = run("log", "--reverse", f"--max-count={MAX_COMMITS}", "--format=%H%x1f%an%x1f%cI%x1f%s",
              f"{base}..{head}")
    commits = []
    for line in log.splitlines():
        sha, author, date, subject = line.split("\x1f", 3)
        files = []
        for nl in run("show", "--numstat", "--format=", sha).splitlines():
            a, d, path = nl.split("\t", 2)
            files.append({"path": path, "additions": int(a) if a.isdigit() else 0,
                          "deletions": int(d) if d.isdigit() else 0})
        commits.append({"sha": sha, "author": author, "date": date, "subject": subject,
                        "files": files})
    return commits


class GitHub:
    """The few REST calls triage needs. urllib only; the token is optional."""

    def __init__(self, repo, token=None):
        self.repo = repo
        self.token = token or os.environ.get("GITHUB_TOKEN") or os.environ.get("GH_TOKEN")

    def get(self, path, accept="application/vnd.github+json"):
        req = urllib.request.Request(f"https://api.github.com/repos/{self.repo}/{path}",
                                     headers={"Accept": accept, "User-Agent": "slang-compile-perf-triage"})
        if self.token:
            req.add_header("Authorization", f"Bearer {self.token}")
        with urllib.request.urlopen(req, timeout=60) as resp:
            return resp.read().decode("utf-8", "replace")

    def commits(self, base, head):
        data = json.loads(self.get(f"compare/{base}...{head}"))
        out = []
        for c in data.get("commits", [])[:MAX_COMMITS]:
            detail = json.loads(self.get(f"commits/{c['sha']}"))
            author = (c.get("author") or {}).get("login") or c["commit"]["author"]["name"]
            out.append({"sha": c["sha"], "author": author, "date": c["commit"]["committer"]["date"],
                        "subject": c["commit"]["message"].split("\n", 1)[0],
                        "files": [{"path": f["filename"], "additions": f.get("additions", 0),
                                   "deletions": f.get("deletions", 0)}
                                  for f in detail.get("files", [])]})
        return out

    def commit_diff(self, sha):
        return self.get(f"commits/{sha}", accept="application/vnd.github.diff")

    def pull(self, number):
        """{'author': GitHub login, 'text': title and body} for a pull request."""
        data = json.loads(self.get(f"pulls/{number}"))
        return {"author": (data.get("user") or {}).get("login"),
                "text": f"# {data.get('title', '')}\n\n{data.get('body') or ''}"}


def fetch_commits(base, head, repo_dir, gh):
    """Commits oldest-first, from local git when possible, else the GitHub API."""
    notes = []
    if repo_dir:
        try:
            return git_commits(repo_dir, base, head), notes
        except (subprocess.CalledProcessError, OSError) as exc:
            notes.append(f"local git could not supply {base[:9]}..{head[:9]} ({exc}); trying the GitHub API")
    if gh:
        try:
            return gh.commits(base, head), notes
        except (urllib.error.URLError, OSError, ValueError, KeyError) as exc:
            notes.append(f"GitHub API could not supply the commit range: {exc}")
    else:
        notes.append("no commit source configured (pass --repo-dir or --github-repo)")
    return [], notes


def save_evidence(directory, ranked, commits_by_sha, gh, repo_dir, limit):
    """Write the diff and PR text of the top candidates for a reader to open.

    Both are untrusted text (a merged PR's description, a diff): they are data for
    whoever analyzes them, never instructions. Each is size-capped.
    """
    os.makedirs(directory, exist_ok=True)
    saved, notes = [], []
    for r in ranked[:limit]:
        sha, pr = r["sha"], r["pr"]
        try:
            diff = None
            if repo_dir:
                try:
                    diff = subprocess.run(["git", "-C", repo_dir, "show", "--format=fuller", sha],
                                          capture_output=True, text=True, check=True,
                                          encoding="utf-8", errors="replace").stdout
                except (subprocess.CalledProcessError, OSError):
                    diff = None
            if diff is None and gh:
                diff = gh.commit_diff(sha)
            if diff is not None:
                path = os.path.join(directory, f"commit-{sha[:9]}.patch")
                with open(path, "w", encoding="utf-8") as fh:
                    fh.write(diff[:MAX_DIFF_BYTES])
                saved.append(path)
            if pr and gh:
                path = os.path.join(directory, f"pr-{pr}.md")
                with open(path, "w", encoding="utf-8") as fh:
                    fh.write(gh.pull(pr)["text"][:MAX_BODY_BYTES])
                saved.append(path)
        except (urllib.error.URLError, OSError, ValueError) as exc:
            notes.append(f"evidence for {sha[:9]} not saved: {exc}")
    return saved, notes


# ---------------------------------------------------------------------- driver

def build_bundle(results_dir, label=None, commit=None, repo_dir=None, gh=None,
                 known_flaky=None, evidence_dir=None, evidence_limit=3,
                 fetch=fetch_commits):
    points = daily_points(results_dir)
    cur = pick_point(points, label, commit)
    label = cur["label"]
    ddir = analyze.results_dir_for(results_dir, label)
    bundle = {"schema": SCHEMA_VERSION, "label": label, "commit": cur["commit"],
              "date": cur["date"], "has_candidates": False, "flagged": [],
              "global_shifts": [], "top_movers": [], "buckets": [], "real_world": [],
              "commit_range": None, "commits": [],
              "candidates": [], "bisect_points": [], "evidence_files": [], "notes": []}
    cpath = os.path.join(ddir, "confirmation.json")
    if not os.path.exists(cpath):
        bundle["notes"].append("no confirmation.json for this night: nothing was flagged, or the "
                               "confirmation step did not run")
        return bundle
    conf = analyze.read_json(cpath)
    plan = conf.get("plan") or {}
    bundle.update({"runner": plan.get("runner", ""), "thresholds": plan.get("thresholds", {}),
                   "baseline_labels": plan.get("baseline_labels", []), "confirmation_status": conf.get("status")})
    n_candidates = len(plan.get("regressions", [])) + len(plan.get("warnings", []))
    bundle["has_candidates"] = n_candidates > 0
    if not n_candidates:
        bundle["notes"].append("the frozen plan has no candidates")
        return bundle

    original_rows = analyze.read_json(analyze.results_path(results_dir, label))
    cur_rows = load_rows(results_dir, label)
    pts = {p["label"]: p for p in points}
    base_by_label = {}
    for bl in bundle["baseline_labels"]:
        if bl in pts:
            base_by_label[bl] = load_rows(results_dir, bl)
        else:
            bundle["notes"].append(f"baseline night {bl} is not on disk")

    flagged, notes = flagged_counters(plan, original_rows, conf.get("repeated", []),
                                      base_by_label, known_flaky or {})
    bundle["flagged"], bundle["notes"] = flagged, bundle["notes"] + notes

    table = shift_table(cur_rows, base_by_label) if len(base_by_label) >= 4 else []
    if not table:
        bundle["notes"].append("fewer than 4 comparable baseline nights: no shift table")
    bundle["global_shifts"] = [t for t in table if t["significant"]]
    bundle["top_movers"] = table[:8]

    if base_by_label:
        bundle["buckets"], bundle["real_world"] = bucket_summary(cur_rows, base_by_label)

    base_pt = resolve_range(pts, cur, bundle["baseline_labels"])
    if base_pt is None:
        bundle["notes"].append("tonight was built from the same commit as every baseline night: "
                               "any difference is measurement variation, not a code change")
        bundle["commit_range"] = {"same_commit": True}
        return bundle
    bundle["commit_range"] = {"same_commit": False, "base_label": base_pt["label"],
                              "base_commit": base_pt["commit"], "head_commit": cur["commit"]}
    commits, notes = fetch(base_pt["commit"], cur["commit"], repo_dir, gh)
    bundle["notes"] += notes
    for c in commits:
        m = PR_RE.search(c["subject"])
        c["pr"] = int(m.group(1)) if m else None
        c["short"] = c["sha"][:9]
        c["files"] = sorted(c["files"], key=lambda f: f["additions"] + f["deletions"],
                            reverse=True)
    story = {f["counter"] for f in flagged if f["outcome"] in ("error", "warning")}
    story |= {t["timer"] for t in bundle["global_shifts"]}
    ranked = score_commits(commits, story)
    bundle["candidates"] = ranked[:8]
    if gh:
        # The git author is a name; the PR author is the GitHub login people use.
        for r in bundle["candidates"]:
            if r["pr"]:
                try:
                    r["pr_author"] = gh.pull(r["pr"])["author"]
                except (urllib.error.URLError, OSError, ValueError):
                    bundle["notes"].append(f"PR author for #{r['pr']} not available")
                    break
    for c in commits:
        c["files"] = c["files"][:MAX_FILES_PER_COMMIT]
    bundle["commits"] = commits
    order = {c["sha"]: i for i, c in enumerate(commits)}
    points_to_build = [base_pt["commit"]] + [r["sha"] for r in sorted(ranked[:6], key=lambda r: order[r["sha"]])]
    if commits and commits[-1]["sha"] not in points_to_build:
        points_to_build.append(commits[-1]["sha"])
    bundle["bisect_points"] = points_to_build
    if evidence_dir and ranked:
        bundle["evidence_files"], notes = save_evidence(evidence_dir, ranked, {c["sha"]: c for c in commits},
                                                        gh, repo_dir, evidence_limit)
        bundle["notes"] += notes
    return bundle


# ------------------------------------------------------------------- rendering

def _f(x, nd=1):
    return "-" if x is None else f"{x:.{nd}f}"


def render_markdown(b):
    out = [f"# Compile-perf triage: {b['label']}", ""]
    out.append(f"commit `{(b.get('commit') or '')[:9]}`, runner `{b.get('runner', '?')}`, "
               f"baseline {len(b.get('baseline_labels', []))} nights")
    out.append("")
    if not b["has_candidates"]:
        out += ["No candidates were flagged for this night.", ""] + [f"- {n}" for n in b["notes"]]
        return "\n".join(out) + "\n"
    out += ["## Flagged counters", "",
            "| workload | counter | alert outcome | baseline | original (med/min) | rerun (med/min) | hint |",
            "|---|---|---|---|---|---|---|"]
    severity = {"error": 0, "warning": 1, "cannot-evaluate": 2, "cleared": 3}
    for f in sorted(b["flagged"], key=lambda f: severity.get(f["outcome"], 9)):
        o, r = f["original"], f["rerun"]
        out.append(f"| {f['workload']} | {f['counter']} | {f['outcome']} | {_f(f.get('baseline_median'))} | "
                   f"{_f(o and o['median'])}/{_f(o and o['min'])} | {_f(r and r['median'])}/{_f(r and r['min'])} | "
                   f"{f['hint']} |")
    details = [f"- `{f['workload']}/{f['counter']}`: {f['hint']} -- {f['hint_detail']}"
               for f in b["flagged"]
               if f["hint_detail"] and f["hint"] in ("known-flaky", "bimodal-history")]
    if details:
        out += [""] + details
    out += ["", "## Timers that shifted across workloads", ""]
    if b["global_shifts"]:
        out += ["| timer | workloads | median ratio | min ratio | share >3% | night-to-night max |",
                "|---|---|---|---|---|---|"]
        for t in b["global_shifts"]:
            out.append(f"| {t['timer']} | {t['n']} | {t['median_ratio']:.3f} | {t['min_ratio']:.3f} | "
                       f"{t['share_over']:.2f} | {_f(t['null_max'], 3)} |")
    else:
        out.append("None clears the night-to-night null.")
    out += ["", "Largest movers regardless of significance:", ""]
    out += [f"- {t['timer']}: {t['median_ratio']:.3f} over {t['n']} workloads"
            f" (null max {_f(t['null_max'], 3)})" for t in b["top_movers"][:5]]
    if b.get("real_world"):
        out += ["", "## Real-world workloads (whole-compile effect)", "",
                "Ratios are from the original sweep only. Check any flagged counter of the same "
                "workload against its rerun before relying on a single-night ratio.", "",
                "| workload | compileInner | wall | front end share | semantic checking share | semantic ratio |",
                "|---|---|---|---|---|---|"]
        for r in b["real_world"]:
            out.append(f"| {r['workload']} | {_f(r['compileInner']['ratio'], 3)} | {_f(r['wall']['ratio'], 3)} | "
                       f"{_f(r['frontEndExecute_share_of_compile'], 2)} | "
                       f"{_f(r['SemanticChecking_share_of_compile'], 2)} | {_f(r['SemanticChecking_ratio'], 3)} |")
    cr = b["commit_range"] or {}
    out += ["", "## Commit range", ""]
    if cr.get("same_commit"):
        out.append("Same commit as every baseline night: a measurement difference, not a code change.")
    elif cr:
        out.append(f"{cr['base_commit'][:9]} ({cr['base_label']}) .. {cr['head_commit'][:9]}: "
                   f"{len(b['commits'])} commits")
        out += ["", "Ordered by changed lines in files that can move the flagged and shifted timers. "
                    "This is a weak signal: line count cannot tell a plain overload from a new "
                    "interface, so the first entry is not necessarily the cause. Build the suggested "
                    "points to attribute a step; do not name a culprit from this list alone.", ""]
        for r in b["candidates"]:
            pr = f"#{r['pr']}" if r["pr"] else r["sha"][:9]
            who = r.get("pr_author") or r["author"]
            comps = ", ".join(f"{k} {v}" for k, v in r["components"].items())
            out.append(f"- {pr} ({who}): {r['subject']} -- {comps}")
        out += ["", "Suggested build points for a bisect: " + ", ".join(s[:9] for s in b["bisect_points"])]
    if b["notes"]:
        out += ["", "## Notes", ""] + [f"- {n}" for n in b["notes"]]
    return "\n".join(out) + "\n"


def main(argv=None):
    for stream in (sys.stdout, sys.stderr):
        if hasattr(stream, "reconfigure"):
            stream.reconfigure(encoding="utf-8", errors="replace")
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--results", required=True, help="perf-results checkout")
    ap.add_argument("--label", help="daily label to analyze (default: newest)")
    ap.add_argument("--commit", help="analyze the newest night built from this commit")
    ap.add_argument("--repo-dir", help="slang checkout used to read commits and diffs")
    ap.add_argument("--github-repo", default="shader-slang/slang",
                    help="repository for the GitHub API fallback ('' disables it)")
    ap.add_argument("--known-flaky", default=os.path.join(HERE, "triage_known_flaky.json"))
    ap.add_argument("--evidence-dir", help="write the top candidates' diffs and PR text here")
    ap.add_argument("--evidence-limit", type=int, default=3)
    ap.add_argument("--out-json")
    ap.add_argument("--out-md")
    args = ap.parse_args(argv)
    gh = GitHub(args.github_repo) if args.github_repo else None
    try:
        bundle = build_bundle(args.results, args.label, args.commit, args.repo_dir, gh,
                              load_known_flaky(args.known_flaky), args.evidence_dir,
                              args.evidence_limit)
    except TriageError as exc:
        print(f"triage: {exc}", file=sys.stderr)
        return 2
    md = render_markdown(bundle)
    if args.out_json:
        with analyze.open_output(args.out_json) as fh:
            json.dump(bundle, fh, indent=2)
    if args.out_md:
        with analyze.open_output(args.out_md) as fh:
            fh.write(md)
    print(md)
    return 0


if __name__ == "__main__":
    sys.exit(main())
