# Slice 214: supervise finite complex compilation batches

## Motivation

The unchanged tiled-brass material already compiles and assembles for `eval_buffer` and
`sample_buffer` at NVRTC O3 and NVVM O0/O3. Running those six cells creates six independent compiler
processes and global sessions. Research 212 measured a 10.767% improvement for complete API session
lifetimes, but excluded executable startup and JSON-RPC supervision. This slice tests that candidate
at the actual process boundary while retaining fresh-process validation.

There are still no application bindings, texture/LUT inputs or output oracle for this material.
The work measures compilation and assembly, with no material kernel execution or runtime claim.

## Proposed solution

The runner now supports an explicit `--test-server` path. It first compiles and assembles each
identity in a fresh `slangc` process, then runs batches of at most six serial requests through the
existing test server. Each shared output must exactly equal its successful fresh reference and
retain the expected entry and SM target; final shared outputs assemble independently. Omitting the
option retains the existing fresh-process behavior, schema 1 and exit meanings.

The predeclared process-lifecycle gate passes: **9.991360 seconds fresh versus 8.190561 seconds
shared**, an **18.02355% median reduction** for the six-cell compilation segment. All 168 warmup and
measured PTX outputs are byte-identical, and all six distinct hashes independently assemble. This
is not an overall runner speedup: the opt-in path also pays for mandatory fresh references and their
assembly. It was slower at both observed complete invocation settings: 24.08 versus 13.40 seconds
for one sample, and 48.83 versus 43.87 seconds at the CLI defaults. Keep the fresh default; this
option is not recommended as a routine checkpoint accelerator.

## Change summary

- `issue-nvvm-backend/complex-test-server.py` supervises the existing framed JSON-RPC protocol,
  serial requests, first-request/startup deadline, bounded shutdown and process cleanup.
- `issue-nvvm-backend/run-complex-corpus.py` adds the explicit opt-in, exact reference checks,
  bounded selection and separate primary/reference/protocol/timing evidence. `compile_command`
  shares the original options between fresh and shared paths.
- `issue-nvvm-backend/test-run-complex-corpus.py` exercises the default and shared paths, controlled
  protocol faults, and the existing real-server DIE/KILL/GARBLE hooks using standard-library tests.
- The completed plan, full outcome tables, runtime manifest, STATUS and architecture document retain
  the decisions and preservation evidence. Raw samples, scripts, stdout/stderr, PTX and cubins remain
  in ignored `build/nvvm-loop/slice-214-*` directories.

There are no compiler, provider, test-server, CMake, ABI, material-source or runtime-oracle edits.

## Concepts and vocabulary

A **primary cell** is one workload/entry/backend/optimization identity. Repeated attempts and fresh
references do not create new coverage. A **batch** is one process containing at most six requests,
including startup and final exit. A **sample** contains all requested cells, possibly across several
bounded processes. Sample lifecycle timing sums those processes before taking its median, so an
unequal final batch cannot distort the statistic.

**Service latency** is the supervisor-visible time for a CLI invocation or a shared request/reply.
It is not an isolated compiler call: the first shared request includes startup, while final shared
global teardown belongs to the batch total. **ExecutionResult** is the existing server's typed
JSON result, including both a signed SlangResult and its signed tool return code. A compilation
failure can be `returnCode=-1` over this protocol versus exit 255 through a POSIX CLI process.

## Process report

### Existing ownership boundary and exact compiler inputs

Consider this invocation from the repository root after sourcing `build/nvvm-loop/slice-203-env.sh`:

```bash
python3 issue-nvvm-backend/run-complex-corpus.py \
    --slangc build/RelWithDebInfo/bin/slangc \
    --test-server build/RelWithDebInfo/bin/test-server \
    --build-label RelWithDebInfo \
    --provider build/RelWithDebInfo/bin/libslang-llvm-nvvm.so \
    --cuda-root /usr/local/cuda-12.9 \
    --warmup 0 --samples 1 --output build/nvvm-complex-shared
```

`compile_command` produces the original source, entry, compute stage, PTX target, SM80 capability,
optimization, backend and `-report-perf-benchmark` arguments. The fresh pass invokes `slangc` with
that array; the server receives the same array excluding only the executable name. Output paths
identify each separate attempt. No source normalization or PTX canonicalization hides differences.

The existing `TestServer::_executeTool` obtains the GLSL-enabled `m_session` from
`getOrCreateGlobalSession` and calls `SlangCTool::innerMain`. That function creates a fresh
`ICompileRequest`, selects command-line compiler mode, processes the exact arguments and compiles.
It releases the request on return. The supervisor reuses only the server's global session, then
sends the existing `quit` call and waits for process exit before closing the batch timer. Quit has
no response. No global session survives to the next batch and no new compiler executable is needed.

