#!/usr/bin/env python3
"""
Analyze which files are most frequently changed in bug-fix PRs.
"""

import json
import re
from collections import Counter, defaultdict
from pathlib import Path

DATA_DIR = Path(__file__).parent / "data"

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

def load_data():
    """Load PR data."""
    with open(DATA_DIR / "pull_requests.json") as f:
        return json.load(f)

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

def get_component_from_file(filename):
    """Extract component from filename."""
    if "slang-emit-spirv" in filename:
        return "spirv-emit"
    elif "slang-emit-dxil" in filename:
        return "dxil-emit"
    elif "slang-emit-cuda" in filename:
        return "cuda-emit"
    elif "slang-emit-metal" in filename:
        return "metal-emit"
    elif "slang-emit-glsl" in filename:
        return "glsl-emit"
    elif "slang-emit-hlsl" in filename:
        return "hlsl-emit"
    elif "slang-emit" in filename:
        return "other-emit"
    elif "slang-lower-to-ir" in filename:
        return "ir-generation"
    elif "slang-ir-inline" in filename:
        return "ir-inlining"
    elif "slang-ir-specialize" in filename:
        return "ir-specialization"
    elif "slang-ir-legalize" in filename:
        return "ir-legalization"
    elif "slang-ir-autodiff" in filename:
        return "ir-autodiff"
    elif "slang-ir" in filename:
        return "ir-passes"
    elif "slang-check" in filename:
        return "semantic-check"
    elif "slang-parser" in filename:
        return "parser"
    elif "slang-type" in filename:
        return "type-system"
    elif "slang-preprocessor" in filename:
        return "preprocessor"
    elif "slang-test" in filename:
        return "test-infrastructure"
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
        "file_type_distribution": Counter(),
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
            
            # Categorize
            file_type = categorize_file(filename)
            analysis["file_type_distribution"][file_type] += 1
            
            # Component
            component = get_component_from_file(filename)
            analysis["component_bugfix_count"][component] += 1
            
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
    print("TOP 30 FILES CHANGED IN BUG FIXES (by frequency)")
    print("-"*70)
    for filename, count in analysis["files_by_bugfix_count"].most_common(30):
        changes = analysis["files_by_changes"][filename]
        component = get_component_from_file(filename)
        print(f"{count:3}x  {changes:5} changes  [{component:20}] {filename}")
    
    print("\n" + "-"*70)
    print("COMPONENTS BY BUG-FIX FREQUENCY")
    print("-"*70)
    for component, count in analysis["component_bugfix_count"].most_common(20):
        print(f"{component:30} {count:4} bug fixes")
    
    print("\n" + "-"*70)
    print("FILE TYPE DISTRIBUTION IN BUG FIXES")
    print("-"*70)
    for file_type, count in analysis["file_type_distribution"].most_common():
        pct = (count / sum(analysis["file_type_distribution"].values()) * 100)
        print(f"{file_type:15} {count:4} files ({pct:5.1f}%)")
    
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
    prs = load_data()
    
    print("Analyzing bug-fix files...")
    analysis, bugfix_prs = analyze_bugfix_files(prs)
    
    print_report(analysis)
    
    print("\n✓ Bug-fix file analysis complete!")

if __name__ == "__main__":
    main()

