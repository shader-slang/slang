import { TextDocumentContentChangeEvent } from 'vscode-languageserver';

export type CompileTarget = 'SPIRV' | 'HLSL' | 'GLSL' | 'METAL' | 'WGSL' | 'CUDA';

export type CompileRequest = {
	target: CompileTarget,
	entrypoint: string,
	sourceCode: string,
	shaderPath: string,
	noWebGPU: boolean,
}

export type EntrypointsRequest = {
	sourceCode: string,
	shaderPath: string,
}

export type ServerInitializationOptions = {
	extensionUri: string,
	workspaceUris: string[],
	files: { uri: string, content: string }[],
}

export type ScalarType = `${"uint" | "int"}${8 | 16 | 32 | 64}` | `${"float"}${16 | 32 | 64}`;

export type SlangFormat = "rgba32f" | "rgba16f" | "rg32f" | "rg16f" | "r11f_g11f_b10f" | "r32f" | "r16f" | "rgba16" | "rgb10_a2" | "rgba8" | "rg16" | "rg8" | "r16" | "r8" | "rgba16_snorm" | "rgba8_snorm" | "rg16_snorm" | "rg8_snorm" | "r16_snorm" | "r8_snorm" | "rgba32i" | "rgba16i" | "rgba8i" | "rg32i" | "rg16i" | "rg8i" | "r32i" | "r16i" | "r8i" | "rgba32ui" | "rgba16ui" | "rgb10_a2ui" | "rgba8ui" | "rg32ui" | "rg16ui" | "rg8ui" | "r32ui" | "r16ui" | "r8ui" | "64ui" | "r64i" | "bgra8"

export type Bindings = { [k: string]: GPUBindGroupLayoutEntry };
export type HashedStringData = { [hash: number]: string }

export type ReflectionBinding = {
	"kind": "uniform",
	"offset": number,
	"size": number,
} | {
	"kind": "descriptorTableSlot",
	"index": number,
};

export type ReflectionType = {
	"kind": "struct",
	"name": string,
	"fields": ReflectionParameter[]
} | {
	"kind": "vector",
	"elementCount": 2 | 3 | 4,
	"elementType": ReflectionType,
} | {
	"kind": "scalar",
	"scalarType": ScalarType,
} | {
	"kind": "resource",
	"baseShape": "structuredBuffer",
	"access"?: "readWrite",
	"resultType": ReflectionType,
} | {
	"kind": "resource",
	"baseShape": "texture2D",
	"access"?: "readWrite" | "write",
	"resultType": ReflectionType,
} | {
	"kind": "samplerState",
};

export type ReflectionParameter = {
	"binding": ReflectionBinding,
	"format"?: SlangFormat,
	"name": string,
	"type": ReflectionType,
	"userAttribs"?: ReflectionUserAttribute[],
}

export type ReflectionJSON = {
	"entryPoints": ReflectionEntryPoint[],
	"parameters": ReflectionParameter[],
	"hashedStrings": { [str: string]: number },
};

export type ReflectionEntryPoint = {
	"name": string,
	"parameters": ReflectionParameter[],
	"stage": string,
	"threadGroupSize": [number, number, number],
	"userAttribs"?: ReflectionUserAttribute[],
};

export type ReflectionUserAttribute = {
	"arguments": (number | string)[],
	"name": string,
};

export type EntrypointsResult = string[]

export type Shader = {
	code: string,
	layout: Bindings,
	hashedStrings: HashedStringData,
	reflection: ReflectionJSON,
	threadGroupSizes: { [key: string]: [number, number, number] },
};

export type Result<T> =
	| { succ: true; result: T }
	| { succ: false; message: string; log?: string };

export type UniformController = { buffer_offset: number } & ({
	type: "SLIDER",
	name: string,
	value: number,
	min: number,
	max: number,
} | {
	type: "COLOR_PICK",
	name: string,
	value: [number, number, number],
} | {
	type: "TIME",
} | {
	type: "FRAME_ID",
} | {
	type: "MOUSE_POSITION",
} | {
	type: "KEY",
	key: string,
	scalarType: ScalarType,
})

export type RunnableShaderType = 'imageMain' | 'printMain';
export type ShaderType = RunnableShaderType | null;

export type CompiledPlayground = {
	uri: string,
	shader: Shader,
	mainEntryPoint: RunnableShaderType,
	resourceCommands: ResourceCommand[],
	callCommands: CallCommand[],
	uniformSize: number,
	uniformComponents: UniformController[],
}

export type PlaygroundMessage = {
	type: "init",
	payload: CompiledPlayground,
} | {
	type: "uniformUpdate",
	payload: UniformController[],
}

export type ParsedCommand = {
	"type": "ZEROS",
	"count": number,
	"elementSize": number,
} | {
	"type": "RAND",
	"count": number,
} | {
	"type": "BLACK",
	"width": number,
	"height": number,
} | {
	"type": "BLACK_SCREEN",
	"width_scale": number,
	"height_scale": number,
} | {
	"type": "URL",
	"url": string,
	"format": GPUTextureFormat,
} | {
	"type": "DATA",
	"url": string,
	"elementSize": number,
} | {
	"type": "SLIDER",
	"default": number,
	"min": number,
	"max": number,
	"elementSize": number,
	"offset": number,
} | {
	"type": "COLOR_PICK",
	"default": [number, number, number],
	"elementSize": number,
	"offset": number,
} | {
	"type": "TIME",
	"offset": number,
} | {
	"type": "FRAME_ID",
	"offset": number,
} | {
	"type": "MOUSE_POSITION",
	"offset": number,
} | {
	"type": "KEY",
	key: string,
	offset: number,
	scalarType: ScalarType,
} | {
	"type": "SAMPLER"
};

export type ResourceCommand = { resourceName: string; parsedCommand: ParsedCommand; };
export type CallCommand = {
	type: "RESOURCE_BASED",
	fnName: string,
	resourceName: string,
	elementSize?: number,
	callOnce?: boolean,
} | {
	type: "FIXED_SIZE",
	fnName: string,
	size: number[],
	callOnce?: boolean,
} | {
	type: "INDIRECT",
	fnName: string,
	bufferName: string,
	offset: number,
	callOnce?: boolean,
};

export type PlaygroundRun = {
	userSource: string,
	ret: Shader,
	uri: string,
}

export type WorkerRequest = {
	type: 'Initialize',
	initializationOptions: ServerInitializationOptions,
} | {
	type: 'DidOpenTextDocument',
	textDocument: {
		uri: string,
		text: string,
	}
} | {
	type: 'DidChangeTextDocument',
	textDocument: {
		uri: string,
	},
	contentChanges: TextDocumentContentChangeEvent[],
} | ({
	type: 'slang/compile',
} & CompileRequest) | ({
	type: 'slang/compilePlayground',
	uri: string,
} & CompileRequest) | ({
	type: 'slang/entrypoints'
} & EntrypointsRequest);