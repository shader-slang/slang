#!/usr/bin/env python3
"""
Deep analysis of critical issues: crashes, compiler errors, and their root causes.
"""

import json
import re
from collections import Counter, defaultdict
from pathlib import Path
from typing import Dict, List, Any, Tuple

DATA_DIR = Path(__file__).parent / "data"

# Patterns to identify issue types
CRASH_PATTERNS = [
    r"crash",
    r"segfault",
    r"segmentation fault",
    r"access violation",
    r"assertion.*failed",
    r"abort",
    r"core dump",
    r"fatal error",
]

ERROR_PATTERNS = {
    "ice": r"internal compiler error|ICE",
    "assertion": r"assertion.*failed|assert\(",
    "access_violation": r"access violation|segmentation fault|segfault",
    "null_pointer": r"null.*pointer|nullptr|null reference",
    "stack_overflow": r"stack overflow",
    "memory": r"out of memory|memory allocation failed",
    "infinite_loop": r"infinite loop|hangs|freezes",
    "validation": r"validation.*fail|invalid.*spirv|spirv.*validation",
    "link_error": r"link.*error|unresolved.*symbol",
    "codegen": r"incorrect.*code|wrong.*code|bad.*codegen",
}

COMPONENT_PATTERNS = {
    "spirv-emit": r"source/slang/slang-emit-spirv",
    "dxil-emit": r"source/slang/slang-emit-dxil",
    "cuda-emit": r"source/slang/slang-emit-cuda",
    "metal-emit": r"source/slang/slang-emit-metal",
    "glsl-emit": r"source/slang/slang-emit-glsl",
    "hlsl-emit": r"source/slang/slang-emit-hlsl",
    "ir-generation": r"slang-lower-to-ir\.cpp",
    "ir-inlining": r"slang-ir-inline",
    "ir-specialization": r"slang-ir-specialize",
    "ir-legalization": r"slang-ir-legalize",
    "ir-autodiff": r"slang-ir-autodiff",
    "semantic-check": r"slang-check",
    "parser": r"slang-parser\.cpp",
    "preprocessor": r"slang-preprocessor\.cpp",
    "type-system": r"slang-type",
}

def load_data():
    """Load issues and PRs."""
    with open(DATA_DIR / "issues.json") as f:
        issues = json.load(f)
    
    try:
        with open(DATA_DIR / "pull_requests.json") as f:
            prs = json.load(f)
    except FileNotFoundError:
        prs = []
    
    return issues, prs

def is_critical_issue(issue: Dict[str, Any]) -> Tuple[bool, str]:
    """Check if issue is critical and return the type."""
    title = issue.get("title", "").lower()
    body = (issue.get("body") or "").lower()
    combined = f"{title} {body}"
    
    # Check for crashes
    for pattern in CRASH_PATTERNS:
        if re.search(pattern, combined, re.IGNORECASE):
            return True, "crash"
    
    # Check for other critical errors
    for error_type, pattern in ERROR_PATTERNS.items():
        if re.search(pattern, combined, re.IGNORECASE):
            return True, error_type
    
    # Check labels
    labels = [label["name"].lower() for label in issue.get("labels", [])]
    if any("bug" in label for label in labels):
        return True, "bug"
    
    return False, ""

def extract_component_from_text(text: str) -> List[str]:
    """Extract component mentions from text."""
    components = []
    for component, pattern in COMPONENT_PATTERNS.items():
        if re.search(pattern, text, re.IGNORECASE):
            components.append(component)
    return components

def extract_error_messages(body: str) -> List[str]:
    """Extract error messages from issue body."""
    if not body:
        return []
    
    errors = []
    # Look for code blocks with errors
    code_blocks = re.findall(r"```[\s\S]*?```", body)
    for block in code_blocks:
        # Look for error-like lines
        for line in block.split('\n'):
            if re.search(r"error|Error|ERROR|fail|Fail|FAIL", line):
                errors.append(line.strip())
    
    # Look for quoted errors
    quoted = re.findall(r'`[^`]*(?:error|fail)[^`]*`', body, re.IGNORECASE)
    errors.extend(quoted)
    
    return errors[:5]  # Limit to first 5 errors

def is_critical_pr(pr: Dict[str, Any]) -> Tuple[bool, str]:
    """Check if PR is fixing a critical issue."""
    title = pr.get("title", "").lower()
    body = (pr.get("body") or "").lower()
    combined = f"{title} {body}"
    
    # Check for crash fixes
    for pattern in CRASH_PATTERNS:
        if re.search(pattern, combined, re.IGNORECASE):
            return True, "crash_fix"
    
    # Check for ICE fixes
    if re.search(r"ice|internal compiler error", combined, re.IGNORECASE):
        return True, "ice_fix"
    
    # Check for validation fixes
    if re.search(r"validation|invalid.*spirv", combined, re.IGNORECASE):
        return True, "validation_fix"
    
    # Check for assertion fixes
    if re.search(r"assertion.*fail", combined, re.IGNORECASE):
        return True, "assertion_fix"
    
    return False, ""

