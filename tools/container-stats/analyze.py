#!/usr/bin/env python3
"""Rank container declaration sites for conversion to inline storage.

Reads the JSON files written by a build configured with `SLANG_ENABLE_CONTAINER_STATS=ON`, merges
them across processes and runs, and reports, per container family, which declarations would benefit
from holding their elements inline instead of on the heap.

The families are reported separately because the question is different for each: a `Dictionary` or
`List` site is a candidate for conversion to the `Short*` variant, an existing `Short*` site is a
verdict on the inline capacity it already declares, an `OrderedDictionary` site has no inline
variant to convert to at all, and a string record is an allocation event rather than an object with
a lifetime.

Usage:
    analyze.py 'stats.*.json' [--top 40] [--family List]
"""

import argparse
import glob
import json
import os
import re
import subprocess
import sys
from collections import defaultdict

# Must match `ContainerOp` in source/core/slang-container-stats.h.
OPS = {
    "remove": 1 << 0,
    "removeIf": 1 << 1,
    "set": 1 << 2,
    "clear": 1 << 3,
    "clearAndDeallocate": 1 << 4,
    "reserve": 1 << 5,
    "indexUpdate": 1 << 6,
    "copyAssign": 1 << 7,
    "moveAssign": 1 << 8,
    "swap": 1 << 9,
    "insert": 1 << 10,
    "removeRange": 1 << 11,
    "setCount": 1 << 12,
    "iterate": 1 << 13,
    "getCount": 1 << 14,
    "contiguousBuffer": 1 << 17,
    "attachBuffer": 1 << 18,
    "sort": 1 << 19,
}


class Family:
    """What a group of container types is, and what converting one would mean.

    `hard` lists the operations the conversion target does not support at all, so that a site which
    performs any of them cannot be converted however small it stays. `soft` lists operations that
    are merely suspicious -- typically ones whose call could simply be deleted -- and are reported
    as a note rather than as a disqualification.
    """

    def __init__(self, name, target, has_value, hard=(), soft=(), sweep=(2, 4, 8, 16, 32)):
        self.name = name
        self.target = target
        self.has_value = has_value
        self.hard = list(hard)
        self.soft = list(soft)
        self.sweep = list(sweep)

    @property
    def is_short(self):
        """Whether this family already has inline storage, so the question is its capacity."""
        return self.name in ("ShortList", "ShortDictionary")


# `ShortDictionary`'s entire API is `add` and `tryGetValue`, so anything else a site does
# disqualifies it. `reserve` is a soft case because it is a call that could simply be deleted.
DICTIONARY_HARD = [
    "remove",
    "removeIf",
    "set",
    "clear",
    "clearAndDeallocate",
    "indexUpdate",
    "swap",
    "iterate",
    "getCount",
]

# `ShortList` splits its elements between an inline array and an overflow buffer. That rules out
# anything needing one contiguous range (`sort`, `getBuffer`, `attachBuffer`) and anything needing
# to shift elements about (`insert`, ordered removal, `swapWith`). `clear`, `setCount`, indexing and
# iteration are all supported and so are not listed.
LIST_HARD = [
    "insert",
    "remove",
    "removeRange",
    "sort",
    "swap",
    "contiguousBuffer",
    "attachBuffer",
]

