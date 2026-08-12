"""Where a workload's .slang sources come from, and putting them on disk.

Benchmarking and authoring are separate concerns. What `bench.py` measures is
a DIRECTORY OF SLANG FILES; how that directory came to exist — a Python
generator, a template, a checked-in corpus, an artifact unpacked from another
machine — is not the measurement's business. Keeping the two apart is what
lets generation move (or be skipped entirely) without touching the runner.

So a workload declares its sources one of two ways, and exactly one:

  gen         a callable(n) -> {filename: source}, for workloads whose whole
              point is scaling with N
  source_dir  a directory under corpus/, for workloads that ARE real code

Before this split, "not generated" had to be faked: mdl_dxr carried a
generator that read files off disk, a default_size of 0 documented as
"size ignored", and an external_corpus flag whose only job was to exempt it
from the determinism guard. Three accommodations for one idea.
"""
import os
import shutil
import tempfile

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CORPUS_ROOT = os.path.join(HERE, "corpus")


def sources(spec, size):
    """Return {filename: text} for a workload, without writing anything.

    Pure so the reporting layer can show a workload's source without
    materializing a corpus — breakdown.py renders these on the per-workload
    pages.
    """
    if spec.source_dir:
        return read_corpus(spec.source_dir)
    return spec.gen(size)


def read_corpus(name):
    """Read a checked-in / fetched corpus directory as {filename: text}.

    Decoded tolerantly: these are third-party sources (license headers, author
    names) and may legitimately contain non-ASCII, unlike anything we generate.
    """
    d = os.path.join(CORPUS_ROOT, name)
    if not os.path.isdir(d):
        raise FileNotFoundError(
            f"corpus '{name}' missing at {d}; copy .slang files there before running")
    out = {}
    for fn in sorted(os.listdir(d)):
        if fn.endswith(".slang"):
            with open(os.path.join(d, fn), "rb") as fh:
                out[fn] = fh.read().decode("utf-8", "replace")
    if not out:
        raise FileNotFoundError(f"no .slang files in corpus '{name}' ({d})")
    return out


def dir_name(spec, size):
    """The per-workload directory name inside a corpus root.

    Shared by the prepare step and the bench step so a corpus prepared on one
    machine is found by a bench run on another — that handoff is the whole
    point of preparing separately.
    """
    return f"{spec.name}_n{size}"


def materialize(spec, size, dest):
    """Replace `dest` with a workload's sources and return their filenames, sorted.

    `dest` is REPLACED, not written over. Writing over it would leave behind any
    file the generator no longer emits, and prepared_files() cannot tell an
    orphan from a source — so a later `bench.py --corpus` run would compile a
    file that no version of the generator produces. Clearing lives here rather
    than in the two callers because both need it and only one used to do it.

    The return is sorted for the same reason prepared_files() sorts: those two
    are the only ways a caller learns a corpus's file list, and build_commands
    picks the entry point positionally (`list(files)[0]`) when a workload names
    no main file. If the two disagreed, preparing in-process and preparing on
    another machine could measure DIFFERENT files from identical bytes on disk
    — silently, and only for multi-file workloads. Sorting both makes them
    identical by construction; the round-trip is pinned at import below.

    Generated sources must be pure ASCII: a typographic character would make
    the corpus bytes platform-dependent, and the series compares bytes across
    machines. A raise rather than an assert, so the contract holds under
    python -O. Static corpora are exempt by construction — they are input, not
    something our generators promise about.
    """
    if os.path.isdir(dest):
        shutil.rmtree(dest)
    os.makedirs(dest, exist_ok=True)
    files = sources(spec, size)
    for fn, src in files.items():
        # Flat by contract. materialize() could write a nested path, but
        # read_corpus() and prepared_files() both list one directory level, so
        # a nested source would be written and then never read back — the file
        # would silently vanish from the measurement. Rejecting the shape is
        # honest where supporting it halfway is not; no generator emits one,
        # and adding nested support means teaching all three functions at once.
        if "/" in fn or os.sep in fn:
            raise ValueError(
                f"source filename {fn!r} is nested; corpus directories are flat "
                f"because read_corpus/prepared_files list a single level")
        if not spec.source_dir and not src.isascii():
            raise ValueError(
                f"generated source {fn} contains non-ASCII; generators must "
                f"emit ASCII only so the corpus is byte-identical everywhere")
        with open(os.path.join(dest, fn), "w", encoding="utf-8", newline="\n") as fh:
            fh.write(src)
    return sorted(files)


