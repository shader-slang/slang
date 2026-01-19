# Switch Reconverged Lowering Tests

These tests verify the switch lowering pass that transforms `switch` statements with 
fallthrough to use reconverged control flow via a two-switch approach.

## Purpose

The switch lowering is needed for SPIR-V targets because `OpSwitch` has undefined 
reconvergence behavior with non-trivial fallthrough (cases with wave/subgroup operations 
that fall through to subsequent cases).

See GitHub issue #6441 for details: https://github.com/shader-slang/slang/issues/6441

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

Each test runs on multiple backends to verify the lowering produces correct results:
- `-cpu` (always available, used as baseline)
- `-dx12` (when available)
- `-vk` (when available)
- `-mtl` (when available on macOS)
- `-wgpu` (WebGPU)
