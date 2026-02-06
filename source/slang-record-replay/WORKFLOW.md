# Iterative Proxy Implementation Workflow

This document describes the recommended workflow for implementing missing proxy methods using the replay system to identify and fix unimplemented functions.

## Prerequisites

- Built Slang in Debug configuration
- `slang-test` and `slang-replay` tools available in `build/Debug/bin/`

## Setup (One-time)

```powershell
# Set environment variable to enable replay recording
$env:SLANG_RECORD_LAYER=1

# Optional: Set custom replay directory (default: .slang-replays)
# $env:SLANG_RECORD_PATH="C:\slang-replays-custom"
```

## Iteration Cycle

### Step 1: Run Test and Capture Replay

Run a failing test with replay recording enabled:

```powershell
# Run a specific test
./build/Debug/bin/slang-test.exe "tests/compute/array-param" -v 2>&1 | Tee-Object test-output.txt

# Or run a test category
./build/Debug/bin/slang-test.exe -category compute -v 2>&1 | Tee-Object test-output.txt

# Or run a test prefix
./build/Debug/bin/slang-test.exe "tests/language-feature" -v 2>&1 | Tee-Object test-output.txt
```

### Step 2: Find Latest Replay

Locate the most recent replay directory:

```powershell
# Get the most recent replay directory
$latestReplay = Get-ChildItem -Path .slang-replays -Directory | 
    Where-Object { $_.Name -match '^\d{4}-\d{2}-\d{2}' } | 
    Sort-Object Name -Descending | 
    Select-Object -First 1

Write-Host "Latest replay: $($latestReplay.FullName)"
```

### Step 3: Decode and Analyze Replay

Decode the binary replay stream to human-readable format:

```powershell
# Decode to file for easier analysis
./build/Debug/bin/slang-replay.exe -d "$($latestReplay.FullName)\stream.bin" -o replay-analysis.txt

# Find all unimplemented functions
Select-String -Path replay-analysis.txt -Pattern "UNIMPLEMENTED" | 
    ForEach-Object { $_.Line -replace '.*String: "UNIMPLEMENTED: ([^"]+)".*', '$1' } | 
    Select-Object -Unique | 
    Sort-Object

# Save to file
Select-String -Path replay-analysis.txt -Pattern "UNIMPLEMENTED" | 
    ForEach-Object { $_.Line -replace '.*String: "UNIMPLEMENTED: ([^"]+)".*', '$1' } | 
    Select-Object -Unique | 
    Sort-Object | 
    Out-File unimplemented-list.txt
```

### Step 4: Examine Context Around Failures

See what happened before each unimplemented call:

```powershell
# See context around each unimplemented function (10 lines before, 5 after)
Select-String -Path replay-analysis.txt -Pattern "UNIMPLEMENTED" -Context 10,5 | 
    Out-File unimplemented-context.txt

# Review the file to understand the call sequence
code unimplemented-context.txt
```

### Step 5: Identify Implementation Targets

Analyze the unimplemented functions to determine:

1. **Which proxy class** needs updating
2. **What the method should do** (forward to actual, record data, wrap results)
3. **Whether it returns objects** that need proxy wrapping
4. **Any special handling** needed (e.g., callbacks, memory management)

**Example Analysis:**

If you find:
```
UNIMPLEMENTED: ModuleProxy::getSpecializationParamCount
UNIMPLEMENTED: GlobalSessionProxy::parseCommandLineArguments
```

Then:
- **ModuleProxy::getSpecializationParamCount**
  - Location: `source/slang-record-replay/proxy/proxy-module.h`
  - Action: Forward to actual implementation, record call and return value
  - Returns: `SlangInt` (simple integer, no wrapping needed)
  
- **GlobalSessionProxy::parseCommandLineArguments**
  - Location: `source/slang-record-replay/proxy/proxy-global-session.h`
  - Action: Forward to actual, record arguments and result
  - Returns: `SlangResult` + populates output struct (may need deep copy for recording)

### Step 6: Implement the Methods

For each unimplemented method:

