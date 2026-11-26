#!/usr/bin/env python3
"""Reliability indicators for Slang crashes, from public GitHub data only.

Joins GitHub issues with release download counts. An issue is a "crash issue"
if its body contains a Slang crash report (report mode, exact) or its title
uses crash wording (keyword mode, approximate).

Data comes from the `gh` CLI (`gh api ...`); no token is read or printed by
this script. With --from-files the script runs offline from saved API output.
"""

import argparse
import csv
import datetime
import json
import re
import subprocess
import sys
from collections import Counter, defaultdict

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

REPORT_HEADER_RE = re.compile(r"^\s*====\s*Slang crash report\b.*====\s*$", re.IGNORECASE)
REPORT_KEY_VALUE_RE = re.compile(r"^\s*([a-z][a-z-]*):\s*(.*?)\s*$")

# Keyword classification, checked in this priority order: crash, internal-error, assert.
CRASH_TITLE_RE = re.compile(
    r"segfault|segmentation fault|sigsegv|access violation|stack overflow|crash",
    re.IGNORECASE,
)
INTERNAL_ERROR_TITLE_RE = re.compile(
    r"(?i:internal (?:compiler )?error)|\bICE\b",
)
ASSERT_TITLE_RE = re.compile(r"assert(?:ion)?", re.IGNORECASE)
KEYWORD_CLASSES = (
    ("crash", CRASH_TITLE_RE),
    ("internal-error", INTERNAL_ERROR_TITLE_RE),
    ("assert", ASSERT_TITLE_RE),
)

PACKAGE_RE = re.compile(r"(linux|windows|macos)-(x86_64|aarch64)(-glibc-\d+(?:\.\d+)*)?")
EXCLUDED_ASSET_MARKERS = ("debug-info",)
EXCLUDED_ASSET_PREFIXES = ("slang-macos-dist-",)

SEARCH_RESULT_CAP = 1000
SEARCH_PAGE_SIZE = 100


# ---------------------------------------------------------------------------
# Pure parsing and joining
# ---------------------------------------------------------------------------


def parse_reports(body):
    """Return a list of dicts, one per crash report found in an issue body.

    Each dict holds the `key: value` pairs of the report's `[report]` section
    (first occurrence of a key wins). Only sections that follow a
    `==== Slang crash report ... ====` header count, so text elsewhere in the
    issue, including other sections such as `[details]`, is never parsed.
    """
    reports = []
    in_report_block = False
    in_report_section = False
    current = None
    for raw in (body or "").splitlines():
        line = raw.rstrip("\r")
        if REPORT_HEADER_RE.match(line):
            in_report_block = True
            in_report_section = False
            current = None
            continue
        if not in_report_block:
            continue
        stripped = line.strip()
        if stripped.startswith("["):
            if stripped == "[report]" and current is None:
                current = {}
                reports.append(current)
                in_report_section = True
            else:
                in_report_section = False
            continue
        if in_report_section and current is not None:
            match = REPORT_KEY_VALUE_RE.match(line)
            if match and match.group(1) not in current:
                current[match.group(1)] = match.group(2)
    return [r for r in reports if r]


def classify_title(title):
    """Return 'crash', 'internal-error' or 'assert' for a crash-like title, else None."""
    for name, regex in KEYWORD_CLASSES:
        if regex.search(title or ""):
            return name
    return None


def package_of_asset(name):
    """Return the package ('linux-x86_64-glibc-2.27', 'wasm', ...) of an asset name, or None."""
    match = PACKAGE_RE.search(name)
    if match:
        return match.group(0)
    if "wasm" in name:
        return "wasm"
    return None


def is_binary_asset(name):
    """Return True if a release asset counts as a binary download."""
    if any(marker in name for marker in EXCLUDED_ASSET_MARKERS):
        return False
    return not name.startswith(EXCLUDED_ASSET_PREFIXES)


def parse_time(text):
    """Return a timezone-aware datetime for an ISO-8601 string such as 2026-08-01T12:00:00Z."""
    return datetime.datetime.fromisoformat(text.replace("Z", "+00:00"))


def normalize_tag(tag):
    """Return a release tag or slang version without any leading 'v'."""
    return (tag or "").strip().lstrip("vV")