def analyze_critical_issues(issues: List[Dict[str, Any]], prs: List[Dict[str, Any]]) -> Dict[str, Any]:
    """Analyze critical issues in detail."""
    
    analysis = {
        "by_type": Counter(),
        "by_component": Counter(),
        "by_state": Counter(),
        "by_year": Counter(),
        "open_critical": [],
        "error_patterns": Counter(),
        "root_cause_components": Counter(),
        "critical_with_prs": 0,
        "critical_without_fix": 0,
    }
    
    critical_issues = []
    
    for issue in issues:
        is_crit, crit_type = is_critical_issue(issue)
        if not is_crit:
            continue
        
        critical_issues.append(issue)
        
        # Categorize
        analysis["by_type"][crit_type] += 1
        analysis["by_state"][issue.get("state", "unknown")] += 1
        
        # Year
        created = issue.get("created_at", "")
        if created:
            year = created[:4]
            analysis["by_year"][year] += 1
        
        # Extract components from title and body
        title = issue.get("title", "")
        body = issue.get("body", "") or ""
        combined = f"{title}\n{body}"
        
        components = extract_component_from_text(combined)
        for comp in components:
            analysis["by_component"][comp] += 1
            analysis["root_cause_components"][comp] += 1
        
        # Track open critical issues
        if issue.get("state") == "open":
            analysis["open_critical"].append({
                "number": issue.get("number"),
                "title": issue.get("title"),
                "type": crit_type,
                "comments": issue.get("comments", 0),
                "created_at": issue.get("created_at"),
                "labels": [l["name"] for l in issue.get("labels", [])],
                "components": components,
            })
        
        # Check if has related PRs
        if issue.get("related_prs"):
            analysis["critical_with_prs"] += 1
        elif issue.get("state") == "closed":
            analysis["critical_without_fix"] += 1
    
    # Analyze files involved in critical bug fixes from PRs
    critical_bug_files = Counter()
    critical_bug_files_by_changes = Counter()
    critical_pr_count = 0
    
    for pr in prs:
        if pr.get("state") != "closed":
            continue
        
        is_crit_pr, crit_pr_type = is_critical_pr(pr)
        if not is_crit_pr:
            continue
        
        critical_pr_count += 1
        
        files = pr.get("files_changed", [])
        for file_info in files:
            filename = file_info["filename"]
            changes = file_info.get("changes", 0)
            
            # Only count source files, not tests
            if "test" not in filename.lower():
                critical_bug_files[filename] += 1
                critical_bug_files_by_changes[filename] += changes
    
    analysis["critical_bug_files"] = critical_bug_files
    analysis["critical_bug_files_by_changes"] = critical_bug_files_by_changes
    analysis["critical_pr_count"] = critical_pr_count
    analysis["total_critical"] = len(critical_issues)
    
    return analysis

def print_critical_report(analysis: Dict[str, Any]):
    """Print detailed critical issues report."""
    
    print("\n" + "="*70)
    print("CRITICAL ISSUES DEEP DIVE ANALYSIS")
    print("="*70)
    
    print(f"\nTotal critical issues: {analysis['total_critical']}")
    print(f"Open: {analysis['by_state'].get('open', 0)}")
    print(f"Closed: {analysis['by_state'].get('closed', 0)}")
    print(f"\nCritical bug-fix PRs analyzed: {analysis.get('critical_pr_count', 0)}")
    
    # Show file hotspots first - most actionable info
    if analysis["critical_bug_files"]:
        print("\n" + "-"*70)
        print("🔥 TOP 30 FILES MOST OFTEN FIXED FOR CRITICAL BUGS")
        print("-"*70)
        changes_by_file = analysis.get("critical_bug_files_by_changes", {})
        for filename, count in analysis["critical_bug_files"].most_common(30):
            changes = changes_by_file.get(filename, 0)
            print(f"{count:3}x  {changes:5} changes  {filename}")
    
    print("\n" + "-"*70)
    print("CRITICAL ISSUE TYPES")
    print("-"*70)
    for issue_type, count in analysis["by_type"].most_common(20):
        open_count = len([i for i in analysis["open_critical"] if i["type"] == issue_type])
        print(f"{issue_type:25} {count:4} total  ({open_count:3} open)")
    
    print("\n" + "-"*70)
    print("ROOT CAUSE COMPONENTS (Critical Issues)")
    print("-"*70)
    if analysis["by_component"]:
        for component, count in analysis["by_component"].most_common(15):
            print(f"{component:30} {count:4} critical issues")
    else:
        print("No component-level data available (file mentions in issues)")
        print("Run with --pr-files flag for file-level analysis")
    
    print("\n" + "-"*70)
    print("CRITICAL ISSUES BY YEAR")
    print("-"*70)
    for year, count in sorted(analysis["by_year"].items()):
        print(f"{year:10} {count:4}")
    
    print("\n" + "-"*70)
    print(f"TOP 20 OPEN CRITICAL ISSUES (by discussion volume)")
    print("-"*70)
    open_crit = sorted(analysis["open_critical"], key=lambda x: x["comments"], reverse=True)[:20]
    for issue in open_crit:
        components_str = ",".join(issue["components"][:2]) if issue["components"] else "unknown"
        print(f"#{issue['number']:5} [{issue['type']:15}] ({issue['comments']:2} comments) {components_str:20} {issue['title'][:40]}")
    
    print("\n" + "="*70)

def export_critical_csv(analysis: Dict[str, Any]):
    """Export critical issues to CSV."""
    import csv
    
    output_file = DATA_DIR / "critical_issues.csv"
    with open(output_file, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow([
            "number", "title", "type", "state", "created_at",
            "comments", "labels", "components"
        ])
        
        for issue in analysis["open_critical"]:
            writer.writerow([
                issue["number"],
                issue["title"],
                issue["type"],
                "open",
                issue["created_at"],
                issue["comments"],
                "|".join(issue["labels"]),
                "|".join(issue["components"]),
            ])
    
    print(f"\nCritical issues CSV exported to: {output_file}")

def main():
    """Main entry point."""
    print("Loading data...")
    issues, prs = load_data()
    
    print("Analyzing critical issues...")
    analysis = analyze_critical_issues(issues, prs)
    
    print_critical_report(analysis)
    export_critical_csv(analysis)
    
    print("\n✓ Critical issues analysis complete!")

if __name__ == "__main__":
    main()

