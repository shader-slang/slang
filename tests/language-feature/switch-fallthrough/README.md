# Switch Fall-Through Tests

This directory contains tests for switch statement fall-through behavior in Slang.

## Test Categories

### Trivial Fall-Through (Currently Supported)

These tests cover "trivial fall-through" where multiple case labels share the same block.
This is currently supported in Slang.

- `trivial-fallthrough-case-grouping.slang` - Basic case grouping
- `trivial-fallthrough-with-default.slang` - Case grouped with default
- `trivial-fallthrough-var-different-init.slang` - Variables with different init values per case

### True Fall-Through (Implementation Pending)

These tests cover "true fall-through" where code executes in one case and then
continues to the next case without a break. These tests are DISABLED until the
feature is implemented.

- `basic-fallthrough-var.slang` - Basic variable modification across fall-through
- `fallthrough-chain.slang` - Chain of fall-through cases
- `fallthrough-transitive-chain.slang` - Variables assigned in one case, used 2+ cases later
- `fallthrough-to-default.slang` - Fall-through from case to default
- `fallthrough-var-declared-in-case.slang` - Variable scoping with fall-through
- `fallthrough-mixed-break-and-fallthrough.slang` - Mix of break and fall-through
- `fallthrough-nested-switch.slang` - Nested switch with fall-through
- `fallthrough-with-conditional.slang` - Conditional logic in fall-through cases
- `fallthrough-loop-interaction.slang` - Loops in fall-through cases
- `fallthrough-multiple-vars.slang` - Multiple variables across fall-through
- `fallthrough-only-some-vars.slang` - Optimization: only some vars need IRVar
- `fallthrough-break-label.slang` - Variable access after switch (at break label)

## Implementation Notes

The IRVar approach for fall-through involves:

1. **Detection**: During lowering, identify variables used in cases that are
   fall-through targets.

2. **IRVar Emission**: For such variables, emit `IRVar` (memory-based) at the
   switch scope instead of `IRParam` (SSA block parameters).

3. **Scoping**: The `IRVar` is emitted before the switch instruction so it's
   visible across all cases and after the switch.

4. **Optimization**: Only variables that actually need to persist across
   fall-through boundaries should use `IRVar`. Variables used only within
   a single case can continue to use `IRParam`.

See `docs/design/switch-fallthrough-irvar-plan.md` for the full implementation plan.

