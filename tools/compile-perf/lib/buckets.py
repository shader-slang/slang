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



def _t(timers, name):
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
        cm = [(c, _t(timers, c[0])) for c in children]
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

    alloc(tree, _t(timers, tree[0]))
    return out