def normalize_release(raw):
    """Return a release dict with tag, published (datetime), draft and per-package downloads.

    `downloads` is the sum over binary assets; `by_package` maps package name to
    downloads for assets that map to a package.
    """
    by_package = Counter()
    downloads = 0
    for asset in raw.get("assets") or []:
        name = asset.get("name", "")
        if not is_binary_asset(name):
            continue
        count = int(asset.get("download_count") or 0)
        downloads += count
        package = package_of_asset(name)
        if package:
            by_package[package] += count
    return {
        "tag": raw["tag_name"],
        "published": parse_time(raw["published_at"]) if raw.get("published_at") else None,
        "draft": bool(raw.get("draft")),
        "prerelease": bool(raw.get("prerelease")),
        "downloads": downloads,
        "by_package": dict(by_package),
    }


def normalize_issue(raw):
    """Return an issue dict from either a search-API item or `gh issue list --json` output."""
    author = raw.get("user") or raw.get("author") or {}
    labels = raw.get("labels") or []
    created = raw.get("created_at") or raw.get("createdAt")
    closed = raw.get("closed_at") or raw.get("closedAt")
    return {
        "number": raw["number"],
        "title": raw.get("title") or "",
        "state": (raw.get("state") or "").lower(),
        "created": parse_time(created),
        "closed": parse_time(closed) if closed else None,
        "body": raw.get("body") or "",
        "labels": [l["name"] if isinstance(l, dict) else l for l in labels],
        "author": author.get("login", ""),
        "url": raw.get("html_url") or raw.get("url") or "",
    }


def analyze_issue(issue):
    """Return the issue dict extended with `reports` and `keyword` (class or None)."""
    analyzed = dict(issue)
    analyzed["reports"] = parse_reports(issue["body"])
    analyzed["keyword"] = classify_title(issue["title"])
    return analyzed


def find_release_by_slang(releases, slang_version):
    """Return the release whose tag equals 'v<slang_version>' (a 'v' prefix is optional), or None."""
    wanted = normalize_tag(slang_version)
    for release in releases:
        if normalize_tag(release["tag"]) == wanted:
            return release
    return None


def find_release_by_date(releases, when):
    """Return the latest release published on or before `when`, or None."""
    candidates = [r for r in releases if r["published"] and r["published"] <= when]
    return max(candidates, key=lambda r: r["published"]) if candidates else None


def attribute_issue(issue, releases):
    """Return (tags, how) for a crash issue.

    `how` is 'report' when the issue has crash reports (tags are the distinct
    releases named by the reports' `slang:` fields, in report order; an
    unknown version is kept as its own string) and 'date' otherwise (tags hold
    the latest release published on or before creation, or nothing).
    """
    if issue["reports"]:
        tags = []
        for report in issue["reports"]:
            version = report.get("slang")
            if not version:
                continue
            release = find_release_by_slang(releases, version)
            tag = release["tag"] if release else version
            if tag not in tags:
                tags.append(tag)
        return tags, "report"
    release = find_release_by_date(releases, issue["created"])
    return ([release["tag"]] if release else []), "date"


def issues_per_10k(count, downloads):
    """Return crash issues per 10,000 downloads, or None when downloads is zero."""
    if downloads <= 0:
        return None
    return count * 10000.0 / downloads


def is_crash_issue(issue):
    """Return True if an analyzed issue is a crash issue in either mode."""
    return bool(issue["reports"]) or issue["keyword"] is not None


