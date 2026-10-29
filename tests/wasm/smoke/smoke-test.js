import * as wasmModule from '../slang-wasm.js';
import { readFileSync } from 'fs';
import { resolve, basename } from 'path';

async function runSmokeTest() {
    try {
        // Get the file path from command line arguments
        const filePath = process.argv[2];
        const entryPointName = process.argv[3];
        if (!filePath || !entryPointName) {
            console.error('Please provide a path to a .slang file and an entry point name');
            console.error('Usage: node test/smoke-test.js <path-to-slang-file> <entry-point-name>');
            process.exit(1);
        }

        console.log(`Starting Slang WASM smoke test with file: ${filePath} and entry point: ${entryPointName}`);
        
        // Read the source file
        const absolutePath = resolve(filePath);
        const source = readFileSync(absolutePath, 'utf8');
        const fileName = basename(filePath);
        
        // Load the WASM module
        const module = await wasmModule.default();
        console.log('WASM module loaded successfully');

        // Print available compile targets
        const targets = module.getCompileTargets();
        if (!targets) {
            throw new Error('Failed to get compile targets');
        }
        console.log('Available compile targets:', JSON.stringify(targets, null, 2));

        // Print version info.
        const version = module.getVersionString();
        if (!version) {
            throw new Error('Failed to get version string');
        }
        console.log('Slang version:', version);

        // Find SPIRV target value
        const spirvTarget = targets.findIndex(target => target.name.toLowerCase() === 'spirv');
        if (spirvTarget === -1) {
            throw new Error('SPIRV target not found in available targets');
        }
        console.log('Found SPIRV target at index:', spirvTarget);

        // Get the actual SPIRV target value
        const spirvTargetValue = targets[spirvTarget].value;
        console.log('SPIRV target value:', spirvTargetValue);

        // Create a global session
        const globalSession = module.createGlobalSession();
        if (!globalSession) {
            throw new Error('Failed to create global session');
        }
        console.log('Global session created');

        // Create a session with SPIRV as the target
        const session = globalSession.createSession(spirvTargetValue);
        if (!session) {
            throw new Error('Failed to create session');
        }
        console.log('Session created with SPIRV target');

        // Load the shader source
        const module1 = session.loadModuleFromSource(source, fileName, '');
        if (!module1) {
            const error = module.getLastError();
            throw new Error(`Failed to load module: ${error ? error.message : 'Unknown error'}`);
        }
        console.log('Shader module loaded');

        // Check for compilation errors
        const error = module.getLastError();
        if (error && error.result !== module.SLANG_OK) {
            throw new Error(`Compilation failed: ${error.message}`);
        }
        console.log('No compilation errors found');

        // Try to find the entry point
        const entryPoint = module1.findEntryPointByName(entryPointName);
        if (!entryPoint) {
            throw new Error(`Could not find entry point "${entryPointName}"`);
        }
        console.log(`Entry point "${entryPointName}" found`);

        // Create and link the program
        const program = session.createCompositeComponentType([module1]);
        if (!program) {
            throw new Error('Failed to create composite component type');
        }
        const linkedProgram = program.link();
        if (!linkedProgram) {
            throw new Error('Failed to link program');
        }
        console.log('Program created and linked successfully');

        // Try to get the SPIRV code
        console.log('\nTrying to generate SPIRV code:');
        const spirvBinary = linkedProgram.getTargetCodeBlob(0); // 0 is the target index
        if (!spirvBinary) {
            throw new Error('Could not generate SPIRV binary');
        }
        console.log('SPIRV binary generated successfully');
        console.log('Generated binary length:', spirvBinary.length);

        const layout = linkedProgram.getLayout(0);
        if (!layout) {
            throw new Error('Could not get program layout');
        }
        const defaultValueType = layout.findTypeByName('WasmDefaultValueSmoke');
        if (!defaultValueType) {
            throw new Error('Could not find default-value smoke type');
        }
        const answerVar = layout.findVarByNameInType(defaultValueType, 'Answer');
        if (!answerVar) {
            throw new Error('Could not find default-value smoke variable');
        }
        if (!answerVar.hasDefaultValue()) {
            throw new Error('Default-value smoke variable did not report a default value');
        }
        const defaultValueBlob = answerVar.getDefaultValueBlob();
        if (!(defaultValueBlob instanceof Uint8Array)) {
            throw new Error('Default-value blob was not returned as a Uint8Array');
        }
        if (defaultValueBlob.length !== 4) {
            throw new Error(`Unexpected default-value blob size: ${defaultValueBlob.length}`);
        }
        const answer = new DataView(
            defaultValueBlob.buffer,
            defaultValueBlob.byteOffset,
            defaultValueBlob.byteLength).getInt32(0, true);
        if (answer !== 0x12345678) {
            throw new Error(`Unexpected default-value blob contents: 0x${answer.toString(16)}`);
        }
        console.log('Default-value blob reflection smoke test passed');

        const coreSource = globalSession.getBuiltinModuleSource('core');
        for (const marker of ['public module core;', 'struct RayDesc', 'struct NullDifferential']) {
            if (!coreSource.includes(marker)) {
                throw new Error(`Core builtin module source is missing: ${marker}`);
            }
        }
        const glslSource = globalSession.getBuiltinModuleSource('glsl');
        if (!glslSource.includes('#define highp')) {
            throw new Error('GLSL builtin module source is missing: #define highp');
        }
        // Prefix-matching contract: names beginning with core/glsl select that module.
        if (globalSession.getBuiltinModuleSource('core-extra') !== coreSource) {
            throw new Error('Prefix-matched "core-extra" should return the same source as "core"');
        }
        if (globalSession.getBuiltinModuleSource('glsl-450') !== glslSource) {
            throw new Error('Prefix-matched "glsl-450" should return the same source as "glsl"');
        }
        if (globalSession.getBuiltinModuleSource('nonexistent') !== '') {
            throw new Error('Unknown builtin module name should return an empty string');
        }
        if (globalSession.getBuiltinModuleSource('') !== '') {
            throw new Error('Empty builtin module name should return an empty string');
        }
        console.log('Builtin module source smoke test passed');

        // Session options: a SessionDesc that mirrors slang::SessionDesc. Macros, search paths
        // and compiler option entries apply to every module of the session, including one
        // imported from a search path. Enum values are those in include/slang.h.
        const CompilerOptionValueKind = { Int: 0, String: 1 };
        const CompilerOptionName = {
            MacroDefine: 0, IgnoreCapabilities: 11, RestrictiveCapabilityCheck: 12, Capability: 39,
            LineDirectiveMode: 45,
        };
        module.FS.mkdirTree('/session-options/lib');
        module.FS.writeFile('/session-options/lib/helper.slang',
            'module helper;\n' +
            '#if HELPER_VALUE != 7\n#error "HELPER_VALUE did not reach the imported module"\n#endif\n' +
            'public uint helperValue() { return HELPER_VALUE; }\n');
        const wgslTarget = targets.find(target => target.name === 'WGSL').value;
        const glslTarget = targets.find(target => target.name === 'GLSL').value;
        const optionsSession = globalSession.createSession({
            targets: [{ format: wgslTarget }],
            preprocessorMacros: [{ name: 'HELPER_VALUE', value: '7' }, { name: 'MAIN_VALUE', value: '5' }],
            searchPaths: ['/session-options/lib'],
            compilerOptionEntries: [
                // MacroDefine: stringValue0 = name, stringValue1 = value.
                {
                    name: CompilerOptionName.MacroDefine,
                    value: { kind: CompilerOptionValueKind.String, stringValue0: 'OPTION_VALUE', stringValue1: '3' },
                },
                {
                    name: CompilerOptionName.IgnoreCapabilities,
                    value: { kind: CompilerOptionValueKind.Int, intValue0: 1 },
                },
            ],
        });
        if (!optionsSession) {
            const error = module.getLastError();
            throw new Error(`Failed to create a session from a SessionDesc: ${error ? error.message : ''}`);
        }
        const optionsModule = optionsSession.loadModuleFromSource(
            'import helper;\n' +
            '#if HELPER_VALUE != 7 || MAIN_VALUE != 5 || OPTION_VALUE != 3\n' +
            '#error "a session macro value was lost"\n' +
            '#endif\n' +
            '[require(spirv)] uint spirvOnly() { return 1; }\n' +
            'RWStructuredBuffer<uint> outputBuffer;\n' +
            '[shader("compute")] [numthreads(1, 1, 1)]\n' +
            'void computeMain() { outputBuffer[0] = helperValue() * 1000 + MAIN_VALUE * 10 + OPTION_VALUE + spirvOnly(); }\n',
            'session-options', '/session-options/main.slang');
        if (!optionsModule) {
            const error = module.getLastError();
            throw new Error(`Session options: failed to load module: ${error ? error.message : 'Unknown error'}`);
        }
        // The module only compiles if all three macros reached it with their values, `helper`
        // was found, and IgnoreCapabilities applied (`spirvOnly` is not available on WGSL
        // without it).
        const optionsProgram = optionsSession.createCompositeComponentType([optionsModule]).link();
        const wgsl = optionsProgram ? optionsProgram.getTargetCode(0) : '';
        if (!wgsl || !wgsl.includes('@compute')) {
            const error = module.getLastError();
            throw new Error(`Session options: no WGSL generated: ${error ? error.message : ''}`);
        }

        // Several targets, each with its own TargetDesc fields or compiler option entries, and
        // the session's matrix layout. GLSL shows the profile as `#version`, the line directive
        // mode as the presence of `#line`, and the layout as a default qualifier (Slang's
        // column-major layout, 2, is GLSL's `row_major`; the default, row-major, is
        // `column_major`).
        const shaderSource = 'RWStructuredBuffer<uint> outputBuffer;\n' +
            '[shader("compute")] [numthreads(1, 1, 1)]\n' +
            'void computeMain() { outputBuffer[0] = 1; }\n';
        // Returns the code for each target, or throws with getLastError()'s message when the
        // session, the module, the link or a target's code fails.
        let codeModuleCount = 0;
        const targetCodes = (desc, targetCount, source = shaderSource) => {
            const lastErrorMessage = () => {
                const error = module.getLastError();
                return error ? error.message : '';
            };
            const codeSession = globalSession.createSession(desc);
            if (!codeSession) {
                throw new Error(`Failed to create a session from ${JSON.stringify(desc)}: ${lastErrorMessage()}`);
            }
            const codes = [];
            const codeModule = codeSession.loadModuleFromSource(
                source, `target-options-${codeModuleCount}`, `/target-options-${codeModuleCount}.slang`);
            codeModuleCount++;
            let codeComposite = null;
            try {
                if (!codeModule) {
                    throw new Error(`Failed to load the module for ${JSON.stringify(desc)}: ${lastErrorMessage()}`);
                }
                codeComposite = codeSession.createCompositeComponentType([codeModule]);
                const codeProgram = codeComposite.link();
                if (!codeProgram) {
                    throw new Error(`Failed to link for ${JSON.stringify(desc)}: ${lastErrorMessage()}`);
                }
                for (let i = 0; i < targetCount; i++) {
                    const code = codeProgram.getTargetCode(i);
                    if (!code) {
                        const message = lastErrorMessage();
                        codeProgram.delete();
                        throw new Error(`No code for target ${i} of ${JSON.stringify(desc)}: ${message}`);
                    }
                    codes.push(code);
                }
                codeProgram.delete();
            } finally {
                if (codeComposite) codeComposite.delete();
                if (codeModule) codeModule.delete();
                codeSession.delete();
            }
            return codes;
        };
        const [defaultGlsl] = targetCodes(glslTarget, 1);
        if (!defaultGlsl.includes('#version 450') ||
            !defaultGlsl.includes('layout(column_major) uniform;') ||
            !defaultGlsl.includes('#line')) {
            throw new Error(`Unexpected default GLSL:\n${defaultGlsl.slice(0, 300)}`);
        }
        const glsl460 = globalSession.findProfile('glsl_460');
        if (!glsl460) {
            throw new Error('findProfile("glsl_460") should return a profile');
        }
        if (globalSession.findProfile('not_a_profile') !== 0) {
            throw new Error('findProfile should return 0 (SLANG_PROFILE_UNKNOWN) for an unknown name');
        }
        const SLANG_LINE_DIRECTIVE_MODE_NONE = 1;
        const [multiTargetWgsl, configuredGlsl, optionGlsl] = targetCodes({
            targets: [
                { format: wgslTarget },
                { format: glslTarget, profile: glsl460, lineDirectiveMode: SLANG_LINE_DIRECTIVE_MODE_NONE },
                {
                    format: glslTarget,
                    compilerOptionEntries: [{
                        name: CompilerOptionName.LineDirectiveMode,
                        value: { kind: CompilerOptionValueKind.Int, intValue0: SLANG_LINE_DIRECTIVE_MODE_NONE },
                    }],
                },
            ],
            defaultMatrixLayoutMode: 2,
        }, 3);
        if (!multiTargetWgsl.includes('@compute')) {
            throw new Error(`Target 0 should be WGSL:\n${multiTargetWgsl.slice(0, 300)}`);
        }
        if (!configuredGlsl.includes('#version 460') ||
            !configuredGlsl.includes('layout(row_major) uniform;') ||
            configuredGlsl.includes('#line')) {
            throw new Error(`profile, lineDirectiveMode or defaultMatrixLayoutMode did not apply:\n${configuredGlsl.slice(0, 300)}`);
        }
        if (!optionGlsl.includes('#version 450') || optionGlsl.includes('#line')) {
            throw new Error(`A target's compilerOptionEntries did not apply to that target:\n${optionGlsl.slice(0, 300)}`);
        }

        // An HLSL target without a profile gets sm_6_6, and an explicit profile replaces it.
        // RestrictiveCapabilityCheck makes code above the target's profile an error instead of
        // an implicit upgrade: ResourceDescriptorHeap needs SM 6.6 and QuadAny needs SM 6.7.
        const hlslTarget = targets.find(target => target.name === 'HLSL').value;
        const restrictive = [{
            name: CompilerOptionName.RestrictiveCapabilityCheck,
            value: { kind: CompilerOptionValueKind.Int, intValue0: 1 },
        }];
        const heapSource = 'RWStructuredBuffer<uint> outputBuffer;\n' +
            '[shader("compute")] [numthreads(1, 1, 1)]\n' +
            'void computeMain() { RWStructuredBuffer<uint> b = ResourceDescriptorHeap[0]; outputBuffer[0] = b[0]; }\n';
        const quadAnySource = 'RWStructuredBuffer<uint> outputBuffer;\n' +
            '[shader("compute")] [numthreads(4, 1, 1)]\n' +
            'void computeMain(uint i : SV_GroupIndex) { outputBuffer[0] = QuadAny(i == 0) ? 1 : 0; }\n';
        // Returns null if `source` compiles for `target`, or else the error message.
        const hlslError = (target, source) => {
            try {
                targetCodes({ targets: [target], compilerOptionEntries: restrictive }, 1, source);
                return null;
            } catch (error) {
                return error.message;
            }
        };
        const defaultHeapError = hlslError({ format: hlslTarget }, heapSource);
        if (defaultHeapError) {
            throw new Error(`An HLSL target without a profile should compile SM 6.6 code: ${defaultHeapError}`);
        }
        const defaultQuadAnyError = hlslError({ format: hlslTarget }, quadAnySource);
        if (!defaultQuadAnyError || !defaultQuadAnyError.includes("Missing capabilities are: 'sm_6_7'")) {
            throw new Error(`An HLSL target without a profile should be sm_6_6, so SM 6.7 code should fail: ${defaultQuadAnyError}`);
        }
        // `profile: null` counts as leaving the profile out, so the target is sm_6_6 too.
        const nullProfileQuadAnyError = hlslError({ format: hlslTarget, profile: null }, quadAnySource);
        if (!nullProfileQuadAnyError || !nullProfileQuadAnyError.includes("Missing capabilities are: 'sm_6_7'")) {
            throw new Error(`An HLSL target with profile: null should be sm_6_6: ${nullProfileQuadAnyError}`);
        }
        const sm50HeapError = hlslError({ format: hlslTarget, profile: globalSession.findProfile('sm_5_0') }, heapSource);
        if (!sm50HeapError || !sm50HeapError.includes('E41013')) {
            throw new Error(`An explicit sm_5_0 profile should replace the HLSL default: ${sm50HeapError}`);
        }

        // A macro without a `value` is defined (as empty), as with `-D NAME`.
        targetCodes({ targets: [{ format: wgslTarget }], preprocessorMacros: [{ name: 'FLAG' }] }, 1,
            '#ifndef FLAG\n#error "FLAG is not defined"\n#endif\n' + shaderSource);

        // forceGLSLScalarBufferLayout gives a float3 structured buffer the scalar layout.
        const float3Source = 'RWStructuredBuffer<float3> outputBuffer;\n' +
            '[shader("compute")] [numthreads(1, 1, 1)]\n' +
            'void computeMain() { outputBuffer[0] = float3(1, 2, 3); }\n';
        const [scalarGlsl, defaultLayoutGlsl] = targetCodes({
            targets: [
                { format: glslTarget, forceGLSLScalarBufferLayout: true },
                { format: glslTarget },
            ],
        }, 2, float3Source);
        if (!scalarGlsl.includes('GL_EXT_scalar_block_layout') || !scalarGlsl.includes('layout(scalar')) {
            throw new Error(`forceGLSLScalarBufferLayout did not apply:\n${scalarGlsl.slice(0, 400)}`);
        }
        if (defaultLayoutGlsl.includes('layout(scalar')) {
            throw new Error(`A target without forceGLSLScalarBufferLayout should not use the scalar layout:\n${defaultLayoutGlsl.slice(0, 400)}`);
        }

        // A malformed descriptor returns null and sets a USER error naming the problem.
        const expectInvalid = (desc, messagePart) => {
            const badSession = globalSession.createSession(desc);
            if (badSession) {
                badSession.delete();
                throw new Error(`createSession(${JSON.stringify(desc)}) should fail`);
            }
            const error = module.getLastError();
            if (!error || error.type !== 'USER' || error.result === module.SLANG_OK ||
                !error.message.includes(messagePart)) {
                throw new Error(`createSession(${JSON.stringify(desc)}) should set a USER error about ${messagePart}`);
            }
        };
        const ignoreCapabilities = { kind: CompilerOptionValueKind.Int, intValue0: 1 };
        expectInvalid('WGSL', 'expected a compile target number or a SessionDesc object');
        expectInvalid(null, 'expected a compile target number or a SessionDesc object');
        expectInvalid(undefined, 'expected a compile target number or a SessionDesc object');
        expectInvalid([{ format: wgslTarget }], 'expected a compile target number or a SessionDesc object');
        expectInvalid(100000, 'the compile target number is not a SlangCompileTarget');
        expectInvalid(1.5, 'the compile target number is not a SlangCompileTarget');
        expectInvalid(NaN, 'the compile target number is not a SlangCompileTarget');
        expectInvalid({ targets: wgslTarget }, "'targets' must be an array");
        expectInvalid({ targets: [{ format: NaN }] }, "'targets[0].format' must be an integer in the int32 range");
        expectInvalid({ targets: [{ format: 5.9 }] }, "'targets[0].format' must be an integer in the int32 range");
        expectInvalid({ compilerOptionEntries: [{ name: 2 ** 40, value: ignoreCapabilities }] },
            "'compilerOptionEntries[0].name' must be an integer in the int32 range");
        for (const atom of [-1, 0x7fffffff]) {
            expectInvalid({ compilerOptionEntries: [{ name: CompilerOptionName.Capability, value: { kind: 0, intValue0: atom } }] },
                "'compilerOptionEntries[0].value.intValue0' is not a capability atom");
        }
        expectInvalid({ targets: [wgslTarget] }, "'targets[0]' must be an object");
        expectInvalid({ targets: [{ profile: glsl460 }] }, "'targets[0].format' is required");
        expectInvalid({ targets: [{ format: 'WGSL' }] }, "'targets[0].format' must be a number");
        expectInvalid({ targets: [{ format: 100000 }] }, "'targets[0].format' is not a SlangCompileTarget");
        expectInvalid({ targets: [{ format: glslTarget, forceGLSLScalarBufferLayout: 1 }] },
            "'targets[0].forceGLSLScalarBufferLayout' must be a boolean");
        expectInvalid({ searchPaths: '/session-options/lib' }, "'searchPaths' must be an array");
        expectInvalid({ searchPaths: [7] }, "'searchPaths[0]' must be a string");
        expectInvalid({ preprocessorMacros: { name: 'A' } }, "'preprocessorMacros' must be an array");
        expectInvalid({ preprocessorMacros: [{ value: '1' }] }, "'preprocessorMacros[0].name' is required");
        expectInvalid({ preprocessorMacros: [{ name: 'A', value: 1 }] }, "'preprocessorMacros[0].value' must be a string");
        expectInvalid({ compilerOptionEntries: {} }, "'compilerOptionEntries' must be an array");
        expectInvalid({ compilerOptionEntries: [{ value: ignoreCapabilities }] },
            "'compilerOptionEntries[0].name' is required");
        expectInvalid({ compilerOptionEntries: [{ name: CompilerOptionName.IgnoreCapabilities }] },
            "'compilerOptionEntries[0].value' is required");
        expectInvalid({ compilerOptionEntries: [{ name: CompilerOptionName.IgnoreCapabilities, value: { intValue0: 1 } }] },
            "'compilerOptionEntries[0].value.kind' is required");
        expectInvalid({ compilerOptionEntries: [{ name: 100000, value: ignoreCapabilities }] },
            "'compilerOptionEntries[0].name' is outside the CompilerOptionName range");
        expectInvalid({ compilerOptionEntries: [{ name: -1, value: ignoreCapabilities }] },
            "'compilerOptionEntries[0].name' is outside the CompilerOptionName range");
        expectInvalid({ compilerOptionEntries: [{ name: CompilerOptionName.IgnoreCapabilities, value: { kind: 2 } }] },
            "'compilerOptionEntries[0].value.kind' must be 0 (Int) or 1 (String)");
        expectInvalid({ compilerOptionEntries: [{ name: CompilerOptionName.IgnoreCapabilities, value: { kind: 0, intValue0: '1' } }] },
            "'compilerOptionEntries[0].value.intValue0' must be a number");
        expectInvalid({
            targets: [{ format: wgslTarget }, { format: glslTarget, compilerOptionEntries: [{ value: { kind: 0 } }] }],
        }, "'targets[1].compilerOptionEntries[0].name' is required");
        optionsProgram.delete();
        optionsModule.delete();
        optionsSession.delete();
        console.log('Session options smoke test passed');

        // Clean up
        linkedProgram.delete();
        program.delete();
        entryPoint.delete();
        module1.delete();
        session.delete();
        globalSession.delete();
        console.log('Smoke test completed successfully');
        process.exit(0); // Explicit success exit code
    } catch (error) {
        console.error('Smoke test failed:', error);
        process.exit(1); // Error exit code
    }
}

runSmokeTest();
