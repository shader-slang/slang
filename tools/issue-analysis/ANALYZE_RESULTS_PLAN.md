# How to Create the Issue Analysis Report

**Purpose:** This is a process guide for creating comprehensive issue analysis reports from the generated data.

**Actual Analysis Location:** The analysis report itself goes in **`results/ISSUE_ANALYSIS.md`** (not this file).

This document describes:
- What data is available in `results/*.txt`
- How to structure the analysis report
- What to include in each section
- How to prioritize findings

## Objective

The `results/ISSUE_ANALYSIS.md` should provide:
1. Executive summary of the Slang codebase quality
2. Deep dive into 10 most critical areas requiring attention
3. Data-driven recommendations for improving quality and reducing bugs

## Data Sources

### Phase 1: Analysis Outputs (Identify Problem Areas)

Three complementary reports in `results/`:

1. **general-analysis.txt** - Overall PR and issue trends
   - Total issues: 3,573
   - Total PRs: 5,425
   - Bug fix rate: 26.1% (1,417 bug-fix PRs)
   - Provides: PR velocity, issue trends, test coverage, file-level bug frequencies

2. **critical-analysis.txt** - Critical bugs (crashes, ICEs, etc.)
   - Total critical issues: 1,066
   - Critical bug-fix PRs: 702
   - Provides: Root causes, critical components, severity breakdown

3. **bugfix-files-analysis.txt** - File and component bug patterns
   - Analyzes all 1,417 bug-fix PRs
   - Provides: Component-level metrics (fixes, changes, LOC)
   - File-level bug fix frequencies

### Phase 2: Raw Data (Deep-Dive for Evidence)

Raw issue and PR data in `data/`:

1. **data/issues.json** - All 3,573 issues
   - Fields: number, title, body, labels, state, created_at, closed_at, comments, user
   - Use for: Finding specific examples, understanding issue descriptions, analyzing patterns

2. **data/pull_requests.json** - All 5,425 PRs
   - Fields: number, title, body, labels, state, created_at, merged_at, files_changed
   - files_changed includes: filename, additions, deletions, changes
   - Use for: Understanding fixes, finding related issues, analyzing change patterns

3. **data/critical_issues.csv** - Export of critical issues
   - Pre-filtered critical issues for easier analysis
   - Use for: Quick access to critical issue details

## Analysis Structure

### 1. Executive Summary (1 page)

**Key Metrics** (extract from `results/*.txt`):
- Total issues and PRs analyzed
- Bug fix rate (% of PRs that are bug fixes)
- Critical issue count and types
- Test coverage statistics
- Average time to close issues/merge PRs

**Trends** (calculate from raw data):
- Bug fix trend over time (increasing/decreasing)
- Critical issue trend over time
- PR velocity trend
- Test coverage trend

