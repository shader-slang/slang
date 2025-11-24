#!/usr/bin/env python3
"""
Analyze GitHub issues and pull requests.
"""

import re
import json
from collections import Counter, defaultdict
from datetime import datetime
from typing import Dict, List, Any, Tuple
import csv

from analyze_common import get_file_loc, get_component_from_file, load_issues, load_prs, DATA_DIR

# Common source paths in Slang
SOURCE_PATTERNS = {
    "compiler-core": r"source/compiler-core/",
    "slang-core": r"source/slang/",
    "slang-ir": r"source/slang/slang-ir-.*\.cpp",
    "slang-emit": r"source/slang/slang-emit-.*\.cpp",
    "slang-check": r"source/slang/slang-check.*\.cpp",
    "parser": r"source/slang/slang-parser\.cpp",
    "lexer": r"source/compiler-core/slang-lexer\.cpp",
    "preprocessor": r"source/slang/slang-preprocessor\.cpp",
    "spirv": r"spirv|SPIRV|spir-v|SPIR-V",
    "dxil": r"dxil|DXIL|DirectX",
    "cuda": r"cuda|CUDA",
    "metal": r"metal|Metal|MSL",
    "glsl": r"glsl|GLSL",
    "hlsl": r"hlsl|HLSL",
    "wgsl": r"wgsl|WGSL|WebGPU",
    "autodiff": r"autodiff|auto-diff|differentiation",
    "generics": r"generic|template",
    "cooperative-matrix": r"cooperative.matrix|CooperativeMatrix",
}

def extract_keywords(text: str) -> List[str]:
    """Extract relevant keywords from issue text."""
    if not text:
        return []

    keywords = []
    text_lower = text.lower()

    # Check for each pattern
    for category, pattern in SOURCE_PATTERNS.items():
        if re.search(pattern, text, re.IGNORECASE):
            keywords.append(category)

    return keywords

def is_feature(item: Dict[str, Any]) -> bool:
    """Determine if an issue/PR is a feature addition."""
    title = item.get("title", "").lower()
    body = (item.get("body") or "").lower()
    labels = [label["name"].lower() for label in item.get("labels", [])]

    # Check labels first (most reliable)
    feature_labels = ["feature", "enhancement", "new feature", "feature request"]
    if any(label in feature_labels for label in labels):
        return True

    # Check title/body for feature keywords
    feature_patterns = [
        r"\b(add|implement|introduce|support|enable)\s+(new\s+)?feature",
        r"\benhancement\b",
        r"\bnew\s+(functionality|capability|support|API|feature)",
        r"\bimplement\s+(support\s+for|new)",
        r"\bintroduce\s+",
    ]

    combined = f"{title} {body}"
    for pattern in feature_patterns:
        if re.search(pattern, combined, re.IGNORECASE):
            return True

    return False

