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
    # for multi-file/corpus workloads: the file to compile (imports resolve via
    # -I <gendir>). If None, bench picks the file containing "main", else first.
    main_file: str = None


# Standard target invocation avoids GPU drivers and stays comparable across
# releases. Downstream spirv-opt may run if bundled; primary_timers are all
# Slang-internal (measured before downstream) so localization stays clean.
SPIRV = ["-target", "spirv", "-emit-spirv-directly"]

WORKLOADS = [
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