**Overall Assessment** (synthesize from data):
- Overall quality trend (improving/stable/declining)
- Top 5 risk areas (from priority score analysis)
- Top 5 areas in open issues and PRs (current status)
- Key strengths (what's working well)
- Immediate attention needed (critical findings)

### 2. Top 10 Critical Areas for Improvement

Each area should include:

**Format:**
```
## Area N: [Component/System Name]

**Severity:** Critical/High/Medium
**Impact:** [Scope of impact - crashes, correctness, performance, etc.]

### Current State
- Bug frequency: X fixes per 1000 LOC
- Total bug fixes: X PRs
- Critical issues: X crashes/ICEs
- Test coverage: X%

### Root Causes
1. [Primary cause with evidence]
2. [Secondary cause with evidence]
3. [Contributing factors]

### Evidence
- File: [filename] - X fixes, Y LOC, Z fix frequency
- Issues: #[number], #[number] (examples)
- Patterns: [observed patterns from analysis]

### Recommendations

Prioritize recommendations into short-term and long-term actions (no specific timelines).
Order by priority within each category.

**Short-term priorities:**
1. Action 1 with expected impact (highest priority quick win)
2. Action 2 with expected impact
3. Action 3 with expected impact

**Long-term priorities:**
1. Strategic improvement 1 (highest priority long-term)
2. Strategic improvement 2
3. Architectural or process change

### Success Metrics
- Reduce bug fix frequency to < X per 1000 LOC
- Reduce critical issues by X%
- Achieve X% test coverage
```

## Discovering Critical Areas (No Assumptions)

The analysis must identify problem areas purely from data. Do not make assumptions about which components are problematic.

### Selection Criteria

Select the top 10 areas based on data from `results/*.txt` files:

1. **High Bug Fix Frequency (normalized by LOC)**
   - From: "TOP 40 FILES BY BUG FIX FREQUENCY" sections
   - Look for: Files with >50 fixes per 1000 LOC
   - Group related files into components

2. **High Absolute Bug Count**
   - From: "ALL COMPONENTS BY BUG-FIX FREQUENCY" section
   - Look for: Components with >100 total bug fixes
   - Consider: Both total fixes and LOC for context

3. **Critical Issue Concentration**
   - From: "ROOT CAUSE COMPONENTS" section in critical-analysis.txt
   - Look for: Components with >5 critical issues
   - Prioritize: Crashes and ICEs over other issue types

4. **Cross-Category Appearance**
   - Components that appear in:
     - High bug frequency lists
     - Critical issue lists
     - High change volume lists
   - These are strong candidates for deep-dive

5. **Test Coverage Gaps**
   - From: General analysis test coverage sections
   - Components with low test coverage AND high bug counts
   - Indicates systemic quality issues

### Decision Matrix

For each component found in the data, calculate a priority score:

```
Priority Score = (Bug Fix Frequency × 0.3) +
                 (Critical Issues × 0.4) +
                 (Total Fixes / 100 × 0.2) +
                 (Cross-Category Appearances × 0.1)
```

Select the top 10 by priority score for detailed analysis.

## Analysis Process

### Phase 1: Identify Problem Areas (From Analysis Outputs)

#### Step 1: Extract Key Metrics
```bash
# Get component-level statistics
grep -A 50 "ALL COMPONENTS BY BUG-FIX FREQUENCY" results/bugfix-files-analysis.txt

# Get file-level bug frequencies
grep -A 40 "TOP 40 FILES BY BUG FIX FREQUENCY" results/bugfix-files-analysis.txt
grep -A 40 "TOP 40 FILES BY BUG FIX FREQUENCY" results/general-analysis.txt
grep -A 40 "TOP 40 FILES BY CRITICAL BUG FIX FREQUENCY" results/critical-analysis.txt

# Get critical issue patterns
grep -A 20 "ROOT CAUSE COMPONENTS" results/critical-analysis.txt
grep -A 20 "CRITICAL ISSUES BY TYPE" results/critical-analysis.txt
```

#### Step 2: Cross-Reference Analysis
- Correlate high bug fix frequency with critical issues
- Identify components appearing in multiple problem categories
- Look for patterns in error types (crashes, ICEs, validation)
- Note files with disproportionately high bug fix frequency per LOC

#### Step 3: Identify Top 10 Areas

**A. Extract Component Metrics**

Create a table with all components found in the analysis:

```
Component Name | Bug Fixes | Bug Fix Freq | LOC | Critical Issues | In Multiple Lists
---------------|-----------|--------------|-----|-----------------|------------------
```

Sources:
- Bug Fixes: From "ALL COMPONENTS BY BUG-FIX FREQUENCY"
- Bug Fix Freq: From "TOP 40 FILES BY BUG FIX FREQUENCY" (average for component)
- LOC: From "ALL COMPONENTS BY BUG-FIX FREQUENCY" (LOC column)
- Critical Issues: From "ROOT CAUSE COMPONENTS"
- In Multiple Lists: Count how many analysis sections mention this component

**B. Calculate Priority Scores**

For each component:
```python
# Normalize values to 0-1 scale first
normalized_freq = bug_fix_freq / max_bug_fix_freq
normalized_critical = critical_issues / max_critical_issues
normalized_fixes = total_fixes / max_total_fixes
cross_category = appearances_count / 3  # Max 3 categories

priority_score = (normalized_freq * 0.3) + \
                 (normalized_critical * 0.4) + \
                 (normalized_fixes * 0.2) + \
                 (cross_category * 0.1)
```

**C. Select Top 10**

- Sort components by priority score
- Take top 10
- Exclude "test" and "docs" unless they show exceptional problems
- Include at least 2-3 high-frequency files even if they're in same component
  (e.g., specific problematic files like slang.cpp, slang-compiler.cpp)

**D. Validate Selection**

Ensure selected areas:
- Represent actual code quality issues (not just high activity)
- Have actionable scope (not too broad like "all IR")
- Show clear patterns that suggest root causes
- Have sufficient data for deep-dive analysis

### Phase 2: Deep-Dive Using Raw Data

For each of the identified top 10 areas:

#### Step 4: Find Specific Issues and PRs

**Example: For "IR Optimization" area identified in Phase 1**

```python
import json

# Load raw data
with open('data/issues.json') as f:
    issues = json.load(f)
with open('data/pull_requests.json') as f:
    prs = json.load(f)

# Find issues mentioning IR components
ir_issues = [
    issue for issue in issues
    if 'slang-ir' in issue.get('title', '').lower() or
       'slang-ir' in (issue.get('body') or '').lower()
]

# Find PRs that modified IR files
ir_prs = [
    pr for pr in prs
    if any('slang-ir' in f['filename'] for f in pr.get('files_changed', []))
]

# Find critical IR issues
critical_ir = [
    issue for issue in ir_issues
    if any(keyword in issue.get('title', '').lower()
           for keyword in ['crash', 'ice', 'assert', 'segfault'])
]
```

#### Step 5: Extract Evidence

For each critical area, gather:

**Issue Examples:**
- Find 3-5 representative issues (issue number, title, key symptoms)
- Look for recurring patterns in issue descriptions
- Note user-reported impact

**PR Analysis:**
- Examine files changed in bug-fix PRs
- Identify common fix patterns
- Calculate average time to fix
- Note if fixes clustered in specific time periods

**Pattern Recognition:**
```python
# Example: Analyze issue titles for patterns
from collections import Counter

keywords = []
for issue in critical_ir:
    title = issue['title'].lower()
    # Extract meaningful keywords
    for word in ['specialization', 'inlining', 'lowering', 'legalization',
                 'optimization', 'transformation']:
        if word in title:
            keywords.append(word)

pattern_frequency = Counter(keywords)
# Shows which IR operations are most problematic
```

#### Step 6: Root Cause Analysis

For each area, synthesize evidence to identify:
- **Technical root causes**: Architecture issues, complexity, missing validation
- **Process root causes**: Insufficient testing, documentation gaps
- **Patterns**: Are bugs clustered in new features? Legacy code? Specific backends?

Example questions to answer:
- What types of bugs are most common? (crashes vs. correctness vs. performance)
- Are bugs in new code or old code?
- Are certain code paths undertested?
- Is the component trying to do too much?
- Are there missing abstractions?

#### Step 7: Prioritization

Rank the 10 areas by:
1. **Impact**: Critical issues > Correctness > Performance > Usability
2. **Frequency**: Bug fix rate normalized by LOC
3. **Trend**: Use issue `created_at` dates to see if problems increasing/decreasing
4. **Blast radius**: How many users/features affected (check issue comment count, labels)
5. **Fix difficulty**: Average PR size, time to merge for fixes

#### Step 8: Recommendation Development

For each area, use evidence to develop prioritized recommendations.
Order by priority (most impactful first) within each category.

**Short-term priorities:**
- Based on quick wins from PR patterns
- Focus on high-frequency, similar bugs
- High impact/effort ratio
- Can be implemented with existing architecture
- Example: "Add validation for X based on 15 similar crashes"

**Long-term priorities:**
- Based on architectural issues seen in multiple PRs
- Requires significant refactoring or redesign
- Addresses fundamental design issues
- Higher effort but prevents entire classes of bugs
- Examples:
  - "Refactor Y to reduce complexity (seen in 45 bug fixes)"
  - "Redesign Z architecture (root cause of 30% of crashes)"

### Step 9: Validate with Data

For each recommendation:
- Show specific issue/PR numbers as evidence
- Calculate expected impact (e.g., "Could prevent 20% of IR crashes")
- Reference specific patterns from the data

## Output Format

The final `results/ISSUE_ANALYSIS.md` should include:
- Clear structure with table of contents
- Executive summary (1 page)
- Top 10 critical areas (detailed analysis)
- Appendices with supporting data
- No emojis
- Professional, data-driven tone
- Actionable recommendations with expected impact

## Success Criteria

The final analysis should:
- Be immediately actionable
- Provide clear prioritization (by priority order, not timelines)
- Include quantifiable metrics
- Show evidence-based reasoning
- Separate short-term and long-term priorities
- Define success criteria for each recommendation

## How to Use This Guide

### Prerequisites

1. **Ensure data is available:**
   ```bash
   ls data/issues.json data/pull_requests.json  # Raw data
   ls results/*.txt  # Analysis outputs
   ```

2. **If data is missing, fetch it:**
   ```bash
   python3 fetch_github_issues.py  # Fetches to data/
   ```

3. **Run all analysis scripts:**
   ```bash
   python3 analyze_issues.py > results/general-analysis.txt
   python3 analyze_critical_issues.py > results/critical-analysis.txt
   python3 analyze_bugfix_files.py > results/bugfix-files-analysis.txt
   ```

### Creating the Analysis

**Phase 1: Identify Top 10 Problem Areas**
- Follow Steps 1-3 in "Analysis Process" above
- Extract metrics from `results/*.txt` files
- Cross-reference to identify problem areas
- Select top 10 areas for deep-dive

**Phase 2: Deep-Dive with Raw Data**
- Follow Steps 4-8 in "Analysis Process" above
- For each of the 10 areas, write Python scripts or use jq/grep to:
  - Find relevant issues in `data/issues.json`
  - Find relevant PRs in `data/pull_requests.json`
  - Extract specific examples and evidence
  - Identify patterns and root causes

**Phase 3: Write the Report**
- Create `results/ISSUE_ANALYSIS.md`
- Follow the structure defined in this document
- Include data-backed evidence from both phases
- Add specific issue/PR numbers as examples

**Phase 4: Validate**
- Ensure all 10 areas have concrete evidence
- Verify recommendations are actionable
- Check that success metrics are quantifiable
- Confirm the analysis meets all success criteria
- **Verify data-driven approach**: Every claim must reference specific data
  - No assumptions about which components are problematic
  - Priority scores calculated from actual metrics
  - Issue/PR numbers cited for all examples
  - Patterns backed by frequency counts from raw data

### Tools for Deep-Dive Analysis

**Command-line tools:**
```bash
# Find issues by keyword
jq '.[] | select(.title | contains("crash")) | {number, title}' data/issues.json

# Find PRs modifying specific files
jq '.[] | select(.files_changed[]?.filename | contains("slang-ir")) | .number' data/pull_requests.json

# Count issues by label
jq '[.[] | .labels[].name] | group_by(.) | map({label: .[0], count: length})' data/issues.json
```

**Python snippets:**
- See Step 4 in Analysis Process for examples
- Can create ad-hoc scripts to analyze patterns
- Use pandas for more complex analysis if needed

## Common Pitfalls to Avoid

### 1. Making Assumptions
**Don't:**
- Assume certain components are problematic based on intuition
- Pre-select areas to investigate
- Cherry-pick data to support preconceived notions

**Do:**
- Let the data guide you to problem areas
- Follow the priority score methodology
- Be surprised by what the data shows

### 2. Ignoring Context
**Don't:**
- Look only at absolute bug counts (large components naturally have more bugs)
- Ignore LOC when comparing components
- Compare components without considering their complexity

**Do:**
- Always normalize by LOC (bugs per 1000 LOC)
- Consider component purpose (compiler core vs. utility functions)
- Look at bug fix frequency trends, not just snapshots

### 3. Insufficient Evidence
**Don't:**
- Make recommendations without citing specific issues/PRs
- Generalize from 1-2 examples
- Rely solely on metrics without examining actual issues

**Do:**
- Provide 3-5 concrete issue/PR examples per area
- Show patterns across multiple instances
- Quote actual issue descriptions and error messages
- Link metrics to real-world impact

### 4. Vague Recommendations
**Don't:**
- Say "improve testing" without specifics
- Suggest "refactor component X" without identifying what to refactor
- Give recommendations without success metrics

**Do:**
- Specify what to test (e.g., "Add validation tests for IR inlining edge cases")
- Identify specific code patterns to refactor
- Define measurable success criteria for each recommendation

### 5. Analysis Staleness
**Don't:**
- Use outdated analysis outputs
- Assume patterns from old data still apply
- Mix data from different time periods

**Do:**
- Re-run all analysis scripts before starting
- Note the data snapshot date in the report
- Consider trends over time, not just current state
- Update analysis regularly (quarterly recommended)

