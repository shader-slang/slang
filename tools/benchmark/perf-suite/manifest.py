"""Data-driven description of the perf suite: which workloads exist, how to
invoke slangc for each, and which phase timers are the primary signal.

A WorkloadSpec drives both generation (Phase 2) and the release sweep (Phase 3).

Compile modes:
- "target"  : single-file compile to a GPU target with an entry point. Triggers
              the full pipeline incl. linkAndOptimizeIR / specializeModule /
              autodiff lowering. Default target spirv (-emit-spirv-directly).
- "module"  : compile a single file to a serialized .slang-module. Downstream-
              free; exercises only parse / sema / generateIR. Used for front-end
              buckets where a clean, tool-independent number is preferable.
- "link"    : multi-file. Precompile every non-main file to .slang-module, then
              compile the main file (the one whose name contains "main") to a
              target against them. Stresses module read + linkIR.
"""

from dataclasses import dataclass, field

import workloads


@dataclass
class WorkloadSpec:
    name: str
    bucket: str
    gen: object  # callable(n) -> {filename: source}
    default_size: int
    mode: str = "target"
    # extra slangc flags appended after the standard ones
    extra_flags: list = field(default_factory=list)
    # phase timers that best localize this bucket's cost
    primary_timers: list = field(default_factory=lambda: ["compileInner"])
    # intentionally non-zero exit (compile errors expected)
    expect_fail: bool = False
    # additional sizes worth sweeping for scaling curves (optional)
    sweep_sizes: list = field(default_factory=list)
    # emit reflection JSON (bench.py supplies a writable per-run path). Exercises
    # the reflection serializer in addition to the layout engine.
    reflection_json: bool = False
    # for multi-file/corpus workloads: the file to compile (imports resolve via
    # -I <gendir>). If None, bench picks the file containing "main", else first.
    main_file: str = None


# Standard target invocation avoids GPU drivers and stays comparable across
# releases. Downstream spirv-opt may run if bundled; primary_timers are all
# Slang-internal (measured before downstream) so localization stays clean.
SPIRV = ["-target", "spirv", "-emit-spirv-directly"]

