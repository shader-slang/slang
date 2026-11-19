#!/usr/bin/python3

"""
Generates generic type promotion inference tests covering all scalar type pair
combinations. The generated compute tests validate both the arithmetic result
and the inferred type identity for each pair whose types join; pairs whose
types tie at equal conversion cost are generated into a diagnostic test that
asserts inference fails (issue #10164).

Run from the repository root:
    python3 extras/test-generators/generate-generic-type-promotion-tests.py
"""

import os
import sys
from itertools import combinations


# Output directory relative to slang top
genTargetDirectory = "tests/generated/generics/"

# Conversion ranks matching slang-embedded-core-module-source.cpp
kRank_Bool = 0
kRank_Int8 = 1
kRank_Int16 = 2
kRank_Int32 = 3
kRank_IntPtr = 4
kRank_Int64 = 5

kKind_Bool = "bool"
kKind_Signed = "signed"
kKind_Unsigned = "unsigned"
kKind_Float = "float"


class TypeInfo:
    """Scalar type descriptor matching the compiler's BaseTypeConversionInfo."""

    def __init__(self, name, kind, rank, sizeof, literalFmt):
        self.name = name
        self.kind = kind
        self.rank = rank
        self.sizeof = sizeof
        # Python format string: use {v} for the value
        self.literalFmt = literalFmt

    def literal(self, value):
        if self.kind == kKind_Bool:
            # Bool has no numeric literal spelling; use testValue(), which
            # yields `true`/`false` with the matching numeric value.
            raise ValueError("use testValue() for bool operands")
        return self.literalFmt.format(v=value)

    def testValue(self, value):
        """(source literal, numeric value) for an operand in a generated case.

        Bool has no numeric literal spelling, so any nonzero value becomes
        `true` (1) and zero becomes `false` (0)."""
        if self.kind == kKind_Bool:
            return ("true" if value else "false", 1 if value else 0)
        return (self.literal(value), value)

    def isInteger(self):
        return self.kind in (kKind_Signed, kKind_Unsigned)


# Core scalar types (supported on both VK and CPU)
coreTypes = [
    TypeInfo("bool",     kKind_Bool,     kRank_Bool, 1, "{v}"),
    TypeInfo("int8_t",   kKind_Signed,   kRank_Int8,  1, "int8_t({v})"),
    TypeInfo("int16_t",  kKind_Signed,   kRank_Int16, 2, "int16_t({v})"),
    TypeInfo("int32_t",  kKind_Signed,   kRank_Int32, 4, "{v}"),
    TypeInfo("uint8_t",  kKind_Unsigned, kRank_Int8,  1, "uint8_t({v})"),
    TypeInfo("uint16_t", kKind_Unsigned, kRank_Int16, 2, "uint16_t({v})"),
    TypeInfo("uint32_t", kKind_Unsigned, kRank_Int32, 4, "{v}u"),
    TypeInfo("half",     kKind_Float,    kRank_Int16, 2, "half({v})"),
    TypeInfo("float",    kKind_Float,    kRank_Int32, 4, "float({v})"),
]

# 64-bit types (may require extensions on some backends)
extendedTypes = [
    TypeInfo("int64_t",  kKind_Signed,   kRank_Int64,  8, "int64_t({v})"),
    TypeInfo("uint64_t", kKind_Unsigned, kRank_Int64, 8, "uint64_t({v})"),
    TypeInfo("intptr_t", kKind_Signed,   kRank_IntPtr, 8, "intptr_t({v})"),
    TypeInfo("uintptr_t", kKind_Unsigned, kRank_IntPtr, 8, "uintptr_t({v})"),
    TypeInfo("double",   kKind_Float,    kRank_Int64, 8, "double({v})"),
]


def getConversionCost(fromKind, fromRank, toKind, toRank):
    """Replicate getBaseTypeConversionCost from slang-embedded-core-module-source.cpp."""

    if fromKind == toKind and fromRank == toRank:
        return 0  # same type

    # Bool: conversion kind Unsigned, rank Bool (lowest). Bool-to-int32 is the
    # BoolToInt special case (tag == Int); other signed targets take the
    # unsigned-to-signed promotion, except pointer-sized targets, which the
    # compiler also excludes from that promotion (the pointer size might be
    # as small as the source), so they fall to the general conversion.
    if fromKind == kKind_Bool and toKind == kKind_Signed:
        if toRank == kRank_Int32:
            return 120  # kConversionCost_BoolToInt
        if toRank != kRank_IntPtr:
            return 200  # kConversionCost_UnsignedToSignedPromotion
        return 900  # kConversionCost_GeneralConversion
    if toKind == kKind_Bool:
        return 900  # kConversionCost_GeneralConversion
    if fromKind == kKind_Bool and toKind == kKind_Unsigned:
        return 150  # kConversionCost_RankPromotion

    # Same kind: rank promotion (150) or demotion (900)
    if fromKind == toKind:
        return 150 if toRank > fromRank else 900

    # Unsigned -> Signed. Pointer-sized ranks on either side are excluded
    # from both promotion flavors for the same pointer-size reason as above.
    if fromKind == kKind_Unsigned and toKind == kKind_Signed:
        if toRank != kRank_IntPtr and fromRank != kRank_IntPtr:
            if toRank > fromRank:
                return 200  # kConversionCost_UnsignedToSignedPromotion
            if toRank == fromRank:
                return 300  # kConversionCost_SameSizeUnsignedToSignedConversion
        return 900

    # Signed -> Unsigned
    if fromKind == kKind_Signed and toKind == kKind_Unsigned:
        return 250 if toRank >= fromRank else 900

    # Integer -> Float/Double (to.rank >= Int32)
    if toKind == kKind_Float and toRank >= kRank_Int32 and fromRank >= kRank_Int8:
        return 400  # kConversionCost_IntegerToFloatConversion

    # Integer -> Half (to.rank >= Int16)
    if toKind == kKind_Float and toRank >= kRank_Int16 and fromRank >= kRank_Int8:
        return 500  # kConversionCost_IntegerToHalfConversion

    return 900  # kConversionCost_GeneralConversion