This is intentionally supported test-server input. There is no alternate AST/IR/Val/witness spelling
to repair, no consumer-side compiler patch, and no new compiler helper or fallback. The new Python
helpers own command construction, wire framing, process lifecycle and artifact evidence, which are
the responsibilities tested here.

### Failure accounting and bounded transport

`_frame`, `_read_responses`, `_unique_object` and `_validate_response` implement the existing
Content-Length protocol. Headers are limited to 4096 bytes and bodies to 16 MiB. Duplicate headers
or JSON fields, malformed/truncated frames, deeply nested parser failures, wrong IDs, JSON-RPC
errors and invalid ExecutionResult fields fail the active request. The returnCode/SlangResult
relationship follows `TestToolUtil::getReturnCode`; negative protocol codes remain signed.

For example, if request 2 dies after request 1 completed, the report retains request 1's successful
reply, request 2's crash or timeout, and request 3 through 6 as incomplete. It never retries or
falls back to fresh compilation. Output paths are removed before dispatch so an earlier artifact
cannot satisfy an interrupted request. A failed fresh reference stops shared dispatch, and a
byte mismatch or missing shared PTX fails validation. Six successful replies still fail the batch
if quit times out or the process exits unsuccessfully; their completed evidence remains visible.

The first deadline begins before `Popen`; later deadlines begin before their request write.
A writer handles short writes while an independent reader drains stdout, and stderr goes directly
to a raw file. The supervisor kills and waits on errors, including an owned POSIX process group
whose leader has exited while a child retains a pipe. Windows uses server-process termination;
that platform path was not exercised here. Full raw stdout and stderr survive faults.

Schema 2 separates six primary cells from auxiliary fresh-reference and batch records. Compiler
phase medians remain null because global profiler totals can accumulate across requests. Failed
batches retain elapsed latency but cannot contribute to successful timing medians. The final report
status checks both cell results and batch lifecycle success.

### Fixed paired performance evidence

Before implementation, the plan required at least 10% median total six-cell lifecycle reduction
and no greater than 5% per-identity service-latency regression where boundaries can be compared.
Two paired warmups precede twelve measured pairs. Policy order alternates, and six forward plus
six reverse rotations place each identity twice at every position under both policies. There are
no timing-based exclusions, threshold changes, retries or competing builds/tests during this phase.

Both policies use `perf_counter` and precise pipe supervision; the fresh baseline uses
`Popen.communicate` rather than the default runner helper's roughly 50 ms polling. Each fresh
sample includes six process creations/exits. Each shared sample includes server creation, requests,
framing, diagnostics, quit, process exit and pipe cleanup. Validation and independent assembly occur
outside the compilation segment. Commands differ only in output filenames and the expected process
transport. All 28 batches and 168 requests complete; every shared process exits 0.

| Complete six-cell lifecycle, seconds | Fresh processes    | Shared server     |
| ------------------------------------ | ------------------ | ----------------- |
| Median                               | 9.991360           | 8.190561          |
| Inclusive interquartile range        | 9.964907–10.018505 | 8.175442–8.193139 |
| Range                                | 9.947981–10.073678 | 8.138789–8.236507 |

Paired reductions range from 17.5312% to 19.0119%, with median 18.1555%. The modest order variation
contains no correctness failure. Per-identity service medians improve as follows:

| Identity          | Fresh seconds | Shared seconds | Reduction |
| ----------------- | ------------- | -------------- | --------- |
| eval / NVRTC O3   | 1.618909      | 1.254330       | 22.5201%  |
| eval / NVVM O0    | 1.605303      | 1.237950       | 22.8838%  |
| eval / NVVM O3    | 1.693765      | 1.338027       | 21.0028%  |
| sample / NVRTC O3 | 1.642529      | 1.279914       | 22.0766%  |
| sample / NVVM O0  | 1.631552      | 1.276095       | 21.7865%  |
| sample / NVVM O3  | 1.795692      | 1.441352       | 19.7328%  |

These service latencies distribute lifecycle work differently. They are not new measurements of
isolated compiler-call or phase speed. Research 212's compile-call observations remain separate
inherited research, and aggregate profiler output is never interpreted as per-cell phases.

