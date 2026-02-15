# Switch Reconverged Lowering Tests

These tests verify the switch lowering pass that transforms `switch` statements with 
fallthrough to use reconverged control flow via a two-switch approach.

## Purpose

The switch lowering is needed for SPIR-V targets because `OpSwitch` has undefined 
reconvergence behavior with non-trivial fallthrough (cases with wave/subgroup operations 
that fall through to subsequent cases).

See GitHub issue #6441 for details: https://github.com/shader-slang/slang/issues/6441

## How Tests Trigger Lowering

The lowering pass uses an allowlist-based check to determine if transformation is needed.
It only transforms switches where the fallthrough path contains operations NOT on the 
allowlist (e.g., wave operations, function calls).

These tests use `WaveGetLaneIndex()` multiplied by 0 in the fallthrough paths:
```hlsl
result += 1 + int(WaveGetLaneIndex()) * 0;  // Triggers lowering without affecting value
```

This pattern:
1. Includes a wave operation (triggers the lowering)
2. Multiplies by 0 (doesn't affect the actual computed value)
3. Keeps expected outputs simple and predictable

## Transformation Approach

The pass splits switches with fallthrough into two switches:

1. **First Switch (Dispatch)**: Sets `fallthroughSelector` and `fallthroughStage` variables,
   then branches to the second switch. All cases break (no fallthrough in IR structure).

2. **Second Switch (Execution)**: Dispatches on `fallthroughSelector`. Each case contains
   if-guarded stages using `if (fallthroughStage <= N)`. This ensures all threads in a
   fallthrough group reconverge at each if-merge point.

Non-fallthrough cases execute their code directly after the first switch dispatch and 
don't go through the second switch.

## Key Test: wave-fallthrough-gh6441.slang

This test reproduces the original issue from #6441 - a switch with `WaveActiveSum` in a 
case that falls through to default. This is the primary motivating case for the lowering.

## Test Categories

### Wave Operations in Fallthrough
Tests that verify wave operations work correctly when a case with wave ops falls through 
to another case. These are the primary motivating cases for this lowering.

### Conditional Break with Fallthrough
Tests where a case body contains conditional logic that may break or fall through 
depending on runtime values.

### Control Flow Interactions
Tests for switch inside loops with break/continue, nested switches, loops inside switch
cases, and switches nested inside loops.

## Test Approach

Each test runs on GPU backends that support wave operations:
- `-dx11` (DirectX 11)
- `-dx12` (DirectX 12)
- `-vk` (Vulkan)
- `-mtl` (Metal, when available on macOS)

CPU testing is not included because wave operations are not supported on CPU.

## Known Issues

The Metal wave tests (`wave-break-if-version.slang`, `wave-break-done-flag-if-version.slang`)
fail on Metal. This is a pre-existing Metal issue unrelated to the switch lowering pass.
