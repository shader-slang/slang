# Iterative Proxy Implementation Workflow

This document describes the recommended workflow for implementing missing proxy methods using the replay system to identify and fix unimplemented functions.

## Prerequisites

- Built Slang in Debug configuration
- `slang-test` and `slang-replay` tools available in `build/Debug/bin/`

## Setup (One-time)

```powershell
# Set environment variable to enable replay recording
$env:SLANG_RECORD_LAYER=1

# Set replay capture path
$env:SLANG_RECORD_PATH="$PWD\replay-capture"
```

All replay data will be written to the `replay-capture/` directory in the workspace root.

## Iteration Cycle

### Step 1: Run Test and Capture Replay

Run a failing test with replay recording enabled:

```powershell
# Run a specific test
./build/Debug/bin/slang-test.exe "tests/compute/array-param" -v 2>&1 | Tee-Object test-output.txt

# Or run a test prefix
./build/Debug/bin/slang-test.exe "tests/language-feature" -v 2>&1 | Tee-Object test-output.txt
```

NOTE: If test finishes with none-zero exit code but not error, this indicates an unhandled exception,
probably caused by missing functions, so proceed to step 2.

### Step 2: Decode and Analyze Replay

Decode the binary replay stream to human-readable format:

```powershell
# Decode to file for easier analysis
./build/Debug/bin/slang-replay.exe -d "replay-capture\stream.bin" -o replay-analysis.txt

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
    virtual SLANG_NO_THROW void SLANG_MCALL
    getLanguagePrelude(SlangSourceLanguage sourceLanguage, ISlangBlob** outPrelude) override
    {
        RECORD_CALL();
        RECORD_INPUT(sourceLanguage);
        ISlangBlob* preludePtr;
        if(!outPrelude)
            outPrelude = &preludePtr;
        getActual<IGlobalSession>()->getLanguagePrelude(sourceLanguage, outPrelude);
        RECORD_COM_OUTPUT(outPrelude);
    }
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

### Step 9: Replay the Recording

Once the test passes, verify the replay can be played back correctly:

```powershell
./build/Debug/bin/slang-replay.exe "replay-capture\stream.bin"
```

**If replay fails:** The recording may have missing or incorrect serialization. Debug the replay failure before proceeding - this indicates a gap in the proxy implementation that needs to be fixed.

### Step 10: Verify Replay Quality

Check the new replay for remaining issues:

```powershell
# Check for remaining unimplemented calls
./build/Debug/bin/slang-replay.exe -d "replay-capture\stream.bin" | 
    Select-String -Pattern "UNIMPLEMENTED" | 
    ForEach-Object { $_.Line -replace '.*String: "UNIMPLEMENTED: ([^"]+)".*', '$1' } | 
    Select-Object -Unique |
    Tee-Object remaining-unimplemented.txt

# If empty, all functions are implemented!
if ((Get-Content remaining-unimplemented.txt).Count -eq 0) {
    Write-Host "✓ All functions implemented!" -ForegroundColor Green
}
```

### Step 11: Repeat or Complete

- **If replay fails** → Debug the replay, fix serialization gaps
- **If unimplemented functions remain** → Go to Step 5
- **If tests pass but different test fails** → Go to Step 1 with new test
- **If all tests in category pass** → Move to next test category
- **If all tests pass and replay succeeds** → Success! 🎉