1. **Remove `REPLAY_UNIMPLEMENTED_X` call**
2. **Add proper implementation** following the pattern:
   ```cpp
   SLANG_NO_THROW ReturnType SLANG_MCALL methodName(params) override
   {
       auto& ctx = ReplayContext::get();
       ctx.beginCall(__FUNCSIG__, this);
       
       // Record input parameters
       ctx.record(RecordFlag::Input, param1);
       ctx.record(RecordFlag::Input, param2);
       
       // Call actual implementation
       ReturnType result = getActual<ActualInterface>()->methodName(param1, param2);
       
       // Record output/return values
       ctx.record(RecordFlag::ReturnValue, result);
       
       return result;
   }
   ```

3. **For methods returning COM interfaces**, wrap the result:
   ```cpp
   ISlangBlob* result = nullptr;
   actual->loadFile(path, &result);
   
   // Wrap the result before recording
   ISlangBlob* wrappedResult = static_cast<ISlangBlob*>(wrapObject(result));
   ctx.record(RecordFlag::Output, wrappedResult);
   
   return wrappedResult;
   ```

### Step 7: Rebuild

```powershell
# Rebuild the slang library with your changes
cmake --build build --config Debug --target slang -j 8

# If you modified slang-test specific code
cmake --build build --config Debug --target slang-test -j 8
```

### Step 8: Verify Fix

Re-run the same test to verify the implementation:

```powershell
# Re-run the same test
./build/Debug/bin/slang-test.exe "tests/compute/array-param" -v 2>&1 | Tee-Object test-output-v2.txt

# Compare results
Write-Host "Before fix:" -ForegroundColor Yellow
Select-String -Path test-output.txt -Pattern "FAILED|passed" | Measure-Object | 
    Select-Object Count

Write-Host "After fix:" -ForegroundColor Yellow
Select-String -Path test-output-v2.txt -Pattern "FAILED|passed" | Measure-Object | 
    Select-Object Count
```

### Step 9: Verify Replay Quality

Check the new replay for remaining issues:

```powershell
# Get the newest replay
$newReplay = Get-ChildItem -Path .slang-replays -Directory | 
    Where-Object { $_.Name -match '^\d{4}-\d{2}-\d{2}' } | 
    Sort-Object Name -Descending | 
    Select-Object -First 1

# Check for remaining unimplemented calls
./build/Debug/bin/slang-replay.exe -d "$($newReplay.FullName)\stream.bin" | 
    Select-String -Pattern "UNIMPLEMENTED" | 
    ForEach-Object { $_.Line -replace '.*String: "UNIMPLEMENTED: ([^"]+)".*', '$1' } | 
    Select-Object -Unique |
    Tee-Object remaining-unimplemented.txt

# If empty, all functions are implemented!
if ((Get-Content remaining-unimplemented.txt).Count -eq 0) {
    Write-Host "✓ All functions implemented!" -ForegroundColor Green
}
```

### Step 10: Repeat or Complete

- **If unimplemented functions remain** → Go to Step 5
- **If tests pass but different test fails** → Go to Step 1 with new test
- **If all tests in category pass** → Move to next test category
- **If all tests pass** → Success! 🎉

## Quick Helper Script

Save this as `analyze-replay.ps1` in the Slang root directory:

```powershell
param(
    [string]$TestPattern = "tests/compute/array-param",
    [switch]$ShowContext
)

Write-Host "=== Running Test ===" -ForegroundColor Cyan
Write-Host "Test: $TestPattern`n"

# Run test with replay
$env:SLANG_RECORD_LAYER=1
./build/Debug/bin/slang-test.exe $TestPattern -v 2>&1 | Tee-Object test-output.txt

# Get latest replay
$latest = Get-ChildItem -Path .slang-replays -Directory | 
    Where-Object { $_.Name -match '^\d{4}-\d{2}-\d{2}' } | 
    Sort-Object Name -Descending | 
    Select-Object -First 1

