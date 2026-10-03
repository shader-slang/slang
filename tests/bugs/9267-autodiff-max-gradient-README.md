# Narrow repro for #9267: autodiff max() gradient depends on load pattern

Two kernels compute the same loss (`abs(pos) - half` -> `max(exceed, 0)` -> sum).
`loss_kernel_vec_max` uses `loadVecOnce<3>` (loop with mutable index).
`loss_kernel_vec_max_manual` uses three manual `loadOnce` calls.
Same `max()`, same inputs. Gradients differ -- max abs error 0.1667.

The bug is in how the backward pass reverses the loadVecOnce loop index,
not in the `max()` tie-breaking rule.

## Run

```bash
python3 tests/bugs/9267-autodiff-max-gradient-runner.py
# Exit 1 = gradients differ (bug present)
# Exit 0 = gradients match (fixed)
```

Requires PyTorch, slangtorch, CUDA. Tested with Slang 2026.2.2,
slangtorch 1.3.19, PyTorch 2.10.0, CUDA 13.0.