def build_stats(releases, issues, n_releases):
    """Return the full statistics dict from normalized releases and raw-normalized issues.

    `releases` are normalize_release dicts, `issues` are normalize_issue dicts.
    Drafts are ignored. The latest `n_releases` published releases are shown.
    """
    published = sorted(
        (r for r in releases if not r["draft"] and r["published"]),
        key=lambda r: r["published"],
        reverse=True,
    )
    shown = published[:n_releases]
    shown_tags = {r["tag"] for r in shown}
    crash = [a for a in (analyze_issue(i) for i in issues) if is_crash_issue(a)]

    per_tag = defaultdict(lambda: {"report": 0, "date": 0})
    unattributed = {"report": 0, "date": 0}
    for issue in crash:
        tags, how = attribute_issue(issue, published)
        counted = [t for t in tags if t in shown_tags]
        for tag in counted:
            per_tag[tag][how] += 1
        if not counted:
            unattributed[how] += 1

    release_rows = []
    for release in shown:
        counts = per_tag[release["tag"]]
        total = counts["report"] + counts["date"]
        release_rows.append(
            {
                "tag": release["tag"],
                "published": release["published"].date().isoformat(),
                "downloads": release["downloads"],
                "report_issues": counts["report"],
                "keyword_issues": counts["date"],
                "issues": total,
                "per_10k": issues_per_10k(total, release["downloads"]),
            }
        )

    package_downloads = Counter()
    for release in shown:
        package_downloads.update(release["by_package"])
    package_total = sum(package_downloads.values())
    host_counts = Counter()
    for issue in crash:
        for host in {r.get("host") for r in issue["reports"] if r.get("host")}:
            host_counts[host] += 1
    package_rows = [
        {
            "package": name,
            "downloads": count,
            "download_share": count / package_total if package_total else 0.0,
        }
        for name, count in sorted(package_downloads.items(), key=lambda kv: -kv[1])
    ]
    host_rows = [{"host": h, "issues": c} for h, c in host_counts.most_common()]

    signatures = {}
    for issue in crash:
        seen_in_issue = set()
        for report in issue["reports"]:
            sig = report.get("signature")
            if not sig or sig in seen_in_issue:
                continue
            seen_in_issue.add(sig)
            entry = signatures.setdefault(
                sig, {"signature": sig, "issues": [], "open": 0, "closed": 0, "releases": set()}
            )
            entry["issues"].append(issue["number"])
            entry["open" if issue["state"] == "open" else "closed"] += 1
            if report.get("slang"):
                entry["releases"].add(report["slang"])
    signature_rows = []
    for entry in signatures.values():
        entry["first_seen"] = first_seen_release(entry.pop("releases"), published)
        entry["issues"].sort()
        signature_rows.append(entry)
    signature_rows.sort(key=lambda e: (-len(e["issues"]), e["signature"]))

    by_mode_kind = Counter()
    for issue in crash:
        if issue["reports"]:
            by_mode_kind[("report", issue["reports"][0].get("kind", "unknown"))] += 1
        if issue["keyword"]:
            by_mode_kind[("keyword", issue["keyword"])] += 1
    with_report = sum(1 for i in crash if i["reports"])
    totals = {
        "crash_issues": len(crash),
        "with_report": with_report,
        "report_share": with_report / len(crash) if crash else 0.0,
        "open": sum(1 for i in crash if i["state"] == "open"),
        "closed": sum(1 for i in crash if i["state"] != "open"),
        "by_mode_kind": [
            {"mode": m, "kind": k, "issues": c} for (m, k), c in sorted(by_mode_kind.items())
        ],
    }
    return {
        "releases": release_rows,
        "unattributed": unattributed,
        "hosts": host_rows,
        "packages": package_rows,
        "signatures": [
            {
                "signature": e["signature"],
                "count": len(e["issues"]),
                "first_seen": e["first_seen"],
                "open": e["open"],
                "closed": e["closed"],
                "issues": e["issues"],
            }
            for e in signature_rows
        ],
        "totals": totals,
    }


def first_seen_release(slang_versions, releases):
    """Return the earliest-published release tag among the given slang versions.

    Versions that match no release are used only when none match, and returned as
    given. Returns None when `slang_versions` is empty.
    """
    known = []
    unknown = []
    for version in sorted(slang_versions):
        release = find_release_by_slang(releases, version)
        if release:
            known.append(release)
        else:
            unknown.append(version)
    if known:
        return min(known, key=lambda r: r["published"])["tag"]
    return unknown[0] if unknown else None


# ---------------------------------------------------------------------------
# Formatting
# ---------------------------------------------------------------------------