def prepared_files(dest):
    """Sorted filenames of an already-prepared corpus directory.

    Used by `bench.py --corpus`, where the sources were produced elsewhere
    (another job, another machine) and this run only measures them. Named for
    what it returns rather than what it tests — it reads a list, not a
    predicate — and sorted to match materialize()'s order exactly.
    """
    if not os.path.isdir(dest):
        raise FileNotFoundError(f"no prepared corpus at {dest}")
    files = sorted(f for f in os.listdir(dest) if f.endswith(".slang"))
    if not files:
        raise FileNotFoundError(f"no .slang files in prepared corpus {dest}")
    return files


# Import-time self-checks (the directory idiom).
from . import manifest  # noqa: E402  circular-safe: manifest imports no lib module

# Exactly one source of truth per workload, enforced here rather than trusted:
# a spec with both would silently prefer one, and a spec with neither would
# fail only when that workload is next run.
for _s in manifest.WORKLOADS:
    assert bool(_s.gen) != bool(_s.source_dir), \
        (f"{_s.name}: set exactly one of gen / source_dir "
         f"(gen={bool(_s.gen)}, source_dir={_s.source_dir!r})")
del _s

assert dir_name(manifest.BY_NAME["minimal"], 0) == "minimal_n0"

# The prepare -> bench round-trip, over a throwaway tmpdir. This is the
# invariant the whole split rests on: --prepare writes a corpus and --corpus
# measures it, possibly on another machine, and the two must agree on WHICH
# files are in it and in WHAT order. Order matters because build_commands
# selects the entry point positionally for workloads that name no main file,
# so a disagreement would silently measure a different file rather than fail.
#
# Driven by a stub spec, not a real workload: the manifest's multi-file
# generators are large, and the property under test is materialize's, not any
# workload's. The filenames deliberately expose BOTH ways insertion order
# diverges from sorted order — "m10" sorts before "m2", and "link_main" sorts
# before every "m*" while generators emit it last.
class _StubSpec:
    name = "_selfcheck"
    source_dir = None

    @staticmethod
    def gen(n):
        return {"m2.slang": "// a\n", "m10.slang": "// b\n", "link_main.slang": "// c\n"}


_tmp = tempfile.mkdtemp(prefix="corpus_selfcheck_")
try:
    _dest = os.path.join(_tmp, dir_name(_StubSpec, 1))
    _written = materialize(_StubSpec, 1, _dest)
    assert _written == prepared_files(_dest), \
        (f"materialize and prepared_files must agree on the file list AND its "
         f"order; got {_written} vs {prepared_files(_dest)}")
    assert _written == ["link_main.slang", "m10.slang", "m2.slang"], \
        "both sides must be sorted, not in generator emission order"

    # Re-preparing must REPLACE, not accumulate: an orphan left by an earlier
    # run is indistinguishable from a source once it is on disk.
    with open(os.path.join(_dest, "orphan.slang"), "w", encoding="utf-8") as _fh:
        _fh.write("// stale\n")
    assert "orphan.slang" in prepared_files(_dest), "self-check setup failed"
    assert "orphan.slang" not in materialize(_StubSpec, 1, _dest), \
        "materialize must clear dest; a stale file would be compiled by --corpus"
    assert "orphan.slang" not in prepared_files(_dest), \
        "the orphan must be gone from disk, not merely absent from the return"

    # Nested filenames are rejected rather than half-supported (see materialize).
    class _NestedSpec(_StubSpec):
        @staticmethod
        def gen(n):
            return {"sub/a.slang": "// x\n"}

    try:
        materialize(_NestedSpec, 1, os.path.join(_tmp, "nested"))
        raise AssertionError("materialize must reject a nested source filename")
    except ValueError as _e:
        assert "nested" in str(_e), f"the rejection must name the shape; got {str(_e)!r}"
finally:
    shutil.rmtree(_tmp, ignore_errors=True)
del _StubSpec, _NestedSpec, _tmp, _dest, _written, _fh
