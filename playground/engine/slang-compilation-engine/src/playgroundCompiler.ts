import { CallCommand, CompiledPlayground, ParsedCommand, ReflectionJSON, ReflectionType, ResourceCommand, Result, ScalarType, Shader, SlangFormat, UniformController } from "slang-playground-shared";
import { ACCESS_MAP, webgpuFormatfromSlangFormat, getTextureFormat } from "./compilationUtils";

export function compilePlayground(compilation: Shader, uri: string, entrypoint: string): Result<CompiledPlayground> {
    let resourceCommandsResult = getResourceCommandsFromAttributes(compilation.reflection);
    if (resourceCommandsResult.succ == false) {
        return resourceCommandsResult;
    }
    let uniformSize = getUniformSize(compilation.reflection)
    let uniformComponents = getUniformControllers(resourceCommandsResult.result)

    let callCommandResult = parseCallCommands(compilation.reflection);
    if (callCommandResult.succ == false) {
        return callCommandResult;
    }

    return {
        succ: true,
        result: {
            uri,
            shader: compilation,
            mainEntryPoint: entrypoint as any,
            resourceCommands: resourceCommandsResult.result,
            callCommands: callCommandResult.result,
            uniformSize,
            uniformComponents
        }
    }
}


/**
 * See help panel for details on commands
 */
function getResourceCommandsFromAttributes(reflection: ReflectionJSON): Result<ResourceCommand[]> {
    let commands: { resourceName: string, parsedCommand: ParsedCommand }[] = [];

    for (let parameter of reflection.parameters) {
        if (parameter.userAttribs == undefined) continue;
        for (let attribute of parameter.userAttribs) {
            let command: ParsedCommand | null = null;

            if (!attribute.name.startsWith("playground_")) continue;

            let playground_attribute_name = attribute.name.slice(11);
            if (playground_attribute_name == "ZEROS") {
                if (parameter.type.kind != "resource" || parameter.type.baseShape != "structuredBuffer") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports buffers`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    count: attribute.arguments[0] as number,
                    elementSize: getSize(parameter.type.resultType),
                };
            } else if (playground_attribute_name == "SAMPLER") {
                if (parameter.type.kind != "samplerState") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports samplers`,
                    };
                }
                command = {
                    type: playground_attribute_name
                };
            } else if (playground_attribute_name == "RAND") {
                if (parameter.type.kind != "resource" || parameter.type.baseShape != "structuredBuffer") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports buffers`,
                    };
                }
                if (parameter.type.resultType.kind != "scalar" || parameter.type.resultType.scalarType != "float32") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports float buffers`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    count: attribute.arguments[0] as number
                };
            } else if (playground_attribute_name == "BLACK") {
                if (parameter.type.kind != "resource" || parameter.type.baseShape != "texture2D") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports 2D textures`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    width: attribute.arguments[0] as number,
                    height: attribute.arguments[1] as number,
                };
            } else if (playground_attribute_name == "BLACK_SCREEN") {
                if (parameter.type.kind != "resource" || parameter.type.baseShape != "texture2D") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports 2D textures`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    width_scale: attribute.arguments[0] as number,
                    height_scale: attribute.arguments[1] as number,
                };
            } else if (playground_attribute_name == "URL") {
                if (parameter.type.kind != "resource" || parameter.type.baseShape != "texture2D") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports 2D textures`,
                    };
                }
                let slangAccess: keyof typeof ACCESS_MAP = parameter.type.access || "read";
                let access = ACCESS_MAP[slangAccess];

                let scalarType: ScalarType;
                let componentCount: 1 | 2 | 3 | 4;
                if (parameter.type.resultType.kind == "scalar") {
                    componentCount = 1;
                    scalarType = parameter.type.resultType.scalarType;
                } else if (parameter.type.resultType.kind == "vector") {
                    componentCount = parameter.type.resultType.elementCount;
                    if (parameter.type.resultType.elementType.kind != "scalar") return {
                        succ: false,
                        message: `Unhandled inner type for ${name}`,
                    };
                    scalarType = parameter.type.resultType.elementType.scalarType;
                } else {
                    return {
                        succ: false,
                        message: `Unhandled inner type for ${name}`,
                    };
                }

                let format: GPUTextureFormat;
                if (parameter.format) {
                    format = webgpuFormatfromSlangFormat(parameter.format);
                } else {
                    try {
                        format = getTextureFormat(componentCount, scalarType, access);
                    } catch (e) {
                        if (e instanceof Error)
                            return {
                        succ: false,
                        message: `Could not get texture format for ${name}: ${e.message}`,
                    };
                        else
                            return {
                        succ: false,
                        message: `Could not get texture format for ${name}`,
                    };
                    }
                }

                command = {
                    type: playground_attribute_name,
                    url: attribute.arguments[0] as string,
                    format,
                };
            } else if (playground_attribute_name == "TIME") {
                if (parameter.type.kind != "scalar" || !parameter.type.scalarType.startsWith("float") || parameter.binding.kind != "uniform") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports floats`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    offset: parameter.binding.offset,
                };
            } else if (playground_attribute_name == "FRAME_ID") {
                if (parameter.type.kind != "scalar" || !parameter.type.scalarType.startsWith("float") || parameter.binding.kind != "uniform") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports floats`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    offset: parameter.binding.offset,
                };
            } else if (playground_attribute_name == "MOUSE_POSITION") {
                if (parameter.type.kind != "vector" || parameter.type.elementCount <= 3 || parameter.type.elementType.kind != "scalar" || !parameter.type.elementType.scalarType.startsWith("float") || parameter.binding.kind != "uniform") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports float vectors`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    offset: parameter.binding.offset,
                };
            } else if (playground_attribute_name == "SLIDER") {
                if (parameter.type.kind != "scalar" || !parameter.type.scalarType.startsWith("float") || parameter.binding.kind != "uniform") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports floats`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    default: attribute.arguments[0] as number,
                    min: attribute.arguments[1] as number,
                    max: attribute.arguments[2] as number,
                    elementSize: parameter.binding.size,
                    offset: parameter.binding.offset,
                };
            } else if (playground_attribute_name == "COLOR_PICK") {
                if (parameter.type.kind != "vector" || parameter.type.elementCount <= 2 || parameter.type.elementType.kind != "scalar" || !parameter.type.elementType.scalarType.startsWith("float") || parameter.binding.kind != "uniform") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports float vectors`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    default: attribute.arguments as [number, number, number],
                    elementSize: parseInt(parameter.type.elementType.scalarType.slice(5)) / 8,
                    offset: parameter.binding.offset,
                };
            } else if (playground_attribute_name == "KEY") {
                // Only allow on scalar uniforms (float or int)
                if (parameter.type.kind != "scalar" || parameter.binding.kind != "uniform") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute can only be applied to scalar uniforms`,
                    };
                }
                if (!attribute.arguments || attribute.arguments.length !== 1 || typeof attribute.arguments[0] !== "string") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute requires a single string argument (the key name)`,
                    };
                }
                command = {
                    type: playground_attribute_name,
                    key: attribute.arguments[0] as string,
                    offset: parameter.binding.offset,
                    scalarType: parameter.type.scalarType,
                };
            } else if (playground_attribute_name == "DATA") {
                if (parameter.type.kind != "resource" || parameter.type.baseShape != "structuredBuffer") {
                    return {
                        succ: false,
                        message: `${playground_attribute_name} attribute cannot be applied to ${parameter.name}, it only supports structured buffers`,
                    };
                }
                // Size in bytes of each element within the buffer (e.g., sizeof(T)).
                const elementSize = getSize(parameter.type.resultType);
                command = {
                    type: playground_attribute_name,
                    url: attribute.arguments[0] as string,
                    elementSize,
                };
            }

            if (command != null) {
                commands.push({
                    resourceName: parameter.name,
                    parsedCommand: command
                });
            }
        }
    }

    return {
        succ: true,
        result: commands,
    };
}

