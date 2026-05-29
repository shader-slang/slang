#!/usr/bin/python3

"""
Generates generic type promotion inference tests covering all scalar type pair
combinations. The generated test validates both the arithmetic result and the
inferred type identity for each pair.

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
        return self.literalFmt.format(v=value)

    def isInteger(self):
        return self.kind in (kKind_Signed, kKind_Unsigned)


# Core scalar types (supported on both VK and CPU)
coreTypes = [
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
    TypeInfo("int64_t",  kKind_Signed,   kRank_Int64, 8, "int64_t({v})"),
    TypeInfo("uint64_t", kKind_Unsigned, kRank_Int64, 8, "uint64_t({v})"),
    TypeInfo("double",   kKind_Float,    kRank_Int64, 8, "double({v})"),
]


def getConversionCost(fromKind, fromRank, toKind, toRank):
    """Replicate getBaseTypeConversionCost from slang-embedded-core-module-source.cpp."""

    if fromKind == toKind and fromRank == toRank:
        return 0  # same type

    # Same kind: rank promotion (150) or demotion (900)
    if fromKind == toKind:
        return 150 if toRank > fromRank else 900

    # Unsigned -> Signed
    if fromKind == kKind_Unsigned and toKind == kKind_Signed:
        if toRank > fromRank:
            return 200  # kConversionCost_UnsignedToSignedPromotion
        elif toRank == fromRank:
            return 300  # kConversionCost_SameSizeUnsignedToSignedConversion
        else:
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
    generic parameter. Mirrors TryJoinTypes in slang-check-constraint.cpp."""

    # cost to convert B to A
    costBtoA = getConversionCost(typeB.kind, typeB.rank, typeA.kind, typeA.rank)
    # cost to convert A to B
    costAtoB = getConversionCost(typeA.kind, typeA.rank, typeB.kind, typeB.rank)

    # TryJoinTypes: "if (costConvertRightToLeft > costConvertLeftToRight) return right;"
    if costBtoA > costAtoB:
        return typeB
    else:
        return typeA


def getTypeCode(t):
    """Expected getInferredTypeCode result: +sizeof for unsigned, -sizeof for signed/float."""
    return t.sizeof if t.kind == kKind_Unsigned else -t.sizeof


def shouldSkipPair(ta, tb):
    """Skip pairs that produce compiler warnings or backend errors:
    - Integer + half -> T=half: E30081 'implicit conversion not recommended'
      and CPU backend cannot convert half<->int.
    - float + double -> T=double: E30082 'implicit float-to-double conversion'
      warns about performance."""
    inferred = inferType(ta, tb)
    if inferred.name == "half":
        if ta.isInteger() or tb.isInteger():
            return True
    if inferred.name == "double":
        if (ta.name == "float" or tb.name == "float"):
            return True
    return False


def filterPairs(pairs):
    """Remove pairs that are known to be problematic."""
    return [(a, b) for a, b in pairs if not shouldSkipPair(a, b)]


def generateTestBody(pairs):
    """Generate the computeMain body for a list of (TypeInfo, TypeInfo) pairs."""

    lines = []
    lines.append("    int idx = 0;")

    for i, (ta, tb) in enumerate(pairs):
        inferred = inferType(ta, tb)
        code = getTypeCode(inferred)
        litA = ta.literal(3)
        litB = tb.literal(7)

        lines.append("")
        lines.append(
            f"    // Case {i}: {ta.name} + {tb.name} -> T = {inferred.name} (code {code})"
        )
        lines.append(f"    {{")
        lines.append(f"        {ta.name} a = {litA};")
        lines.append(f"        {tb.name} b = {litB};")
        lines.append(f"        outputBuffer[idx++] = int(add(a, b));")
        lines.append(f"        // CHECK: 10")
        lines.append(f"        outputBuffer[idx++] = getInferredTypeCode(a, b);")
        lines.append(f"        // CHECK: {code}")
        lines.append(f"    }}")

    return "\n".join(lines)


def generateTestFile(pairs, description, testDirectives, extraNote=""):
    """Generate a complete test file for the given pairs."""

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
// Bool promotion is not tested due to non-deterministic inference behavior
// tracked in issue #10164.
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


def main():
    if len(sys.argv) != 1:
        print("Generates generic type promotion inference tests for all scalar type pairs")
        print("")
        print("Generated files:")
        print("  gen-generic-type-promotion-inference.slang       (core types)")
        print("  gen-generic-type-promotion-inference-64bit.slang (64-bit types)")
        sys.exit(1)

    slangTopDirectory = os.path.abspath(os.path.join(os.path.dirname(__file__), "../.."))
    genTestDirectory = os.path.join(slangTopDirectory, genTargetDirectory)
    os.makedirs(genTestDirectory, exist_ok=True)
    os.chdir(genTestDirectory)

    # Core types: int8-32, uint8-32, half, float
    corePairs = filterPairs(list(combinations(coreTypes, 2)))
    coreFile = "gen-generic-type-promotion-inference.slang"
    coreDirectives = [
        "//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -shaderobj -output-using-type",
        "//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type",
    ]
    with open(coreFile, "w") as f:
        f.write(
            generateTestFile(
                corePairs,
                "Generic type promotion inference: core scalar types (8-32 bit)",
                coreDirectives,
            )
        )
    print(f"Generated: {genTargetDirectory}{coreFile} ({len(corePairs)} pairs)")

    # 64-bit type pairs: all pairs involving at least one 64-bit type
    allTypes = coreTypes + extendedTypes
    allPairs = list(combinations(allTypes, 2))
    pairs64 = filterPairs(
        [(a, b) for a, b in allPairs if a in extendedTypes or b in extendedTypes]
    )

    ext64File = "gen-generic-type-promotion-inference-64bit.slang"
    ext64Directives = [
        "//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -cpu -shaderobj -output-using-type",
    ]
    extNote = (
        "//\n"
        "// 64-bit types (int64_t, uint64_t, double) may require extensions on GPU\n"
        "// backends, so this test runs on CPU only.\n"
    )
    with open(ext64File, "w") as f:
        f.write(
            generateTestFile(
                pairs64,
                "Generic type promotion inference: 64-bit scalar types",
                ext64Directives,
                extraNote=extNote,
            )
        )
    print(f"Generated: {genTargetDirectory}{ext64File} ({len(pairs64)} pairs)")


if __name__ == "__main__":
    main()
