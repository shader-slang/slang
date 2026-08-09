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
    """Write a workload's sources into `dest` and return their filenames.

    Generated sources must be pure ASCII: a typographic character would make
    the corpus bytes platform-dependent, and the series compares bytes across
    machines. A raise rather than an assert, so the contract holds under
    python -O. Static corpora are exempt by construction — they are input, not
    something our generators promise about.
    """
    os.makedirs(dest, exist_ok=True)
    files = sources(spec, size)
    for fn, src in files.items():
        if not spec.source_dir and not src.isascii():
            raise ValueError(
                f"generated source {fn} contains non-ASCII; generators must "
                f"emit ASCII only so the corpus is byte-identical everywhere")
        path = os.path.join(dest, fn)
        os.makedirs(os.path.dirname(path), exist_ok=True)
        with open(path, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(src)
    return list(files)


def existing(dest):
    """Filenames of an already-prepared corpus directory.

    Used by `bench.py --corpus`, where the sources were produced elsewhere
    (another job, another machine) and this run only measures them.
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
