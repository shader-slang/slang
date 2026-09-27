# Qualify the typed SSA copy-helper boundary

This bounded ExecPlan follows `.agent/PLANS.md` and the committed NVVM-plan exception. The loop stays
active; skip Slack, no push/system changes. Root owns acceptance/docs, reused author array_probe280
owns ignored prototypes, and separate reused reviewer pch275 audits read-only. No production edits.

## Purpose and Observable Result

Determine whether281's small NVVM noinline/optnone copy helper remains correct for values that cannot
be reconstructed by rereading a source address, underaligned storage, and larger aggregate signatures.
This gate decides whether to pursue a production helper; it is not itself feature implementation.

## Progress

- [x] 2026-09-27: Prototype281 committedafbdaaa80; read currentSTATUS/WORKFLOW, clean checkout.
- [x] 2026-09-27: Select helper producer/alignment/size gate; independent reviewer provides shape rules.
- [x] 2026-09-27: All37runtime/8source/2config/576input/22pin identities match279.
- [x] 2026-09-27: Eight cells and runners frozen; root and independent pre-execution source/oracle
      review pass. Small kernels check pointerlowbits and canaries0/37/38/39 around36bytepayload.
- [x] 2026-09-27: Small ordinaryO0/helperO0/helperO3 pass; ordinaryO3 wrong18.
      Helper preserves constructed/earlier-load/phi values, actualalignment1 and allcanaries.
- [x] 2026-09-27: N17O0/O3 compile; N65536O0 hits120-second bound (rc124),
      N65536O3 explicitly not run under frozen stopping rule. Caller ABI expands strongly atN17.
- [x] 2026-09-27: Separate reused reviewer verifies36 references, all8 dispositions, output/SSA/
      alignment/growth traces and source deltas. Root verifies unchanged identities and strict record
      generation. Completed report/record/navigation ready for formatting and local research commit.

## Context and Current Pipeline

Research280 shows preexisting wrong whole-array stores for nested integer records in NVRTC O3/NVVM O3.
Prototype281 fixes small direct-vendor cases through a typed function containing only the whole store,
marked noinline/optnone, while caller compilation remainsO3. Pointer memcpy/loops also pass but need
correct snapshot storage; rereading an earlier load's address can change semantics. The provider's
store interface already receives a canonical typed SSA value. This gate tests whether that existing
boundary can be preserved without inventing a second representation or expanding arrays in provider IR.

## Scope and Non-Goals

Small N3 GPU prototypes cover fully constructed aggregate values, phi/selection between distinct
complete values with both branches, a retained earlier load after all source fields change, and a
real byte-offset1 destination with align1 and surrounding canaries. Use NVVM O0/O3 and a corresponding
ordinary-store control where informative. Freeze two combined small sources (ordinary control/helper)×O0/O3, four GPU cells. Large
N17/N65536 helper probes compile only, under120seconds and4GiB virtual memory each; no large GPU launch.
The memory bound is an explicit experiment constraint; failure establishes rejection under that bound. Stop remaining large cells after the first size-gate timeout/rejection, retaining explicit not-run
records for every frozen cell. No NVRTC remedy,
production compiler/provider/harness/corpus changes, timing optimization claim or huge-array support.

## Architecture and Invariants

Canonical Cell={i16,{i16,i32}} has leaves0/4/8, size12; Payload is its fixed array. Constructed SSA values
must initialize every semantic leaf. Phi inputs are complete and distinct. Earlier-load tests capture
before mutation, then observe both old snapshot and updated source independently. No source-pointer
substitution is permitted. The destination allocation for underalignment has enough bytes for the
entire DataLayout size plus canaries, an align4 byte root with actual offset1 alignment1, and a typed
bitcast. Helpers and verification loads must not claim align4 there. Padding inside the object is
not observed as a semantic value; canaries lie outside the complete object footprint.

Caller parameter materialization is part of the test, not just the helper body. No noalias/dereferenceable
claims or ABI changes are inferred. Compile-only probes must keep the helper call and observable copy
live; a dead-code-eliminated helper would not establish size behavior. A large compile timeout/failure
rejects that size qualification and requires reconsideration, not an arbitrary unrolling cutoff.

## Interfaces and Dependencies

Reuse281 direct libNVVM/CUDA executor under `build/nvvm-ssa-copy-boundary282`, SM80 on L4SM89,
CUDA12.9.2, legacy NVVM IR. Actual loaded compiler/provider remain accepted279 (compiler9e013b2c,
version295-g0043e8d17, providera861b242/ABI42). Preserve37runtime/8source/2config/576inputs/22pins.
The NVVM12.9 spec lists noinline/optnone support;281 proves installed behavior for small aligned values.

## Milestones

1. Root verifies inherited identities. Author freezes source-generated independent outputs, complete
   test inventory, modes and runner hashes, then reviewer checks valid source/pointer/SSA shapes.
2. Run serialized process groups with per-cell120-second bounds and overall gate≤1800seconds, max4CPU.
   Retain every failed/unsupported attempt. Require actual complete output, not compiler agreement.
3. Inspect emitted PTX helper and callers, field offsets, canary preservation and source mutation.
   Compare source instruction counts with actual PTX/ABI growth at17/65536; never equate compact source
   IR with bounded backend compilation or storage.
4. If correctness and size evidence justify a helper, identify a bounded production slice and required
   provider/full checkpoint obligations. If not, close the rejected candidate honestly and select a
   different principled boundary. Do not add a fallback merely to rescue the proposed method.
5. Verify accepted identities unchanged; finish compact report/record/plan/navigation, format and commit.

## Validation and Acceptance

Every requested cell records compile/process return codes, executed/passed/ignored counts, diagnostic,
canonical shape, source/options hashes and actual buffer. Independent reviewer audits contracts and
complete evidence. No production changes means full279/targeted233/cadence0 and1740corpus outcomes
remain inherited, not fresh. Research280 failures stay explicitly open. Any later implementation needs
its own bounded plan and full gates because provider changes have shared scope.

## Failure and Recovery

No installed binaries are overwritten. Retain timeout/compile errors and original prototypes before
correcting syntax or harness issues in a separate attempt. A semantic failure blocks adoption of this
method; it does not erase prior accepted279 evidence. User stop instructions take precedence. No
system/toolkit changes or retries that hide failures.

## Decision Log

- 2026-09-27, root: Test non-addressable SSA and real underalignment before selecting the281 helper.
  Test large signature/caller growth independently of pointer-copy growth; they have different ABI costs.

## Surprises and Discoveries

Pending. Existing evidence qualifies only N3 aligned loaded aggregates; it says nothing about huge
by-value parameter lowering or underalignment.

## Outcomes and Retrospective

The typed helper passes the broader small correctness domain but fails the frozen large-size gate.
Reject it as a general production correction under these constraints; no arbitrary size cutoff or
fallback follows. Installed279 identities remain unchanged. Next candidate: test whether weakening
the store alignment annotation alone preserves an actually aligned destination without new call ABI.

## Artifacts and Hand-Off

Raw root above; durable report.slice-282-ssa-copy-boundary.md, research-evidence.slice-282.json and
this plan. Resume using frozen inventory and owned gate results before re-executing any cell.
