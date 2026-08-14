"""The phase-bucket partition: nested timer trees whose leaves + synthetic
"(self)" residuals tile a root total exactly (compileInner for the compiler
TREE, apiTotal for API_TREE).

This lives in lib/ because it is an ANALYSIS contract, not a rendering
detail: breakdown.py renders stacked areas from it, daily_movers.py computes
pp-of-total attribution from it (the pp column sums to the overall %-change
precisely because these buckets tile the headline), and both must agree.
Keeping it here also breaks the former breakdown <-> daily_movers import
cycle that forced lazy imports and constrained where self-check fixtures
could live.
"""

# (timer, [children]) — the nested timer tree. Each parent gets a synthetic
# "<parent> (self)" residual = parent − Σ children, so buckets tile compileInner.
TREE = ("compileInner", [
    ("frontEndExecute", [
        ("parseTranslationUnit", []),
        ("SemanticChecking", []),
        ("generateIR", []),
    ]),
    ("generateOutput", [
        ("linkAndOptimizeIR", [
            ("specializeModule", []),
            ("simplifyIR", []),
            ("linkIR", []),
            ("unrollLoopsInModule", []),
            ("legalizeResourceTypes", []),
            ("legalizeExistentialTypeLayout", []),
            ("performMandatoryEarlyInlining", []),
            ("performForceInlining", []),
        ]),
        ("emitEntryPointsSourceFromIR", []),
    ]),
])


# Canonical bucket order + colors for the stacked view, grouped by stage:
# front-end = greens, linkAndOptimizeIR subtree = blues/purples, emit = oranges,
# residual = grey. Keeping order/colors fixed makes bars comparable across
# workloads at a glance.
BUCKET_ORDER = [
    ("parseTranslationUnit", "#c7e9c0"),
    ("SemanticChecking", "#41ab5d"),
    ("generateIR", "#006d2c"),
    ("frontEndExecute (self)", "#74c476"),
    ("specializeModule", "#6baed6"),
    ("simplifyIR", "#2171b5"),
    ("linkIR", "#08306b"),
    ("unrollLoopsInModule", "#9e9ac8"),
    ("legalizeResourceTypes", "#807dba"),
    ("legalizeExistentialTypeLayout", "#6a51a3"),
    ("performMandatoryEarlyInlining", "#bcbddc"),
    ("performForceInlining", "#dadaeb"),
    ("linkAndOptimizeIR (self)", "#4a1486"),
    ("emitEntryPointsSourceFromIR", "#fd8d3c"),
    ("generateOutput (self)", "#e6550d"),
    ("compileInner (self)", "#969696"),
]
BUCKET_COLOR = dict(BUCKET_ORDER)

# API-path phase tree: the api-driver's timers nest under apiTotal the same way
# the compiler timers nest under compileInner, so the same top-down allocator
# renders api workloads (mode="api") as stacked areas with apiTotal as the top
# edge. apiLoadModuleSource/apiWriteModule are deliberately absent: they time
# module-graph-bin's SETUP, which runs outside the apiTotal scope.
API_TREE = ("apiTotal", [
    ("apiCreateGlobalSession", []),
    ("apiCreateSession", []),
    ("apiLoadModule", []),
    ("apiFindEntryPoint", []),
    ("apiComposite", []),
    ("apiSpecialize", []),
    ("apiLink", []),
    ("apiGetCode", []),
    ("apiReflection", []),
])

# Session setup = greens, module/entry resolution = blues, per-target work
# (specialize/link/codegen) = oranges/purples, reflection + residual = greys —
# fixed like BUCKET_ORDER so api panels stay comparable at a glance.
API_BUCKET_ORDER = [
    ("apiCreateGlobalSession", "#c7e9c0"),
    ("apiCreateSession", "#41ab5d"),
    ("apiLoadModule", "#2171b5"),
    ("apiFindEntryPoint", "#6baed6"),
    ("apiComposite", "#9e9ac8"),
    ("apiSpecialize", "#807dba"),
    ("apiLink", "#6a51a3"),
    ("apiGetCode", "#fd8d3c"),
    ("apiReflection", "#b5bdc4"),
    ("apiTotal (self)", "#969696"),
]


def api_buckets(timers):
    """buckets() over the API-path tree — {bucket: ms} tiling apiTotal."""
    return buckets(timers, API_TREE)



def timer_ms(timers, name):
    """One timer's milliseconds out of a {name: value} map, 0.0 when it is
    absent or not a number.

    An unmeasured phase is a 0 ms phase, not a missing key: buckets() relies
    on that to treat a timer the compiler did not report as a zero-width band
    rather than a hole in the partition. Public (no leading underscore)
    because breakdown.py imports it — it is part of this module's surface
    alongside buckets/api_buckets, not an internal detail."""
    st = timers.get(name)
    return st if isinstance(st, (int, float)) else 0.0