if ($latest) {
    Write-Host "`n=== Replay Analysis ===" -ForegroundColor Cyan
    Write-Host "Replay: $($latest.Name)`n"
    
    # Decode
    ./build/Debug/bin/slang-replay.exe -d "$($latest.FullName)\stream.bin" -o replay-analysis.txt
    
    # Find unimplemented
    $unimplemented = Select-String -Path replay-analysis.txt -Pattern "UNIMPLEMENTED" | 
        ForEach-Object { $_.Line -replace '.*String: "UNIMPLEMENTED: ([^"]+)".*', '$1' } | 
        Select-Object -Unique | 
        Sort-Object
    
    if ($unimplemented) {
        Write-Host "Unimplemented Functions:" -ForegroundColor Yellow
        $unimplemented | ForEach-Object { Write-Host "  ❌ $_" -ForegroundColor Red }
        
        if ($ShowContext) {
            Write-Host "`nContext saved to unimplemented-context.txt" -ForegroundColor Cyan
            Select-String -Path replay-analysis.txt -Pattern "UNIMPLEMENTED" -Context 10,5 | 
                Out-File unimplemented-context.txt
        }
    } else {
        Write-Host "✓ No unimplemented functions!" -ForegroundColor Green
    }
    
    # Test results
    Write-Host "`n=== Test Results ===" -ForegroundColor Cyan
    $passed = (Select-String -Path test-output.txt -Pattern "passed test").Count
    $failed = (Select-String -Path test-output.txt -Pattern "FAILED test").Count
    $total = $passed + $failed
    
    if ($total -gt 0) {
        $passRate = [math]::Round(($passed / $total) * 100, 1)
        Write-Host "  ✓ Passed: $passed / $total ($passRate%)" -ForegroundColor Green
        Write-Host "  ✗ Failed: $failed / $total" -ForegroundColor Red
    } else {
        Write-Host "  No tests matched pattern" -ForegroundColor Yellow
    }
} else {
    Write-Host "No replay found!" -ForegroundColor Red
}
```

**Usage:**

```powershell
# Basic usage
.\analyze-replay.ps1 "tests/compute/array-param"

# With context dump
.\analyze-replay.ps1 "tests/compute/array-param" -ShowContext

# Test category
.\analyze-replay.ps1 -TestPattern "tests/language-feature" -ShowContext
```

## Common Patterns

### Pattern 1: Simple Value Return

```cpp
SLANG_NO_THROW SlangInt SLANG_MCALL getCount() override
{
    auto& ctx = ReplayContext::get();
    ctx.beginCall(__FUNCSIG__, this);
    
    SlangInt result = getActual<ActualInterface>()->getCount();
    ctx.record(RecordFlag::ReturnValue, result);
    
    return result;
}
```

### Pattern 2: Output Parameter

```cpp
SLANG_NO_THROW SlangResult SLANG_MCALL getData(ISlangBlob** outBlob) override
{
    auto& ctx = ReplayContext::get();
    ctx.beginCall(__FUNCSIG__, this);
    
    ISlangBlob* result = nullptr;
    SlangResult hr = getActual<ActualInterface>()->getData(&result);
    
    // Wrap result before recording
    ISlangBlob* wrapped = static_cast<ISlangBlob*>(wrapObject(result));
    ctx.record(RecordFlag::Output, wrapped);
    ctx.record(RecordFlag::ReturnValue, hr);
    
    if (outBlob) *outBlob = wrapped;
    return hr;
}
```

### Pattern 3: Input Array

```cpp
SLANG_NO_THROW SlangResult SLANG_MCALL process(
    int count,
    const char** items) override
{
    auto& ctx = ReplayContext::get();
    ctx.beginCall(__FUNCSIG__, this);
    
    ctx.record(RecordFlag::Input, count);
    ctx.recordArray(RecordFlag::Input, items, count);
    
    SlangResult result = getActual<ActualInterface>()->process(count, items);
    ctx.record(RecordFlag::ReturnValue, result);
    
    return result;
}
```

## Tips

1. **Start with simple tests** that hit fewer API calls
2. **Group related methods** - if one method fails, related methods likely need implementation too
3. **Check existing implementations** in other proxy files for examples
4. **Test incrementally** - implement 1-2 methods, rebuild, test
5. **Keep replays** from successful runs as regression tests
6. **Use `git diff`** to verify you're only changing what you intended

## Troubleshooting

**Test fails with "Handle not found"**
- You forgot to wrap a returned COM interface with `wrapObject()`

**Replay decoder shows garbled data**
- Type mismatch in record/playback order
- Check that input/output flags match between record and playback

**Test still fails after implementation**
- Check for nested unimplemented calls in returned objects
- Verify you're wrapping all COM interface returns

**Build errors after changes**
- Missing includes (add to proxy header file)
- Wrong method signature (check against actual interface in `slang.h`)