def inferType(typeA, typeB):
    """Determine which type T is inferred when both types constrain the same
    generic parameter. Mirrors TryJoinTypes in slang-check-constraint.cpp:
    the strictly cheaper conversion direction wins, and an equal-cost tie
    between distinct types has no better side, so inference fails."""

    # cost to convert B to A
    costBtoA = getConversionCost(typeB.kind, typeB.rank, typeA.kind, typeA.rank)
    # cost to convert A to B
    costAtoB = getConversionCost(typeA.kind, typeA.rank, typeB.kind, typeB.rank)

    # TryJoinTypes: "if (costConvertRightToLeft > costConvertLeftToRight)
    # return right;"
    if costBtoA > costAtoB:
        return typeB
    if costAtoB > costBtoA:
        return typeA
    return None  # equal cost: no better side, inference fails


def getTypeCode(t):
    """Expected getInferredTypeCode result: +sizeof for unsigned, -sizeof for signed/float."""
    return t.sizeof if t.kind == kKind_Unsigned else -t.sizeof


def shouldSkipPair(ta, tb, inferred):
    """Skip success pairs that produce compiler warnings or backend errors:
    - Integer + half -> T=half: E30081 'implicit conversion not recommended'
      and CPU backend cannot convert half<->int.
    - float + double -> T=double: E30082 'implicit float-to-double conversion'
      warns about performance."""
    if inferred.name == "half":
        if ta.isInteger() or tb.isInteger():
            return True
    if inferred.name == "double":
        if (ta.name == "float" or tb.name == "float"):
            return True
    return False


def partitionPairs(pairs):
    """Split pairs into success cases (the types join; a triple with the
    inferred type) and tie pairs (equal cost; inference fails)."""
    success = []
    ties = []
    for a, b in pairs:
        inferred = inferType(a, b)
        if inferred is None:
            ties.append((a, b))
        else:
            success.append((a, b, inferred))
    return success, ties


def generateTestBody(pairs):
    """Generate the computeMain body for a list of (TypeInfo, TypeInfo, inferred) triples."""

    lines = []
    lines.append("    int idx = 0;")

    for i, (ta, tb, inferred) in enumerate(pairs):
        code = getTypeCode(inferred)
        litA, valA = ta.testValue(3)
        litB, valB = tb.testValue(7)
        expected = valA + valB

        lines.append("")
        lines.append(
            f"    // Case {i}: {ta.name} + {tb.name} -> T = {inferred.name} (code {code})"
        )
        lines.append(f"    {{")
        lines.append(f"        {ta.name} a = {litA};")
        lines.append(f"        {tb.name} b = {litB};")
        lines.append(f"        outputBuffer[idx++] = int(add(a, b));")
        lines.append(f"        // CHECK: {expected}")
        lines.append(f"        outputBuffer[idx++] = getInferredTypeCode(a, b);")
        lines.append(f"        // CHECK: {code}")
        lines.append(f"    }}")

    return "\n".join(lines)


def generateTestFile(pairs, description, testDirectives, extraNote=""):
    """Generate a complete compute test file for the given success triples."""

    bufferSize = len(pairs) * 2
    body = generateTestBody(pairs)
    zeros = " ".join(["0"] * bufferSize)
    directives = "\n".join(testDirectives)

    return f"""\
// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// {description}
// Tests {len(pairs)} type pair combinations.
//
// Each case validates both the arithmetic result (add) and the inferred type
// identity (getInferredTypeCode). The type code encodes sizeof(T) with sign:
//   positive = unsigned type, negative = signed or float type.
//
// Bool pairs join with integer types (the integer side is strictly better)
// and tie with floating-point types at equal conversion cost; the tie pairs
// fail inference and are generated into
// gen-generic-type-promotion-tie-rejection.slang (issue #10164). Bool
// itself therefore never appears as an inferred type here: it either loses
// the join to an integer type or is partitioned into the rejection file.
//
// Excluded pairs: integer+half (E30081, CPU backend limitation) and
// float+double (E30082 performance warning).
{extraNote}
{directives}

T add<T : IArithmetic>(T a, T b)
{{
    return a + b;
}}

// Returns a code identifying the inferred type T:
//   sizeof(T)  for unsigned types  (T(-1) > T(0))
//  -sizeof(T)  for signed / float types
int getInferredTypeCode<T : IArithmetic>(T a, T b)
{{
    if (T(-1) > T(0))
        return int(sizeof(T));
    else
        return -int(sizeof(T));
}}

//TEST_INPUT:ubuffer(data=[{zeros}], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{{
{body}
}}
"""