WORKLOADS = [
    # ---- per-compile floor (core-module load + link) ---------------------
    WorkloadSpec(
        name="minimal",
        bucket="core_link",
        gen=workloads.gen_minimal,
        default_size=0,  # fixed; near-empty shader
        mode="target",
        extra_flags=SPIRV,
        # loadBuiltinModule is per-process (excluded from compileInner) but still
        # reported; readSerializedModuleIR + linkIR are the core-module-size signal.
        primary_timers=["compileInner", "linkIR", "readSerializedModuleIR",
                        "loadBuiltinModule"],
    ),
    # ---- shared-infrastructure / scaling stressors -----------------------
    WorkloadSpec(
        name="ir_builder",
        bucket="ir_infra",
        gen=workloads.gen_ir_builder,
        default_size=4000,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["generateIR", "simplifyIR", "compileInner"],
        sweep_sizes=[1000, 2000, 4000, 8000],
    ),
    WorkloadSpec(
        name="serialize",
        bucket="ir_infra",
        gen=workloads.gen_serialize,
        default_size=1500,
        mode="module",
        primary_timers=["writeSerializedModuleAST", "writeSerializedModuleIR", "compileInner"],
        sweep_sizes=[500, 1000, 1500, 3000],
    ),
    WorkloadSpec(
        name="conformance",
        bucket="sema",
        gen=workloads.gen_conformance,
        default_size=600,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[150, 300, 600, 1200],
    ),
    WorkloadSpec(
        name="loop_unroll",
        bucket="loop_unroll",
        gen=workloads.gen_loop_unroll,
        default_size=300,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["unrollLoopsInModule", "simplifyIR", "compileInner"],
        # default_size must be a member so a --sweep run also yields the canonical
        # (default-size) point used for cross-release comparison.
        sweep_sizes=[100, 200, 300, 600],
    ),
    # ---- suspected-regression features -----------------------------------
    WorkloadSpec(
        name="autodiff",
        bucket="autodiff",
        gen=workloads.gen_autodiff,
        default_size=200,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["compileInner", "linkAndOptimizeIR", "frontEndExecute"],
        sweep_sizes=[50, 100, 200, 400],
    ),
    WorkloadSpec(
        name="dynamic_dispatch",
        bucket="dynamic_dispatch",
        gen=workloads.gen_dynamic_dispatch,
        default_size=200,
        mode="target",
        # NB: -report-dynamic-dispatch-sites is informational but was added
        # mid-window (absent in older releases), so it is intentionally NOT used
        # here — dispatch lowering cost is captured via specializeModule anyway.
        extra_flags=SPIRV,
        primary_timers=["compileInner", "specializeModule", "linkIR", "linkAndOptimizeIR"],
        sweep_sizes=[50, 100, 200, 400],
    ),
    WorkloadSpec(
        name="existential_aggregate",
        bucket="dynamic_dispatch",
        gen=workloads.gen_existential_aggregate,
        default_size=100,
        mode="target",
        extra_flags=SPIRV,
        # existential field in a struct -> legalizeExistentialTypeLayout, plus a
        # witness-table-per-case specialization blowup. Neither is the primary
        # signal of the bare-local dynamic_dispatch workload.
        primary_timers=["compileInner", "specializeModule",
                        "legalizeExistentialTypeLayout", "simplifyIR"],
        # default_size (100) must be a member so --sweep yields the canonical point.
        sweep_sizes=[40, 80, 100, 200, 320],
    ),
    WorkloadSpec(
        name="diagnostics_errors",
        bucket="diagnostics",
        gen=workloads.gen_diagnostics_errors,
        default_size=400,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["frontEndExecute", "SemanticChecking", "compileInner"],
        expect_fail=True,
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="diagnostics_clean",
        bucket="diagnostics",
        gen=workloads.gen_diagnostics_clean,
        default_size=400,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["frontEndExecute", "SemanticChecking", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    # ---- core compiler-stage buckets --------------------------------------
    WorkloadSpec(
        name="parse",
        bucket="parse",
        gen=workloads.gen_parse,
        default_size=2000,
        mode="module",
        primary_timers=["parseTranslationUnit", "frontEndExecute"],
        sweep_sizes=[500, 1000, 2000, 4000],
    ),
    WorkloadSpec(
        name="sema_generics",
        bucket="sema",
        gen=workloads.gen_sema_generics,
        default_size=1000,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[250, 500, 1000, 2000],
    ),
    # ---- type checking: operator overload resolution + implicit conversion -
    # Front-end-only (module mode) stressors for the quietly expensive part of
    # semantic checking: every binary operator and cross-type assignment runs
    # overload resolution + conversion-cost ranking. parse/sema_generics don't
    # isolate this (uniform types / generic-constraint cost dominate there).
    WorkloadSpec(
        name="operator_typecheck",
        bucket="typecheck",
        gen=workloads.gen_operator_typecheck,
        default_size=800,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[200, 400, 800, 1600],
    ),
    WorkloadSpec(
        name="implicit_conversion",
        bucket="typecheck",
        gen=workloads.gen_implicit_conversion,
        default_size=600,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[150, 300, 600, 1200],
    ),
    WorkloadSpec(
        name="overload_resolution",
        bucket="typecheck",
        gen=workloads.gen_overload_resolution,
        default_size=600,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[150, 300, 600, 1200],
    ),
    WorkloadSpec(
        name="specialization",
        bucket="specialization",
        gen=workloads.gen_specialization,
        default_size=300,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["specializeModule", "linkAndOptimizeIR", "compileInner"],
        sweep_sizes=[75, 150, 300, 600],
    ),
    WorkloadSpec(
        name="inlining",
        bucket="inlining",
        gen=workloads.gen_inlining,
        default_size=400,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["simplifyIR", "linkAndOptimizeIR", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="codegen_spirv",
        bucket="codegen",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="module_link",
        bucket="module_link",
        gen=workloads.gen_module_link,
        default_size=100,
        mode="link",
        extra_flags=SPIRV,
        primary_timers=["linkIR", "compileInner"],
        sweep_sizes=[25, 50, 100, 200],
    ),
    # ---- source-target emission (the text backends spirv-directly skips) --
    # Same shader as codegen_spirv, but emitted to a textual GPU language so the
    # whole emitEntryPointsSourceFromIR path + target legalization (legalizeIRForMetal /
    # legalizeIRForWGSL) is exercised — entirely bypassed by -emit-spirv-directly,
    # so no other workload covers it. Metal/WGSL emit text with no external toolchain.
    WorkloadSpec(
        name="emit_metal",
        bucket="codegen_source",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "metal"],
        primary_timers=["emitEntryPointsSourceFromIR", "generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="emit_wgsl",
        bucket="codegen_source",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "wgsl"],
        primary_timers=["emitEntryPointsSourceFromIR", "generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    # ---- complexity ladder: realistic mixed shader, simple -> complex ------
    # Sweep this to see the holistic compile-time curve as a representative
    # shader grows in complexity (control flow + generics + dispatch + resources
    # + call depth all scale together), vs the single-axis stressors above.
    WorkloadSpec(
        name="complexity_ladder",
        bucket="realistic_scaling",
        gen=workloads.gen_complexity_ladder,
        default_size=160,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["compileInner", "frontEndExecute", "linkAndOptimizeIR",
                        "simplifyIR"],
        sweep_sizes=[20, 40, 80, 160, 320, 640, 1280],
    ),
    # ---- coverage-gap stressors (passes / paths no other workload hits) ---
    WorkloadSpec(
        name="resource_aggregate",
        bucket="resource_legalize",
        gen=workloads.gen_resource_aggregate,
        default_size=80,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["legalizeResourceTypes", "linkAndOptimizeIR", "compileInner"],
        sweep_sizes=[20, 40, 80, 160],
    ),
    WorkloadSpec(
        name="reflection_layout",
        bucket="reflection_layout",
        gen=workloads.gen_reflection_layout,
        default_size=120,
        mode="target",
        extra_flags=SPIRV,
        reflection_json=True,
        primary_timers=["compileInner", "frontEndExecute", "generateOutput"],
        sweep_sizes=[30, 60, 120, 240],
    ),
    WorkloadSpec(
        name="control_flow_ssa",
        bucket="control_flow",
        gen=workloads.gen_control_flow_ssa,
        default_size=120,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["simplifyIR", "frontEndExecute", "compileInner"],
        sweep_sizes=[30, 60, 120, 240],
    ),
    # ---- real-shader corpus ----------------------------------------------
    WorkloadSpec(
        name="mdl_dxr",
        bucket="real_world",
        gen=workloads.gen_mdl_dxr,
        default_size=0,  # fixed corpus; size ignored
        mode="target",
        extra_flags=SPIRV,
        main_file="hit.slang",
        primary_timers=["compileInner", "frontEndExecute", "linkAndOptimizeIR"],
    ),
]

BY_NAME = {w.name: w for w in WORKLOADS}