FAMILIES = {
    "Dictionary": Family("Dictionary", "ShortDictionary", True, DICTIONARY_HARD, ["reserve"]),
    "HashSet": Family("HashSet", "ShortDictionary", False, DICTIONARY_HARD, ["reserve"]),
    "List": Family("List", "ShortList", False, LIST_HARD, ["reserve"]),
    "ShortList": Family("ShortList", None, False, sweep=(2, 4, 8, 16, 32, 64)),
    "ShortDictionary": Family("ShortDictionary", None, True, sweep=(2, 4, 8, 16, 32, 64)),
    # There is no `ShortOrderedDictionary`, so these sites are reported for their size distribution
    # rather than ranked for conversion.
    "OrderedDictionary": Family("OrderedDictionary", None, True),
    # A string record counts buffer allocations, and the candidate capacities are the ones a
    # small-string optimization would plausibly use. Note that the histogram is exact only up to 32
    # elements, so a capacity above that is evaluated conservatively. These are two separate types
    # rather than one type with a flag because whether a buffer is a string's first or replaces one
    # it already had is a property of each allocation, not of the line that made it.
    "StringBufferAllocation": Family(
        "StringBufferAllocation", "inline buffer", False, sweep=(8, 16, 24, 32)
    ),
    "StringBufferGrowth": Family(
        "StringBufferGrowth", "inline buffer", False, sweep=(8, 16, 24, 32)
    ),
    # An interned identifier. Reported separately from strings in general because identifiers are
    # short in a way that strings are not, and it is their distribution, not the aggregate one,
    # that says whether this type should carry an inline buffer.
    "ImmutableHashedString": Family(
        "ImmutableHashedString", "inline buffer", False, sweep=(8, 16, 24, 32)
    ),
}

UNKNOWN_FAMILY = Family("other", None, True)

# An `OrderedHashSet` is an `OrderedDictionary` with no value type. It is reported alongside the
# ordered dictionaries, so it shares their family name, but it must not be charged for a value.
ORDERED_HASH_SET_FAMILY = Family("OrderedDictionary", None, False)

# Rough costs used only to order candidates against one another; the ranking is the deliverable,
# not the absolute figure.
NANOS_PER_ALLOCATION = 50.0
NANOS_PER_COMPARISON = 1.0


def bucket_upper_bound(index):
    """Return the largest peak size that falls in a histogram bucket.

    Buckets 0..32 are exact sizes, so the bound is the index itself. Above that each bucket covers
    one power of two, with bucket 33 covering 33..64.
    """
    if index <= 32:
        return index
    return 1 << (index - 27)


def parse_container_type(signature):
    """Extract the container type from the compiler's spelling of `getContainerTypeInfo`.

    GCC and Clang render the template arguments as `[with TContainer = Slang::Dictionary<...>]`,
    while MSVC uses a different form; fall back to the raw signature when neither matches.
    """
    match = re.search(r"TContainer = ([^;\]]+?)(?:;|\])", signature)
    if match:
        name = match.group(1)
    else:
        match = re.search(r"getContainerTypeInfo<([^>]+(?:<[^>]*>)?[^>]*)>", signature)
        name = match.group(1) if match else signature
    name = name.replace("Slang::", "").replace("std::", "")
    # The hash, comparator and allocator arguments are always the defaults and only add noise.
    name = re.sub(r",\s*Hash<[^>]*>,\s*equal_to<[^>]*>", "", name)
    name = re.sub(r",\s*StandardAllocator", "", name)
    # Removing an argument leaves the space the compiler prints before a closing angle bracket, and
    # the patterns below anchor on that bracket.
    name = re.sub(r"\s+>", ">", name).strip()
    # A `HashSet<T>` is a `Dictionary<T, _DummyClass>` underneath, and it is the dictionary that
    # carries the probe. Restore the name the reader wrote.
    match = re.match(r"Dictionary<(.*),\s*_DummyClass>$", name)
    if match:
        return f"HashSet<{match.group(1)}>"
    match = re.match(r"OrderedDictionary<(.*),\s*_DummyClass>$", name)
    if match:
        return f"OrderedHashSet<{match.group(1)}>"
    # An anonymous-namespace tag type such as the string-allocation marker.
    name = re.sub(r"\(anonymous namespace\)::", "", name)
    name = re.sub(r"\{anonymous\}::", "", name)
    return name


def family_of(container):
    """Return the `Family` a container type name belongs to."""
    base = container.split("<")[0].strip()
    if base == "OrderedHashSet":
        return ORDERED_HASH_SET_FAMILY
    return FAMILIES.get(base, UNKNOWN_FAMILY)


