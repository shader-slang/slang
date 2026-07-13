"""Data-driven description of the perf suite: which workloads exist, how to
invoke slangc for each, and which phase timers are the primary signal.

A WorkloadSpec describes one workload: how to generate its source, how to invoke
slangc, and which phase timers are the primary regression signal.

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
- "api"     : driven by native/api-driver.cpp against libslang instead of by
              slangc. Measures the compilation-API dimension (session setup,
              module loading, per-compile fixed overhead) that a one-shot CLI
              invocation cannot separate. api_cmd selects the driver mode.
"""

from dataclasses import dataclass, field

from . import workloads


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
    # additional sizes worth sweeping for scaling curves (optional; used by
    # bench.py --sweep). default_size must be a member so a swept run also
    # yields the canonical point used for cross-release comparison.
    sweep_sizes: list = field(default_factory=list)
    # emit reflection JSON (bench.py supplies a writable per-run path). Exercises
    # the reflection serializer in addition to the layout engine.
    reflection_json: bool = False
    # for multi-file/corpus workloads: the file to compile (imports resolve via
    # -I <gendir>). If None, bench heuristically picks the file whose name
    # contains "main", or the first file if none does. If set, used directly
    # as the compile entry point.
    main_file: str = None
    # sys.platform values this workload can run on (None = all). Workloads
    # needing a platform-bound downstream toolchain (dxc, nvrtc) set this;
    # bench.py excludes them from the DEFAULT set elsewhere but still runs
    # them when named explicitly in --only, failing loudly if the tool is
    # genuinely absent (downstream_required below is what enforces that).
    platforms: list = None
    # True for workloads whose sources come from an external (third-party)
    # corpus rather than a generator. The ASCII byte-determinism guard in
    # bench.py applies only to GENERATED sources; external corpora are read
    # with a tolerant decode (errors="replace") and may legitimately contain
    # non-ASCII (license headers, author names).
    # Contract: set True ONLY when `gen` reads a third-party corpus from disk;
    # generators that emit source must leave it False so the determinism guard
    # applies to them.
    external_corpus: bool = False
    # The workload's number is meaningless without its downstream compiler:
    # missing-downstream diagnostics (E00100 etc.), which bench.py normally
    # treats as benign, fail this workload instead — otherwise a host without
    # the toolchain would record Slang-internal timers and report OK.
    downstream_required: bool = False
    # for mode="api": the api-driver subcommand ("session-create",
    # "many-kernels", "module-graph", "module-graph-bin", "specialize",
    # "rt-composite"), the
    # root module name for the by-name-loading modes, and extra driver flags
    # (e.g. --reflect).
    api_cmd: str = None
    api_root: str = None
    api_flags: list = field(default_factory=list)


# Standard target invocation avoids GPU drivers and stays comparable across
# releases. Downstream spirv-opt may run if bundled; primary_timers are all
# Slang-internal (measured before downstream) so localization stays clean.
SPIRV = ["-target", "spirv", "-emit-spirv-directly"]