function getUniformSize(reflection: ReflectionJSON): number {
    let size = 0;

    for (let parameter of reflection.parameters) {
        if (parameter.binding.kind != "uniform") continue;
        size = Math.max(size, parameter.binding.offset + parameter.binding.size)
    }

    return roundUpToNearest(size, 16)
}

function roundUpToNearest(x: number, nearest: number) {
    return Math.ceil(x / nearest) * nearest;
}

function getUniformControllers(resourceCommands: ResourceCommand[]): UniformController[] {
    let controllers: UniformController[] = [];
    for (let resourceCommand of resourceCommands) {
        if (resourceCommand.parsedCommand.type == 'SLIDER') {
            controllers.push({
                type: resourceCommand.parsedCommand.type,
                buffer_offset: resourceCommand.parsedCommand.offset,
                name: resourceCommand.resourceName,
                value: resourceCommand.parsedCommand.default,
                min: resourceCommand.parsedCommand.min,
                max: resourceCommand.parsedCommand.max,
            })
        } else if (resourceCommand.parsedCommand.type == 'COLOR_PICK') {
            controllers.push({
                type: resourceCommand.parsedCommand.type,
                buffer_offset: resourceCommand.parsedCommand.offset,
                name: resourceCommand.resourceName,
                value: resourceCommand.parsedCommand.default,
            })
        } else if (resourceCommand.parsedCommand.type == 'TIME') {
            controllers.push({
                type: resourceCommand.parsedCommand.type,
                buffer_offset: resourceCommand.parsedCommand.offset,
            })
        } else if (resourceCommand.parsedCommand.type == 'FRAME_ID') {
            controllers.push({
                type: resourceCommand.parsedCommand.type,
                buffer_offset: resourceCommand.parsedCommand.offset,
            })
        } else if (resourceCommand.parsedCommand.type == 'MOUSE_POSITION') {
            controllers.push({
                type: resourceCommand.parsedCommand.type,
                buffer_offset: resourceCommand.parsedCommand.offset,
            })
        } else if (resourceCommand.parsedCommand.type == 'KEY') {
            controllers.push({
                type: resourceCommand.parsedCommand.type,
                buffer_offset: resourceCommand.parsedCommand.offset,
                key: resourceCommand.parsedCommand.key,
                scalarType: resourceCommand.parsedCommand.scalarType,
            })
        }
    }
    return controllers;
}