class Site:
    def __init__(self, file, line, function, container):
        self.file = file
        self.line = line
        self.function = function
        self.container = container
        self.family = family_of(container)
        self.instances = 0
        self.moved_away = 0
        self.ever_allocated = 0
        self.live_high_water = 0
        self.op_mask = 0
        self.key_size = 0
        self.value_size = 0
        self.inline_capacity = 0
        self.sample_rate = 1
        self.key_default_constructible = True
        self.value_default_constructible = True
        self.peaks = defaultdict(int)
        self.lookups = defaultdict(int)
        self.inserts = defaultdict(int)
        self.backtraces = []

    @property
    def key(self):
        # A backtrace-keyed record has no source location of its own, so its first stack frame is
        # what tells two of them apart.
        if self.file == "<backtrace>":
            return ("<backtrace>", self.container, self._backtrace_key)
        return (self.file, self.line, self.container)

    def merge(self, record):
        # Backtrace-keyed records are sampled, so every count they carry stands for `sampleRate`
        # real events. Scaling here keeps the rest of the script unaware of the distinction.
        scale = record.get("sampleRate", 1) or 1
        self.sample_rate = scale
        self.instances += record["instances"] * scale
        self.moved_away += record["movedAway"] * scale
        self.ever_allocated += record["everAllocated"] * scale
        # A high-water mark is a maximum over a process, so merging takes the largest rather than
        # the sum; two processes' peaks did not necessarily coincide.
        self.live_high_water = max(self.live_high_water, record["liveHighWater"])
        self.op_mask |= record["opMask"]
        self.key_size = record["keySize"]
        self.value_size = record["valueSize"]
        self.inline_capacity = record.get("inlineCapacity", 0)
        self.key_default_constructible = record["keyDefaultConstructible"]
        self.value_default_constructible = record["valueDefaultConstructible"]
        for name, target in (
            ("peakHistogram", self.peaks),
            ("lookupsByPeak", self.lookups),
            ("insertsByPeak", self.inserts),
        ):
            for bucket, count in record[name].items():
                target[int(bucket)] += count * scale
        if len(self.backtraces) < 4:
            self.backtraces.extend(record.get("backtraces", [])[: 4 - len(self.backtraces)])

    @property
    def _backtrace_key(self):
        trace = self.backtraces[0] if self.backtraces else []
        return tuple(trace[:8])

    @property
    def element_size(self):
        """The bytes one inline slot would cost, which is the key plus, if any, the value."""
        return self.key_size + (self.value_size if self.family.has_value else 0)

    @property
    def folded(self):
        """The number of instances that contributed a peak, which excludes moved-from ones."""
        return sum(self.peaks.values())

    def op_names(self):
        return sorted(name for name, bit in OPS.items() if self.op_mask & bit)

    def disqualifiers(self):
        return [n for n in self.op_names() if n in self.family.hard]

    def warnings(self):
        result = [n for n in self.op_names() if n in self.family.soft]
        if not self.key_default_constructible:
            result.append("key-not-default-constructible")
        if self.family.has_value and not self.value_default_constructible:
            result.append("value-not-default-constructible")
        return result

    def fraction_fitting(self, capacity):
        """The fraction of instances whose peak size would have stayed in an inline array."""
        if not self.folded:
            return 0.0
        fitting = sum(c for b, c in self.peaks.items() if bucket_upper_bound(b) <= capacity)
        return fitting / self.folded

    @property
    def promotion_rate(self):
        """For a `Short*` site, the fraction of instances that outgrew its declared capacity.

        This is what says whether the capacity written at the declaration was a good choice: a rate
        near zero means the inline array is doing its job, and a high rate means the site is paying
        for inline storage it then abandons.
        """
        if not self.inline_capacity:
            return 0.0
        return 1.0 - self.fraction_fitting(self.inline_capacity)

    def evaluate(self, capacity):
        """Estimate the benefit and cost of giving this site an inline array of `capacity`.

        The benefit is the heap allocations avoided: instances that allocated at all but would have
        stayed inline. The cost is extra key comparisons, and it has two parts -- instances that
        stay inline pay a linear scan instead of a hash, and instances that are promoted keep
        scanning the full inline array on every lookup forever afterwards.
        """
        inline_lookups = 0
        promoted_lookups = 0
        allocations_avoided = 0
        weighted_peak = 0
        inline_instances = 0
        for bucket, count in self.peaks.items():
            peak = bucket_upper_bound(bucket)
            if peak <= capacity:
                inline_lookups += self.lookups.get(bucket, 0)
                inline_instances += count
                weighted_peak += peak * count
                if peak > 0:
                    allocations_avoided += count
            else:
                promoted_lookups += self.lookups.get(bucket, 0)

        mean_peak = (weighted_peak / inline_instances) if inline_instances else 0.0
        # A linear scan of a half-full inline array, against one hash and probe.
        comparisons = inline_lookups * max(mean_peak / 2.0 - 1.0, 0.0)
        comparisons += promoted_lookups * capacity

        score = allocations_avoided * NANOS_PER_ALLOCATION - comparisons * NANOS_PER_COMPARISON
        memory = capacity * self.element_size * self.live_high_water
        return {
            "capacity": capacity,
            "allocationsAvoided": allocations_avoided,
            "comparisons": comparisons,
            "score": score,
            "memoryBytes": memory,
            "fractionFitting": self.fraction_fitting(capacity),
        }

    def best(self):
        """Choose the inline capacity to recommend.

        Not simply the highest-scoring capacity: the score counts avoided allocations against extra
        comparisons but not the memory an inline array costs in every instance, so it grows almost
        monotonically with capacity and would recommend the largest one on offer. Instead take the
        smallest capacity that still achieves most of the available benefit, which is the choice a
        person would make when reading the sweep.
        """
        viable = [self.evaluate(c) for c in self.family.sweep]
        best_score = max(e["score"] for e in viable)
        if best_score <= 0:
            return max(viable, key=lambda e: e["score"])
        for entry in viable:
            if entry["score"] >= 0.95 * best_score:
                return entry
        return max(viable, key=lambda e: e["score"])


