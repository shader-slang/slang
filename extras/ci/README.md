# CI Parallelization Analyzer

A tool to analyze GitHub Actions workflow runs and identify parallelization bottlenecks.

## Features

- **Workflow Metrics**: Total duration, CPU time, parallelization efficiency
- **Critical Path Analysis**: Identifies longest-running jobs
- **Sequential Chain Detection**: Finds build→test dependencies and wait times
- **Runner Utilization**: Shows how efficiently runners are being used
- **Actionable Recommendations**: Provides specific optimization suggestions

## Installation

No installation required! Just download the script and run it with Python 3.6+.

```bash
# Make it executable
chmod +x analyze-ci-parallelization.py
```

## Usage

### Basic Usage (Recommended)

Pipe GitHub Actions job data directly from `gh` CLI:

```bash
gh api repos/OWNER/REPO/actions/runs/RUN_ID/jobs | python3 analyze-ci-parallelization.py
```

**Example:**

```bash
gh api repos/shader-slang/slang/actions/runs/21618930965/jobs | python3 analyze-ci-parallelization.py
```

### Alternative: Save to File First

```bash
# Save jobs data
gh api repos/shader-slang/slang/actions/runs/21618930965/jobs > jobs.json

# Analyze the saved file
python3 analyze-ci-parallelization.py jobs.json
```

### Finding the Run ID

1. Go to your GitHub Actions page: `https://github.com/OWNER/REPO/actions`
2. Click on a workflow run
3. The URL will be: `https://github.com/OWNER/REPO/actions/runs/RUN_ID`
4. Copy the `RUN_ID` number

Or use `gh` CLI to find recent runs:

```bash
# List recent runs
gh run list --repo OWNER/REPO

# Get the latest run ID
gh run list --repo OWNER/REPO --limit 1 --json databaseId -q '.[0].databaseId'
```

## Output Sections

### 1. Workflow Overview

- Total duration and CPU time
- Number of jobs
- Parallelization efficiency (how many jobs run concurrently on average)
- Failed job detection

### 2. Longest Running Jobs

- Top 10 jobs by duration
- Helps identify optimization targets

### 3. Sequential Job Chains

- Groups related jobs (e.g., build → test)
- Shows wait time gaps
- Identifies platforms with sequential bottlenecks

### 4. Runner Utilization

- Shows how each runner is used
- Identifies runners handling too many sequential jobs
- Calculates utilization percentage and idle time

### 5. Optimization Recommendations

- Overall efficiency rating (🔴 low, 🟡 moderate, 🟢 good)
- Lists long-running jobs (>20 minutes)
- Identifies runner bottlenecks
- Provides actionable suggestions

## Interpreting Results

### Parallelization Efficiency

```
Parallelization Efficiency: 3.4x
```

This means on average, 3.4 jobs run in parallel. For a workflow using 15+ runners, this indicates low efficiency.

**Good**: 10x+ (most runners actively working)
**Moderate**: 5-10x (some optimization possible)
**Poor**: <5x (significant sequential bottlenecks)

### Wait Time Gaps

```
test-linux-debug-gcc-x86_64:
  Jobs: 3, Total work: 29.6 min, Elapsed: 39.0 min
  🔍 Gap: 9.4 min (potential wait time)
```

The gap indicates time where jobs could have run in parallel but didn't. Investigate job dependencies.

### Runner Bottlenecks

```
slang-win-gcp-t4-runner-4:
  Jobs: 4, Total work: 45.3 min, Idle: 5.7 min, Utilization: 89%
  ⚠️  Runner handles 4 sequential jobs - potential bottleneck
```

This runner is overloaded. Consider:

- Adding more runners of this type
- Allowing some jobs to run in parallel instead of sequentially

## Common Optimization Strategies

### 1. Parallelize Test Jobs

**Before:**

```yaml
test-suite-1:
  needs: build
test-suite-2:
  needs: test-suite-1 # ❌ Unnecessary dependency
```

**After:**

```yaml
test-suite-1:
  needs: build
test-suite-2:
  needs: build # ✅ Both can run in parallel
```

### 2. Add More Runners

If specific runner types are bottlenecks:

- Add more self-hosted runners
- Use matrix strategies to distribute work
- Consider using faster runner types (GitHub-hosted runners with more CPU)

### 3. Split Long-Running Jobs

If a single job takes >20 minutes:

- Use matrix strategy to split tests
- Parallelize build steps
- Use caching to reduce rebuild time

### 4. Optimize Job Dependencies

Review `needs:` clauses in your workflow:

- Remove unnecessary dependencies
- Allow independent jobs to run in parallel
- Consider if all tests really need all builds

## Example Analysis

```bash
$ gh api repos/shader-slang/slang/actions/runs/21618930965/jobs | python3 analyze-ci-parallelization.py

====================================================================================================
CI RUN PARALLELIZATION ANALYSIS
====================================================================================================

Workflow Duration: 67.7 minutes
Total Jobs: 22
Total CPU Time: 228.9 minutes
Parallelization Efficiency: 3.4x

====================================================================================================
OPTIMIZATION RECOMMENDATIONS
====================================================================================================

🔴 LOW PARALLELIZATION EFFICIENCY
   Current: 3.4x
   Many jobs are running sequentially. Consider:
   - Reviewing job dependencies to allow more parallel execution
   - Adding more runners to handle parallel workloads

⏱️  LONG-RUNNING JOBS (3 jobs over 20 minutes)
   - build-macos-release-clang-aarch64 / build: 28.3 min
   - test-windows-debug-cl-x86_64-gpu / test-slang: 22.2 min

🔧 RUNNER BOTTLENECKS (2 runners with 3+ sequential jobs)
   - slang-win-gcp-t4-runner-4: 4 jobs, 45.3 min
   Consider adding more runners or redistributing work
```

## Requirements

- Python 3.6 or later
- GitHub CLI (`gh`) for fetching workflow data
- No additional Python packages required (uses only stdlib)

## License

Free to use and modify for analyzing CI workflows.

## Contributing

Suggestions and improvements welcome! This tool analyzes any GitHub Actions workflow.