def is_bug_fix(item: Dict[str, Any]) -> bool:
    """Determine if an issue/PR is a bug fix."""
    title = item.get("title", "").lower()
    body = (item.get("body") or "").lower()
    labels = [label["name"].lower() for label in item.get("labels", [])]

    # Check labels for bugs (including infrastructure bugs like CI Bug)
    bug_labels = ["regression", "known_issues"]
    if any(bug_label in label for label in labels for bug_label in bug_labels):
        return True

    # "bug" in label (includes "CI Bug", "Vendor Driver Bug", etc.)
    for label in labels:
        # Match any label with "bug" but exclude "GoodFirstBug" (that's a difficulty marker, not bug type)
        if "bug" in label and "goodfirstbug" not in label:
            return True

    # Check title/body for bug fix patterns (IMPROVED!)
    # Primary pattern: Look for "bug" or "fix" combinations
    combined = f"{title} {body}"

    # Pattern 1: Explicit bug mentions
    bug_patterns = [
        r"\bbugfix\b",                    # "bugfix" as single word
        r"\bbug[\s-]fix",                 # "bug fix" or "bug-fix"
        r"\bfix\b.*\bbug\b",              # "fix ... bug" anywhere in text
        r"\bbug\b.*\bfix",                # "bug ... fix" anywhere in text
    ]

    for pattern in bug_patterns:
        if re.search(pattern, combined, re.IGNORECASE):
            return True

    # Pattern 2: References to issue numbers (usually bug fixes)
    issue_ref_patterns = [
        r"\bfix(es|ed|ing)?\s+#\d+",      # "fix #123", "fixes #456"
        r"\bresolve(s|d)?\s+#\d+",        # "resolve #123", "resolved #456"
        r"\bclose(s|d)?\s+#\d+",          # "close #123", "closes #456"
    ]

    for pattern in issue_ref_patterns:
        if re.search(pattern, combined, re.IGNORECASE):
            return True

    # Pattern 2b: "Fix X" at start of title (common bug fix pattern)
    # But exclude obvious features: "Fix formatting", "Fix typo", "Fix comment"
    title_only = title.strip()
    if re.match(r"^fix(es|ed|ing)?\s+", title_only, re.IGNORECASE):
        # Exclude non-bug fixes
        non_bugs = [r"typo", r"comment", r"formatting", r"whitespace", r"style",
                    r"documentation", r"readme", r"license"]
        if not any(re.search(nb, title_only, re.IGNORECASE) for nb in non_bugs):
            return True

    # Pattern 3: Critical error keywords (these are always bugs)
    critical_patterns = [
        r"\bcrash(es|ed|ing)?\b",         # crash, crashes, crashing
        r"\bsegfault",                    # segmentation fault
        r"\bsegmentation\s+fault",        # segmentation fault
        r"\bassert(ion)?\s+fail",         # assertion fail, assertion failed
        r"\binternal\s+compiler\s+error", # ICE long form
        r"\bICE\b",                       # ICE abbreviation
        r"\bnull\s+pointer",              # null pointer issues
        r"\bmemory\s+leak",               # memory leaks
        r"\buse[\s-]after[\s-]free",      # use-after-free
        r"\binfinite\s+loop",             # infinite loops
        r"\bhang(s|ing)?\b",              # hangs, hanging
    ]

    for pattern in critical_patterns:
        if re.search(pattern, combined, re.IGNORECASE):
            return True

    # Pattern 4: Correctness issues (these are bugs)
    correctness_patterns = [
        r"\bincorrect\s+(output|code|behavior|result|codegen)",
        r"\binvalid\s+(code|output|spirv|hlsl|glsl)",
        r"\bwrong\s+(output|code|result)",
        r"\bvalidation\s+(error|fail)",
        r"\bmiscompil",                   # miscompile, miscompilation
    ]

    for pattern in correctness_patterns:
        if re.search(pattern, combined, re.IGNORECASE):
            return True

    return False