def load(patterns):
    sites = {}
    files = []
    for pattern in patterns:
        matched = glob.glob(pattern)
        if not matched and os.path.exists(pattern):
            matched = [pattern]
        files.extend(matched)
    if not files:
        sys.exit(f"no stats files matched {patterns}")

    for path in files:
        with open(path) as f:
            data = json.load(f)
        for record in data["records"]:
            container = parse_container_type(record["signature"])
            site = Site(record["file"], record["line"], record["function"], container)
            # A backtrace-keyed record's identity is its stack, which is only known once the
            # backtrace has been read out of the record.
            site.backtraces = record.get("backtraces", [])[:4]
            existing = sites.get(site.key)
            if existing is None:
                sites[site.key] = site
                existing = site
                existing.backtraces = []
            existing.merge(record)
    return list(sites.values()), files


def symbolize(sites, binary_override=None):
    """Resolve the sampled stack frames to `function at file:line` using addr2line.

    Frames are recorded as `<module path>+0x<offset>` rather than as raw addresses, because the
    modules are position independent and are loaded somewhere different on every run. They are
    grouped by module so that one addr2line invocation handles each.
    """
    by_module = defaultdict(set)
    for site in sites:
        for trace in site.backtraces:
            for frame in trace:
                if "+0x" in frame:
                    module, _, offset = frame.rpartition("+")
                    by_module[binary_override or module].add((frame, offset))

    symbols = {}
    for module, frames in by_module.items():
        if not os.path.exists(module):
            continue
        frames = sorted(frames)
        try:
            proc = subprocess.run(
                ["addr2line", "-e", module, "-f", "-C"] + [o for _, o in frames],
                capture_output=True,
                text=True,
                check=True,
            )
        except (subprocess.CalledProcessError, FileNotFoundError) as error:
            print(f"warning: could not symbolize {module} ({error})", file=sys.stderr)
            continue
        # addr2line without -i prints two lines per address: the function, then file:line.
        lines = proc.stdout.splitlines()
        for index, (frame, _) in enumerate(frames):
            function = lines[index * 2] if index * 2 < len(lines) else "??"
            location = lines[index * 2 + 1] if index * 2 + 1 < len(lines) else "??"
            symbols[frame] = f"{function} at {location}"
    return symbols


