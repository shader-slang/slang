#!/usr/bin/env python3
"""
Analyze GitHub issues to identify quality gaps in the Slang codebase.
"""

import json
import re
from collections import Counter, defaultdict
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Any, Tuple
import sys

DATA_DIR = Path(__file__).parent / "data"

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

def load_issues() -> List[Dict[str, Any]]:
    """Load issues from JSON file."""
    issues_file = DATA_DIR / "issues.json"
    if not issues_file.exists():
        print(f"Error: {issues_file} not found. Run fetch_github_issues.py first.")
        sys.exit(1)

    with open(issues_file) as f:
        return json.load(f)

def load_prs() -> List[Dict[str, Any]]:
    """Load PRs from JSON file."""
    prs_file = DATA_DIR / "pull_requests.json"
    if not prs_file.exists():
        print(f"Warning: {prs_file} not found. PR analysis will be skipped.")
        return []

    with open(prs_file) as f:
        return json.load(f)

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

def categorize_issues(issues: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Categorize issues by various dimensions."""

    categories = {
        "by_label": Counter(),
        "by_component": Counter(),
        "by_state": Counter(),
        "by_year": Counter(),
        "bugs_by_component": Counter(),
        "open_bugs_by_component": Counter(),
    }

    bug_issues = []
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

        # Identify bugs (Slang repo uses specific bug labels)
        is_bug = any(
            "bug" in label.lower() or
            label.lower() in ["regression", "known_issues"]
            for label in labels
        )
        is_crash = "crash" in combined_text.lower()
        has_error = re.search(r"error|fail|invalid|incorrect", combined_text, re.IGNORECASE)
        is_codegen = any(k in keywords for k in ["spirv", "dxil", "cuda", "metal", "glsl", "hlsl", "wgsl"])

        if is_bug:
            bug_issues.append(issue)
            for keyword in keywords:
                categories["bugs_by_component"][keyword] += 1
                if state == "open":
                    categories["open_bugs_by_component"][keyword] += 1

        if is_crash:
            crash_issues.append(issue)

        if has_error:
            compiler_errors.append(issue)

        if is_codegen:
            codegen_issues.append(issue)

    return {
        "categories": categories,
        "bug_issues": bug_issues,
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
    """Analyze pull requests for quality insights."""
    if not prs:
        return {}

    pr_analysis = {
        "by_state": Counter(),
        "by_component": Counter(),
        "by_year": Counter(),
        "files_by_component": Counter(),
        "most_changed_files": Counter(),
        "test_coverage": {"with_tests": 0, "without_tests": 0},
        "avg_time_to_merge": {},
    }

    component_merge_times = defaultdict(list)
    files_changed_count = []
    all_merge_times = []

    for pr in prs:
        # Basic stats
        state = pr.get("state", "unknown")
        pr_analysis["by_state"][state] += 1

        # Year
        created_at = pr.get("created_at", "")
        if created_at:
            year = created_at[:4]
            pr_analysis["by_year"][year] += 1

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
            else:
                pr_analysis["test_coverage"]["without_tests"] += 1

            # Track file changes
            for file_info in files:
                filename = file_info["filename"]
                pr_analysis["most_changed_files"][filename] += 1

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
    print("SLANG GITHUB ISSUES ANALYSIS - QUALITY GAPS REPORT")
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

    print("\n" + "-"*70)
    print("TOP 15 COMPONENTS BY ISSUE COUNT (Quality Gap Indicators)")
    print("-"*70)
    for component, count in cats["by_component"].most_common(15):
        bugs = cats["bugs_by_component"].get(component, 0)
        open_bugs = cats["open_bugs_by_component"].get(component, 0)
        print(f"{component:25} {count:4} issues  ({bugs:3} bugs, {open_bugs:3} open bugs)")

    print("\n" + "-"*70)
    print("BUGS BY COMPONENT (Areas needing attention)")
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
    print("MOST DISCUSSED OPEN ISSUES (High complexity indicators)")
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
        print("TOP 20 MOST FREQUENTLY CHANGED FILES (Quality Hot Spots)")
        print("-"*70)
        for filename, count in pr_analysis["most_changed_files"].most_common(20):
            print(f"{count:3}x  {filename}")

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