WORKLOADS = [
    # The list order is CANONICAL: it is the report's constant panel order
    # (via display_order below) and bench.py's run order. Sections: real-world
    # / holistic first, then the API-path workloads, then the compiler
    # pipeline front end -> back end (parse .. sema .. IR .. specialization ..
    # backends), downstream-compiler workloads last.
    # ---- real-shader corpus ----------------------------------------------
    WorkloadSpec(
        name="mdl_dxr",
        bucket="real_world",
        gen=workloads.gen_mdl_dxr,
        default_size=0,  # fixed corpus; size ignored
        external_corpus=True,
        mode="target",
        extra_flags=SPIRV,
        main_file="hit.slang",
        primary_timers=["compileInner", "frontEndExecute", "linkAndOptimizeIR"],
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
        sweep_sizes=[160, 320, 640, 1280],
    ),
    # ---- API-path workloads (application-integration dimension) -----------
    # Driven by native/api-driver.cpp against libslang (see DESIGN.md
    # "API-path workloads"). These cover the costs a one-shot slangc run pays
    # exactly once and cannot separate: session creation (core-module load),
    # per-compile fixed overhead across many small kernels, and import
    # resolution over a deep module graph.
    # ---- rt_renderer: generated renderer-shaped corpus (DESIGN.md Phase 2) --
    # Few×HEAVY programs over a ~100-module utility/scene/material library
    # behind IMaterial/IBSDF interfaces — the real-application shape where each
    # program pays the whole library's import cost. n = material count.
    WorkloadSpec(
        name="rt_renderer",
        bucket="rt_renderer",
        gen=workloads.gen_rt_renderer,
        default_size=24,
        mode="api",
        api_cmd="rt-composite",
        api_root="rt_kernels",
        primary_timers=["apiTotal", "apiLoadModule", "apiGetCode"],
    ),
    # One compute-kernel variant per material via IEntryPoint::specialize —
    # link-time specialization against interface-heavy cross-module code.
    WorkloadSpec(
        name="rt_renderer_specialize",
        bucket="rt_renderer",
        gen=workloads.gen_rt_renderer,
        default_size=24,
        mode="api",
        api_cmd="specialize",
        api_root="rt_compute",
        api_flags=["--impl-prefix", "Material_"],
        primary_timers=["apiTotal", "apiGetCode", "apiSpecialize"],
    ),
    WorkloadSpec(
        name="api_session_create",
        bucket="api_overhead",
        gen=workloads.gen_api_none,
        default_size=10,  # createGlobalSession+createSession iterations
        mode="api",
        api_cmd="session-create",
        primary_timers=["apiCreateGlobalSession", "apiCreateSession", "apiTotal"],
    ),
    WorkloadSpec(
        name="api_many_kernels",
        bucket="api_overhead",
        gen=workloads.gen_api_kernels,
        default_size=100,
        mode="api",
        api_cmd="many-kernels",
        primary_timers=["apiTotal", "apiLoadModule", "apiGetCode"],
    ),
    WorkloadSpec(
        name="api_module_graph",
        bucket="api_overhead",
        gen=workloads.gen_api_module_graph,
        default_size=150,
        mode="api",
        api_cmd="module-graph",
        api_root="graph_main",
        primary_timers=["apiTotal", "apiLoadModule", "apiGetCode"],
    ),
    # Same DAG loaded through serialized .slang-module binaries — the import
    # path where the 2026-07-03 module-loading regression (#11952) lives;
    # source-based loads (above) were flat across it.
    WorkloadSpec(
        name="api_module_graph_bin",
        bucket="api_overhead",
        gen=workloads.gen_api_module_graph,
        default_size=150,
        mode="api",
        api_cmd="module-graph-bin",
        api_root="graph_main",
        primary_timers=["apiTotal", "apiLoadModule"],
    ),
    # Per-program reflection walk (getLayout + full parameter/type-layout
    # traversal) over parameter-rich kernels — the binding-table query pattern
    # every API client pays per compiled program.
    WorkloadSpec(
        name="api_reflection",
        bucket="api_overhead",
        gen=workloads.gen_api_reflect_kernels,
        default_size=40,
        mode="api",
        api_cmd="many-kernels",
        api_flags=["--reflect"],
        primary_timers=["apiReflection", "apiTotal", "apiGetCode"],
    ),
    # One generic entry point specialized per impl type via
    # IEntryPoint::specialize — the one-kernel-per-material pattern; stresses
    # specialization + link per variant.
    WorkloadSpec(
        name="api_specialize",
        bucket="api_overhead",
        gen=workloads.gen_api_specialize,
        default_size=60,
        mode="api",
        api_cmd="specialize",
        api_root="spec_root",
        primary_timers=["apiSpecialize", "apiLink", "apiGetCode", "apiTotal"],
    ),

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
    # ---- core compiler-stage buckets --------------------------------------
    WorkloadSpec(
        name="parse",
        bucket="parse",
        gen=workloads.gen_parse,
        default_size=2000,
        mode="module",
        primary_timers=["parseTranslationUnit", "frontEndExecute"],
        sweep_sizes=[250, 500, 1000, 2000],
    ),
    WorkloadSpec(
        name="diagnostics_clean",
        bucket="diagnostics",
        gen=workloads.gen_diagnostics_clean,
        default_size=400,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["frontEndExecute", "SemanticChecking", "compileInner"],
        sweep_sizes=[400, 800, 1600, 3200],
    ),
    WorkloadSpec(
        name="sema_generics",
        bucket="sema",
        gen=workloads.gen_sema_generics,
        default_size=1000,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[125, 250, 500, 1000],
    ),
    WorkloadSpec(
        name="conformance",
        bucket="sema",
        gen=workloads.gen_conformance,
        default_size=600,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[600, 1200, 2400, 4800],
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
        sweep_sizes=[300, 600, 1200, 2400],
    ),
    WorkloadSpec(
        name="overload_resolution",
        bucket="typecheck",
        gen=workloads.gen_overload_resolution,
        default_size=600,
        mode="module",
        primary_timers=["SemanticChecking", "frontEndExecute"],
        sweep_sizes=[600, 1200, 2400, 4800],
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
        sweep_sizes=[500, 1000, 2000, 4000],
    ),
    WorkloadSpec(
        name="serialize",
        bucket="ir_infra",
        gen=workloads.gen_serialize,
        default_size=1500,
        mode="module",
        primary_timers=["writeSerializedModuleAST", "writeSerializedModuleIR", "compileInner"],
        sweep_sizes=[375, 750, 1500, 3000],
    ),
    WorkloadSpec(
        name="module_link",
        bucket="module_link",
        gen=workloads.gen_module_link,
        default_size=100,
        mode="link",
        extra_flags=SPIRV,
        primary_timers=["linkIR", "compileInner"],
        sweep_sizes=[50, 100, 200, 400],
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
        sweep_sizes=[50, 100, 200, 400],
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
        sweep_sizes=[25, 50, 100, 200],
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
        name="loop_unroll",
        bucket="loop_unroll",
        gen=workloads.gen_loop_unroll,
        default_size=300,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["unrollLoopsInModule", "simplifyIR", "compileInner"],
        sweep_sizes=[75, 150, 300, 600],
    ),
    WorkloadSpec(
        name="control_flow_ssa",
        bucket="control_flow",
        gen=workloads.gen_control_flow_ssa,
        default_size=120,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["simplifyIR", "frontEndExecute", "compileInner"],
        sweep_sizes=[60, 120, 240, 480],
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
        # Window starts at default_size: below N=80 the total is dominated by a
        # quasi-fixed front-end cost (type sharing makes per-item sema cheap)
        # that would floor-distort the fit; [80..640] measures the pass, not it.
        sweep_sizes=[80, 160, 320, 640],
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
        name="codegen_spirv",
        bucket="codegen",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=SPIRV,
        primary_timers=["generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
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
    WorkloadSpec(
        name="emit_hlsl",
        bucket="codegen_source",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "hlsl", "-entry", "computeMain"],
        primary_timers=["emitEntryPointsSourceFromIR", "generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="emit_glsl",
        bucket="codegen_source",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "glsl", "-entry", "computeMain"],
        primary_timers=["emitEntryPointsSourceFromIR", "generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="emit_cuda",
        bucket="codegen_source",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "cuda"],
        primary_timers=["emitEntryPointsSourceFromIR", "generateOutput", "compileInner"],
        sweep_sizes=[100, 200, 400, 800],
    ),
    # ---- downstream compilers (Windows perf runner only) -------------------
    # These measure the full pipeline INCLUDING the downstream compiler (dxc
    # for DXIL, nvrtc for PTX) — an internal application benchmark showed
    # downstream time is ~60% of a real app's combined compile time, and the
    # suite had no signal for it. generateOutput spans slang emit + the
    # downstream invocation; wall_ms is the end-to-end number.
    WorkloadSpec(
        name="codegen_dxil",
        bucket="codegen_downstream",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "dxil", "-profile", "sm_6_6"],
        primary_timers=["generateOutput", "compileInner"],
        platforms=["win32"],
        downstream_required=True,
        sweep_sizes=[100, 200, 400, 800],
    ),
    WorkloadSpec(
        name="codegen_ptx",
        bucket="codegen_downstream",
        gen=workloads.gen_codegen,
        default_size=400,
        mode="target",
        extra_flags=["-target", "ptx"],
        primary_timers=["generateOutput", "compileInner"],
        platforms=["win32"],
        downstream_required=True,
        sweep_sizes=[100, 200, 400, 800],
    ),
]

BY_NAME = {w.name: w for w in WORKLOADS}

# default_size must be a member of every sweep ladder so a swept run also
# yields the canonical point: analyze.canonical_runs collapses multi-size runs
# by `size == default_size`, and a ladder that omits it would silently feed a
# wrong-N point into the cross-release tracking series.
for _w in WORKLOADS:
    assert not _w.sweep_sizes or _w.default_size in _w.sweep_sizes, (
        f"{_w.name}: default_size {_w.default_size} not in sweep_sizes {_w.sweep_sizes}")


def display_order(names):
    """Return `names` sorted into the canonical manifest order (the WORKLOADS
    list above), which is also the order report panels render in. Names not in
    the manifest (e.g. retired workloads still present in stored results) sort
    after all known ones, alphabetically, so nothing silently disappears from
    the report."""
    pos = {w.name: i for i, w in enumerate(WORKLOADS)}
    return sorted(names, key=lambda n: (pos.get(n, len(pos)), n))