# Frames belonging to the instrumentation itself, or to the container implementation whose
# construction is being recorded. Skipping these by name rather than by position keeps the result
# correct whether or not the build inlined them.
INTERNAL_FRAME_PATTERNS = (
    "slang-container-stats.",
    "slang-dictionary.h",
    "slang-list.h",
    "slang-short-list.h",
    "slang-short-dictionary.h",
    "slang-string.cpp",
    "ContainerStatsProbe",
    "containerStats",
    "StringRepresentation::",
)


def caller_frames(site, symbols, count=1):
    """Return the innermost stack frames outside the instrumentation and the container."""
    if not site.backtraces or not symbols:
        return []
    result = []
    for frame in site.backtraces[0]:
        resolved = symbols.get(frame)
        if not resolved or "??" in resolved:
            continue
        if any(pattern in resolved for pattern in INTERNAL_FRAME_PATTERNS):
            continue
        result.append(resolved)
        if len(result) >= count:
            break
    return result


def find_ambiguous_fields(site, source_root):
    """Report how many fields of this container's type the class at this line declares.

    A container declared as a member reports its enclosing class's line rather than its own, so two
    fields of the same type in one class merge into a single record. Detecting that is what stops a
    merged bimodal histogram from being read as one strange distribution.
    """
    path = os.path.join(source_root, site.file)
    if not os.path.exists(path) or not site.container:
        return 0
    base = site.container.split("<")[0].split("::")[-1]
    try:
        with open(path, errors="replace") as f:
            lines = f.readlines()
    except OSError:
        return 0
    start = site.line - 1
    if start >= len(lines) or not re.search(r"\b(struct|class)\b", lines[start]):
        return 0
    depth = 0
    count = 0
    for line in lines[start : start + 400]:
        depth += line.count("{") - line.count("}")
        if re.search(rf"\b{re.escape(base)}\s*<", line) and ";" in line:
            count += 1
        if depth <= 0 and count:
            break
    return count


def short_location(site, width=45):
    loc = f"{site.file}:{site.line}"
    if len(loc) > width:
        loc = "..." + loc[-(width - 3) :]
    return loc


def report_conversion_candidates(family, sites, args, symbols):
    """Rank the sites of a family that could be converted to its inline-storage counterpart."""
    candidates = [s for s in sites if not s.disqualifiers()]
    disqualified = [s for s in sites if s.disqualifiers()]
    candidates.sort(key=lambda s: -s.best()["score"])

    print()
    print("=" * 120)
    print(f"{family.name.upper()}  ->  {family.target}   ({len(sites)} sites)")
    print("=" * 120)
    print(
        f"{'site':<46} {'container':<30} {'inst':>9} {'C':>3} "
        f"{'fit%':>6} {'allocs saved':>13} {'mem':>9}"
    )
    print("-" * 120)
    for site in candidates[: args.top]:
        best = site.best()
        print(
            f"{short_location(site):<46} {site.container[:29]:<30} {site.instances:>9,} "
            f"{best['capacity']:>3} {best['fractionFitting'] * 100:>5.1f}% "
            f"{best['allocationsAvoided']:>13,} {best['memoryBytes']:>8,}B"
        )
        sweep = "  ".join(
            f"C={c}:{site.fraction_fitting(c) * 100:.0f}%" for c in family.sweep
        )
        print(f"{'':<46} sweep: {sweep}")
        if site.warnings():
            print(f"{'':<46} note: {', '.join(site.warnings())}")
        ambiguous = find_ambiguous_fields(site, args.source_root)
        if ambiguous > 1:
            print(
                f"{'':<46} WARNING: the class at this line declares {ambiguous} fields of this "
                f"type; their statistics are merged"
            )
        for context in caller_frames(site, symbols):
            print(f"{'':<46} context: {context}")

    print(f"\nDisqualified by operation mix: {len(disqualified)} of {len(sites)} sites")
    reasons = defaultdict(int)
    for site in disqualified:
        for name in site.disqualifiers():
            reasons[name] += 1
    for name, count in sorted(reasons.items(), key=lambda kv: -kv[1]):
        print(f"  {name:<22} {count:>5} sites")