def format_text(stats, repo, since):
    """Return the human-readable report as a string."""
    out = []
    out.append("Slang crash indicators for %s (issues created on or after %s)" % (repo, since))
    out.append("")
    out.append("== 1. Per release ==")
    out.append(
        "Attribution: 'report' = issue has a crash report, release taken from its slang: field;"
    )
    out.append(
        "'keyword' = title match only, attributed to the latest release published on or before creation."
    )
    out.append(
        "Downloads exclude debug-info and slang-macos-dist-* assets. Issues = report + keyword."
    )
    out.append("")
    header = "%-14s %-10s %10s %7s %8s %7s %10s" % (
        "tag", "published", "downloads", "report", "keyword", "issues", "per 10k dl",
    )
    out.append(header)
    for row in stats["releases"]:
        rate = "n/a" if row["per_10k"] is None else "%.2f" % row["per_10k"]
        out.append(
            "%-14s %-10s %10d %7d %8d %7d %10s"
            % (row["tag"], row["published"], row["downloads"], row["report_issues"],
               row["keyword_issues"], row["issues"], rate)
        )
    un = stats["unattributed"]
    out.append(
        "(not attributed to a shown release: %d by report, %d by keyword/date)"
        % (un["report"], un["date"])
    )
    out.append("")
    out.append("== 2. Per package ==")
    out.append("Report-mode issues by host: field:")
    for row in stats["hosts"] or [{"host": "(none)", "issues": 0}]:
        out.append("  %-28s %5d" % (row["host"], row["issues"]))
    out.append("Download share by package (selected releases, binary assets that map to a package):")
    for row in stats["packages"]:
        out.append(
            "  %-28s %12d %6.1f%%" % (row["package"], row["downloads"], 100 * row["download_share"])
        )
    out.append("")
    out.append("== 3. Per signature (report mode) ==")
    if not stats["signatures"]:
        out.append("  (no crash reports found)")
    else:
        out.append("%-20s %5s %-14s %4s %6s  %s" % ("signature", "count", "first seen", "open", "closed", "issues"))
        for row in stats["signatures"]:
            out.append(
                "%-20s %5d %-14s %4d %6d  %s"
                % (row["signature"], row["count"], row["first_seen"] or "?", row["open"],
                   row["closed"], ", ".join("#%d" % n for n in row["issues"]))
            )
    out.append("")
    out.append("== 4. Totals ==")
    t = stats["totals"]
    out.append("Crash issues (either mode): %d" % t["crash_issues"])
    out.append("Open: %d  Closed: %d" % (t["open"], t["closed"]))
    out.append(
        "Carrying a crash report: %d (%.1f%% of crash issues)" % (t["with_report"], 100 * t["report_share"])
    )
    for row in t["by_mode_kind"]:
        out.append("  %-8s %-16s %5d" % (row["mode"], row["kind"], row["issues"]))
    return "\n".join(out) + "\n"


def write_csv(path, stats):
    """Write the per-release table to `path` as CSV."""
    with open(path, "w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            ["tag", "published", "downloads", "report_issues", "keyword_issues", "issues", "issues_per_10k_downloads"]
        )
        for row in stats["releases"]:
            writer.writerow(
                [row["tag"], row["published"], row["downloads"], row["report_issues"],
                 row["keyword_issues"], row["issues"],
                 "" if row["per_10k"] is None else "%.4f" % row["per_10k"]]
            )


# ---------------------------------------------------------------------------
# Data loading (the only code that calls `gh`)
# ---------------------------------------------------------------------------


def decode_concatenated_json(text):
    """Return a list of the JSON documents concatenated in `text` (as `gh api --paginate` prints)."""
    decoder = json.JSONDecoder()
    docs = []
    pos = 0
    while pos < len(text):
        while pos < len(text) and text[pos].isspace():
            pos += 1
        if pos >= len(text):
            break
        doc, pos = decoder.raw_decode(text, pos)
        docs.append(doc)
    return docs


def run_gh_api(args):
    """Run `gh api <args>` and return the list of JSON documents it printed."""
    try:
        result = subprocess.run(
            ["gh", "api"] + args, capture_output=True, text=True, check=True
        )
    except FileNotFoundError:
        sys.exit("error: the `gh` CLI is not installed or not on PATH")
    except subprocess.CalledProcessError as exc:
        sys.exit("error: `gh api` failed: %s" % exc.stderr.strip())
    return decode_concatenated_json(result.stdout)


def fetch_releases(repo):
    """Return all raw releases of `repo` (a list of API dicts)."""
    releases = []
    for page in run_gh_api(["repos/%s/releases?per_page=100" % repo, "--paginate"]):
        releases.extend(page)
    return releases


