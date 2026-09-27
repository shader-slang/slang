# Slice290: enforce the final catch-all output

## Motivation

`catch-all.slang` used `// CHECK-NEXT 7` without a colon, so FileCheck ignored its final output.
Slice289 independently confirmed the correct value on all three CUDA modes, but the native test could
still pass a wrong final result. The defect belongs to the test oracle.

## Proposed solution

Add the missing colon. The controlled comparison proves that this activates the intended check:

| Case                                                 | Complete output, decimal | Native result | Assessment                   |
| ---------------------------------------------------- | ------------------------ | ------------- | ---------------------------- |
| Old CHECK, deliberately corrupted shader             | 1,16,48,9                | Pass          | Oracle false pass            |
| Fixed CHECK, identical corrupted shader              | 1,16,48,9                | Fail          | Expected rejection of final9 |
| Fixed CHECK, normal shader; CPU and three CUDA modes | 1,16,48,7                | Four passes   | Correct output preserved     |

The two corrupted controls use NVVM O0. Their executable bodies are identical: only the CHECK colon
differs. The fixed rejection specifically reports `CHECK-NEXT: expected string not found` at the
final `CHECK-NEXT: 7`, after actual hexadecimal30. It is an output-check failure, not a compiler failure.

## Change summary

The sole tracked test change is one colon in
`tests/language-feature/error-handling/catch-all.slang`. Shader code, inputs and other CHECKs are
unchanged. This report, completed plan, compact [evidence](test-evidence.slice-290.json) and navigation
retain the proof. Raw control sources, commands, outputs and reviews stay under
`build/nvvm-catchall-oracle290`.

## Concepts and vocabulary

**Oracle false pass** means the native test reports success even though the deliberately altered output
disagrees with its intended final value. **Expected rejection** is the matching controlled failure after
the check is repaired. The maintained classifier's raw labels remain separate from these assessments.
Untyped output is hexadecimal, so the first three displayed words1/10/30 mean decimal1/16/48.

## Process report

The normal source calls:

```slang
handlerFunc(0, 0); // CHECK: 1
handlerFunc(1, 1); // CHECK-NEXT: 10
handlerFunc(2, 2); // CHECK-NEXT: 30
handlerFunc(3, 3); // CHECK-NEXT: 7
```

Only the final raw-control argument changes from3 to4. That call takes both success branches and
writes `4 + 4 + 1 = 9`; earlier outputs remain unchanged. Comparing the old and fixed CHECK on this
same shader isolates the missing colon. Removing the repair makes the controlled wrong result pass
again, establishing why the change is necessary at the oracle rather than in compiler lowering.
No helper, fallback, semantic representation or compiler special case is added.

A fresh bounded author prepared the exact diff and six-cell freeze. Root and a separate reused
reviewer checked the source relationships, independent buffers, mode routing and output paths before
execution. Three normal CUDA mirrors preserve the authored CPU ordinal2 contract with maintained
adaptation; the normal CPU cell runs the actual tracked `.slang.2` test. All six processes run serially
with retries disabled and180-second bounds. Complete hexadecimal outputs and native counts are checked.
There are no timeouts, missing cells or retries. Original API variants0/1 were not freshly executed.

The old-corrupt cell retains raw `correct` and one native pass, but its semantic assessment is an oracle
false pass against intended7. The fixed-corrupt cell retains raw `unclassified`, return1, one executed
and zero passed, alongside the specific FileCheck diagnostic and exact corrupt buffer. Neither is
misreported as a newly correct shader or an unexplained regression. All four positive cells return0,
execute/pass one test and ignore none.

Installed100 layout entries,37 runtime,11 qualified source,2 configuration,576 main input hashes and
22 dependency pins remain exact285. The changed test is outside the main corpus; broader289 results
retain their original source hash and remain inherited. Full285 remains580cases/576sources/1740cells,
1703correct/37unresolved/20histories, with unchanged checkpoint cadence. No compiler build or new
runtime-exception/performance claim. Next qualify runtime-loaded error inputs, since289's literals
could fold control flow. The authorized loop continues; Slack stays skipped.