def report_short_capacities(family, sites, args, symbols):
    """Judge the inline capacity each existing `Short*` site already declares.

    A site whose instances never promote has a capacity that is at least large enough, and possibly
    larger than it needs to be; a site that promotes often is paying for inline storage it abandons,
    and either wants a larger capacity or should not have been a `Short*` at all.
    """
    sites = sorted(sites, key=lambda s: -s.instances)
    print()
    print("=" * 120)
    print(f"{family.name.upper()}  (verdict on the declared inline capacity)   ({len(sites)} sites)")
    print("=" * 120)
    print(
        f"{'site':<46} {'container':<34} {'inst':>9} {'declared':>9} "
        f"{'promoted':>9} {'suggest':>8}"
    )
    print("-" * 120)
    for site in sites[: args.top]:
        suggestion = site.best()["capacity"]
        print(
            f"{short_location(site):<46} {site.container[:33]:<34} {site.instances:>9,} "
            f"{site.inline_capacity:>9} {site.promotion_rate * 100:>8.1f}% {suggestion:>8}"
        )
        sweep = "  ".join(
            f"C={c}:{site.fraction_fitting(c) * 100:.0f}%" for c in family.sweep
        )
        print(f"{'':<46} sweep: {sweep}")
        for context in caller_frames(site, symbols):
            print(f"{'':<46} context: {context}")


def report_sizes_only(family, sites, args, symbols):
    """Report a family that has no inline-storage counterpart to convert to.

    There is nothing to recommend here, so the useful output is simply how large these containers
    get and how much they are used, which is what would justify writing such a counterpart.
    """
    sites = sorted(sites, key=lambda s: -s.instances)
    print()
    print("=" * 120)
    print(f"{family.name.upper()}  (no inline variant exists; sizes only)   ({len(sites)} sites)")
    print("=" * 120)
    print(f"{'site':<46} {'container':<34} {'inst':>9} {'fit@8':>7} {'lookups':>12}")
    print("-" * 120)
    for site in sites[: args.top]:
        print(
            f"{short_location(site):<46} {site.container[:33]:<34} {site.instances:>9,} "
            f"{site.fraction_fitting(8) * 100:>6.0f}% {sum(site.lookups.values()):>12,}"
        )


def report_strings(family, sites, args, symbols):
    """Report string-buffer allocations: how large they are, and who causes them.

    Two kinds of record appear here. The ones with a source location are exact counts attributed to
    the line inside the string implementation that asked for a buffer, which says *why* buffers are
    allocated. The ones keyed by a stack are sampled and say *who* asked, which the source location
    cannot, because every allocation passes through the same few lines.
    """
    by_site = [s for s in sites if s.file != "<backtrace>"]
    by_stack = [s for s in sites if s.file == "<backtrace>"]

    total = sum(s.instances for s in by_site)
    growth = sum(s.instances for s in by_site if s.family.name == "StringBufferGrowth")

    print()
    print("=" * 120)
    print(f"STRING BUFFER ALLOCATIONS   ({total:,} allocations)")
    print("=" * 120)
    if total:
        print(
            f"{growth:,} ({growth / total * 100:.1f}%) replaced a buffer the string already had; "
            f"the rest were a string's first."
        )

    # The combined distribution answers the question a small-string optimization asks: what
    # fraction of all string allocations would an inline buffer of each candidate size remove?
    combined = defaultdict(int)
    for site in by_site:
        for bucket, count in site.peaks.items():
            combined[bucket] += count
    if combined:
        folded = sum(combined.values())
        print("\nAll allocations by capacity:")
        for capacity in family.sweep:
            fitting = sum(
                c for b, c in combined.items() if bucket_upper_bound(b) <= capacity
            )
            print(
                f"  an inline buffer of {capacity:>3} chars would remove "
                f"{fitting / folded * 100:>5.1f}% of them ({fitting:,})"
            )

    print("\nBy reason (the line inside the string implementation that asked):")
    print(f"{'site':<46} {'allocations':>13} {'kind':>8} {'fit@16':>8} {'fit@32':>8}")
    print("-" * 120)
    for site in sorted(by_site, key=lambda s: -s.instances)[: args.top]:
        kind = "growth" if site.family.name == "StringBufferGrowth" else "fresh"
        print(
            f"{short_location(site):<46} {site.instances:>13,} {kind:>8} "
            f"{site.fraction_fitting(16) * 100:>7.0f}% {site.fraction_fitting(32) * 100:>7.0f}%"
        )

    if by_stack:
        sample_rate = by_stack[0].sample_rate
        print(
            f"\nBy caller (sampled 1 in {sample_rate}, so these counts are estimates) -- "
            f"top {min(args.top, len(by_stack))} of {len(by_stack)} distinct stacks:"
        )
        print("-" * 120)
        for site in sorted(by_stack, key=lambda s: -s.instances)[: args.top]:
            print(
                f"  ~{site.instances:>10,} allocations  fit@16={site.fraction_fitting(16) * 100:.0f}%"
            )
            for context in caller_frames(site, symbols, count=3):
                print(f"                   {context}")