def search_issues_total(repo, first, last):
    """Return the search API's total_count of issues created in [first, last] (dates)."""
    query = "repo:%s is:issue created:%s..%s" % (repo, first, last)
    docs = run_gh_api(["-X", "GET", "search/issues", "-f", "q=" + query, "-f", "per_page=1"])
    return docs[0]["total_count"]


def fetch_issue_range(repo, first, last):
    """Return raw issues created in [first, last] (dates), splitting ranges over the search cap.

    The search API returns at most 1000 results per query, so a range whose
    total_count exceeds that is bisected until each part fits; a single day
    over the cap is fetched truncated with a warning on stderr.
    """
    total = search_issues_total(repo, first, last)
    if total > SEARCH_RESULT_CAP:
        if first < last:
            mid = first + (last - first) // 2
            return fetch_issue_range(repo, first, mid) + fetch_issue_range(
                repo, mid + datetime.timedelta(days=1), last
            )
        print("warning: %s has %d issues; only the first %d are fetched" % (first, total, SEARCH_RESULT_CAP),
              file=sys.stderr)
    if total == 0:
        return []
    query = "repo:%s is:issue created:%s..%s" % (repo, first, last)
    items = []
    for page in run_gh_api(
        ["-X", "GET", "search/issues", "-f", "q=" + query, "-f", "per_page=%d" % SEARCH_PAGE_SIZE, "--paginate"]
    ):
        items.extend(page["items"])
    return items


def fetch_issues(repo, since):
    """Return raw issues of `repo` created from `since` (a date) up to today."""
    return fetch_issue_range(repo, since, datetime.date.today())


def load_issue_file(path):
    """Return raw issues from a saved file: a list, or search-API pages with `items`."""
    with open(path, encoding="utf-8") as handle:
        docs = decode_concatenated_json(handle.read())
    issues = []
    for doc in docs:
        if isinstance(doc, dict) and "items" in doc:
            issues.extend(doc["items"])
        else:
            issues.extend(doc)
    return issues


def load_release_file(path):
    """Return raw releases from a saved file (one list, or several concatenated lists)."""
    with open(path, encoding="utf-8") as handle:
        docs = decode_concatenated_json(handle.read())
    releases = []
    for doc in docs:
        releases.extend(doc)
    return releases


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def parse_args(argv):
    """Return the parsed command-line namespace."""
    default_since = datetime.date.today() - datetime.timedelta(days=180)
    parser = argparse.ArgumentParser(
        description="Crash-report reliability indicators from GitHub issues and release downloads."
    )
    parser.add_argument("--repo", default="shader-slang/slang", help="GitHub repository (default: %(default)s)")
    parser.add_argument(
        "--since",
        type=datetime.date.fromisoformat,
        default=default_since,
        metavar="YYYY-MM-DD",
        help="only issues created on or after this date (default: 180 days ago)",
    )
    parser.add_argument("--releases", type=int, default=12, help="number of latest releases to show (default: %(default)s)")
    parser.add_argument("--json", action="store_true", help="print JSON instead of a text table")
    parser.add_argument("--csv", metavar="PATH", help="also write the per-release table to this CSV file")
    parser.add_argument(
        "--from-files",
        nargs=2,
        metavar=("RELEASES.json", "ISSUES.json"),
        help="run offline from saved API output instead of calling gh",
    )
    return parser.parse_args(argv)


def main(argv=None):
    """Run the tool; return the process exit code."""
    args = parse_args(argv)
    if args.from_files:
        raw_releases = load_release_file(args.from_files[0])
        raw_issues = load_issue_file(args.from_files[1])
    else:
        raw_releases = fetch_releases(args.repo)
        raw_issues = fetch_issues(args.repo, args.since)
    since = datetime.datetime.combine(args.since, datetime.time.min, tzinfo=datetime.timezone.utc)
    issues = [i for i in map(normalize_issue, raw_issues) if i["created"] >= since]
    releases = [normalize_release(r) for r in raw_releases]
    stats = build_stats(releases, issues, args.releases)
    if args.json:
        print(json.dumps(stats, indent=2))
    else:
        sys.stdout.write(format_text(stats, args.repo, args.since.isoformat()))
    if args.csv:
        write_csv(args.csv, stats)
    return 0


if __name__ == "__main__":
    sys.exit(main())
