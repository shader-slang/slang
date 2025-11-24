#!/usr/bin/env python3
"""
Analyze which files are most frequently changed in bug-fix PRs.
"""

import re
from collections import Counter, defaultdict

from analyze_common import get_file_loc, get_component_from_file, load_prs

def is_bugfix_pr(pr):
    """Determine if PR is a bug fix based on title and labels."""
    title = pr.get("title", "").lower()
    body = (pr.get("body") or "").lower()
    labels = [label["name"].lower() for label in pr.get("labels", [])]

    # Check labels
    if any("bug" in label for label in labels):
        return True, "labeled_bug"
    if "regression" in labels:
        return True, "regression"

    # Check title for fix keywords
    fix_keywords = [
        r"\bfix\b", r"\bfixed\b", r"\bfixes\b",
        r"\bcrash\b", r"\bice\b",
        r"\bassert", r"\bassertfail",
        r"\bcorrect\b", r"\brepair\b",
        r"\bresolve\b", r"\bresolved\b"
    ]

    for keyword in fix_keywords:
        if re.search(keyword, title):
            return True, "fix_keyword"

    return False, ""

def categorize_file(filename):
    """Categorize file by type."""
    if "test" in filename.lower():
        return "test"
    elif filename.endswith((".h", ".hpp")):
        return "header"
    elif filename.endswith(".cpp"):
        return "source"
    elif filename.endswith(".slang"):
        return "slang_code"
    elif filename.endswith(".md"):
        return "docs"
    else:
        return "other"

def analyze_bugfix_files(prs):
    """Analyze files changed in bug fix PRs."""

    analysis = {
        "total_prs": len(prs),
        "bugfix_prs": 0,
        "bugfix_by_type": Counter(),
        "files_by_bugfix_count": Counter(),
        "files_by_changes": Counter(),
        "component_bugfix_count": Counter(),
        "component_changes": Counter(),  # Track changes per component
        "component_loc": Counter(),  # Track LOC per component
        "file_type_distribution": Counter(),
        "source_by_component": Counter(),  # Track source files by component
        "file_loc": {},  # NEW: Lines of code per file
        "top_changed_per_component": defaultdict(Counter),
    }

    bugfix_pr_list = []

    for pr in prs:
        is_bugfix, bugfix_type = is_bugfix_pr(pr)
        if not is_bugfix:
            continue

        if pr.get("state") != "closed":
            continue  # Only count merged bug fixes

        analysis["bugfix_prs"] += 1
        analysis["bugfix_by_type"][bugfix_type] += 1

        bugfix_pr_list.append({
            "number": pr.get("number"),
            "title": pr.get("title"),
            "type": bugfix_type,
        })

        # Analyze files
        files = pr.get("files_changed", [])
        for file_info in files:
            filename = file_info["filename"]
            changes = file_info.get("changes", 0)

            analysis["files_by_bugfix_count"][filename] += 1
            analysis["files_by_changes"][filename] += changes

            # Get LOC for this file (cache it)
            if filename not in analysis["file_loc"]:
                analysis["file_loc"][filename] = get_file_loc(filename)

            # Categorize
            file_type = categorize_file(filename)
            analysis["file_type_distribution"][file_type] += 1

            # Component
            component = get_component_from_file(filename)
            analysis["component_bugfix_count"][component] += 1
            analysis["component_changes"][component] += changes

            # Add LOC to component total
            loc = analysis["file_loc"].get(filename)
            if loc:
                analysis["component_loc"][component] += loc

            # Track source files by component
            if file_type == "source":
                analysis["source_by_component"][component] += 1

            # Track per-component files
            if file_type in ["source", "header"]:
                analysis["top_changed_per_component"][component][filename] += 1

    return analysis, bugfix_pr_list

