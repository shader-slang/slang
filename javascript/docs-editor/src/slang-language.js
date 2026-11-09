// Slang-specific rules adapted from shader-slang/slang-playground's language-server.ts.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
export function registerSlang(monaco) {
    monaco.languages.register({ id: "slang" });
    monaco.languages.setLanguageConfiguration("slang", {
        comments: { lineComment: "//", blockComment: ["/*", "*/"] },
        brackets: [["{", "}"], ["[", "]"], ["(", ")"]],
        autoClosingPairs: [
            { open: "{", close: "}" }, { open: "[", close: "]" }, { open: "(", close: ")" },
            { open: '"', close: '"', notIn: ["string", "comment"] },
        ],
    });
    monaco.languages.setMonarchTokensProvider("slang", {
        keywords: (`if else switch case default return try throw throws catch while for let var
            spirv_asm no_diff dynamic_uniform fwd_diff bwd_diff module implementing __include
            __dispatch_kernel row_major column_major nointerpolation snorm unorm globallycoherent
            extern layout do static const in out inout ref __subscript __init property get set
            class struct interface public private internal protected typedef typealias uniform export
            groupshared extension associatedtype this namespace This using __generic __exported import
            enum break continue discard defer cbuffer tbuffer func is as nullptr none true false functype
            sizeof alignof __target_switch __intrinsic_asm typename each expand where register packoffset
            host device Differentiable ForwardDifferentiable BackwardDifferentiable`).split(/\s+/),
        tokenizer: {
            root: [
                [/R"(\w*)\(/, { token: "string", next: "@rawstring.$1" }],
                [/\/\//, "comment", "@linecomment"],
                [/\/\*/, "comment", "@comment"],
                [/\b(?:RW|RasterizerOrdered)?(?:StructuredBuffer|ByteAddressBuffer|Texture\w*)\b|\b(?:RaytracingAccelerationStructure|SamplerState|ConstantBuffer|ParameterBlock)\b/, "type"],
                [/\b(?:bool|int|uint|half|float|double|void)(?:[1-4](?:x[1-4])?)?\b/, "type"],
                [/[A-Za-z_]\w*/, { cases: { "@keywords": "keyword", "@default": "identifier" } }],
                [/#[a-zA-Z_]\w*/, "keyword"],
                [/0[xX][\da-fA-F]+|(?:\d*\.)?\d+(?:[eE][+-]?\d+)?[fFuUlL]*/, "number"],
                [/"/, "string", "@string"],
                [/'(?:[^'\\]|\\.)'/, "string"],
                [/[{}()[\]]/, "@brackets"],
                [/[;,.]/, "delimiter"],
                [/[=><!~?:&|+*/^%\-]+/, "operator"],
            ],
            linecomment: [[/.*$/, "comment", "@pop"]],
            comment: [[/\/\*/, "comment", "@push"], [/\*\//, "comment", "@pop"], [/[^/*]+|[/*]/, "comment"]],
            string: [[/\\./, "string.escape"], [/"/, "string", "@pop"], [/[^"\\]+/, "string"]],
            rawstring: [
                [/\)(\w*)"/, { cases: { "$1==$S2": { token: "string", next: "@pop" }, "@default": "string" } }],
                [/./, "string"],
            ],
        },
    });
}
