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

        // Session options: macros, search paths and compiler options apply to every module of
        // the session, including one imported from a search path.
        module.FS.mkdirTree('/session-options/lib');
        module.FS.writeFile('/session-options/lib/helper.slang',
            'module helper;\n' +
            '#if HELPER_VALUE != 7\n#error "HELPER_VALUE did not reach the imported module"\n#endif\n' +
            'public uint helperValue() { return HELPER_VALUE; }\n');
        const wgslTarget = targets.find(target => target.name === 'WGSL').value;
        const optionsSession = globalSession.createSession({
            target: wgslTarget,
            preprocessorMacros: [{ name: 'HELPER_VALUE', value: '7' }, { name: 'MAIN_VALUE', value: '5' }],
            searchPaths: ['/session-options/lib'],
            compilerOptions: [
                // slang::CompilerOptionName::MacroDefine (0): stringValue0 = name, stringValue1 = value.
                { name: 0, stringValue0: 'OPTION_VALUE', stringValue1: '3' },
                // slang::CompilerOptionName::IgnoreCapabilities (11).
                { name: 11, intValue0: 1 },
            ],
        });
        if (!optionsSession) {
            throw new Error('Failed to create a session from a descriptor');
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
        // A valid profile and the matrix layout reach the target code. GLSL shows both: the
        // profile as `#version`, and the layout as a default qualifier (Slang's column-major
        // layout, 2, is GLSL's `row_major`; the default, row-major, is `column_major`).
        const glslTarget = targets.find(target => target.name === 'GLSL').value;
        const glslShaderSource = 'RWStructuredBuffer<uint> outputBuffer;\n' +
            '[shader("compute")] [numthreads(1, 1, 1)]\n' +
            'void computeMain() { outputBuffer[0] = 1; }\n';
        const glslCode = (desc) => {
            const glslSession = globalSession.createSession(desc);
            if (!glslSession) {
                throw new Error(`Failed to create a GLSL session from ${JSON.stringify(desc)}`);
            }
            const glslModule = glslSession.loadModuleFromSource(
                glslShaderSource, 'glsl-options', '/glsl-options.slang');
            const glslProgram = glslModule ?
                glslSession.createCompositeComponentType([glslModule]).link() : null;
            const code = glslProgram ? glslProgram.getTargetCode(0) : '';
            if (glslProgram) glslProgram.delete();
            if (glslModule) glslModule.delete();
            glslSession.delete();
            return code;
        };
        const defaultGlsl = glslCode(glslTarget);
        if (!defaultGlsl.includes('#version 450') ||
            !defaultGlsl.includes('layout(column_major) uniform;')) {
            throw new Error(`Unexpected default GLSL header:\n${defaultGlsl.slice(0, 200)}`);
        }
        const configuredGlsl =
            glslCode({ target: glslTarget, profile: 'glsl_460', defaultMatrixLayoutMode: 2 });
        if (!configuredGlsl.includes('#version 460') ||
            !configuredGlsl.includes('layout(row_major) uniform;')) {
            throw new Error(`profile or defaultMatrixLayoutMode did not apply:\n${configuredGlsl.slice(0, 200)}`);
        }
        const badSession = globalSession.createSession({ profile: 'sm_6_6' });
        if (badSession) {
            throw new Error('A descriptor without a target should not create a session');
        }
        const badError = module.getLastError();
        if (!badError || badError.type !== 'USER' || badError.result === module.SLANG_OK) {
            throw new Error('A descriptor without a target should set a USER invalid-argument error');
        }
        const badProfileSession =
            globalSession.createSession({ target: wgslTarget, profile: 'not_a_profile' });
        if (badProfileSession) {
            throw new Error('An unknown profile should not create a session');
        }
        const badProfileError = module.getLastError();
        if (!badProfileError || badProfileError.type !== 'USER' ||
            badProfileError.result === module.SLANG_OK ||
            !badProfileError.message.includes('not_a_profile')) {
            throw new Error('An unknown profile should set a USER error naming the profile');
        }
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