def categorize_issues(issues: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Categorize issues by various dimensions."""

    categories = {
        "by_label": Counter(),
        "by_component": Counter(),
        "by_state": Counter(),
        "by_year": Counter(),
        "by_type": {"features": 0, "bugs": 0, "other": 0},  # NEW: Issue type breakdown
        "bugs_by_component": Counter(),
        "open_bugs_by_component": Counter(),
    }

    bug_issues = []
    feature_issues = []  # NEW
    crash_issues = []
    compiler_errors = []
    codegen_issues = []

    for issue in issues:
        # Basic categorization
        state = issue.get("state", "unknown")
        categories["by_state"][state] += 1

        # Year
        created_at = issue.get("created_at", "")
        if created_at:
            year = created_at[:4]
            categories["by_year"][year] += 1

        # Labels
        labels = [label["name"] for label in issue.get("labels", [])]
        for label in labels:
            categories["by_label"][label] += 1

        # Extract component from title and body
        title = issue.get("title", "")
        body = issue.get("body", "") or ""
        combined_text = f"{title} {body}"

        keywords = extract_keywords(combined_text)
        for keyword in keywords:
            categories["by_component"][keyword] += 1

        # Classify issue type (NEW!)
        is_feat = is_feature(issue)
        is_bug_issue = is_bug_fix(issue)

        if is_feat:
            issue_type = "features"
            feature_issues.append(issue)
        elif is_bug_issue:
            issue_type = "bugs"
            bug_issues.append(issue)
            # Track bugs by component
            for keyword in keywords:
                categories["bugs_by_component"][keyword] += 1
                if state == "open":
                    categories["open_bugs_by_component"][keyword] += 1
        else:
            issue_type = "other"

        categories["by_type"][issue_type] += 1

        # Additional classifications
        is_crash = "crash" in combined_text.lower()
        has_error = re.search(r"error|fail|invalid|incorrect", combined_text, re.IGNORECASE)
        is_codegen = any(k in keywords for k in ["spirv", "dxil", "cuda", "metal", "glsl", "hlsl", "wgsl"])

        if is_crash:
            crash_issues.append(issue)

        if has_error:
            compiler_errors.append(issue)

        if is_codegen:
            codegen_issues.append(issue)

    return {
        "categories": categories,
        "bug_issues": bug_issues,
        "feature_issues": feature_issues,  # NEW
        "crash_issues": crash_issues,
        "compiler_errors": compiler_errors,
        "codegen_issues": codegen_issues,
    }

def analyze_time_to_close(issues: List[Dict[str, Any]]) -> Dict[str, float]:
    """Calculate average time to close issues by component."""
    component_times = defaultdict(list)

    for issue in issues:
        if issue.get("state") != "closed":
            continue

        created_at = datetime.fromisoformat(issue["created_at"].replace("Z", "+00:00"))
        closed_at = datetime.fromisoformat(issue["closed_at"].replace("Z", "+00:00"))
        days_to_close = (closed_at - created_at).days

        title = issue.get("title", "")
        body = issue.get("body", "") or ""
        keywords = extract_keywords(f"{title} {body}")

        for keyword in keywords:
            component_times[keyword].append(days_to_close)

    avg_times = {}
    for component, times in component_times.items():
        if times:
            avg_times[component] = sum(times) / len(times)

    return avg_times

def analyze_prs(prs: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Analyze pull requests."""
    if not prs:
        return {}

    pr_analysis = {
        "by_state": Counter(),
        "by_component": Counter(),
        "by_year": Counter(),
        "by_type": {"bug_fixes": 0, "other": 0},  # PR type breakdown: bugs vs other
        "files_by_component": Counter(),
        "most_changed_files": Counter(),
        "bugfix_files": Counter(),  # Track files changed in bug fix PRs
        "file_loc": {},  # Lines of code per file
        "test_coverage": {"with_tests": 0, "without_tests": 0},
        "test_coverage_by_type": {  # Test coverage per type
            "bug_fixes": {"with_tests": 0, "without_tests": 0},
            "other": {"with_tests": 0, "without_tests": 0},
        },
        "avg_time_to_merge": {},
    }

    component_merge_times = defaultdict(list)
    files_changed_count = []
    all_merge_times = []

    bug_fix_prs = []
    other_prs = []

    for pr in prs:
        # Basic stats
        state = pr.get("state", "unknown")
        pr_analysis["by_state"][state] += 1

        # Year
        created_at = pr.get("created_at", "")
        if created_at:
            year = created_at[:4]
            pr_analysis["by_year"][year] += 1

        # Classify PR type: bug fix or other
        is_bug = is_bug_fix(pr)

        if is_bug:
            pr_type = "bug_fixes"
            bug_fix_prs.append(pr)
        else:
            pr_type = "other"
            other_prs.append(pr)

        pr_analysis["by_type"][pr_type] += 1

        # Extract components from title and body
        title = pr.get("title", "")
        body = pr.get("body", "") or ""
        combined = f"{title} {body}"
        keywords = extract_keywords(combined)

        for keyword in keywords:
            pr_analysis["by_component"][keyword] += 1

        # Analyze files if available
        files = pr.get("files_changed", [])
        if files:
            files_changed_count.append(len(files))

            # Check for test files
            has_test = any("test" in f["filename"].lower() for f in files)
            if has_test:
                pr_analysis["test_coverage"]["with_tests"] += 1
                pr_analysis["test_coverage_by_type"][pr_type]["with_tests"] += 1
            else:
                pr_analysis["test_coverage"]["without_tests"] += 1
                pr_analysis["test_coverage_by_type"][pr_type]["without_tests"] += 1

            # Track file changes
            for file_info in files:
                filename = file_info["filename"]
                pr_analysis["most_changed_files"][filename] += 1

                # Track bug fix files specifically
                if is_bug:
                    pr_analysis["bugfix_files"][filename] += 1

                    # Get LOC for this file (cache it)
                    if filename not in pr_analysis["file_loc"]:
                        pr_analysis["file_loc"][filename] = get_file_loc(filename)

                # Categorize by component
                for keyword in keywords:
                    pr_analysis["files_by_component"][keyword] += 1

        # Time to merge for closed PRs (use closed_at as proxy for merged_at)
        # Note: /issues API doesn't include merged_at, so we use closed_at for closed PRs
        if pr.get("state") == "closed" and pr.get("closed_at") and pr.get("created_at"):
            try:
                created = datetime.fromisoformat(pr["created_at"].replace("Z", "+00:00"))
                closed = datetime.fromisoformat(pr["closed_at"].replace("Z", "+00:00"))
                days_to_close = (closed - created).days

                all_merge_times.append(days_to_close)

                # Track by component
                for keyword in keywords:
                    component_merge_times[keyword].append(days_to_close)
            except (ValueError, TypeError):
                pass  # Skip if date parsing fails

    # Calculate average merge times
    for component, times in component_merge_times.items():
        if times:
            pr_analysis["avg_time_to_merge"][component] = sum(times) / len(times)

    # Overall average merge time
    if all_merge_times:
        pr_analysis["overall_avg_merge_time"] = sum(all_merge_times) / len(all_merge_times)
        pr_analysis["median_merge_time"] = sorted(all_merge_times)[len(all_merge_times) // 2]
        pr_analysis["merged_pr_count"] = len(all_merge_times)
    else:
        pr_analysis["overall_avg_merge_time"] = 0
        pr_analysis["median_merge_time"] = 0
        pr_analysis["merged_pr_count"] = 0

    # Average files changed per PR
    if files_changed_count:
        pr_analysis["avg_files_per_pr"] = sum(files_changed_count) / len(files_changed_count)
    else:
        pr_analysis["avg_files_per_pr"] = 0

    return pr_analysis

def print_report(analysis: Dict[str, Any], issues: List[Dict[str, Any]]):
    """Print analysis report."""
    cats = analysis["categories"]

    print("\n" + "="*70)
    print("SLANG GITHUB ISSUES AND PULL REQUESTS ANALYSIS")
    print("="*70)

    # Load metadata
    metadata_file = DATA_DIR / "metadata.json"
    if metadata_file.exists():
        with open(metadata_file) as f:
            metadata = json.load(f)
            print(f"\nData fetched: {metadata['fetched_at']}")
            print(f"Repository: {metadata['repo']}")

    print(f"\nTotal issues analyzed: {len(issues)}")
    print(f"Open issues: {cats['by_state']['open']}")
    print(f"Closed issues: {cats['by_state']['closed']}")

    # Issue Type Breakdown (NEW!)
    print("\n" + "-"*70)
    print("ISSUE TYPE BREAKDOWN")
    print("-"*70)
    by_type = cats.get("by_type", {})
    total_typed = sum(by_type.values())
    if total_typed > 0:
        feat_count = by_type.get("features", 0)
        bug_count = by_type.get("bugs", 0)
        other_count = by_type.get("other", 0)

        feat_pct = (feat_count / total_typed) * 100
        bug_pct = (bug_count / total_typed) * 100
        other_pct = (other_count / total_typed) * 100

        print(f"Feature Requests:  {feat_count:4} issues  ({feat_pct:5.1f}%)")
        print(f"Bug Reports:       {bug_count:4} issues  ({bug_pct:5.1f}%)")
        print(f"Other:             {other_count:4} issues  ({other_pct:5.1f}%)")
        print(f"\nBug report rate:   {bug_pct:.1f}% of all issues")
        print(f"Feature req rate:  {feat_pct:.1f}% of all issues")

    print("\n" + "-"*70)
    print("TOP 15 COMPONENTS BY ISSUE COUNT")
    print("-"*70)
    for component, count in cats["by_component"].most_common(15):
        bugs = cats["bugs_by_component"].get(component, 0)
        open_bugs = cats["open_bugs_by_component"].get(component, 0)
        print(f"{component:25} {count:4} issues  ({bugs:3} bugs, {open_bugs:3} open bugs)")

    print("\n" + "-"*70)
    print("BUGS BY COMPONENT")
    print("-"*70)
    for component, count in cats["bugs_by_component"].most_common(15):
        open_count = cats["open_bugs_by_component"].get(component, 0)
        closed_count = count - open_count
        print(f"{component:25} {count:4} bugs total  ({open_count:3} open, {closed_count:3} closed)")

    print("\n" + "-"*70)
    print("TOP LABELS")
    print("-"*70)
    for label, count in cats["by_label"].most_common(15):
        print(f"{label:35} {count:4}")

    print("\n" + "-"*70)
    print("ISSUES BY YEAR")
    print("-"*70)
    for year, count in sorted(cats["by_year"].items()):
        print(f"{year:10} {count:4}")

    print("\n" + "-"*70)
    print("CRITICAL ISSUES")
    print("-"*70)
    print(f"Crash issues: {len(analysis['crash_issues'])}")
    print(f"Compiler errors: {len(analysis['compiler_errors'])}")
    print(f"Code generation issues: {len(analysis['codegen_issues'])}")

    # Time to close analysis
    print("\n" + "-"*70)
    print("AVERAGE TIME TO CLOSE (days) BY COMPONENT")
    print("-"*70)
    time_to_close = analyze_time_to_close(issues)
    for component, avg_days in sorted(time_to_close.items(), key=lambda x: x[1], reverse=True)[:15]:
        issue_count = cats["by_component"].get(component, 0)
        print(f"{component:25} {avg_days:6.1f} days  ({issue_count} issues)")

    # Most commented issues (indicates complexity/difficulty)
    print("\n" + "-"*70)
    print("MOST DISCUSSED OPEN ISSUES")
    print("-"*70)
    open_issues = [i for i in issues if i.get("state") == "open"]
    top_commented = sorted(open_issues, key=lambda x: x.get("comments", 0), reverse=True)[:10]
    for issue in top_commented:
        print(f"#{issue['number']:5} ({issue.get('comments', 0):3} comments) {issue['title'][:60]}")

    print("\n" + "="*70)

def print_pr_report(pr_analysis: Dict[str, Any], prs: List[Dict[str, Any]]):
    """Print PR analysis report."""
    if not pr_analysis:
        print("\nNo PR data available. Run with --pr-files to get detailed PR analysis.")
        return

    print("\n" + "="*70)
    print("PULL REQUEST ANALYSIS")
    print("="*70)

    print(f"\nTotal PRs analyzed: {len(prs)}")
    print(f"Merged PRs: {pr_analysis['by_state'].get('closed', 0)}")
    print(f"Open PRs: {pr_analysis['by_state'].get('open', 0)}")

    # PR Type Breakdown
    print("\n" + "-"*70)
    print("PR TYPE BREAKDOWN")
    print("-"*70)
    by_type = pr_analysis.get("by_type", {})
    total_classified = sum(by_type.values())
    if total_classified > 0:
        bug_count = by_type.get("bug_fixes", 0)
        other_count = by_type.get("other", 0)

        bug_pct = (bug_count / total_classified) * 100
        other_pct = (other_count / total_classified) * 100

        print(f"Bug Fixes:         {bug_count:4} PRs  ({bug_pct:5.1f}%)")
        print(f"Other:             {other_count:4} PRs  ({other_pct:5.1f}%)")
        print(f"\nBug fix rate:      {bug_pct:.1f}% of all PRs")

        # Test coverage by type
        print("\n" + "-"*70)
        print("TEST COVERAGE BY PR TYPE")
        print("-"*70)
        test_by_type = pr_analysis.get("test_coverage_by_type", {})
        for pr_type in ["bug_fixes", "other"]:
            type_name = pr_type.replace("_", " ").title()
            with_tests = test_by_type.get(pr_type, {}).get("with_tests", 0)
            without_tests = test_by_type.get(pr_type, {}).get("without_tests", 0)
            total = with_tests + without_tests
            if total > 0:
                pct = (with_tests / total) * 100
                print(f"{type_name:15} {with_tests:4} / {total:4} ({pct:5.1f}% with tests)")

    # Merge time stats
    if pr_analysis.get("overall_avg_merge_time"):
        print(f"\nAverage time to close PR: {pr_analysis['overall_avg_merge_time']:.1f} days")
        print(f"Median time to close PR: {pr_analysis['median_merge_time']:.1f} days")
        print(f"(Based on {pr_analysis['merged_pr_count']} closed PRs)")

    # File change stats
    if pr_analysis.get("avg_files_per_pr"):
        print(f"\nAverage files changed per PR: {pr_analysis['avg_files_per_pr']:.1f}")

    # Test coverage
    test_cov = pr_analysis.get("test_coverage", {})
    total_with_files = test_cov.get("with_tests", 0) + test_cov.get("without_tests", 0)
    if total_with_files > 0:
        test_pct = (test_cov.get("with_tests", 0) / total_with_files) * 100
        print(f"\nPRs with test files: {test_cov.get('with_tests', 0)} / {total_with_files} ({test_pct:.1f}%)")

    print("\n" + "-"*70)
    print("TOP 15 COMPONENTS BY PR COUNT")
    print("-"*70)
    for component, count in pr_analysis["by_component"].most_common(15):
        avg_close = pr_analysis["avg_time_to_merge"].get(component)
        if avg_close is not None:
            print(f"{component:25} {count:4} PRs  (avg {avg_close:.1f} days to close)")
        else:
            print(f"{component:25} {count:4} PRs")

    if pr_analysis["avg_time_to_merge"]:
        print("\n" + "-"*70)
        print("AVERAGE TIME TO CLOSE PR (days) BY COMPONENT")
        print("-"*70)
        for component, avg_days in sorted(pr_analysis["avg_time_to_merge"].items(),
                                           key=lambda x: x[1], reverse=True)[:15]:
            pr_count = pr_analysis["by_component"].get(component, 0)
            print(f"{component:25} {avg_days:6.1f} days  ({pr_count} PRs)")

    # Most changed files (if available)
    if pr_analysis["most_changed_files"]:
        print("\n" + "-"*70)
        print("TOP 40 MOST FREQUENTLY CHANGED FILES (Hot Spots)")
        print("-"*70)
        for filename, count in pr_analysis["most_changed_files"].most_common(40):
            print(f"{count:3}x  {filename}")

    # Bug fix frequency analysis
    if pr_analysis.get("bugfix_files") and pr_analysis.get("file_loc"):
        print("\n" + "-"*70)
        print("TOP 40 FILES BY BUG FIX FREQUENCY (bug fix PRs per 1000 LOC) - source/ only")
        print("-"*70)

        # Calculate bug fix frequency for files with known LOC
        bug_density = []
        for filename, bugfix_count in pr_analysis["bugfix_files"].items():
            loc = pr_analysis["file_loc"].get(filename)
            if loc and loc > 0:
                # Only include source files under source/ directory
                if filename.startswith('source/') and filename.endswith(('.cpp', '.h', '.hpp', '.c')):
                    density = (bugfix_count / loc) * 1000  # bug fix PRs per 1000 LOC
                    bug_density.append((filename, bugfix_count, loc, density))

        # Sort by density (highest first)
        bug_density.sort(key=lambda x: x[3], reverse=True)

        for filename, bugfix_count, loc, density in bug_density[:40]:
            print(f"{density:5.2f}  {bugfix_count:3}x fixes  {loc:6} LOC  {filename}")

    print("\n" + "-"*70)
    print("PRs BY YEAR")
    print("-"*70)
    for year, count in sorted(pr_analysis["by_year"].items()):
        print(f"{year:10} {count:4}")

    print("\n" + "="*70)

def export_detailed_csv(analysis: Dict[str, Any], issues: List[Dict[str, Any]]):
    """Export detailed data to CSV for further analysis."""
    import csv

    output_file = DATA_DIR / "issues_detailed.csv"
    with open(output_file, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)

        # Check if issues have PR relationships
        has_pr_links = any("related_prs" in issue for issue in issues)

        header = [
            "number", "state", "title", "created_at", "closed_at",
            "comments", "labels", "components", "is_bug", "is_crash"
        ]
        if has_pr_links:
            header.append("related_prs")

        writer.writerow(header)

        for issue in issues:
            title = issue.get("title", "")
            body = issue.get("body", "") or ""
            combined = f"{title} {body}"

            keywords = extract_keywords(combined)
            labels = [label["name"] for label in issue.get("labels", [])]
            is_bug = any(
                "bug" in label.lower() or
                label.lower() in ["regression", "known_issues"]
                for label in labels
            )
            is_crash = "crash" in combined.lower()

            row = [
                issue.get("number", ""),
                issue.get("state", ""),
                title,
                issue.get("created_at", ""),
                issue.get("closed_at", ""),
                issue.get("comments", 0),
                "|".join(labels),
                "|".join(keywords),
                is_bug,
                is_crash
            ]

            if has_pr_links:
                related_prs = issue.get("related_prs", [])
                row.append("|".join(str(pr) for pr in related_prs))

            writer.writerow(row)

    print(f"\nDetailed CSV exported to: {output_file}")

def main():
    """Main entry point."""
    print("Loading issues...")
    issues = load_issues()

    print("Loading PRs...")
    prs = load_prs()

    print("Analyzing issues...")
    analysis = categorize_issues(issues)

    print_report(analysis, issues)

    if prs:
        print("\nAnalyzing PRs...")
        pr_analysis = analyze_prs(prs)
        print_pr_report(pr_analysis, prs)

    export_detailed_csv(analysis, issues)

    print("\n✓ Analysis complete!")

if __name__ == "__main__":
    main()

