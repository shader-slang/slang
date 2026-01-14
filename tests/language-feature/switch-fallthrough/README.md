# Switch Fall-Through Tests

These tests verify switch statement fall-through behavior in Slang.

## Test Categories

### Trivial Fall-Through
Cases where multiple case labels share the same block (e.g., `case 0: case 1: ...`).

### True Fall-Through
Cases where code executes and flows into the next case without a break.
Variables modified in one case retain their values in subsequent cases.

### Target-Specific Behavior
Some targets (FXC/DX11, WGSL) don't support fall-through natively.
The compiler restructures fall-through by duplicating code for these targets,
which may affect wave/subgroup convergence (warning 41026 is emitted).
