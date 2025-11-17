# GitHub Issues Analysis for Quality Gap Identification

This directory contains scripts to analyze Slang's GitHub issues and identify quality gaps in the codebase.

## Quick Start

### 1. Fetch Data

First, fetch all issues and PRs from GitHub:

```bash
cd tools/issue-analysis

# Optional: Set GitHub token to avoid rate limits
export GITHUB_TOKEN="your_github_token_here"

# Basic fetch (fast, ~2-3 minutes)
python3 fetch_github_issues.py

# Fetch with PR relationships (~20-30 minutes)
python3 fetch_github_issues.py --enrich

# Fetch with PR file changes (~15-20 minutes)
python3 fetch_github_issues.py --pr-files

# Full enrichment: both PR relationships and files (slowest, ~40-50 minutes)
python3 fetch_github_issues.py --full
```

This will download all issues and PRs to `data/` subdirectory.

**Fetch modes:**
- **Basic mode** (default): Fetches issue and PR metadata only
- **--enrich**: Also fetches which PRs fixed each issue
- **--pr-files**: Also fetches which files were changed in each PR
- **--full**: Both issue-PR links and PR file changes (most complete)

**Note**: Without a GitHub token, you may hit rate limits (60 requests/hour). With a token, you get 5000 requests/hour.
Create a token at: https://github.com/settings/tokens (only needs public repo read access)

### 2. Analyze Data

Run the general analysis:

```bash
python3 analyze_issues.py
```

This will:
- Generate a comprehensive report on terminal
- Export detailed CSV to `data/issues_detailed.csv`

### 3. Analyze Critical Issues (Crashes, ICEs)

For deep dive into crashes and critical bugs:

```bash
python3 analyze_critical_issues.py
```

This will:
- Identify all crashes, ICEs, and validation errors
- Show root cause components
- List open critical issues by urgency
- Export critical issues CSV

See `CRITICAL_FINDINGS.md` for detailed critical issues analysis.

### 4. Analyze Bug-Fix Files

For file-level hotspot analysis:

```bash
python3 analyze_bugfix_files.py
```

This will:
- Show which files are changed most often for bug fixes
- Identify file-level quality hotspots
- Categorize by component

See `GENERAL_FINDINGS.md` for comprehensive overview of all analyses.

## 📄 Key Findings Documents

After running the analyses, you'll have comprehensive reports:

### ⚡ SUMMARY.md - **Start Here!**
Executive summary with key findings and immediate actions:
- Top 3 critical quality gaps
- Critical file hotspots (top 5)
- Backend quality comparison
- Immediate action items with priorities
- 12-month success metrics
- Strategic recommendations
- **Quick read**: ~5 minutes

### 📊 GENERAL_FINDINGS.md
Complete overview of Slang quality based on all 3,535 issues and 5,392 PRs:
- Overall quality trends and metrics
- Issue and PR velocity analysis
- Backend quality comparison (SPIRV, HLSL, GLSL, etc.)
- File-level bug hotspots (top 30 files)
- Component-level bug distribution
- Test coverage analysis
- Development velocity trends
- Actionable recommendations
- **Detailed read**: ~20 minutes

### 🔥 CRITICAL_FINDINGS.md
Deep dive into critical issues (crashes, ICEs, validation errors):
- Analysis of 1,049 critical issues
- 699 critical bug-fix PRs analyzed
- Top 10 critical file hotspots
- Root cause analysis by component
- Immediate action items
- Success criteria and metrics
- **Detailed read**: ~15 minutes

## What the Analysis Reveals

The analysis identifies quality gaps by examining:

1. **Component-level Issue Distribution**: Which parts of Slang have the most issues
2. **Bug Concentration**: Where bugs cluster (IR passes, emitters, checker, etc.)
3. **File-Level Hotspots**: Exact source files requiring most bug fixes
4. **Time to Resolution**: Components that take longer to fix (complexity indicators)
5. **Target-specific Issues**: SPIRV, DXIL, CUDA, Metal, etc.
6. **Feature Areas**: Autodiff, generics, cooperative matrix, etc.
7. **Test Coverage Gaps**: Which PRs lack tests
8. **Development Velocity**: PR merge times and throughput