def print_report(analysis):
    """Print analysis report."""

    print("\n" + "="*70)
    print("BUG-FIX FILES ANALYSIS")
    print("="*70)

    print(f"\nTotal PRs: {analysis['total_prs']}")
    print(f"Bug-fix PRs (merged): {analysis['bugfix_prs']}")
    print(f"Bug-fix rate: {(analysis['bugfix_prs'] / analysis['total_prs'] * 100):.1f}%")

    print("\n" + "-"*70)
    print("BUG-FIX PR TYPES")
    print("-"*70)
    for bugfix_type, count in analysis["bugfix_by_type"].most_common():
        pct = (count / analysis['bugfix_prs'] * 100)
        print(f"{bugfix_type:20} {count:4} ({pct:5.1f}%)")

    print("\n" + "-"*70)
    print("TOP 40 FILES CHANGED IN BUG FIXES (by frequency)")
    print("-"*70)
    for filename, count in analysis["files_by_bugfix_count"].most_common(40):
        changes = analysis["files_by_changes"][filename]
        component = get_component_from_file(filename)
        print(f"{count:3}x  {changes:5} changes  [{component:20}] {filename}")

    print("\n" + "-"*70)
    print("TOP 40 FILES BY BUG FIX FREQUENCY (bug fix PRs per 1000 LOC) - source/ only")
    print("-"*70)

    # Calculate bug fix frequency for files with known LOC
    bug_density = []
    for filename, bugfix_count in analysis["files_by_bugfix_count"].items():
        loc = analysis["file_loc"].get(filename)
        if loc and loc > 0:
            # Only include source/header files under source/ directory
            file_type = categorize_file(filename)
            if file_type in ["source", "header"] and filename.startswith('source/'):
                density = (bugfix_count / loc) * 1000  # bug fix PRs per 1000 LOC
                bug_density.append((filename, bugfix_count, loc, density))

    # Sort by density (highest first)
    bug_density.sort(key=lambda x: x[3], reverse=True)

    for filename, bugfix_count, loc, density in bug_density[:40]:
        component = get_component_from_file(filename)
        print(f"{density:5.2f}  {bugfix_count:3}x fixes  {loc:6} LOC  [{component:20}] {filename}")

    print("\n" + "-"*70)
    print("ALL COMPONENTS BY BUG-FIX FREQUENCY")
    print("-"*70)
    print(f"{'Component':<30} {'Fixes':>6} {'Changes':>8} {'LOC':>10}")
    print("-"*70)
    for component, count in analysis["component_bugfix_count"].most_common():
        changes = analysis["component_changes"][component]
        loc = analysis["component_loc"][component]
        print(f"{component:<30} {count:6} {changes:8} {loc:10}")

    print("\n" + "-"*70)
    print("FILE TYPE DISTRIBUTION IN BUG FIXES")
    print("-"*70)
    for file_type, count in analysis["file_type_distribution"].most_common():
        pct = (count / sum(analysis["file_type_distribution"].values()) * 100)
        print(f"{file_type:15} {count:4} files ({pct:5.1f}%)")

        # Show second-level breakdown for source files
        if file_type == "source" and analysis["source_by_component"]:
            print(f"  Source files by component (top 15):")
            for component, src_count in analysis["source_by_component"].most_common(15):
                src_pct = (src_count / count * 100)
                print(f"    {component:28} {src_count:4} files ({src_pct:5.1f}%)")

    # Top changed files per critical component
    critical_components = ["spirv-emit", "ir-generation", "semantic-check", "ir-specialization", "type-system"]
    for component in critical_components:
        if component in analysis["top_changed_per_component"]:
            print(f"\n" + "-"*70)
            print(f"TOP FILES IN {component.upper()}")
            print("-"*70)
            for filename, count in analysis["top_changed_per_component"][component].most_common(10):
                print(f"{count:3}x  {filename}")

    print("\n" + "="*70)

def main():
    """Main entry point."""
    print("Loading PR data...")
    prs = load_prs()

    print("Analyzing bug-fix files...")
    analysis, bugfix_prs = analyze_bugfix_files(prs)

    print_report(analysis)

    print("\n✓ Bug-fix file analysis complete!")

if __name__ == "__main__":
    main()