def report_hashed_strings(family, sites, args, symbols):
    """Report the interned identifiers: how long they are, and how many exist at once.

    Two numbers decide whether this type should hold its characters inline. The length
    distribution says what fraction of identifiers an inline buffer of a given size would hold, and
    the count of *allocating* constructions -- as opposed to copies, which share a buffer that
    already exists -- says how many heap allocations such a buffer would actually remove. The
    distinction matters because a copy still pays the memory an inline buffer costs while saving
    nothing.
    """
    total = sum(s.instances for s in sites)
    allocating = sum(sum(s.inserts.values()) for s in sites)
    live = sum(s.live_high_water for s in sites)

    print()
    print("=" * 120)
    print(f"IMMUTABLEHASHEDSTRING   ({total:,} constructions, {len(sites)} sites)")
    print("=" * 120)
    if total:
        print(
            f"{allocating:,} ({allocating / total * 100:.1f}%) built a buffer; the rest shared one "
            f"that already existed."
        )

    combined = defaultdict(int)
    for site in sites:
        for bucket, count in site.peaks.items():
            combined[bucket] += count
    if combined:
        folded = sum(combined.values())
        print("\nAll constructions by identifier length:")
        for capacity in family.sweep:
            fitting = sum(c for b, c in combined.items() if bucket_upper_bound(b) <= capacity)
            element_bytes = capacity * live
            print(
                f"  an inline buffer of {capacity:>3} chars would hold "
                f"{fitting / folded * 100:>5.1f}% of them, and cost {element_bytes:,}B "
                f"at the high-water mark"
            )

    print("\nBy declaration site:")
    print(f"{'site':<46} {'constructions':>14} {'allocating':>11} {'fit@16':>8} {'fit@32':>8}")
    print("-" * 120)
    for site in sorted(sites, key=lambda s: -s.instances)[: args.top]:
        print(
            f"{short_location(site):<46} {site.instances:>14,} "
            f"{sum(site.inserts.values()):>11,} "
            f"{site.fraction_fitting(16) * 100:>7.0f}% {site.fraction_fitting(32) * 100:>7.0f}%"
        )
        for context in caller_frames(site, symbols):
            print(f"{'':<46} context: {context}")