def generateRejectionTestFile(pairs):
    """Generate a diagnostic test asserting that equal-cost tie pairs fail
    inference with a conflicting-requirements diagnostic naming the two
    types in argument order."""

    cases = []
    for ta, tb in pairs:
        litA, _ = ta.testValue(3)
        litB, _ = tb.testValue(7)
        cases.append(
            f"""\
    {{
        {ta.name} a = {litA};
        {tb.name} b = {litB};
        joinArgs(a, b);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements '{ta.name}' and '{tb.name}'
*/
        joinArgs(b, a);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements '{tb.name}' and '{ta.name}'
*/
    }}"""
        )
    body = "\n\n".join(cases)

    return f"""\
// THIS IS A GENERATED FILE. DO NOT EDIT!
// Instead, edit extras/test-generators/generate-generic-type-promotion-tests.py
// and regenerate by running:
//
//     python3 extras/test-generators/generate-generic-type-promotion-tests.py
//
// Equal-cost ties: generic parameter promotion inference.
// Tests {len(pairs)} type pair combinations in both argument orders.
//
// Each pair's conversion costs tie in both directions, so neither type is
// better and inference fails instead of picking a side (issue #10164).
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void joinArgs<T>(T a, T b)
{{
}}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{{
{body}
}}
"""


def main():
    if len(sys.argv) != 1:
        print("Generates generic type promotion inference tests for all scalar type pairs")
        print("")
        print("Generated files:")
        print("  gen-generic-type-promotion-inference.slang          (core types)")
        print("  gen-generic-type-promotion-inference-64bit.slang    (64-bit types)")
        print("  gen-generic-type-promotion-tie-rejection.slang      (equal-cost ties)")
        sys.exit(1)

    slangTopDirectory = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
    genTestDirectory = os.path.join(slangTopDirectory, genTargetDirectory)
    os.makedirs(genTestDirectory, exist_ok=True)
    os.chdir(genTestDirectory)

    # Partition the full type set once: success triples feed the compute
    # tests (core pairs, then pairs involving a 64-bit type), and tie pairs
    # feed the rejection test, all from one inference pass.
    allTypes = coreTypes + extendedTypes
    allSuccess, allTies = partitionPairs(list(combinations(allTypes, 2)))

    # Core types: bool, int8-32, uint8-32, half, float
    coreTriples = [
        t
        for t in allSuccess
        if t[0] in coreTypes and t[1] in coreTypes and not shouldSkipPair(*t)
    ]
    coreFile = "gen-generic-type-promotion-inference.slang"
    coreDirectives = [
        "//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type",
        "//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type",
    ]
    with open(coreFile, "w") as f:
        f.write(
            generateTestFile(
                coreTriples,
                "Generic type promotion inference: core scalar types (8-32 bit)",
                coreDirectives,
            )
        )
    print(f"Generated: {genTargetDirectory}{coreFile} ({len(coreTriples)} pairs)")

    # 64-bit type pairs: all pairs involving at least one 64-bit type
    pairs64Triples = [
        t
        for t in allSuccess
        if (t[0] in extendedTypes or t[1] in extendedTypes) and not shouldSkipPair(*t)
    ]

    ext64File = "gen-generic-type-promotion-inference-64bit.slang"
    ext64Directives = [
        "//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type",
    ]
    extNote = (
        "//\n"
        "// 64-bit types (int64_t, uint64_t, double, intptr_t, uintptr_t) may\n"
        "// require extensions on GPU backends, so this test runs on CPU only.\n"
    )
    with open(ext64File, "w") as f:
        f.write(
            generateTestFile(
                pairs64Triples,
                "Generic type promotion inference: 64-bit scalar types",
                ext64Directives,
                extraNote=extNote,
            )
        )
    print(f"Generated: {genTargetDirectory}{ext64File} ({len(pairs64Triples)} pairs)")

    # Equal-cost ties (both argument orders)
    tieFile = "gen-generic-type-promotion-tie-rejection.slang"
    with open(tieFile, "w") as f:
        f.write(generateRejectionTestFile(allTies))
    print(f"Generated: {genTargetDirectory}{tieFile} ({len(allTies)} pairs)")


if __name__ == "__main__":
    main()