After timing, review added only `RecursionError` to the protocol reader's error-handler tuple and
its controlled-peer test. The exact timed supervisor snapshot and final hash are recorded in
`slice-214-performance/source-identity.json`; the successful-response path is unchanged. Final
material command observations and protocol tests use the corrected final source. No timing rerun
is justified for that cold error-path correction.

### Whole-command costs and checkpoint

The external GNU `time` observations include Python startup, tool/version discovery, compilation,
reference work, assembly, report serialization and exit. They have 0.01-second resolution and were
run serially on an otherwise idle host. Each is one observation, not a repeated benchmark estimate.

| Invocation                           | Fresh whole command | Shared whole command | Shared reference work |
| ------------------------------------ | ------------------- | -------------------- | --------------------- |
| `--warmup 0 --samples 1`             | 13.40 s             | 24.08 s              | 12.815 s              |
| Actual defaults: warmup 1, samples 3 | 43.87 s             | 48.83 s              | 12.866 s              |

The first row has six primary attempts per policy; the second has 24. Both shared invocations add
six fresh reference attempts. Each fresh invocation assembles six final outputs; each shared
invocation assembles six reference and six final shared outputs. These repeated attempts remain
six canonical identities. Every PTX output in all four observations exactly matches accepted 213.
Neither setting amortizes the mandatory extra work. The opt-in capability and measured batch result
are useful bounded evidence, but do not justify making it the default or calling it a routine
checkpoint accelerator.

The full checkpoint compares frozen 452 × 3 and discovery 98 × 3 against the union of full 210 and
targeted 213. All 1,650 identities are present exactly once, with zero changes in classification,
return code, complete execution counts, diagnostic or canonical shape. All 546 distinct runtime
source files match the accepted starting tree; frozen/discovery selection and oracle arguments are
unchanged. The cumulative result remains **1,597 correct cells and 53 open failures**, with all four
resolved histories and original first-known/reproduction records retained. There are no additions,
missing/duplicate cells, repaired old failures or baseline resets.

| Gate                              | Final evidence                                                                                                  |
| --------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| Frozen                            | 1,356 fresh: 1,333 correct, five infrastructure and 18 preflight cells unchanged                                |
| Discovery                         | 294 fresh: 264 correct, 22 infrastructure, four runtime mismatch and four preflight cells unchanged             |
| GPU smoke                         | 4/4                                                                                                             |
| Routing/reporter                  | 32/32                                                                                                           |
| Discovery contracts               | 4/4                                                                                                             |
| Protocol/default/shared contracts | 15/15, including deeply nested JSON and real server failure hooks                                               |
| Complex                           | Six canonical compile/assembly cells pass on default and shared paths; reference and repeated work is auxiliary |
| Compiler units                    | Inherit 213: 477/477 plus one existing Windows-only skip                                                        |
| Toolkit                           | Inherit 213: 18/18                                                                                              |

Both full runtime runners exit 2 for the retained failures. The exact structured comparisons decide
preservation. Compiler/provider/library/toolkit/test source hashes match 213, so inherited unit and
toolkit evidence is explicit; the changed Python boundary receives fresh direct and full-checkpoint
validation. Final-source protocol tests and all material observations passed after the parser-only
correction. No GPU loss or retry occurred. The candidate full checkpoint resets cadence to zero only
when the parent accepts it.

Reproduce the retained command sets with `slice-214-after/run-gates.sh`,
`observe-default-counts.sh` and `protocol-final.sh` under `build/nvvm-loop`; use new output paths for
a real rerun. `runtime-validation.slice-214.json` records full outcome tables, all failure histories,
exact hashes, every timing sample and the supporting raw evidence. Parent independently recomputed
the timing medians and all 1,650 preservation results before handoff.

Starting revision is `786a2452f576fc620c9eeb4019bb277a20f394cd` on `nvvm-backend`. Native Ubuntu,
L4 SM89 driver 580.126.09, CUDA 12.9.2/NVRTC 12.9.86, LLVM 14, target SM80 and ABI 36 are unchanged.
Both tools resolve the same optimized compiler library, SHA256
`2775a5783a7dd1310ab9773d259bf1bdafc464a4f55a376c1fc22c472a6b1bf0`; provider SHA256 is
`ae0e7859f6062a69f191cace4ae8d96340c074815866808c5b43467f41144372`.
The final manifest records source/tool hashes and commands. No driver change, reboot, push or
worker commit occurred. Parent owns final acceptance and local commit.

Parent accepted the full checkpoint after independent diff, exact-result, timing and artifact review.
Latest implementation and full checkpoint are 214; implementation cadence resets to 0. The default
remains unchanged because neither observed short/default command is faster overall.
