# slang_include_directories = [
# '../../',
# '../../external/spirv-headers/include', ]

set(SLANG_SOURCE
    slang-api.cpp
    slang-artifact-output-util.cpp
    slang-ast-base.cpp
    slang-ast-builder.cpp
    slang-ast-decl-ref.cpp
    slang-ast-decl.cpp
    slang-ast-dump.cpp
    slang-ast-modifier.cpp
    slang-ast-natural-layout.cpp
    slang-ast-print.cpp
    slang-ast-reflect.cpp
    slang-ast-support-types.cpp
    slang-ast-synthesis.cpp
    slang-ast-type.cpp
    slang-ast-val.cpp
    slang-capability.cpp
    slang-check-conformance.cpp
    slang-check-constraint.cpp
    slang-check-conversion.cpp
    slang-check-decl.cpp
    slang-check-expr.cpp
    slang-check-inheritance.cpp
    slang-check-modifier.cpp
    slang-check-overload.cpp
    slang-check-resolve-val.cpp
    slang-check-shader.cpp
    slang-check-stmt.cpp
    slang-check-type.cpp
    slang-check.cpp
    slang-compiler.cpp
    slang-container-pool.cpp
    slang-diagnostics.cpp
    slang-doc-ast.cpp
    slang-doc-markdown-writer.cpp
    slang-emit-base.cpp
    slang-emit-c-like.cpp
    slang-emit-cpp.cpp
    slang-emit-cpp.h
    slang-emit-cuda.cpp
    slang-emit-glsl.cpp
    slang-emit-hlsl.cpp
    slang-emit-precedence.cpp
    slang-emit-source-writer.cpp
    slang-emit-spirv.cpp
    slang-emit-torch.cpp
    slang-emit.cpp
    slang-glsl-extension-tracker.cpp
    slang-hlsl-to-vulkan-layout-options.cpp
    slang-intrinsic-expand.cpp
    slang-ir-addr-inst-elimination.cpp
    slang-ir-address-analysis.cpp
    slang-ir-any-value-inference.cpp
    slang-ir-any-value-marshalling.cpp
    slang-ir-augment-make-existential.cpp
    slang-ir-autodiff-cfg-norm.cpp
    slang-ir-autodiff-fwd.cpp
    slang-ir-autodiff-pairs.cpp
    slang-ir-autodiff-primal-hoist.cpp
    slang-ir-autodiff-region.cpp
    slang-ir-autodiff-rev.cpp
    slang-ir-autodiff-transcriber-base.cpp
    slang-ir-autodiff-unzip.cpp
    slang-ir-autodiff.cpp
    slang-ir-bind-existentials.cpp
    slang-ir-byte-address-legalize.cpp
    slang-ir-check-differentiability.cpp
    slang-ir-cleanup-void.cpp
    slang-ir-clone.cpp
    slang-ir-collect-global-uniforms.cpp
    slang-ir-com-interface.cpp
    slang-ir-composite-reg-to-mem.cpp
    slang-ir-constexpr.cpp
    slang-ir-dce.cpp
    slang-ir-deduplicate-generic-children.cpp
    slang-ir-deduplicate.cpp
    slang-ir-defunctionalization.cpp
    slang-ir-diff-call.cpp
    slang-ir-dll-export.cpp
    slang-ir-dll-import.cpp
    slang-ir-dominators.cpp
    slang-ir-eliminate-multilevel-break.cpp
    slang-ir-eliminate-phis.cpp
    slang-ir-entry-point-pass.cpp
    slang-ir-entry-point-raw-ptr-params.cpp
    slang-ir-entry-point-uniforms.cpp
    slang-ir-explicit-global-context.cpp
    slang-ir-explicit-global-init.cpp
    slang-ir-extract-value-from-type.cpp
    slang-ir-fuse-satcoop.cpp
    slang-ir-generics-lowering-context.cpp
    slang-ir-glsl-legalize.cpp
    slang-ir-glsl-liveness.cpp
    slang-ir-init-local-var.cpp
    slang-ir-inline.cpp
    slang-ir-layout.cpp
    slang-ir-legalize-array-return-type.cpp
    slang-ir-legalize-mesh-outputs.cpp
    slang-ir-legalize-types.cpp
    slang-ir-legalize-uniform-buffer-load.cpp
    slang-ir-legalize-varying-params.cpp
    slang-ir-legalize-vector-types.cpp
    slang-ir-link.cpp
    slang-ir-liveness.cpp
    slang-ir-loop-inversion.cpp
    slang-ir-loop-unroll.cpp
    slang-ir-lower-append-consume-structured-buffer.cpp
    slang-ir-lower-binding-query.cpp
    slang-ir-lower-bit-cast.cpp
    slang-ir-lower-buffer-element-type.cpp
    slang-ir-lower-com-methods.cpp
    slang-ir-lower-error-handling.cpp
    slang-ir-lower-existential.cpp
    slang-ir-lower-generic-call.cpp
    slang-ir-lower-generic-function.cpp
    slang-ir-lower-generic-type.cpp
    slang-ir-lower-generics.cpp
    slang-ir-lower-l-value-cast.cpp
    slang-ir-lower-optional-type.cpp
    slang-ir-lower-reinterpret.cpp
    slang-ir-lower-result-type.cpp
    slang-ir-lower-size-of.cpp
    slang-ir-lower-tuple-types.cpp
    slang-ir-lower-witness-lookup.cpp
    slang-ir-marshal-native-call.cpp
    slang-ir-metadata.cpp
    slang-ir-missing-return.cpp
    slang-ir-obfuscate-loc.cpp
    slang-ir-optix-entry-point-uniforms.cpp
    slang-ir-peephole.cpp
    slang-ir-propagate-func-properties.cpp
    slang-ir-pytorch-cpp-binding.cpp
    slang-ir-pytorch-cpp-binding.h
    slang-ir-reachability.cpp
    slang-ir-redundancy-removal.cpp
    slang-ir-remove-unused-generic-param.cpp
    slang-ir-restructure-scoping.cpp
    slang-ir-restructure.cpp
    slang-ir-sccp.cpp
    slang-ir-simplify-cfg.cpp
    slang-ir-simplify-for-emit.cpp
    slang-ir-single-return.cpp
    slang-ir-specialize-arrays.cpp
    slang-ir-specialize-buffer-load-arg.cpp
    slang-ir-specialize-dispatch.cpp
    slang-ir-specialize-dynamic-associatedtype-lookup.cpp
    slang-ir-specialize-function-call.cpp
    slang-ir-specialize-matrix-layout.cpp
    slang-ir-specialize-resources.cpp
    slang-ir-specialize-target-switch.cpp
    slang-ir-specialize.cpp
    slang-ir-spirv-legalize.cpp
    slang-ir-spirv-snippet.cpp
    slang-ir-ssa-register-allocate.cpp
    slang-ir-ssa-simplification.cpp
    slang-ir-ssa.cpp
    slang-ir-string-hash.cpp
    slang-ir-strip-cached-dict.cpp
    slang-ir-strip-witness-tables.cpp
    slang-ir-strip.cpp
    slang-ir-synthesize-active-mask.cpp
    slang-ir-use-uninitialized-out-param.cpp
    slang-ir-util.cpp
    slang-ir-validate.cpp
    slang-ir-vk-invert-y.cpp
    slang-ir-witness-table-wrapper.cpp
    slang-ir-wrap-structured-buffers.cpp
    slang-ir.cpp
    slang-language-server-ast-lookup.cpp
    slang-language-server-auto-format.cpp
    slang-language-server-completion.cpp
    slang-language-server-document-symbols.cpp
    slang-language-server-inlay-hints.cpp
    slang-language-server-semantic-tokens.cpp
    slang-language-server.cpp
    slang-legalize-types.cpp
    slang-lookup-glslstd450.cpp
    slang-lookup.cpp
    slang-lower-to-ir.cpp
    slang-mangle.cpp
    slang-mangled-lexer.cpp
    slang-module-library.cpp
    slang-options.cpp
    slang-parameter-binding.cpp
    slang-parser.cpp
    slang-preprocessor.cpp
    slang-profile.cpp
    slang-ref-object-reflect.cpp
    slang-reflection-api.cpp
    slang-repro.cpp
    slang-serialize-ast.cpp
    slang-serialize-container.cpp
    slang-serialize-factory.cpp
    slang-serialize-ir-types.cpp
    slang-serialize-ir.cpp
    slang-serialize-reflection.cpp
    slang-serialize-source-loc.cpp
    slang-serialize-types.cpp
    slang-serialize.cpp
    slang-spirv-core-grammar-embed.cpp
    slang-spirv-val.cpp
    slang-stdlib-api.cpp
    slang-stdlib-textures.cpp
    slang-stdlib.cpp
    slang-syntax.cpp
    slang-type-layout.cpp
    slang-type-system-shared.cpp
    slang-value-reflect.cpp
    slang-workspace-version.cpp
    slang.cpp)

