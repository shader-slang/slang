#!/usr/bin/env python3
"""Generate docs/slang-capability-dependency.md from slang-capabilities.capdef.

The generated Markdown file shows the dependency between capability atoms as
Mermaid flowcharts. Run this script from anywhere:

    python3 extras/generate-capability-dependency.py

Options:
    --input PATH    capdef file (default: source/slang/slang-capabilities.capdef)
    --output PATH   Markdown file (default: docs/slang-capability-dependency.md)
    --check         do not write; exit with 1 if the output file is out of date

How the graph is built
----------------------
An arrow `A --> B` means "A requires B".

* `def A : B + C;`   A inherits B and C, so there is a solid arrow from A to B and from A to C.
* `alias A = B + C;` A is a name for the conjunction B + C. It has solid arrows to B and C.
* `alias A = B | C + D;` A is a disjunction. Each alternative is reached with a dotted arrow.
  An alternative with more than one atom goes through a small "all of" node.
* `abstract A;` A is a "keyhole" (for example `target` or `stage`).
* An alias that only lists targets (for example `alias cpp_cuda = cpp | cuda;`) is not drawn, and
  the arrows to it are removed, because it exists only for readability.

A Mermaid diagram on GitHub accepts at most 500 edges, and a large diagram is too small to read.
So the atoms of each group are split into several diagrams. Atoms with the same first word in the
name (`sm_6_0`, `sm_6_1`, ...) are kept together and sorted in natural order. An atom that is used
by a diagram but defined in another diagram is drawn as a dashed "external" node.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass, replace
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_INPUT = REPO_ROOT / "source" / "slang" / "slang-capabilities.capdef"
DEFAULT_OUTPUT = REPO_ROOT / "docs" / "slang-capability-dependency.md"

# Header groups from the `/// [Group]` tags in the capdef file. Atoms that have no tag
# (for example the internal atoms whose names start with `_`) go to "Internal".
GROUP_ORDER = ["Target", "Stage", "Version", "EXT", "Compound", "Other", "Internal"]
GROUP_DESCRIPTION = {
    "Target": "Code generation targets.",
    "Stage": "Pipeline stages.",
    "Version": "Language, shader model, SPIR-V and other version atoms.",
    "EXT": "Extensions.",
    "Compound": "Compound capabilities that combine other atoms.",
    "Other": "Other atoms.",
    "Internal": "Atoms with no documentation tag. Most of them are internal (`_`) atoms.",
}

# Groups in which the aliases are drawn in their own diagram. In the group "Stage", the stage
# atoms inherit the `stage` keyhole, and the aliases only give them other names or add
# capabilities, so a diagram with both kinds is hard to read.
# The value is (title of the non-alias diagram, title of the alias diagram).
SPLIT_BY_KIND = {"Stage": ("keyhole and stage atoms", "aliases")}

# The order of the diagrams in a group, and the families (see `family_of`) in each diagram.
# A family that is not listed here is packed automatically into the diagrams after the listed ones.
FAMILY_LAYOUT = {
    "Version": [["hlsl", "dxil"], ["spirv"], ["cuda"], ["metallib"], ["sm"], ["glsl"]],
}

# Maximum number of edges in one diagram. GitHub limits a Mermaid diagram to 500 edges, but
# the text of a diagram with more than about 80 edges becomes too small to read when it is
# fitted to the page width.
MAX_EDGES_PER_DIAGRAM = 80

CLASS_DEFS = [
    "    classDef def fill:#dbeafe,stroke:#1e40af,color:#111827",
    "    classDef alias fill:#dcfce7,stroke:#166534,color:#111827",
    "    classDef abstract fill:#fef9c3,stroke:#854d0e,color:#111827",
    "    classDef helper fill:#f3f4f6,stroke:#6b7280,color:#111827",
    "    classDef external fill:#ffffff,stroke:#6b7280,stroke-dasharray:4 3,color:#111827",
]


@dataclass
class Atom:
    kind: str  # "def", "alias" or "abstract"
    name: str
    group: str | None
    # A disjunction of conjunctions: each inner list is a list of atom names joined with `+`, and
    # the inner lists are joined with `|`. A `def` has at most one inner list (its parents).
    alternatives: list[list[str]]

    @property
    def parents(self):
        return [name for alternative in self.alternatives for name in alternative]


# ---------------------------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------------------------


def parse_capdef(text):
    """Return a dict {name: Atom} for all `def`, `alias` and `abstract` statements."""
    atoms = {}
    doc_group = None  # group from the `/// [Group]` line before the statement
    statement = ""

    for raw_line in text.splitlines():
        line = raw_line.strip()
        if line.startswith("///"):
            tag = re.fullmatch(r"///\s*\[(\w+)\]", line)
            if tag:
                doc_group = normalize_group(tag.group(1))
            continue
        if line.startswith("//"):
            continue
        if not line:
            doc_group = None
            continue

        # Remove a trailing comment and collect the statement until `;`.
        statement += " " + re.sub(r"//.*$", "", raw_line)
        if ";" not in statement:
            continue
        atom = parse_statement(statement.split(";", 1)[0].strip(), doc_group)
        statement, doc_group = "", None
        if atom.name in atoms:
            raise ValueError(f"duplicate capability atom: {atom.name}")
        atoms[atom.name] = atom

    assign_abstract_groups(atoms)
    return atoms


def normalize_group(tag):
    for group in GROUP_ORDER:
        if group.lower() == tag.lower():
            return group
    raise ValueError(f"unknown header group: [{tag}]")


def parse_statement(body, group):
    match = re.fullmatch(r"(def|alias|abstract)\s+(\w+)\s*(?:[:=]\s*(.*))?", body, re.S)
    if not match:
        raise ValueError(f"cannot parse statement: {body!r}")
    kind, name, expr = match.groups()
    if expr is not None and re.search(r"[^\w\s+|]", expr):
        raise ValueError(f"unsupported expression in {name!r}: {expr!r}")
    if kind == "alias" and expr is None:
        raise ValueError(f"alias {name} has no expression")

    # Only an alias can have `|`. The other statements have at most one conjunction.
    alternatives = [re.findall(r"\w+", term) for term in (expr or "").split("|")]
    alternatives = [alt for alt in alternatives if alt]
    if group is None and kind != "abstract":
        group = "Internal"
    return Atom(kind, name, group, alternatives)


def assign_abstract_groups(atoms):
    """An abstract atom has no doc-comment. It joins the group of the first atom that uses it."""
    for atom in atoms.values():
        if atom.kind == "abstract" and atom.group is None:
            atom.group = next(
                (other.group for other in atoms.values() if atom.name in other.parents), "Internal"
            )


def remove_target_shorthands(atoms):
    """Remove the aliases that only abbreviate a list of targets, and the references to them.

    An alias such as `alias cuda_glsl_hlsl_spirv = cuda | glsl | hlsl | spirv;` exists for
    readability only. Drawing it adds a hub node with many arrows and no information. An alias is
    a shorthand when all atoms it mentions are targets or other shorthands.

    Returns (remaining atoms, names of the removed atoms).
    """
    shorthands = set()

    def is_target_or_shorthand(name):
        return name in shorthands or (name in atoms and atoms[name].group == "Target")

    changed = True
    while changed:
        changed = False
        for atom in atoms.values():
            if atom.kind == "alias" and atom.name not in shorthands:
                if all(is_target_or_shorthand(name) for name in atom.parents):
                    shorthands.add(atom.name)
                    changed = True

    remaining = {}
    for atom in atoms.values():
        if atom.name not in shorthands:
            alternatives = [[n for n in alt if n not in shorthands] for alt in atom.alternatives]
            remaining[atom.name] = replace(atom, alternatives=[alt for alt in alternatives if alt])
    return remaining, sorted(shorthands)


# ---------------------------------------------------------------------------------------------
# Splitting the atoms of a group into diagrams
# ---------------------------------------------------------------------------------------------


def node_id(name):
    # A prefix avoids a clash with the Mermaid key words, such as `end` and `graph`.
    return "n_" + name


def atom_edges(atom):
    """Return (helper node ids, edges) that one atom adds to a diagram.

    An edge is (source node id, arrow, target node id). A helper node is an "all of" node, which
    groups the atoms of one alternative of a disjunction.
    """
    src = node_id(atom.name)
    helpers, edges = [], []
    if len(atom.alternatives) > 1:
        for index, alternative in enumerate(atom.alternatives):
            if len(alternative) == 1:
                edges.append((src, "-.->", node_id(alternative[0])))
                continue
            helper = f"c_{atom.name}_{index}"
            helpers.append(helper)
            edges.append((src, "-.->", helper))
            edges.extend((helper, "-->", node_id(name)) for name in alternative)
    else:
        edges = [(src, "-->", node_id(name)) for name in atom.parents]
    return helpers, list(dict.fromkeys(edges))


def count_edges(atoms):
    return sum(len(atom_edges(atom)[1]) for atom in atoms)


def split_by_declaration_order(atoms):
    """Split atoms into consecutive lists with at most MAX_EDGES_PER_DIAGRAM edges each."""
    parts, count = [], 0
    for atom in atoms:
        edges = count_edges([atom])
        if not parts or count + edges > MAX_EDGES_PER_DIAGRAM:
            parts.append([])
            count = 0
        parts[-1].append(atom)
        count += edges
    return parts


def family_of(name):
    """Return the family of an atom: the first word of its name, such as `sm` for `sm_6_0`.

    The case and the leading underscores are ignored, so `GLSL_450`, `glsl_spirv_1_0` and
    `_glsl_foo` are all in the family `glsl`. All SPIR-V extensions (`SPV_*`, `spvAbort`, ...) are
    in the family `spv`.
    """
    word = name.lstrip("_").split("_")[0].lower()
    return "spv" if word.startswith("spv") else word


def natural_key(atom):
    """Sort key that orders `sm_6_2` before `sm_6_10`."""
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", atom.name.lower())]


def split_into_chunks(group, atoms_in_group):
    """Split the atoms of a group into diagrams that are small enough to read.

    Returns a list of (title, atoms). For a group in SPLIT_BY_KIND, the aliases and the other
    atoms are never in the same diagram. Otherwise, see `pack_families`.
    """
    if group not in SPLIT_BY_KIND:
        return pack_families(atoms_in_group, FAMILY_LAYOUT.get(group, []))

    other_title, alias_title = SPLIT_BY_KIND[group]
    chunks = []
    for title, is_alias in ((other_title, False), (alias_title, True)):
        subset = [a for a in atoms_in_group if (a.kind == "alias") == is_alias]
        parts = pack_families(subset)
        chunks += [(title if len(parts) == 1 else f"{title}: {t}", part) for t, part in parts]
    return chunks


def pack_families(atoms_in_group, layout=()):
    """Pack the atoms into diagrams that are small enough to read.

    Returns a list of (title, atoms). Atoms of the same family (see `family_of`) are sorted in
    natural order and put in the same diagram, so that for example all `sm_*` atoms are together.

    `layout` is a list of lists of families. Each inner list makes one diagram, and the diagrams
    are in the order of the list. The other families are packed after them in alphabetical order.
    A family that is too large for one diagram is split, and each of its parts has a diagram of
    its own.
    """
    families = {}
    for atom in atoms_in_group:
        families.setdefault(family_of(atom.name), []).append(atom)

    pieces = []  # (label, atoms, whether the piece must have a diagram of its own)

    def add_pieces(label, atoms, own_diagram):
        parts = split_by_declaration_order(atoms)
        for index, part in enumerate(parts, start=1):
            suffix = f" {index}/{len(parts)}" if len(parts) > 1 else ""
            pieces.append((label + suffix, part, own_diagram or len(parts) > 1))

    for bundle in layout:
        members = [a for f in bundle for a in sorted(families.pop(f, []), key=natural_key)]
        add_pieces(", ".join(bundle), members, True)
    for family in sorted(families):
        add_pieces(family, sorted(families[family], key=natural_key), False)

    chunks = []  # [labels, atoms, edge count]
    open_chunk = None
    for label, piece, own_diagram in pieces:
        edges = count_edges(piece)
        if open_chunk is None or own_diagram or open_chunk[2] + edges > MAX_EDGES_PER_DIAGRAM:
            open_chunk = [[], [], 0]
            chunks.append(open_chunk)
        open_chunk[0].append(label)
        open_chunk[1].extend(piece)
        open_chunk[2] += edges
        if own_diagram:
            open_chunk = None
    return [(format_labels(labels), atoms) for labels, atoms, _ in chunks]


def format_labels(labels, limit=4):
    if len(labels) <= limit:
        return ", ".join(labels)
    return f"{labels[0]}, {labels[1]}, ..., {labels[-1]}"


# ---------------------------------------------------------------------------------------------
# Rendering
# ---------------------------------------------------------------------------------------------


def mermaid_label(text):
    return text.replace('"', "&quot;")


def render_legend():
    """Return the lines of a Mermaid flowchart that shows every shape and arrow that is used."""
    return [
        "flowchart LR",
        '    d1["def: a capability atom"]:::def',
        '    d2["def: a parent atom"]:::def',
        '    a1(["alias: a name for other atoms"]):::alias',
        '    d3["def"]:::def',
        '    d4["def"]:::def',
        '    h(("all of")):::helper',
        '    x{{"abstract: a keyhole, such as target or stage"}}:::abstract',
        '    e["atom that is drawn in another diagram"]:::external',
        '    d1 -->|"inherits, or + in an alias"| d2',
        '    a1 -.->|"one alternative of a choice"| d3',
        '    a1 -.->|"one alternative that needs several atoms"| h',
        '    h -->|"needs every atom"| d4',
        *CLASS_DEFS,
    ]


def render_diagram(chunk, atoms):
    """Return the lines of one Mermaid flowchart for the atoms in `chunk`."""
    shapes = {"def": ("[", "]"), "alias": ("([", "])"), "abstract": ("{{", "}}")}
    lines = ["flowchart LR"]
    edge_lines = []

    for atom in chunk:
        left, right = shapes[atom.kind]
        lines.append(f'    {node_id(atom.name)}{left}"{mermaid_label(atom.name)}"{right}:::{atom.kind}')
        helpers, edges = atom_edges(atom)
        lines.extend(f'    {helper}(("all of")):::helper' for helper in helpers)
        edge_lines.extend(f"    {src} {arrow} {dst}" for src, arrow, dst in edges)

    # An atom that is defined in another diagram is drawn as a dashed box.
    in_chunk = {atom.name for atom in chunk}
    for name in dict.fromkeys(n for atom in chunk for n in atom.parents if n not in in_chunk):
        group = atoms[name].group if name in atoms else "undefined"
        lines.append(f'    {node_id(name)}["{mermaid_label(f"{name} ({group})")}"]:::external')

    return lines + edge_lines + CLASS_DEFS


def render_table(header, rows, right_aligned=()):
    """Return Markdown table lines with padded columns, in the style Prettier produces."""
    widths = [max(3, *(len(r[i]) for r in [header] + rows)) for i in range(len(header))]

    def fmt(cells):
        padded = [
            c.rjust(w) if i in right_aligned else c.ljust(w)
            for i, (c, w) in enumerate(zip(cells, widths))
        ]
        return "| " + " | ".join(padded) + " |"

    rule = ["-" * (w - 1) + ":" if i in right_aligned else "-" * w for i, w in enumerate(widths)]
    return [fmt(header), "| " + " | ".join(rule) + " |"] + [fmt(r) for r in rows]


def render_document(atoms, input_display, omitted):
    by_group = {group: [a for a in atoms.values() if a.group == group] for group in GROUP_ORDER}
    by_group = {group: members for group, members in by_group.items() if members}

    out = [
        "<!-- Generated by extras/generate-capability-dependency.py. Do not edit this file. -->",
        "",
        "# Slang Capability Atom Dependency",
        "",
        f"This document shows how the capability atoms in `{input_display}` depend on each other.",
        "Do not edit it by hand. To update it, run:",
        "",
        "```bash",
        "python3 extras/generate-capability-dependency.py",
        "```",
        "",
        "## How to read the diagrams",
        "",
        "An arrow `A --> B` means that `A` requires `B`. This diagram shows all shapes and arrows:",
        "",
        "```mermaid",
        *render_legend(),
        "```",
        "",
        "## Omitted atoms",
        "",
        f"The {len(omitted)} aliases that only abbreviate a list of targets are not drawn, and the",
        "arrows to them are removed. For example, `cuda_glsl_hlsl_spirv` is `cuda | glsl | hlsl | spirv`,",
        "and it exists only for readability. The atom counts below do not include them.",
        "",
        "## Summary",
        "",
    ]

    rows = []
    for group, members in by_group.items():
        kinds = [sum(a.kind == kind for a in members) for kind in ("def", "alias", "abstract")]
        rows.append([group, str(len(members)), *map(str, kinds)])
    rows.append(["**Total**", f"**{len(atoms)}**", "", "", ""])
    header = ["Group", "Atoms", "`def`", "`alias`", "`abstract`"]
    out += render_table(header, rows, right_aligned={1, 2, 3, 4}) + [""]

    for group, members in by_group.items():
        chunks = split_into_chunks(group, members)
        out += [f"## {group}", "", f"{GROUP_DESCRIPTION[group]} ({len(members)} atoms)", ""]
        for title, chunk in chunks:
            if len(chunks) > 1:
                out += [f"### {group}: {title}", "", f"{len(chunk)} atoms.", ""]
            out += ["```mermaid", *render_diagram(chunk, atoms), "```", ""]

    return "\n".join(out).rstrip("\n") + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    parser.add_argument("--input", type=Path, default=DEFAULT_INPUT)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true", help="exit with 1 if the output is out of date")
    args = parser.parse_args()

    try:
        input_display = args.input.resolve().relative_to(REPO_ROOT).as_posix()
    except ValueError:
        input_display = args.input.as_posix()
    atoms, omitted = remove_target_shorthands(parse_capdef(args.input.read_text(encoding="utf-8")))
    document = render_document(atoms, input_display, omitted)

    if args.check:
        current = args.output.read_text(encoding="utf-8") if args.output.exists() else None
        if current != document:
            print(f"{args.output} is out of date. Run extras/generate-capability-dependency.py.", file=sys.stderr)
            return 1
        return 0

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(document, encoding="utf-8", newline="\n")
    print(f"Wrote {args.output} ({len(atoms)} atoms)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
