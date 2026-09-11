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
        # Both separators literally, not os.sep: a corpus is prepared on one
        # machine and measured on another, so the question is not "is this
        # nested HERE" but "is this nested on any machine that will read it".
        # On POSIX os.sep is "/", which would let "sub\\a.slang" through as one
        # flat filename — a name Windows cannot represent, where the same
        # prepare step would create a subdirectory instead.
        if "/" in fn or "\\" in fn:
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

    An empty but PRESENT directory returns [], because materialize() does: a
    workload may legitimately have no sources, as api_session_create's
    generator returns {} for a driver that only creates and destroys sessions.
    These two functions are the two halves of one round-trip, so a
    disagreement about empty means `--corpus` fails a workload the default
    path runs fine. A MISSING directory still raises — that is `--corpus`
    pointed somewhere wrong, a caller error rather than a property of any
    workload. (read_corpus keeps its non-empty requirement: a static corpus is
    only ever named by a workload that HAS sources, so empty there means the
    files were never copied.)
    """
    if not os.path.isdir(dest):
        raise FileNotFoundError(f"no prepared corpus at {dest}")
    return sorted(f for f in os.listdir(dest) if f.endswith(".slang"))


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
# Written as a function rather than the bare module-level asserts used above:
# it needs several stub specs and a temporary CORPUS_ROOT, and local scope
# retires all of that on return instead of leaving a hand-maintained `del`
# list that must be kept in step with every name the block introduces.
def _selfcheck():
    """Check the prepare -> bench round-trip and materialize's two raises."""
    # Driven by stub specs, not real workloads: the manifest's multi-file
    # generators are large, and the properties under test are materialize's,
    # not any workload's. These filenames deliberately expose BOTH ways
    # insertion order diverges from sorted order — "m10" sorts before "m2",
    # and "link_main" sorts before every "m*" while generators emit it last.
    class StubSpec:
        name = "_selfcheck"
        source_dir = None

        @staticmethod
        def gen(n):
            return {"m2.slang": "// a\n", "m10.slang": "// b\n",
                    "link_main.slang": "// c\n"}

    tmp = tempfile.mkdtemp(prefix="corpus_selfcheck_")
    try:
        dest = os.path.join(tmp, dir_name(StubSpec, 1))
        written = materialize(StubSpec, 1, dest)
        assert written == prepared_files(dest), \
            (f"materialize and prepared_files must agree on the file list AND "
             f"its order; got {written} vs {prepared_files(dest)}")
        assert written == ["link_main.slang", "m10.slang", "m2.slang"], \
            "both sides must be sorted, not in generator emission order"

        # Re-preparing must REPLACE, not accumulate: an orphan left by an
        # earlier run is indistinguishable from a source once it is on disk.
        with open(os.path.join(dest, "orphan.slang"), "w", encoding="utf-8",
                  newline="\n") as fh:
            fh.write("// stale\n")
        assert "orphan.slang" in prepared_files(dest), "self-check setup failed"
        assert "orphan.slang" not in materialize(StubSpec, 1, dest), \
            "materialize must clear dest; a stale file would be compiled by --corpus"
        assert "orphan.slang" not in prepared_files(dest), \
            "the orphan must be gone from disk, not merely absent from the return"

        # A workload with NO sources round-trips too. api_session_create's
        # generator returns {} because that driver only creates and destroys
        # sessions, so --prepare writes an empty directory and --corpus must
        # read it back as zero files. The two must agree about empty for the
        # same reason they must agree about order: --corpus is supposed to be
        # the default path minus generation, and a workload that runs fine
        # in-process should not fail merely because it was prepared first.
        def emitting(files):
            """A generated stub spec whose gen() returns exactly `files`."""
            class Spec(StubSpec):
                gen = staticmethod(lambda n: files)
            return Spec

        empty = os.path.join(tmp, "empty")
        assert materialize(emitting({}), 1, empty) == [], \
            "a sourceless workload prepares as [], not as an error"
        assert prepared_files(empty) == [], \
            "prepared_files must agree with materialize on empty; raising here " \
            "fails a sourceless workload under --corpus that runs fine by default"

        # Nested filenames are rejected rather than half-supported (see
        # materialize). Both separators, since a corpus crosses machines.
        for bad in ("sub/a.slang", "sub\\a.slang"):
            try:
                materialize(emitting({bad: "// x\n"}), 1, os.path.join(tmp, "nested"))
                raise AssertionError(f"materialize must reject nested name {bad!r}")
            except ValueError as e:
                assert "nested" in str(e), \
                    f"the rejection must name the shape; got {str(e)!r}"

        # The ASCII byte-determinism guard, and its source_dir exemption. Both
        # halves are checked because inverting the `not spec.source_dir` test
        # would silently drop the guard for exactly the sources it constrains,
        # and nothing downstream would notice until two machines produced
        # different bytes for the "same" corpus.
        try:
            materialize(emitting({"a.slang": "// — em dash\n"}), 1,
                        os.path.join(tmp, "nonascii"))
            raise AssertionError("materialize must reject a non-ASCII generated source")
        except ValueError as e:
            assert "non-ASCII" in str(e), \
                f"the rejection must name the shape; got {str(e)!r}"

        # The source_dir half of the split, over a throwaway CORPUS_ROOT.
        # mdl_dxr is the only real source_dir workload and its corpus/mdl tree
        # is not in the repo, so without this stub read_corpus runs only on a
        # machine that has that corpus — a wrong sort, a decode change, or a
        # lost .slang filter would import cleanly here and surface as a bad
        # number on the nightly. Non-ASCII is expected to be ACCEPTED here:
        # third-party sources carry license headers and author names, and the
        # determinism guard is a promise about our generators, not about input.
        static = os.path.join(tmp, "static_root", "stat")
        os.makedirs(static)
        # newline="\n" for the same reason materialize() writes that way: with
        # the default newline=None, text mode translates \n to \r\n on Windows,
        # while read_corpus reads BINARY and decodes verbatim — so the uni.slang
        # content assertion below compared "// é\r\n" against "// é\n" and the
        # import aborted, taking the whole nightly with it. The fixture must
        # put the bytes on disk that it claims to.
        # m10.slang carries a DELIBERATE CRLF: real third-party corpora are
        # checked out with native line endings, and read_corpus promises to
        # return whatever bytes are there. Asserted below on every platform, so
        # the verbatim contract is pinned where CI actually runs it — the
        # newline="\n" above only misbehaves on Windows, which is nightly-only.
        for fn, src in (("m2.slang", "// a\n"), ("m10.slang", "// b\r\n"),
                        ("notes.txt", "not a source\n"), ("uni.slang", "// é\n")):
            with open(os.path.join(static, fn), "w", encoding="utf-8",
                      newline="\n") as fh:
                fh.write(src)

        class StaticSpec:
            name = "_selfcheck_static"
            gen = None
            source_dir = "stat"

        global CORPUS_ROOT
        saved_root = CORPUS_ROOT
        CORPUS_ROOT = os.path.join(tmp, "static_root")
        try:
            sdest = os.path.join(tmp, dir_name(StaticSpec, 0))
            expected = ["m10.slang", "m2.slang", "uni.slang"]
            assert materialize(StaticSpec, 0, sdest) == expected, \
                ("a source_dir workload must round-trip sorted, .slang only, "
                 "with non-ASCII accepted")
            assert prepared_files(sdest) == expected, \
                "the source_dir round-trip must match materialize exactly"
            # sources() is asserted separately from the round-trip above, which
            # cannot see this: materialize sorts its own return, so it would
            # absorb a read_corpus that stopped sorting. breakdown.py renders a
            # workload page straight from sources(), in this order.
            assert list(sources(StaticSpec, 0)) == expected, \
                "sources() must return a static corpus sorted, for breakdown.py"
            assert sources(StaticSpec, 0)["m10.slang"] == "// b\r\n", \
                ("read_corpus must return CRLF sources verbatim: it reads "
                 "binary precisely so a corpus checked out with native line "
                 "endings is not silently rewritten under the compiler")
            assert sources(StaticSpec, 0)["uni.slang"] == "// é\n", \
                ("read_corpus must decode third-party bytes tolerantly and "
                 "return them verbatim; a stricter codec would mangle the "
                 "license headers and author names real corpora carry")
        finally:
            CORPUS_ROOT = saved_root
    finally:
        shutil.rmtree(tmp, ignore_errors=True)


_selfcheck()
del _selfcheck