#
# Compiling the meta.slang files
#

# List of *.meta.slang headers
set(SLANG_STDLIB_META_SOURCE core.meta.slang hlsl.meta.slang diff.meta.slang)

# Generate the output file list
list(TRANSFORM SLANG_STDLIB_META_SOURCE
     APPEND .h # REPLACE "\.meta\.slang$" ".meta.slang.h"
            OUTPUT_VARIABLE SLANG_STDLIB_META_GENERATED_HEADERS)
# Make them absolute
list(TRANSFORM SLANG_STDLIB_META_SOURCE PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/)

add_custom_command(
  OUTPUT ${SLANG_STDLIB_META_GENERATED_HEADERS}
  COMMAND slang-generate ${SLANG_STDLIB_META_SOURCE} --target-directory
          ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS ${SLANG_STDLIB_META_SOURCE}
  VERBATIM)

add_library(slang-meta-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(slang-meta-headers
               INTERFACE ${SLANG_STDLIB_META_GENERATED_HEADERS})
target_include_directories(slang-meta-headers
                           INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

#
# Slang static
#

add_library(slang-static STATIC EXCLUDE_FROM_ALL ${SLANG_SOURCE})
target_link_libraries(slang-static PRIVATE core compiler-core prelude
                                           slang-meta-headers SPIRV-Headers)
set_default_compile_options(slang-static)
target_compile_definitions(slang-static PRIVATE SLANG_STATIC
                                                SLANG_WITHOUT_EMBEDDED_STD_LIB)

#
# Generate an embeddable stdlib
#

set(SLANG_STDLIB_GENERATED_HEADER
    ${CMAKE_CURRENT_BINARY_DIR}/slang-stdlib-generated.h)
add_custom_command(
  OUTPUT ${SLANG_STDLIB_GENERATED_HEADER}
  COMMAND slang-bootstrap -archive-type riff-lz4 -save-stdlib-bin-source
          ${SLANG_STDLIB_GENERATED_HEADER}
  VERBATIM)
add_library(slang-stdlib-embed-headers INTERFACE EXCLUDE_FROM_ALL)
target_sources(slang-stdlib-embed-headers
               INTERFACE ${SLANG_STDLIB_GENERATED_HEADER})
target_include_directories(slang-stdlib-embed-headers
                           INTERFACE ${CMAKE_CURRENT_BINARY_DIR})

#
# generated headers for reflection
#

set(SLANG_REFLECT_SOURCE
    slang-ast-support-types.h
    slang-ast-base.h
    slang-ast-decl.h
    slang-ast-expr.h
    slang-ast-modifier.h
    slang-ast-stmt.h
    slang-ast-type.h
    slang-ast-val.h)

set(SLANG_REFLECT_GENERATED_HEADERS)
foreach(type obj ast value)
  foreach(macro "" -macro)
    list(APPEND SLANG_REFLECT_GENERATED_HEADERS
         "slang-generated-${type}${macro}.h")
  endforeach()
endforeach()

add_custom_command(
  OUTPUT ${SLANG_REFLECT_GENERATED_HEADERS}
  COMMAND
    slang-cpp-extractor -strip-prefix slang- -o
    ${CMAKE_CURRENT_BINARY_DIR}/slang-generated -output-fields -mark-suffix
    _CLASS ${SLANG_REFLECT_SOURCE}
  DEPENDS ${SLANG_REFLECT_SOURCE}
  VERBATIM)
add_library(slang-reflect-headers INTERFACE EXCLUDE_FROM_ALL
            ${SLANG_REFLECT_SOURCE})

#
# Slang itself
#
add_library(slang ${SLANG_SOURCE})
set_default_compile_options(slang)
target_link_libraries(slang PRIVATE core compiler-core prelude SPIRV-Headers)

# slang.h is in the project root, so include that directory in the interface for
# slang
target_include_directories(
  slang PUBLIC $<BUILD_INTERFACE:${SLANG_ROOT_DIR}>
               $<INSTALL_INTERFACE:include> # <prefix>/include/
)

get_target_property(SLANG_TARGET_TYPE slang TYPE)
if(SLANG_TARGET_TYPE STREQUAL SHARED_LIBRARY)
  target_compile_definitions(slang PRIVATE SLANG_DYNAMIC_EXPORT)
endif()

if(NOT SLANG_EMBED_STDLIB_SOURCE)
  target_compile_definitions(slang PRIVATE SLANG_DISABLE_STDLIB_SOURCE)
endif()

if(SLANG_ENABLE_FULL_DEBUG_VALIDATION)
  target_compile_definitions(slang PRIVATE SLANG_ENABLE_FULL_IR_VALIDATION)
endif()

if(SLANG_EMBED_STDLIB)
  target_link_libraries(slang PRIVATE slang-stdlib-embed-headers)
else()
  target_compile_definitions(slang PRIVATE SLANG_WITHOUT_EMBEDDED_STD_LIB)
endif()