def report_anti_candidates(sites, args):
    """Report sites that should not be converted, which can matter more than the ranking.

    A container that is nearly always empty is pure size overhead in whatever object holds it, and
    wants removing or allocating lazily rather than converting. A container with a very high lookup
    count wants to keep its hash table.
    """
    print()
    print("=" * 120)
    print("ANTI-CANDIDATES")
    print("=" * 120)

    always_empty = [
        s
        for s in sites
        if s.folded >= 100 and s.peaks.get(0, 0) / s.folded >= 0.99 and s.file != "<backtrace>"
    ]
    always_empty.sort(key=lambda s: -s.instances)
    print(f"\nAlways empty (>=99% of instances never held an element) -- {len(always_empty)} sites")
    print("These cost space in their parent object for nothing; consider removing or allocating")
    print("them lazily.")
    for site in always_empty[:15]:
        print(f"  {site.file}:{site.line:<6} {site.container[:40]:<42} {site.instances:>10,}")

    hot_big = sorted(
        (s for s in sites if s.folded and s.family.target),
        key=lambda s: -sum(s.lookups.values()),
    )
    print("\nKeep the hash table (most lookups) -- top 10")
    for site in hot_big[:10]:
        lookups = sum(site.lookups.values())
        print(
            f"  {site.file}:{site.line:<6} {site.container[:36]:<38} "
            f"{lookups:>12,} lookups  fit@8={site.fraction_fitting(8) * 100:.0f}%"
        )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("patterns", nargs="+", help="stats JSON files or globs")
    parser.add_argument(
        "--no-symbolize", action="store_true", help="skip addr2line symbolization"
    )
    parser.add_argument("--source-root", default=".", help="repository root, for field checks")
    parser.add_argument("--top", type=int, default=20, help="how many rows to show per section")
    parser.add_argument(
        "--family",
        action="append",
        help="report only this family (repeatable); default is all of them",
    )
    parser.add_argument("--json", help="write the full ranking to this path")
    args = parser.parse_args()

    sites, files = load(args.patterns)
    total_instances = sum(s.instances for s in sites)
    print(f"merged {len(files)} file(s): {len(sites)} records, {total_instances:,} events")

    by_family = defaultdict(list)
    for site in sites:
        if site.folded:
            by_family[site.family.name].append(site)

    print("\nrecords by family:")
    for name, group in sorted(by_family.items(), key=lambda kv: -sum(s.instances for s in kv[1])):
        events = sum(s.instances for s in group)
        print(f"  {name:<20} {len(group):>5} records  {events:>14,} events")

    symbols = symbolize(sites) if not args.no_symbolize else {}

    order = ["Dictionary", "HashSet", "List", "ShortList", "ShortDictionary", "OrderedDictionary"]
    for name in order:
        if args.family and name not in args.family:
            continue
        group = by_family.get(name)
        if not group:
            continue
        family = FAMILIES[name]
        if family.is_short:
            report_short_capacities(family, group, args, symbols)
        elif family.target:
            report_conversion_candidates(family, group, args, symbols)
        else:
            report_sizes_only(family, group, args, symbols)

    hashed = by_family.get("ImmutableHashedString")
    if hashed and (not args.family or "ImmutableHashedString" in args.family):
        report_hashed_strings(FAMILIES["ImmutableHashedString"], hashed, args, symbols)

    strings = by_family.get("StringBufferAllocation", []) + by_family.get(
        "StringBufferGrowth", []
    )
    if strings and (not args.family or "String" in args.family):
        report_strings(FAMILIES["StringBufferAllocation"], strings, args, symbols)

    if not args.family:
        report_anti_candidates(sites, args)

    if args.json:
        out = []
        for site in sites:
            if not site.folded:
                continue
            out.append(
                {
                    "family": site.family.name,
                    "file": site.file,
                    "line": site.line,
                    "function": site.function,
                    "container": site.container,
                    "instances": site.instances,
                    "folded": site.folded,
                    "everAllocated": site.ever_allocated,
                    "liveHighWater": site.live_high_water,
                    "inlineCapacity": site.inline_capacity,
                    "ops": site.op_names(),
                    "disqualifiers": site.disqualifiers(),
                    "peaks": dict(site.peaks),
                    "sweep": [site.evaluate(c) for c in site.family.sweep],
                }
            )
        with open(args.json, "w") as f:
            json.dump(out, f, indent=2)
        print(f"\nwrote {args.json}")


if __name__ == "__main__":
    main()