function parseCallCommands(reflection: ReflectionJSON): Result<CallCommand[]> {
    const callCommands: CallCommand[] = [];

    for (let entryPoint of reflection.entryPoints) {
        if (!entryPoint.userAttribs) continue;

        const fnName = entryPoint.name;
        let callCommand: CallCommand | null = null;
        let callOnce: boolean = false;

        for (let attribute of entryPoint.userAttribs) {
            if (attribute.name === "playground_CALL_SIZE_OF") {
                const resourceName = attribute.arguments[0] as string;
                const resourceReflection = reflection.parameters.find((param) => param.name === resourceName);

                if (!resourceReflection) {
                    return {
                        succ: false,
                        message: `Cannot find resource ${resourceName} for ${fnName} CALL command`,
                    };
                }

                let elementSize: number | undefined = undefined;
                if (resourceReflection.type.kind === "resource" && resourceReflection.type.baseShape === "structuredBuffer") {
                    elementSize = getSize(resourceReflection.type.resultType);
                }

                if (callCommand != null) {
                    return {
                        succ: false,
                        message: `Multiple CALL commands found for ${fnName}`,
                    };
                }
                callCommand = {
                    type: "RESOURCE_BASED",
                    fnName,
                    resourceName,
                    elementSize
                };
            } else if (attribute.name === "playground_CALL") {
                if (callCommand != null) {
                    return {
                        succ: false,
                        message: `Multiple CALL commands found for ${fnName}`,
                    };
                }
                callCommand = {
                    type: "FIXED_SIZE",
                    fnName,
                    size: attribute.arguments as number[]
                };
            } else if (attribute.name === "playground_CALL_INDIRECT") {
                if (callCommand != null) {
                    return {
                        succ: false,
                        message: `Multiple CALL commands found for ${fnName}`,
                    };
                }
                const bufferName = attribute.arguments[0] as string;
                const offset = attribute.arguments[1] as number;
                callCommand = {
                    type: "INDIRECT",
                    fnName,
                    bufferName,
                    offset,
                };
            } else if (attribute.name === "playground_CALL_ONCE") {
                if (callOnce) {
                    return {
                        succ: false,
                        message: `Multiple CALL ONCE commands found for ${fnName}`,
                    };
                }
                callOnce = true;
            }
        }

        if (callCommand != null) {
            callCommands.push({
                ...callCommand,
                callOnce
            });
        }
    }

    return {
        succ: true,
        result: callCommands
    };
}

function getSize(reflectionType: ReflectionType): number {
    if (reflectionType.kind == "resource" || reflectionType.kind == "samplerState") {
        throw new Error("unimplemented");
    } else if (reflectionType.kind == "scalar") {
        const bitsMatch = reflectionType.scalarType.match(/\d+$/);
        if (bitsMatch == null) {
            throw new Error("Could not get bit count out of scalar type");
        }
        return parseInt(bitsMatch[0]) / 8;
    } else if (reflectionType.kind == "struct") {
        const alignment = reflectionType.fields.map((f) => {
            if (f.binding.kind == "uniform") return f.binding.size;
            else throw new Error("Invalid state")
        }).reduce((a, b) => Math.max(a, b));

        const unalignedSize = reflectionType.fields.map((f) => {
            if (f.binding.kind == "uniform") return f.binding.offset + f.binding.size;
            else throw new Error("Invalid state")
        }).reduce((a, b) => Math.max(a, b));

        return roundUpToNearest(unalignedSize, alignment);
    } else if (reflectionType.kind == "vector") {
        if (reflectionType.elementCount == 3) {
            return 4 * getSize(reflectionType.elementType);
        }
        return reflectionType.elementCount * getSize(reflectionType.elementType);
    } else {
        let x: never = reflectionType;
        throw new Error("Cannot get size of unrecognized reflection type");
    }
}