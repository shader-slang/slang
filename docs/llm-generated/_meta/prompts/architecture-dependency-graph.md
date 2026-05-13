# Prompt: architecture/dependency-graph.md

See [_common.md](_common.md) for universal rules.

## Target

Produce `docs/llm-generated/architecture/dependency-graph.md` —
a high-level dependency graph among the logical unit groups identified
in [module-map.md](module-map.md).

The graph is at the **subsystem** level, not at the file level. The goal
is for a reader to be able to predict, given a change in one subsystem,
which other subsystems are at risk.

## Required content

1. `# Dependency Graph` (title)
2. A short paragraph explaining the granularity (subsystem level), and
   referencing [module-map.md](module-map.md) for the file inventory.
3. A mermaid `flowchart` block showing the major edges. Use one node per
   logical unit group from [module-map.md](module-map.md). Edges represent
   "compiles against the public headers of" or "depends on at runtime".
   Derive edges by reading the `target_link_libraries(...)` (or
   equivalent) lines in the `CMakeLists.txt` files under `source/` —
   these are the watched paths for this document. Do not invent edges
   that are not justified by the build files.
4. `## Notable invariants` — call out a small number of important
   directional rules, for example:
   - `source/core/` does not depend on anything else in the project.
   - `source/compiler-core/` may depend on `source/core/` but not on
     `source/slang/`.
   - Public headers in `include/` must not include private headers from
     `source/`.
   Each invariant must cite the build files or headers that demonstrate
   it.
5. `## Cycles and known irregularities` — if the build files reveal
   cycles, list them; otherwise state explicitly that none were
   observed.

## Quality checklist (in addition to the universal one)

- [ ] Every node in the mermaid diagram corresponds to a directory under
      `source/` or a heading in [module-map.md](module-map.md).
- [ ] Every edge is justified by a citation to a `CMakeLists.txt`.
- [ ] Mermaid diagram follows the project's mermaid-syntax conventions
      (camelCase IDs, no spaces in node names, no explicit colors).
- [ ] Document length under 16 KB.
