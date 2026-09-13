# Matrix run, 2026-09-13

The first full run of the `SLANG_HASH` × `SLANG_HASHMAP` matrix: 8 maps × 4 hashes,
built and tested in both configurations, then benchmarked with `tools/compile-perf`.

**Do not pick a default from these numbers.** The README's Category 1 blockers
([01](01-combine-hash-no-finalisation.md), [14](14-khasuniformhash-audit.md)) and
[03](03-hashset-uses-dictionary-with-dummy-value.md) had not landed when this ran, and each of
them moves the ranking rather than the level. What the run is good for is the correctness result
below, a sanity check that every backend compiles and passes, and a baseline to diff against
once those three land. See [Confounds](#confounds).

---

## Correctness: the run's actual finding

Only `UNORDERED_DENSE` produced a working compiler when this started. Seven of the eight maps
miscompiled or crashed, which turned out to be nine latent defects on master, all of them
independent of this experiment and none of them in the map layer.

`unordered_dense` is the only one of the eight that iterates in insertion order. Every defect
was code that depended on the iteration order of a `Dictionary`, and insertion order happened to
be the order it wanted.

| Fix                                                         | What it was                                                                                                         |
| ----------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------- |
| `descriptor_handle` promotion joined, not expanded          | `{spirv}` became all seven targets; the first target visited decided overload selection, so a SPIR-V compile emitted the HLSL body of `Texture.Load` and failed in spirv-emit |
| Flattened entry point struct fields copied in field order   | Keyed on `IRStructField*`, so ten compiles of one fragment shader gave seven different orderings of the same assignments |
| SPIR-V constant keys compared by bits                       | `Hash<double>` separates `0.0` and `-0.0` deliberately; `operator==` did not, and a hash map may assume equal keys hash equally. Abseil's debug build asserts it, and aborted on any shader with a float constant |
| `checkCapabilityRequirement` reports a fixed failure        | Which capability a diagnostic blamed, and how many it listed, changed between runs                                   |
| Profile-upgrade diagnostics name the right atoms (×2 sites) | Took the first conjunction of two separate maps, not necessarily the same pair from each; an HLSL compile was told its profile had been upgraded to include a GLSL extension |
| `spirv_asm` debug names emitted in source order             | `OpName` order followed the hash table's layout                                                                       |
| `isBetterForTarget` tiebreak scores the right set           | Both difference scores came from `thisSet`, so the comparison could never be true                                     |
| Container pool tests tolerate a map that frees on `clear()` | The pool's saving is unobservable on abseil, which releases capacity; the tests required it                          |

Two of these produce different output on different runs of one binary, from one input: the
keys are pointers, and addresses vary per process. `unordered_dense` hides that because its
iteration order does not depend on the hash at all.

**Validation.** All 32 combinations pass the full suite in Release; 20 in Debug against the set
of tests that ever failed; 64/64 `slang-bootstrap` core-module compiles across both configs.
Failures seen under load were re-run individually with `sti -j1`: 85 such failures, all
transient, none genuine.

---

## Performance

`tools/compile-perf/bench.py`, 30 workloads (`mdl_dxr` did not run, see Caveats), metric
`compileInner` median, one combination at a time on an otherwise idle machine.

Five interleaved passes over the four contending maps × four hashes; one pass over the other
four maps, dropped once the first pass put them out of reach. Ratios are computed within a pass
against that pass's own `wyhash-unordered_dense`, so drift between passes cancels.

### By map, median across that map's hashes and passes

| map                          |  ratio | n   |
| ---------------------------- | ------: | --- |
| `boost_flat`                 | 0.953x | 20  |
| `unordered_dense` (current)  | 0.998x | 20  |
| `tsl_robin`                  | 1.000x | 17  |
| `absl_flat`                  | 1.003x | 20  |
| `boost_node`                 | 1.023x | 4   |
| `boost_unordered`            | 1.112x | 4   |
| `std`                        | 1.119x | 4   |
| `absl_node`                  | 1.119x | 4   |

### By hash, median across that hash's maps and passes

| hash     |  ratio | n   |
| -------- | ------: | --- |
| `ABSL`   | 0.990x | 23  |
| `BOOST`  | 0.995x | 23  |
| `WYHASH` | 1.000x | 24  |
| `STD`    | 1.012x | 23  |

A 2% span, inside the per-pass spread. On this evidence the hash function does not matter —
but see [Confounds](#confounds), because two of the three blockers are specifically about the
hash axis.

### Per combination, five passes

| combination              |  median |  min   |  max   | spread |
| ------------------------ | ------: | -----: | -----: | -----: |
| `std-boost_flat`         | 0.948x | 0.906x | 0.953x |   4.7% |
| `boost-boost_flat`       | 0.956x | 0.907x | 0.978x |   7.1% |
| `wyhash-boost_flat`      | 0.957x | 0.951x | 0.965x |   1.4% |
| `absl-boost_flat`        | 0.963x | 0.913x | 0.975x |   6.2% |
| `wyhash-absl_flat`       | 0.964x | 0.954x | 0.968x |   1.5% |
| `absl-absl_flat`         | 0.972x | 0.931x | 0.998x |   6.6% |
| `boost-tsl_robin`        | 0.991x | 0.942x | 0.999x |   5.7% |
| `boost-unordered_dense`  | 0.993x | 0.982x | 1.001x |   1.9% |
| `std-unordered_dense`    | 0.994x | 0.947x | 1.003x |   5.6% |
| `absl-unordered_dense`   | 0.996x | 0.954x | 1.006x |   5.2% |
| `absl-tsl_robin`         | 0.999x | 0.950x | 1.007x |   5.7% |
| `wyhash-unordered_dense` | 1.000x |      — |      — |      — |
| `wyhash-tsl_robin`       | 1.002x | 0.994x | 1.007x |   1.4% |
| `std-tsl_robin`          | 1.015x | 0.959x | 1.021x |   6.3% |

`boost_flat` is first at every hash and in every pass, by about 4%.

### Measurement spread

Re-running one binary under a second label gave 0.997x on the aggregate, which suggested a 0.3%
noise floor. That was optimistic: across five passes the same combination varies by up to 7%.
Only the median across passes should be read; a single pass cannot resolve a few percent.

The first pass had `absl_flat` at 1.084x and 1.090x with the `BOOST` and `STD` hashes, which
looked like a map/hash interaction worth reporting. It did not reproduce in four further passes.
It was one bad pass.

---

## Confounds

From the README's own analysis, three landed changes will move this ranking:

- **[01](01-combine-hash-no-finalisation.md)** — `combineHash` does not finalise, so the low bits
  of every structured key degenerate. ankerl and boost remix a non-avalanching hash; abseil and
  `tsl::robin_map` do not. `absl_flat` and `tsl_robin` are therefore penalised here for Slang's
  fold, not for anything about those containers.
- **[14](14-khasuniformhash-audit.md)** — ankerl honours `kHasUniformHash` and boost ignores it
  (boost keys off `boost::hash_is_avalanching`). For every string-keyed map those two backends
  are hashing differently, which is part of what the "by hash" table above is measuring.
- **[03](03-hashset-uses-dictionary-with-dummy-value.md)** — `HashSet<T>` is
  `Dictionary<T, _DummyClass>`, so `HashSet<IRInst*>` stores 16-byte entries where 8 would do.
  Entry size is a first-order input to flat-versus-node performance, and the flat maps take the
  top four places here.

The ~4% for `boost_flat` is the number most likely to survive, since it leads at every hash and
the three confounds work against its closest rivals rather than for it. Re-run after those land
before pinning a default.

## Caveats

- **`mdl_dxr` did not run** in any measurement — it needs the MDL-SDK shaders in
  `tools/compile-perf/corpus/mdl`, which are not present. That is the suite's only real shader
  and the README calls it the realistic end-to-end signal. Everything here is synthetic stress
  workloads, which the README says amplify one pass each and are "a sensitivity figure for that
  pass, not a user-facing slowdown".
- Geometric mean over workloads weights a 5 ms workload like a 500 ms one. On summed time the
  same comparison is ~2.5% rather than ~4%.
- One machine, Release builds, compile time only — not shader runtime.

## Reproducing

```bash
extras/hashmap-matrix.sh --all                    # build the matrix
cd tools/compile-perf
python3 bench.py --slangc ../../build-wyhash-boost_flat/Release/bin/slangc --label boost_flat
```

Run one combination at a time on an idle machine, and repeat each several times under different
labels; compare medians across the repeats rather than single runs.
