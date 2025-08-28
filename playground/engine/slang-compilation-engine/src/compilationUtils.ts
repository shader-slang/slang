import { ScalarType, SlangFormat } from "slang-playground-shared";

export const ACCESS_MAP = {
    "readWrite": "read-write",
    "write": "write-only",
    "read": "read-only",
} as const;

const FORMAT_MAP: Partial<Record<SlangFormat, GPUTextureFormat>> = {
    "rgba32f": "rgba32float",
    "rgba16f": "rgba16float",
    "rg32f": "rg32float",
    "rg16f": "rg16float",
    "r32f": "r32float",
    "r16f": "r16float",
    "rgb10_a2": "rgb10a2unorm",
    "rgba8": "rgba8unorm",
    "rg8": "rg8unorm",
    "r8": "r8unorm",
    "rgba8_snorm": "rgba8snorm",
    "rg8_snorm": "rg8snorm",
    "r8_snorm": "r8snorm",
    "rgba32i": "rgba32sint",
    "rgba16i": "rgba16sint",
    "rgba8i": "rgba8sint",
    "rg32i": "rg32sint",
    "rg16i": "rg16sint",
    "rg8i": "rg8sint",
    "r32i": "r32sint",
    "r16i": "r16sint",
    "r8i": "r8sint",
    "rgba32ui": "rgba32uint",
    "rgba16ui": "rgba16uint",
    "rgb10_a2ui": "rgb10a2uint",
    "rgba8ui": "rgba8uint",
    "rg32ui": "rg32uint",
    "rg16ui": "rg16uint",
    "rg8ui": "rg8uint",
    "r32ui": "r32uint",
    "r16ui": "r16uint",
    "r8ui": "r8uint",
    "bgra8": "bgra8unorm"
}

export function webgpuFormatfromSlangFormat(format: SlangFormat): GPUTextureFormat {
    let gpuFormat = FORMAT_MAP[format];
    if (gpuFormat == undefined) {
        throw new Error(`Could not find webgpu format for ${format}`);
    }
    return gpuFormat;
}

export function getTextureFormat(componentCount: 1 | 2 | 3 | 4, scalarType: ScalarType, access: GPUStorageTextureAccess | undefined): GPUTextureFormat {
    if (access == "read-write" && componentCount > 1) {
        throw new Error("There are no texture formats available with more than 1 component that are read-write")
    }
    let scalarSize = getScalarSize(scalarType);

    if (access == "write-only" && componentCount == 2 && scalarSize != 32) {
        throw new Error("There are no non 32 bit texture formats available with 2 components that are write-only")
    }

    if (scalarSize == 64) {
        throw new Error("There are 64 bit texture formats available")
    }

    let scalarRepresentation = getWebGPURepresentation(scalarType);
    if (componentCount == 1) {
        return `r${scalarRepresentation}`
    } else if (componentCount == 2) {
        return `rg${scalarRepresentation}`
    } else {
        return `rgba${scalarRepresentation}`
    }
}

type ScalarRepresentation = `8unorm` | `${16 | 32}float` | `${8 | 16 | 32}${"sint" | "uint"}`;
function getWebGPURepresentation(scalarType: ScalarType): ScalarRepresentation {
    let size = parseInt(scalarType.replace(/^[a-z]*/, ""));
    let type = scalarType.replace(/[0-9]*$/, "");
    if (type == "int")
        type = "sint";

    return `${size}${type}` as any
}

function getScalarSize(scalarType: ScalarType): 8 | 16 | 32 | 64 {
    let size = parseInt(scalarType.replace(/^[a-z]*/, ""));
    return size as any;
}