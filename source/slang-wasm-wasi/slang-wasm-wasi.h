// slang-wasm-wasi.h — flat C ABI for the slang-wasm-wasi WASI reactor.
//
// All pointers are 32-bit offsets into the module's linear memory. All strings
// are UTF-8 byte ranges passed as (ptr, len) pairs. All objects are opaque
// uint32_t handles backed by an internal table; callers never hold raw COM
// pointers and lifetimes are explicit.
//
// Ownership rules:
//   - Memory returned via slang_wasm_alloc is owned by the caller; free with
//     slang_wasm_free.
//   - Result handles are owned by the caller; free with slang_wasm_result_destroy.
//   - Session handles are owned by the caller; free with slang_wasm_session_destroy.
//   - Pointers returned by the result accessor functions (code_ptr, etc.) are
//     valid until slang_wasm_result_destroy is called on that result handle.

#ifndef SLANG_WASM_WASI_H
#define SLANG_WASM_WASI_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // ── Memory helpers ────────────────────────────────────────────────────────────

    // Allocate `size` bytes in the module's linear memory. Returns NULL on failure.
    // Exposed so the host can stage UTF-8 input buffers before calling exports.
    void* slang_wasm_alloc(uint32_t size);

    // Free a pointer previously returned by slang_wasm_alloc.
    void slang_wasm_free(void* ptr);

    // ── Enum metadata ─────────────────────────────────────────────────────────────

    // Pointer and byte length of a static JSON blob of the form
    // `{ "Target": {"SPIRV":6,...}, "Stage": {...}, ... }`, generated from
    // include/slang.h by tools/generate-slang-bindings.py. Lets a runtime binding
    // (Go, Python, …) resolve enum integer values without hardcoding them. Static
    // storage in the module; no free required.
    uint32_t slang_wasm_enum_metadata_ptr(void);
    uint32_t slang_wasm_enum_metadata_len(void);

    // Resolve a SlangCompileTarget by its lowercase string form (e.g. "spirv",
    // "hlsl", "glsl", "wgsl"). Returns the integer enum value, or -1 if unrecognised.
    int32_t slang_wasm_target_from_string(const char* name, uint32_t nameLen);

    // Resolve a SlangStage by its lowercase string form (e.g. "vertex", "fragment",
    // "compute"). Returns the integer enum value, or -1 if unrecognised.
    int32_t slang_wasm_stage_from_string(const char* name, uint32_t nameLen);

    // ── Session descriptor builders ───────────────────────────────────────────────
    //
    // Each builder accumulates entries for one piece of slang::SessionDesc and is
    // consumed by slang_wasm_session_create2. A handle may be 0 (omitted) to leave
    // the corresponding SessionDesc field at its default. Builders are owned by the
    // caller and must be destroyed (or are consumed exactly once by
    // slang_wasm_session_create2, which destroys them internally on return).

    typedef uint32_t SlangWasmTargetList;

    // Create an empty list of compile targets.
    SlangWasmTargetList slang_wasm_target_list_create(void);

    // Append one target: `format` is a SlangCompileTarget value, `profile` may be
    // NULL/empty for the target default, `flags` is a SlangTargetFlags bitmask.
    void slang_wasm_target_list_add(
        SlangWasmTargetList list,
        uint32_t format,
        const char* profile,
        uint32_t profileLen,
        uint32_t flags);

    void slang_wasm_target_list_destroy(SlangWasmTargetList list);

    typedef uint32_t SlangWasmMacroList;

    // Create an empty list of preprocessor macro definitions.
    SlangWasmMacroList slang_wasm_macro_list_create(void);

    // Append one `#define name value` (value may be empty for a valueless define).
    void slang_wasm_macro_list_add(
        SlangWasmMacroList list,
        const char* name,
        uint32_t nameLen,
        const char* value,
        uint32_t valueLen);

    void slang_wasm_macro_list_destroy(SlangWasmMacroList list);

    typedef uint32_t SlangWasmPathList;

    // Create an empty list of module search paths.
    SlangWasmPathList slang_wasm_path_list_create(void);

    void slang_wasm_path_list_add(SlangWasmPathList list, const char* path, uint32_t pathLen);

    void slang_wasm_path_list_destroy(SlangWasmPathList list);

    typedef uint32_t SlangWasmOptions;

    // Create an empty list of session-wide compiler option entries
    // (slang::CompilerOptionEntry), keyed by SlangCompilerOptionName.
    SlangWasmOptions slang_wasm_options_create(void);

    void slang_wasm_options_add_string(
        SlangWasmOptions opts,
        uint32_t name,
        const char* val,
        uint32_t valLen);

    void slang_wasm_options_add_int(SlangWasmOptions opts, uint32_t name, int32_t val);

    void slang_wasm_options_destroy(SlangWasmOptions opts);

    // ── Session ───────────────────────────────────────────────────────────────────

    typedef uint32_t SlangWasmSession;

    // Create a compile session configured for one target format. `targetFormat` is
    // a SlangCompileTarget enum value (e.g. SLANG_SPIRV). `profile` may be NULL or
    // empty to accept the target's default profile. Returns 0 on failure.
    SlangWasmSession slang_wasm_session_create(
        uint32_t targetFormat,
        const char* profile,
        uint32_t profileLen);

    // Create a compile session from the full SessionDesc surface: one or more
    // compile targets, preprocessor macros, module search paths, and session-wide
    // compiler options. Any handle may be 0 to leave that part of the descriptor at
    // its default (e.g. 0 targets is invalid and fails; 0 macros means none).
    // Consumes (destroys) all four builder handles before returning, success or not.
    // Returns 0 on failure.
    SlangWasmSession slang_wasm_session_create2(
        SlangWasmTargetList targets,
        SlangWasmMacroList macros,
        SlangWasmPathList searchPaths,
        SlangWasmOptions sessionOptions);

    // Release a session and all associated resources. The handle must not be used
    // after this call.
    void slang_wasm_session_destroy(SlangWasmSession session);

    // ── Modules ───────────────────────────────────────────────────────────────────
    //
    // A module is parsed once and can then be queried for its defined entry points
    // and compiled from independently of any other module loaded into the same
    // session, unlike slang_wasm_compile which loads, compiles, and discards a
    // module in one call.

    typedef uint32_t SlangWasmModule;

    // Forward declaration: SlangWasmResult is defined fully in the "Compilation"
    // section below, but slang_wasm_module_serialize (a module-handle operation)
    // needs the type here too.
    typedef uint32_t SlangWasmResult;

    // Load `source` as module `name` into `session`. Returns 0 on failure. `diagPtrOut`/
    // `diagLenOut` (each a pointer into the module's own linear memory, e.g. from
    // slang_wasm_alloc) are always written — to a NULL/0 buffer if there were no
    // diagnostics, or to a freshly heap-allocated UTF-8 buffer (owned by the
    // caller; free with slang_wasm_free) otherwise. This is true on both success
    // (warnings) and failure (errors).
    SlangWasmModule slang_wasm_session_load_module(
        SlangWasmSession session,
        const char* name,
        uint32_t nameLen,
        const char* source,
        uint32_t sourceLen,
        uint32_t* diagPtrOut,
        uint32_t* diagLenOut);

    // Release a module handle. The handle must not be used after this call.
    void slang_wasm_module_destroy(SlangWasmModule module);

    // Number of entry points defined in the module (functions marked
    // `[shader("...")]`). Returns 0 for an unknown module handle.
    uint32_t slang_wasm_module_entry_point_count(SlangWasmModule module);

    // Pointer and byte length of the name of the entry point at `index`
    // (0 <= index < slang_wasm_module_entry_point_count(module)). Valid until
    // slang_wasm_module_destroy. Returns 0 for an unknown module handle or an
    // out-of-range index.
    uint32_t slang_wasm_module_entry_point_name_ptr(SlangWasmModule module, uint32_t index);
    uint32_t slang_wasm_module_entry_point_name_len(SlangWasmModule module, uint32_t index);

    // Serialise `module`'s checked IR to a precompiled binary blob,
    // so it can be cached and reloaded later via slang_wasm_session_load_module_ir
    // without re-parsing or re-checking the original source. On success, the
    // result's code_ptr/code_len hold the IR bytes; reflection_json/diagnostics are
    // unused. Never throws.
    SlangWasmResult slang_wasm_module_serialize(SlangWasmModule module);

    // Load a precompiled IR blob (as produced by slang_wasm_module_serialize) back
    // as a module. Returns 0 on failure. `diagPtrOut`/`diagLenOut` behave exactly
    // as in slang_wasm_session_load_module.
    SlangWasmModule slang_wasm_session_load_module_ir(
        SlangWasmSession session,
        const char* name,
        uint32_t nameLen,
        const void* irBlob,
        uint32_t irLen,
        uint32_t* diagPtrOut,
        uint32_t* diagLenOut);

    // ── Type conformance ──────────────────────────────────────────────────────────
    //
    // Explicitly control which concrete types conform to an interface for a
    // compile, and read back the dispatch ID Slang assigns each one (for tagging
    // runtime instance data). A 0/empty handle passed to the compile functions
    // below means "no explicit list": Slang then only finds types it can discover
    // by static analysis (e.g. in-shader branching) — for an interface consumed
    // only through a host-bound resource (e.g. `ParameterBlock<IMaterial>`),
    // that's nothing, and the compile fails until at least one conformance is added.

    typedef uint32_t SlangWasmTypeConformances;

    // Create an empty list of type conformances bound to `module`. Unlike
    // SlangWasmSpecArgs, resolution happens immediately in
    // slang_wasm_type_conformances_add, not deferred to compile time. Returns 0
    // for an unknown module handle.
    SlangWasmTypeConformances slang_wasm_type_conformances_create(SlangWasmModule module);

    // Register that `concreteTypeName` conforms to `interfaceTypeName`, resolved
    // against `module`'s layout. `conformanceIdOverride` may be -1 to auto-assign
    // the dispatch ID, or a non-negative value to pin it. Returns the assigned
    // dispatch ID, or -1 if either name is unresolvable or the type doesn't conform.
    // `diagPtrOut`/`diagLenOut` receive a (ptr, len) diagnostic buffer, as with
    // slang_wasm_session_load_module — (0, 0) if there is nothing to report, and
    // owned by the caller (free with slang_wasm_free) otherwise.
    int32_t slang_wasm_type_conformances_add(
        SlangWasmTypeConformances conformances,
        const char* concreteTypeName,
        uint32_t concreteTypeNameLen,
        const char* interfaceTypeName,
        uint32_t interfaceTypeNameLen,
        int32_t conformanceIdOverride,
        uint32_t* diagPtrOut,
        uint32_t* diagLenOut);

    void slang_wasm_type_conformances_destroy(SlangWasmTypeConformances conformances);

    // ── Specialization ────────────────────────────────────────────────────────────
    //
    // Specialize a generic shader for concrete types/values before compiling.

    typedef uint32_t SlangWasmSpecArgs;

    // Create an empty list of specialization arguments.
    SlangWasmSpecArgs slang_wasm_spec_args_create(void);

    // Append a type argument (e.g. "PbrMaterial" for a `Renderer<T : IMaterial>`
    // generic parameter). Resolved by name against the program's own layout at
    // compile time, so the type must be visible from the module being compiled.
    void slang_wasm_spec_args_add_type(
        SlangWasmSpecArgs args,
        const char* typeName,
        uint32_t typeNameLen);

    // Append a constant-expression argument (e.g. "4" for a generic value parameter).
    void slang_wasm_spec_args_add_expr(SlangWasmSpecArgs args, const char* expr, uint32_t exprLen);

    void slang_wasm_spec_args_destroy(SlangWasmSpecArgs args);

    // Find entry point `entryName` in `module`, specialize it with `args` (in
    // argument-list order, matching the generic parameter declaration order), then
    // link and compile for the target at `targetIndex`, using the session `module`
    // was loaded into (see slang_wasm_session_load_module). `typeConformances` may
    // be 0 or a handle from slang_wasm_type_conformances_add (see "Type
    // conformance" above); unlike `args`, it is not consumed and may be reused
    // across compiles. Consumes (destroys) `args` before returning, success or
    // not. Never throws: internal aborts are caught and returned as a failed
    // result with diagnostics text.
    SlangWasmResult slang_wasm_compile_specialized_entry_point(
        SlangWasmModule module,
        const char* entryName,
        uint32_t entryNameLen,
        SlangWasmSpecArgs args,
        uint32_t targetIndex,
        SlangWasmTypeConformances typeConformances);

    // ── Declaration reflection ────────────────────────────────────────────────────

    // Serialise `module`'s module-level declaration tree: every struct, function,
    // variable, enum, namespace, and generic declared at module scope, recursively
    // — without compiling to any target. On success, the result's reflection_json
    // (see the result accessors below) holds the JSON tree; code/diagnostics are
    // unused on success. Never throws.
    SlangWasmResult slang_wasm_module_decl_reflection_json(SlangWasmModule module);

    // Disassemble `module`'s checked IR to human-readable text. On success, the
    // result's diagnostics_ptr/len (see the result accessors below) hold the
    // disassembly text — reusing that field for "the text I asked for" rather
    // than adding a fifth WasmResult field for one caller; code/reflection_json
    // are unused. Never throws.
    SlangWasmResult slang_wasm_module_disassemble(SlangWasmModule module);

    // ── Compilation ───────────────────────────────────────────────────────────────

    // Compile `source` as module `moduleName`, find entry point `entryName`, link,
    // and produce code for the target at `targetIndex` (its position in the
    // SlangWasmTargetList the session was created with; 0 for a single-target
    // session) plus reflection JSON. Never propagates a C++ exception across the
    // boundary: internal aborts are caught and returned as a failed result with
    // diagnostics text, leaving the session alive and reusable.
    // Returns 0 if a result object could not be allocated at all.
    SlangWasmResult slang_wasm_compile(
        SlangWasmSession session,
        const char* moduleName,
        uint32_t moduleNameLen,
        const char* source,
        uint32_t sourceLen,
        const char* entryName,
        uint32_t entryNameLen,
        uint32_t targetIndex);

    // Compile entry point `entryName` from an already-loaded `module` (see
    // slang_wasm_session_load_module), producing code for the target at
    // `targetIndex`, using the session `module` was loaded into. `typeConformances`
    // may be 0 or a handle from slang_wasm_type_conformances_add (see "Type
    // conformance" above); not consumed, so may be reused across compiles.
    // Equivalent to slang_wasm_compile but reuses a module already parsed once, so
    // multiple entry points from the same source can be compiled independently
    // without re-parsing. Same never-throws contract as slang_wasm_compile.
    SlangWasmResult slang_wasm_compile_entry_point(
        SlangWasmModule module,
        const char* entryName,
        uint32_t entryNameLen,
        uint32_t targetIndex,
        SlangWasmTypeConformances typeConformances);

    // Compile all of `module`'s defined entry points together into one combined
    // code blob for the target at `targetIndex`, using the session `module` was
    // loaded into. `typeConformances` behaves exactly as in
    // slang_wasm_compile_entry_point. Unlike compiling a single entry point, this
    // returns one blob containing every entry point linked into the component,
    // e.g. one SPIR-V module with both a vertex and a fragment entry point.
    // Reflection JSON in the result covers the same combined layout. Same
    // never-throws contract as slang_wasm_compile.
    SlangWasmResult slang_wasm_compile_module(
        SlangWasmModule module,
        uint32_t targetIndex,
        SlangWasmTypeConformances typeConformances);

    // ── Result accessors ──────────────────────────────────────────────────────────
    //
    // Every accessor below returns a 0/empty sentinel for an unknown result
    // handle, rather than trapping the instance.

    // Returns 1 if compilation succeeded, 0 otherwise (including for an
    // unknown handle).
    int32_t slang_wasm_result_succeeded(SlangWasmResult result);

    // Pointer and byte length of the compiled target code blob (e.g. SPIR-V words).
    // Valid until slang_wasm_result_destroy.
    uint32_t slang_wasm_result_code_ptr(SlangWasmResult result);
    uint32_t slang_wasm_result_code_len(SlangWasmResult result);

    // Pointer and byte length of the reflection JSON string (null-terminated).
    // Valid until slang_wasm_result_destroy.
    uint32_t slang_wasm_result_reflection_json_ptr(SlangWasmResult result);
    uint32_t slang_wasm_result_reflection_json_len(SlangWasmResult result);

    // Pointer and byte length of concatenated diagnostic messages (null-terminated).
    // Present on both success (warnings) and failure (errors). Valid until destroy.
    uint32_t slang_wasm_result_diagnostics_ptr(SlangWasmResult result);
    uint32_t slang_wasm_result_diagnostics_len(SlangWasmResult result);

    // Release a result handle and all associated output buffers.
    void slang_wasm_result_destroy(SlangWasmResult result);

    // ── Diagnostics ───────────────────────────────────────────────────────────────

    // Return the Slang build tag string (e.g. "v2025.21") as a null-terminated
    // C string in static storage. Used by the host to verify it is talking to the
    // expected build.
    const char* slang_wasm_version(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SLANG_WASM_WASI_H
