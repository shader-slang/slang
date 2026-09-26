# Bounded shared-session appendix

This is a distinct fixed-order experiment, not the material fresh-process headline. Each of 11
fresh test-server processes handles the same six requests in order: evaluation NVRTC O3, NVVM O0,
NVVM O3, then sampling in those modes. Two batches warm up and nine are measured. The first request
has different initialization exposure; subsequent requests reuse process/session state. Do not infer
an isolated backend advantage from these request medians.

All six cells pass, all 66 request PTX outputs match their fresh references, all 11 batches complete
with exit zero and no retries, and all six fresh reference compiles/assemblies pass.

| Entry / mode           | Median request service time (ms) |
| ---------------------- | -------------------------------: |
| eval_buffer-nvrtc-o3   |                          1284.89 |
| eval_buffer-nvvm-o0    |                           883.38 |
| eval_buffer-nvvm-o3    |                          1067.22 |
| sample_buffer-nvrtc-o3 |                           904.18 |
| sample_buffer-nvvm-o0  |                           960.69 |
| sample_buffer-nvvm-o3  |                          1081.96 |

Request service time omits full process startup/shutdown, the fresh reference pass and output
validation. No compiler-phase median is available from this shared API path. The complete six-request
batch lifetime median is **6.263s** (IQR 6.246–6.283s).

| Cost scope                                                  | Seconds |
| ----------------------------------------------------------- | ------: |
| Fresh reference pass, six compiles/assemblies               |  11.130 |
| All 11 batch process lifetimes                              |  68.922 |
| Warmup batches included above                               |  12.481 |
| Runner entry through validation                             |  83.296 |
| Whole command, including Python startup/final serialization |  83.356 |

These scopes overlap; do not add them. Whole-command wall includes reference work, warmups, all
measured batches, validation, final assembly and report work. It is the cost of the entire protocol,
not one compilation. The sum of independent fresh-process medians is not a measured equivalent
batch and cannot establish a paired end-to-end speedup.

| Batch index | Warmup | Full process lifetime (s) | Shutdown (s) |
| ----------- | ------ | ------------------------: | -----------: |
| 0           | True   |                  6.241978 |     0.077931 |
| 1           | True   |                  6.239384 |     0.081954 |
| 2           | False  |                  6.282779 |     0.077563 |
| 3           | False  |                  6.241825 |     0.079366 |
| 4           | False  |                  6.281908 |     0.078654 |
| 5           | False  |                  6.216224 |     0.076554 |
| 6           | False  |                  6.309057 |     0.079530 |
| 7           | False  |                  6.246328 |     0.078323 |
| 8           | False  |                  6.262388 |     0.077690 |
| 9           | False  |                  6.336656 |     0.080138 |
| 10          | False  |                  6.263190 |     0.079181 |

[Structured appendix](shared-session.json) retains every batch lifetime, compact fresh-reference
results, command/exit evidence and raw-result SHA256. Complete requests, diagnostics and process logs
remain in the referenced raw file. Compiler/provider/toolkit identities match the main package.
Reproduce using the existing shared-session command in [RESULTS](../../RESULTS.md); keep its fixed
order and accounting explicit. No new measurement script is needed.