def buckets(timers, tree=TREE):
    """Mutually-exclusive {bucket: ms} that sum to the given tree's root total
    (compileInner for the default compiler-phase TREE, apiTotal for API_TREE),
    allocated TOP-DOWN from that budget. Each parent places its measured
    children within its budget; the remainder is '<parent> (self)'.

    Slang's phase timers are not perfectly additive — named sub-timers can sum to
    MORE than their parent (e.g. specializeModule + simplifyIR + … exceed
    linkAndOptimizeIR after the v2026.7 specialization/autodiff work). When that
    happens the children are scaled proportionally to fit the parent's budget.
    Proportional scaling is preferred over clamping because it preserves the
    relative child proportions, keeping the visual stacked areas meaningful. It
    also keeps the overshoot LOCAL: without it, a child-sum exceeding its parent
    would produce a negative self-residual that propagates up and zeroes out an
    ancestor's self-time (as happened with generateOutput (self) at v2026.7).
    Either way the buckets sum exactly to compileInner."""
    out = {}

    def alloc(node, budget):
        name, children = node
        if budget <= 0:
            return
        if not children:
            out[name] = out.get(name, 0.0) + budget
            return
        cm = [(c, timer_ms(timers, c[0])) for c in children]
        csum = sum(v for _, v in cm)
        if csum > budget and csum > 0:
            scale = budget / csum  # children overshoot parent -> fit proportionally
            for c, v in cm:
                if v > 0:
                    alloc(c, v * scale)
        else:
            for c, v in cm:
                if v > 0:
                    alloc(c, v)
            self_ms = budget - csum
            # Keep EVERY positive residual: the partition's contract is that
            # buckets tile the root exactly (daily_movers' pp column sums to
            # the overall % because of it). Sub-0.05 ms residuals used to be
            # dropped as chart noise, but a band that thin is invisible in
            # the stacked view anyway, and on a very short workload the
            # dropped slivers could break the tiling invariant.
            if self_ms > 0:
                out[f"{name} (self)"] = out.get(f"{name} (self)", 0.0) + self_ms

    alloc(tree, timer_ms(timers, tree[0]))
    return out


# Import-time self-checks over synthetic timer maps, matching the directory
# idiom (lib/manifest.py, lib/analyze.py). daily_movers' fixture covers the
# pp-sum tiling contract THROUGH this partition; these pin the two allocation
# rules that fixture cannot isolate — the residual threshold and proportional
# scaling — against a tiny two-level tree rather than the real TREE, so a
# future timer added to TREE does not have to be mirrored here.
_FIX_TREE = ("root", [("a", []), ("b", [])])

# 1. Ordinary case: measured children plus a residual, tiling the root.
_b = buckets({"root": 100.0, "a": 60.0, "b": 30.0}, _FIX_TREE)
assert _b == {"a": 60.0, "b": 30.0, "root (self)": 10.0}, \
    "buckets: children keep their measured ms and the remainder is (self)"

# 2. The residual threshold. Every POSITIVE residual is kept, however thin —
# sub-0.05 ms slivers used to be dropped as chart noise, but the partition's
# contract is that buckets tile the root exactly (daily_movers' pp column sums
# to the overall % because of it), and on a very short workload a dropped
# sliver breaks that. A band that thin is invisible in the stacked view
# anyway, so there is no rendering cost to keeping it.
_b = buckets({"root": 100.0, "a": 60.0, "b": 39.99}, _FIX_TREE)
assert "root (self)" in _b and abs(_b["root (self)"] - 0.01) < 1e-9, \
    "buckets: a residual below the old 0.05 ms threshold must be KEPT"
assert abs(sum(_b.values()) - 100.0) < 1e-9, "buckets must tile the root"

# 3. Exact fit: a zero residual is NOT a bucket. Zero-width bands would
# clutter the legend, and the tiling invariant already holds without them.
_b = buckets({"root": 100.0, "a": 60.0, "b": 40.0}, _FIX_TREE)
assert _b == {"a": 60.0, "b": 40.0}, \
    "buckets: an exactly-zero residual must not become a (self) bucket"

# 4. Overshoot: Slang's phase timers are not perfectly additive, so named
# sub-timers can sum to MORE than their parent. The children are then scaled
# proportionally to fit the budget — preserving their relative proportions,
# and keeping the overshoot LOCAL rather than producing a negative residual
# that propagates up and zeroes out an ancestor's self-time.
_b = buckets({"root": 100.0, "a": 90.0, "b": 60.0}, _FIX_TREE)
assert abs(_b["a"] - 60.0) < 1e-9 and abs(_b["b"] - 40.0) < 1e-9, \
    "buckets: overshooting children scale proportionally into the budget"
assert "root (self)" not in _b, \
    "buckets: an overshooting parent has no self-time left to report"
assert abs(sum(_b.values()) - 100.0) < 1e-9, \
    "buckets must tile the root even when its children overshoot it"

# 5. A missing timer is a 0 ms phase, not an absent bucket key: timer_ms
# unmeasured names to 0.0, and alloc drops zero-width children.
_b = buckets({"root": 100.0, "a": 100.0}, _FIX_TREE)
assert _b == {"a": 100.0}, "buckets: an unmeasured child contributes nothing"

del _FIX_TREE, _b
