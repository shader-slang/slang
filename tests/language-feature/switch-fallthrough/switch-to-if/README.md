# Switch-to-If Lowering Tests

These tests verify the switch-to-if lowering pass, which transforms `switch` statements 
to equivalent `if` chains wrapped in a `do { } while(false)` loop.

## Purpose

The switch-to-if lowering is needed for SPIR-V targets because `OpSwitch` has undefined 
reconvergence behavior with non-trivial fallthrough (cases with wave/subgroup operations 
that fall through to subsequent cases).

See GitHub issue #6441 for details: https://github.com/shader-slang/slang/issues/6441

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

### Cumulative Predicate Logic
Tests that verify the cumulative predicate computation handles complex case orderings,
including default not at end, multiple fallthrough chains, and mixed break/fallthrough.

### Control Flow Interactions
Tests for switch inside loops with break/continue, nested switches, and switches 
nested inside loops.

## Test Approach

Each test runs on multiple backends to verify the lowering produces correct results:
- `-cpu` (always available, used as baseline)
- `-dx12` (when available)
- `-vk` (when available)
- `-mtl` (when available on macOS)
- `-wgpu` (WebGPU)
