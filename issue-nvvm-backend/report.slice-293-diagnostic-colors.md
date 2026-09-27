# Slice 293: recognize colored diagnostic output

## Motivation

Slice 290 deliberately changed a shader's final output from 7 to 9. FileCheck correctly rejected the
result, but the census classified its colored diagnostic as `unclassified`. The same text without
ANSI color codes was recognized as `runtime-mismatch`. Styling should not change the recorded cause
of a test failure.

## Proposed solution

Normalize ANSI SGR color/style sequences when interpreting diagnostic text and test summaries.
Preserve raw process output and the existing order that gives compiler, NVVM preflight and provider
errors precedence over secondary output mismatches. Share the normalization with discovery's
unavailable-entry-point check so both runners interpret the same text consistently.

## Change summary

The census parser and focused contracts change, with the discovery wrapper using the same helper.
This report, completed plan, full validation record and navigation preserve the evidence and original
failure histories. Compiler, provider, shader inputs, selected corpora and installed binaries remain
unchanged.

## Concepts and vocabulary

**SGR** means Select Graphic Rendition: terminal escape sequences that select presentation such as
bold or red text. **Failure classification** records the stage or kind of failure; correcting that
label does not make a shader pass. The **five outcome fields** are classification, return code,
execution counts, diagnostic and canonical shape.

## Process report

The actual FileCheck log places SGR escapes before and after `error:` and `CHECK-NEXT`. Those bytes
interrupt the census's existing mismatch expression. FileCheck produced valid styled text, so the
parser owns removing presentation before semantic matching. A compiler or shader change would be
the wrong layer.

The sole new helper, `normalize_diagnostic_output`, belongs at each text reader: `_classify_result`, `execution_counts` and
discovery's unavailable-entry-point wrapper. Normalizing only the classifier could recognize a
colored successful summary while the separately recorded execution counts stayed missing. Leaving
discovery's earlier wrapper unnormalized could lose its existing canonical capability diagnostic.
Raw subprocess output and log writing stay unchanged; no semantic matching rule is broadened.

The new contracts fail against the original parser and pass after the fix: eight census and eight
discovery tests. They cover the actual FileCheck escape pattern, compiler/preflight/provider
precedence, discovery's capability diagnostic, strict counts, preserved raw logs and non-SGR text.
The helper removes numeric/semicolon SGR sequences; it is not a general terminal interpreter.
The actual 290 log now classifies as `runtime-mismatch`, retaining its original hash, failed return
code and 0/1 execution summary. Its original `unclassified` record remains immutable.

All required fresh gates pass. The full checkpoint preserves all 1,740 outcomes across 580 cases
and 576 sources: 1,703 correct and 37 unresolved, with all 20 resolved histories intact. There are
no input or outcome transitions. Native identities remain exact: units 1,097 passed/13 skipped;
semantics 1,170 passed/78 skipped. Runtime smoke passes four cells; toolkit passes 18; the six
material PTX/cubin/resource sets equal285. The four runner suites pass 46 tests with one existing
skip. Earlier read-only reclassification of 1,740 archived logs also had zero deltas; that replay
is separately labeled and does not replace fresh execution.

A fresh author and separate reused reviewer checked the fix and frozen commands. Final evidence
review accepted all obligations without findings. Before/after identity checks preserve all 100 installed entries, 37 runtime
artifacts, 11 qualified sources, 2 configurations, 576 inputs and 22 pins. No build was performed.

The shared-runner change triggered the full checkpoint. Its exact comparisons preserve every
selected outcome and all known failure histories. The compiler's
qualified source and binary identity remain from slice 285, regardless of the later repository HEAD.
No numerical, kernel-speed or new language-support claim follows from this change.