## Output Files

After running the scripts, you'll have:

```
tools/issue-analysis/data/
├── issues.json              # Raw issue data
├── pull_requests.json       # Raw PR data
├── metadata.json            # Fetch metadata
└── issues_detailed.csv      # Processed data for Excel/analysis tools
```

### Issue-PR Relationships

When you use `--enrich`, each issue in `issues.json` will have an additional field:

```json
{
  "number": 9030,
  "title": "Some bug",
  "state": "closed",
  "related_prs": [8999, 9001]  // PRs that fixed this issue
}
```

### PR File Changes

When you use `--pr-files` or `--full`, each PR in `pull_requests.json` will have:

```json
{
  "number": 8999,
  "title": "Fix SPIRV emission bug",
  "files_changed": [
    {
      "filename": "source/slang/slang-emit-spirv.cpp",
      "status": "modified",
      "additions": 25,
      "deletions": 10,
      "changes": 35
    },
    {
      "filename": "tests/spirv/test-case.slang",
      "status": "added",
      "additions": 50,
      "deletions": 0,
      "changes": 50
    }
  ]
}
```

### Powerful Analysis Enabled

With both enrichments (`--full`), you can analyze:
- **Which files have most bugs?** Track issues → PRs → files
- **Component complexity**: Files that change frequently to fix bugs
- **Test coverage gaps**: Issues without corresponding test files
- **Fix quality**: How many PRs touch the same files for same issue?
- **Hot spots**: Files involved in many bug fixes
- **IR pass problems**: Which `slang-ir-*.cpp` files need most fixes?
- **Backend quality**: Compare file changes across SPIRV, DXIL, Metal emitters

## Component Categories

The analysis tracks these key areas:

- **Compiler Pipeline**: parser, lexer, preprocessor, checker
- **IR System**: slang-ir-* passes
- **Code Generation**: slang-emit-* backends
- **Targets**: SPIRV, DXIL, CUDA, Metal, GLSL, HLSL, WGSL
- **Features**: autodiff, generics, cooperative-matrix

## Further Analysis

The CSV file can be imported into:
- **Excel/Google Sheets**: For pivot tables and charts
- **Jupyter Notebook**: For custom Python analysis
- **SQL Database**: For complex queries
- **Pandas**: For data science workflows

Example follow-up analyses:
- Correlation between issue comments and time-to-close
- Bug introduction rate over time
- Most affected users/reporters
- Seasonal patterns in bug reports
- Effectiveness of different bug-fix strategies

## Updating Data

To refresh the analysis with latest data:

```bash
# Re-fetch data
python3 fetch_github_issues.py

# Re-run analysis
python3 analyze_issues.py
```

## Customization

Edit `analyze_issues.py` to:
- Add new component patterns to `SOURCE_PATTERNS`
- Change categorization logic
- Add custom metrics
- Export different formats

## Example Output

The report will show sections like:

```
TOP 15 COMPONENTS BY ISSUE COUNT (Quality Gap Indicators)
----------------------------------------------------------------------
spirv                      234 issues  ( 89 bugs,  34 open bugs)
slang-ir                   187 issues  ( 67 bugs,  23 open bugs)
dxil                       156 issues  ( 54 bugs,  19 open bugs)
...

BUGS BY COMPONENT (Areas needing attention)
----------------------------------------------------------------------
spirv                       89 bugs total  ( 34 open,  55 closed)
slang-ir                    67 bugs total  ( 23 open,  44 closed)
...

AVERAGE TIME TO CLOSE (days) BY COMPONENT
----------------------------------------------------------------------
autodiff                   125.3 days  (45 issues)
generics                    98.7 days  (67 issues)
...
```

This helps identify:
- **High issue count** → Areas with many problems
- **High open bug count** → Current quality gaps
- **Long time to close** → Complex/difficult areas

