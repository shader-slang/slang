#ifndef SLANG_COMPILER_H_INCLUDED
#define SLANG_COMPILER_H_INCLUDED

#include "../core/slang-basic.h"
#include "../core/slang-shared-library.h"

#include "../core/slang-cpp-compiler.h"

#include "../../slang-com-ptr.h"

#include "slang-diagnostics.h"
#include "slang-name.h"
#include "slang-profile.h"
#include "slang-syntax.h"

#include "../../slang.h"

namespace Slang
{
    struct PathInfo;
    struct IncludeHandler;
    class ProgramLayout;
    class PtrType;
    class TargetProgram;
    class TargetRequest;
    class TypeLayout;

    enum class CompilerMode
    {
        ProduceLibrary,
        ProduceShader,
        GenerateChoice
    };

    enum class StageTarget
    {
        Unknown,
        VertexShader,
        HullShader,
        DomainShader,
        GeometryShader,
        FragmentShader,
        ComputeShader,
    };

    enum class CodeGenTarget
    {
        Unknown             = SLANG_TARGET_UNKNOWN,
        None                = SLANG_TARGET_NONE,
        GLSL                = SLANG_GLSL,
        GLSL_Vulkan         = SLANG_GLSL_VULKAN,
        GLSL_Vulkan_OneDesc = SLANG_GLSL_VULKAN_ONE_DESC,
        HLSL                = SLANG_HLSL,
        SPIRV               = SLANG_SPIRV,
        SPIRVAssembly       = SLANG_SPIRV_ASM,
        DXBytecode          = SLANG_DXBC,
        DXBytecodeAssembly  = SLANG_DXBC_ASM,
        DXIL                = SLANG_DXIL,
        DXILAssembly        = SLANG_DXIL_ASM,
        CSource             = SLANG_C_SOURCE,
        CPPSource           = SLANG_CPP_SOURCE,
        Executable          = SLANG_EXECUTABLE,
        SharedLibrary       = SLANG_SHARED_LIBRARY,
        HostCallable        = SLANG_HOST_CALLABLE,
        CountOf             = SLANG_TARGET_COUNT_OF,
    };

    CodeGenTarget calcCodeGenTargetFromName(const UnownedStringSlice& name);
    UnownedStringSlice getCodeGenTargetName(CodeGenTarget target);

    enum class ContainerFormat
    {
        None            = SLANG_CONTAINER_FORMAT_NONE,
        SlangModule     = SLANG_CONTAINER_FORMAT_SLANG_MODULE,
    };

    enum class LineDirectiveMode : SlangLineDirectiveMode
    {
        Default     = SLANG_LINE_DIRECTIVE_MODE_DEFAULT,
        None        = SLANG_LINE_DIRECTIVE_MODE_NONE,
        Standard    = SLANG_LINE_DIRECTIVE_MODE_STANDARD,
        GLSL        = SLANG_LINE_DIRECTIVE_MODE_GLSL,
    };

    enum class ResultFormat
    {
        None,
        Text,
        Binary,
        SharedLibrary,
    };

    // When storing the layout for a matrix-type
    // value, we need to know whether it has been
    // laid out with row-major or column-major
    // storage.
    //
    enum MatrixLayoutMode
    {
        kMatrixLayoutMode_RowMajor      = SLANG_MATRIX_LAYOUT_ROW_MAJOR,
        kMatrixLayoutMode_ColumnMajor   = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR,
    };

    enum class DebugInfoLevel : SlangDebugInfoLevel
    {
        None        = SLANG_DEBUG_INFO_LEVEL_NONE,
        Minimal     = SLANG_DEBUG_INFO_LEVEL_MINIMAL,
        Standard    = SLANG_DEBUG_INFO_LEVEL_STANDARD,
        Maximal     = SLANG_DEBUG_INFO_LEVEL_MAXIMAL,
    };

    enum class OptimizationLevel : SlangOptimizationLevel
    {
        None    = SLANG_OPTIMIZATION_LEVEL_NONE,
        Default = SLANG_OPTIMIZATION_LEVEL_DEFAULT,
        High    = SLANG_OPTIMIZATION_LEVEL_HIGH,
        Maximal = SLANG_OPTIMIZATION_LEVEL_MAXIMAL,
    };

    class Linkage;
    class Module;
    class FrontEndCompileRequest;
    class BackEndCompileRequest;
    class EndToEndCompileRequest;
    class TranslationUnitRequest;

    // Result of compiling an entry point.
    // Should only ever be string, binary or shared library
    class CompileResult
    {
    public:
        CompileResult() = default;
        CompileResult(String const& str) : format(ResultFormat::Text), outputString(str) {}
        CompileResult(List<uint8_t> const& buffer) : format(ResultFormat::Binary), outputBinary(buffer) {}
        CompileResult(ISlangSharedLibrary* inSharedLibrary) : format(ResultFormat::SharedLibrary), sharedLibrary(inSharedLibrary) {}

        void append(CompileResult const& result);

        SlangResult getBlob(ComPtr<ISlangBlob>& outBlob);
        SlangResult getSharedLibrary(ComPtr<ISlangSharedLibrary>& outSharedLibrary);

        ResultFormat format = ResultFormat::None;
        String outputString;
        List<uint8_t> outputBinary;

        ComPtr<ISlangSharedLibrary> sharedLibrary;
        ComPtr<ISlangBlob> blob;
    };

        /// Information collected about global or entry-point shader parameters
    struct ShaderParamInfo
    {
        DeclRef<VarDeclBase>    paramDeclRef;
        Int                     firstSpecializationParamIndex = 0;
        Int                     specializationParamCount = 0;
    };

        /// Extended information specific to global shader parameters
    struct GlobalShaderParamInfo : ShaderParamInfo
    {
        // TODO: This type should be eliminated if/when we remove
        // support for compilation with multiple translation units
        // that all declare the "same" shader parameter (e.g., a
        // `cbuffer`) and expect those duplicate declarations
        // to get the same parameter binding/layout.

        // Additional global-scope declarations that are conceptually
        // declaring the "same" parameter as the `paramDeclRef`.
        List<DeclRef<VarDeclBase>> additionalParamDeclRefs;
    };

        /// A request for the front-end to find and validate an entry-point function
    struct FrontEndEntryPointRequest : RefObject
    {
    public:
            /// Create a request for an entry point.
        FrontEndEntryPointRequest(
            FrontEndCompileRequest* compileRequest,
            int                     translationUnitIndex,
            Name*                   name,
            Profile                 profile);

            /// Get the parent front-end compile request.
        FrontEndCompileRequest* getCompileRequest() { return m_compileRequest; }

            /// Get the translation unit that contains the entry point.
        TranslationUnitRequest* getTranslationUnit();

            /// Get the name of the entry point to find.
        Name* getName() { return m_name; }

            /// Get the stage that the entry point is to be compiled for
        Stage getStage() { return m_profile.GetStage(); }

            /// Get the profile that the entry point is to be compiled for
        Profile getProfile() { return m_profile; }

    private:
        // The parent compile request
        FrontEndCompileRequest* m_compileRequest;

        // The index of the translation unit that will hold the entry point
        int                     m_translationUnitIndex;

        // The name of the entry point function to look for
        Name*                   m_name;

        // The profile to compile for (including stage)
        Profile                 m_profile;
    };

        /// Tracks an ordered list of modules that something depends on.
    struct ModuleDependencyList
    {
    public:
            /// Get the list of modules that are depended on.
        List<Module*> const& getModuleList() { return m_moduleList; }

            /// Add a module and everything it depends on to the list.
        void addDependency(Module* module);

            /// Add a module to the list, but not the modules it depends on.
        void addLeafDependency(Module* module);

    private:
        void _addDependency(Module* module);

        List<Module*>       m_moduleList;
        HashSet<Module*>    m_moduleSet;
    };

        /// Tracks an unordered list of filesystem paths that something depends on
    struct FilePathDependencyList
    {
    public:
            /// Get the list of paths that are depended on.
        List<String> const& getFilePathList() { return m_filePathList; }

            /// Add a path to the list, if it is not already present
        void addDependency(String const& path);

            /// Add all of the paths that `module` depends on to the list
        void addDependency(Module* module);

    private:

        // TODO: We are using a `HashSet` here to deduplicate
        // the paths so that we don't return the same path
        // multiple times from `getFilePathList`, but because
        // order isn't important, we could potentially do better
        // in terms of memory (at some cost in performance) by
        // just sorting the `m_filePathList` every once in
        // a while and then deduplicating.

        List<String>    m_filePathList;
        HashSet<String> m_filePathSet;
    };

    class EntryPoint;

    class ComponentType;
    class ComponentTypeVisitor;

        /// Base class for "component types" that represent the pieces a final
        /// shader program gets linked together from.
        ///
    class ComponentType : public RefObject, public slang::IComponentType
    {
    public:
        //
        // ISlangUnknown interface
        //

        SLANG_REF_OBJECT_IUNKNOWN_ALL;
        ISlangUnknown* getInterface(Guid const& guid);

        //
        // slang::IComponentType interface
        //

        SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() SLANG_OVERRIDE;
        SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL getLayout(
            SlangInt        targetIndex,
            slang::IBlob**  outDiagnostics) SLANG_OVERRIDE;
        SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
            SlangInt        entryPointIndex,
            SlangInt        targetIndex,
            slang::IBlob**  outCode,
            slang::IBlob**  outDiagnostics) SLANG_OVERRIDE;
        SLANG_NO_THROW IComponentType* SLANG_MCALL specialize(
            slang::SpecializationArg const* specializationArgs,
            SlangInt                        specializationArgCount,
            ISlangBlob**                    outDiagnostics) SLANG_OVERRIDE;

            /// Get the linkage (aka "session" in the public API) for this component type.
        Linkage* getLinkage() { return m_linkage; }

            /// Get the target-specific version of this program for the given `target`.
            ///
            /// The `target` must be a target on the `Linkage` that was used to create this program.
        TargetProgram* getTargetProgram(TargetRequest* target);

            /// Get the number of entry points linked into this component type.
        virtual Index getEntryPointCount() = 0;

            /// Get one of the entry points linked into this component type.
        virtual RefPtr<EntryPoint> getEntryPoint(Index index) = 0;

            /// Get the number of global shader parameters linked into this component type.
        virtual Index getShaderParamCount() = 0;

            /// Get one of the global shader parametesr linked into this component type.
        virtual GlobalShaderParamInfo getShaderParam(Index index) = 0;

            /// Get the number of (unspecialized) specialization parameters for the component type.
        virtual Index getSpecializationParamCount() = 0;

            /// Get the specialization parameter at `index`.
        virtual SpecializationParam const& getSpecializationParam(Index index) = 0;

            /// Get the number of "requirements" that this component type has.
            ///
            /// A requirement represents another component type that this component
            /// needs in order to function correctly. For example, the dependency
            /// of one module on another module that it `import`s is represented
            /// as a requirement, as is the dependency of an entry point on the
            /// module that defines it.
            ///
        virtual Index getRequirementCount() = 0;

            /// Get the requirement at `index`.
        virtual RefPtr<ComponentType> getRequirement(Index index) = 0;

            /// Parse a type from a string, in the context of this component type.
            ///
            /// Any names in the string will be resolved using the modules
            /// referenced by the program.
            ///
            /// On an error, returns null and reports diagnostic messages
            /// to the provided `sink`.
            ///
            /// TODO: This function shouldn't be on the base class, since
            /// it only really makes sense on `Module` and (as a compatibility
            /// feature) on `LegacyProgram`.
            ///
        Type* getTypeFromString(
            String const&   typeStr,
            DiagnosticSink* sink);

            /// Get a list of modules that this component type depends on.
            ///
        virtual List<Module*> const& getModuleDependencies() = 0;

            /// Get the full list of filesystem paths this component type depends on.
            ///
        virtual List<String> const& getFilePathDependencies() = 0;

            /// Callback for use with `enumerateIRModules`
        typedef void (*EnumerateIRModulesCallback)(IRModule* irModule, void* userData);

            /// Invoke `callback` on all the IR modules that are (transitively) linked into this component type.
        void enumerateIRModules(EnumerateIRModulesCallback callback, void* userData);

            /// Invoke `callback` on all the IR modules that are (transitively) linked into this component type.
        template<typename F>
        void enumerateIRModules(F const& callback)
        {
            struct Helper
            {
                static void helper(IRModule* irModule, void* userData)
                {
                    (*(F*)userData)(irModule);
                }
            };
            enumerateIRModules(&Helper::helper, (void*)&callback);
        }

            /// Callback for use with `enumerateModules`
        typedef void (*EnumerateModulesCallback)(Module* module, void* userData);

            /// Invoke `callback` on all the modules that are (transitively) linked into this component type.
        void enumerateModules(EnumerateModulesCallback callback, void* userData);

            /// Invoke `callback` on all the modules that are (transitively) linked into this component type.
        template<typename F>
        void enumerateModules(F const& callback)
        {
            struct Helper
            {
                static void helper(Module* module, void* userData)
                {
                    (*(F*)userData)(module);
                }
            };
            enumerateModules(&Helper::helper, (void*)&callback);
        }

            /// Side-band information generated when specializing this component type.
            ///
            /// Difference subclasses of `ComponentType` are expected to create their
            /// own subclass of `SpecializationInfo` as the output of `_validateSpecializationArgs`.
            /// Later, whenever we want to use a specialized component type we will
            /// also have the `SpecializationInfo` available and will expect it to
            /// have the correct (subclass-specific) type.
            ///
        class SpecializationInfo : public RefObject
        {
        };

            /// Validate the given specialization `args` and compute any side-band specialization info.
            ///
            /// Any errors will be reported to `sink`, which can thus be used to test
            /// if the operation was successful.
            ///
            /// A null return value is allowed, since not all subclasses require
            /// custom side-band specialization information.
            ///
            /// This function is an implementation detail of `specialize()`.
            ///
        virtual RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink) = 0;

            /// Validate the given specialization `args` and compute any side-band specialization info.
            ///
            /// Any errors will be reported to `sink`, which can thus be used to test
            /// if the operation was successful.
            ///
            /// A null return value is allowed, since not all subclasses require
            /// custom side-band specialization information.
            ///
            /// This function is an implementation detail of `specialize()`.
            ///
        RefPtr<SpecializationInfo> _validateSpecializationArgs(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink)
        {
            if(argCount == 0) return nullptr;
            return _validateSpecializationArgsImpl(args, argCount, sink);
        }

            /// Specialize this component type given `specializationArgs`
            ///
            /// Any diagnostics will be reported to `sink`, which can be used
            /// to determine if the operation was successful. It is allowed
            /// for this operation to have a non-null return even when an
            /// error is ecnountered.
            ///
        RefPtr<ComponentType> specialize(
            SpecializationArg const*    specializationArgs,
            SlangInt                    specializationArgCount,
            DiagnosticSink*             sink);

            /// Invoke `visitor` on this component type, using the appropriate dynamic type.
            ///
            /// This function implements the "visitor pattern" for `ComponentType`.
            ///
            /// If the `specializationInfo` argument is non-null, it must be specialization
            /// information generated for this specific component type by `_validateSpecializationArgs`.
            /// In that case, appropriately-typed specialization information will be passed
            /// when invoking the `visitor`.
            ///
        virtual void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo) = 0;

    protected:
        ComponentType(Linkage* linkage);

    private:
        RefPtr<Linkage> m_linkage;

        // Cache of target-specific programs for each target.
        Dictionary<TargetRequest*, RefPtr<TargetProgram>> m_targetPrograms;

        // Any types looked up dynamically using `getTypeFromString`
        //
        // TODO: Remove this. Type lookup should only be supported on `Module`s.
        //
        Dictionary<String, RefPtr<Type>> m_types;
    };

        /// A component type built up from other component types.
    class CompositeComponentType : public ComponentType
    {
    public:
        static RefPtr<ComponentType> create(
            Linkage*                            linkage,
            List<RefPtr<ComponentType>> const&  childComponents);

        List<RefPtr<ComponentType>> const& getChildComponents() { return m_childComponents; };
        Index getChildComponentCount() { return m_childComponents.getCount(); }
        RefPtr<ComponentType> getChildComponent(Index index) { return m_childComponents[index]; }

        Index getEntryPointCount() SLANG_OVERRIDE;
        RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE;

        Index getShaderParamCount() SLANG_OVERRIDE;
        GlobalShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE;

        Index getSpecializationParamCount() SLANG_OVERRIDE;
        SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE;

        Index getRequirementCount() SLANG_OVERRIDE;
        RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

        List<Module*> const& getModuleDependencies() SLANG_OVERRIDE;
        List<String> const& getFilePathDependencies() SLANG_OVERRIDE;

        class CompositeSpecializationInfo : public SpecializationInfo
        {
        public:
            List<RefPtr<SpecializationInfo>> childInfos;
        };

    protected:
        void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo) SLANG_OVERRIDE;


        RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink) SLANG_OVERRIDE;

    private:
        CompositeComponentType(
            Linkage*                            linkage,
            List<RefPtr<ComponentType>> const&  childComponents);

        List<RefPtr<ComponentType>> m_childComponents;

        // The following arrays hold the concatenated entry points, parameters,
        // etc. from the child components. This approach allows for reasonably
        // fast (constant time) access through operations like `getShaderParam`,
        // but means that the memory usage of a composite is proportional to
        // the sum of the memory usage of the children, rather than being fixed
        // by the number of children (as it would be if we just stored 
        // `m_childComponents`).
        //
        // TODO: We could conceivably build some O(numChildren) arrays that
        // support binary-search to provide logarithmic-time access to entry
        // points, parameters, etc. while giving a better overall memory usage.
        //
        List<EntryPoint*> m_entryPoints;
        List<GlobalShaderParamInfo> m_shaderParams;
        List<SpecializationParam> m_specializationParams;
        List<ComponentType*> m_requirements;

        ModuleDependencyList m_moduleDependencyList;
        FilePathDependencyList m_filePathDependencyList;
    };

        /// A component type created by specializing another component type.
    class SpecializedComponentType : public ComponentType
    {
    public:
        SpecializedComponentType(
            ComponentType*                  base,
            SpecializationInfo*             specializationInfo,
            List<SpecializationArg> const&  specializationArgs,
            DiagnosticSink*                 sink);

            /// Get the base (unspecialized) component type that is being specialized.
        RefPtr<ComponentType> getBaseComponentType() { return m_base; }

        RefPtr<SpecializationInfo> getSpecializationInfo() { return m_specializationInfo; }

            /// Get the number of arguments supplied for existential type parameters.
            ///
            /// Note that the number of arguments may not match the number of parameters.
            /// In particular, an unspecialized entry point may have many parameters, but zero arguments.
        Index getSpecializationArgCount() { return m_specializationArgs.getCount(); }

            /// Get the existential type argument (type and witness table) at `index`.
        SpecializationArg const& getSpecializationArg(Index index) { return m_specializationArgs[index]; }

            /// Get an array of all existential type arguments.
        SpecializationArg const* getSpecializationArgs() { return m_specializationArgs.getBuffer(); }

        Index getEntryPointCount() SLANG_OVERRIDE { return m_base->getEntryPointCount(); }
        RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE { return m_base->getEntryPoint(index); }

        Index getShaderParamCount() SLANG_OVERRIDE { return m_base->getShaderParamCount(); }
        GlobalShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE { return m_base->getShaderParam(index); }

        Index getSpecializationParamCount() SLANG_OVERRIDE { return 0; }
        SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE { SLANG_UNUSED(index); static SpecializationParam dummy; return dummy; }

        Index getRequirementCount() SLANG_OVERRIDE;
        RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

            /// TODO: These should include requirements/dependencies for the types
            /// referenced in the specialization arguments...
        List<Module*> const& getModuleDependencies() SLANG_OVERRIDE { return m_base->getModuleDependencies(); }
        List<String> const& getFilePathDependencies() SLANG_OVERRIDE { return m_base->getFilePathDependencies(); }

                    /// Get a list of tagged-union types referenced by the specialization parameters.
        List<RefPtr<TaggedUnionType>> const& getTaggedUnionTypes() { return m_taggedUnionTypes; }

        RefPtr<IRModule> getIRModule() { return m_irModule; }

        void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo) SLANG_OVERRIDE;

    protected:

        RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink) SLANG_OVERRIDE
        {
            SLANG_UNUSED(args);
            SLANG_UNUSED(argCount);
            SLANG_UNUSED(sink);
            return nullptr;
        }

    private:
        RefPtr<ComponentType> m_base;
        RefPtr<SpecializationInfo> m_specializationInfo;
        SpecializationArgs m_specializationArgs;
        RefPtr<IRModule> m_irModule;

        // Any tagged union types that were referenced by the specialization arguments.
        List<RefPtr<TaggedUnionType>> m_taggedUnionTypes;

    };

        /// Describes an entry point for the purposes of layout and code generation.
        ///
        /// This class also tracks any generic arguments to the entry point,
        /// in the case that it is a specialization of a generic entry point.
        ///
        /// There is also a provision for creating a "dummy" entry point for
        /// the purposes of pass-through compilation modes. Only the
        /// `getName()` and `getProfile()` methods should be expected to
        /// return useful data on pass-through entry points.
        ///
    class EntryPoint : public ComponentType
    {
    public:
            /// Create an entry point that refers to the given function.
        static RefPtr<EntryPoint> create(
            Linkage*            linkage,
            DeclRef<FuncDecl>   funcDeclRef,
            Profile             profile);

            /// Get the function decl-ref, including any generic arguments.
        DeclRef<FuncDecl> getFuncDeclRef() { return m_funcDeclRef; }

            /// Get the function declaration (without generic arguments).
        RefPtr<FuncDecl> getFuncDecl() { return m_funcDeclRef.getDecl(); }

            /// Get the name of the entry point
        Name* getName() { return m_name; }

            /// Get the profile associated with the entry point
            ///
            /// Note: only the stage part of the profile is expected
            /// to contain useful data, but certain legacy code paths
            /// allow for "shader model" information to come via this path.
            ///
        Profile getProfile() { return m_profile; }

            /// Get the stage that the entry point is for.
        Stage getStage() { return m_profile.GetStage(); }

            /// Get the module that contains the entry point.
        Module* getModule();

            /// Get a list of modules that this entry point depends on.
            ///
            /// This will include the module that defines the entry point (see `getModule()`),
            /// but may also include modules that are required by its generic type arguments.
            ///
        List<Module*> const& getModuleDependencies() SLANG_OVERRIDE; // { return getModule()->getModuleDependencies(); }
        List<String> const& getFilePathDependencies() SLANG_OVERRIDE; // { return getModule()->getFilePathDependencies(); }

            /// Create a dummy `EntryPoint` that is only usable for pass-through compilation.
        static RefPtr<EntryPoint> createDummyForPassThrough(
            Linkage*    linkage,
            Name*       name,
            Profile     profile);

            /// Get the number of existential type parameters for the entry point.
        Index getSpecializationParamCount() SLANG_OVERRIDE;

            /// Get the existential type parameter at `index`.
        SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE;

        Index getRequirementCount() SLANG_OVERRIDE;
        RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

        SpecializationParams const& getExistentialSpecializationParams() { return m_existentialSpecializationParams; }

        Index getGenericSpecializationParamCount() { return m_genericSpecializationParams.getCount(); }
        Index getExistentialSpecializationParamCount() { return m_existentialSpecializationParams.getCount(); }

            /// Get an array of all entry-point shader parameters.
        List<ShaderParamInfo> const& getShaderParams() { return m_shaderParams; }

        Index getEntryPointCount() SLANG_OVERRIDE { return 1; };
        RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE { SLANG_UNUSED(index); return this; }

        Index getShaderParamCount() SLANG_OVERRIDE { return 0; }
        GlobalShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE { SLANG_UNUSED(index); return GlobalShaderParamInfo(); }

        class EntryPointSpecializationInfo : public SpecializationInfo
        {
        public:
            DeclRef<FuncDecl> specializedFuncDeclRef;
            List<ExpandedSpecializationArg> existentialSpecializationArgs;
        };

    protected:
        void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo) SLANG_OVERRIDE;

        RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink) SLANG_OVERRIDE;

    private:
        EntryPoint(
            Linkage*            linkage,
            Name*               name,
            Profile             profile,
            DeclRef<FuncDecl>   funcDeclRef);

        void _collectGenericSpecializationParamsRec(Decl* decl);
        void _collectShaderParams();

        // The name of the entry point function (e.g., `main`)
        //
        Name* m_name = nullptr;

        // The declaration of the entry-point function itself.
        //
        DeclRef<FuncDecl> m_funcDeclRef;

        SpecializationParams m_genericSpecializationParams;
        SpecializationParams m_existentialSpecializationParams;

            /// Information about entry-point parameters
        List<ShaderParamInfo> m_shaderParams;

        // The profile that the entry point will be compiled for
        // (this is a combination of the target stage, and also
        // a feature level that sets capabilities)
        //
        // Note: the profile-version part of this should probably
        // be moving towards deprecation, in favor of the version
        // information (e.g., "Shader Model 5.1") always coming
        // from the target, while the stage part is all that is
        // intrinsic to the entry point.
        //
        Profile m_profile;
    };

    enum class PassThroughMode : SlangPassThrough
    {
        None = SLANG_PASS_THROUGH_NONE,	                    ///< don't pass through: use Slang compiler
        Fxc = SLANG_PASS_THROUGH_FXC,	                    ///< pass through HLSL to `D3DCompile` API
        Dxc = SLANG_PASS_THROUGH_DXC,	                    ///< pass through HLSL to `IDxcCompiler` API
        Glslang = SLANG_PASS_THROUGH_GLSLANG,	            ///< pass through GLSL to `glslang` library
        Clang = SLANG_PASS_THROUGH_CLANG,                   ///< Pass through clang compiler
        VisualStudio = SLANG_PASS_THROUGH_VISUAL_STUDIO,    ///< Visual studio compiler
        Gcc = SLANG_PASS_THROUGH_GCC,                       ///< Gcc compiler
        GenericCCpp = SLANG_PASS_THROUGH_GENERIC_C_CPP,     ///< Generic C/C++ compiler
        CountOf = SLANG_PASS_THROUGH_COUNT_OF,              
    };

    class SourceFile;

        /// A module of code that has been compiled through the front-end
        ///
        /// A module comprises all the code from one translation unit (which
        /// may span multiple Slang source files), and provides access
        /// to both the AST and IR representations of that code.
        ///
    class Module : public ComponentType, public slang::IModule
    {
        typedef ComponentType Super;

    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL

        ISlangUnknown* getInterface(const Guid& guid);


        // Forward `IComponentType` methods

        SLANG_NO_THROW slang::ISession* SLANG_MCALL getSession() SLANG_OVERRIDE
        {
            return Super::getSession();
        }

        SLANG_NO_THROW slang::ProgramLayout* SLANG_MCALL getLayout(
            SlangInt        targetIndex,
            slang::IBlob**  outDiagnostics) SLANG_OVERRIDE
        {
            return Super::getLayout(targetIndex, outDiagnostics);
        }

        SLANG_NO_THROW SlangResult SLANG_MCALL getEntryPointCode(
            SlangInt        entryPointIndex,
            SlangInt        targetIndex,
            slang::IBlob**  outCode,
            slang::IBlob**  outDiagnostics) SLANG_OVERRIDE
        {
            return Super::getEntryPointCode(entryPointIndex, targetIndex, outCode, outDiagnostics);
        }

        SLANG_NO_THROW IComponentType* SLANG_MCALL specialize(
            slang::SpecializationArg const* specializationArgs,
            SlangInt                        specializationArgCount,
            ISlangBlob**                    outDiagnostics) SLANG_OVERRIDE
        {
            return Super::specialize(specializationArgs, specializationArgCount, outDiagnostics);
        }

        //

            /// Create a module (initially empty).
        Module(Linkage* linkage);

            /// Get the AST for the module (if it has been parsed)
        ModuleDecl* getModuleDecl() { return m_moduleDecl; }

            /// The the IR for the module (if it has been generated)
        IRModule* getIRModule() { return m_irModule; }

            /// Get the list of other modules this module depends on
        List<Module*> const& getModuleDependencyList() { return m_moduleDependencyList.getModuleList(); }

            /// Get the list of filesystem paths this module depends on
        List<String> const& getFilePathDependencyList() { return m_filePathDependencyList.getFilePathList(); }

            /// Register a module that this module depends on
        void addModuleDependency(Module* module);

            /// Register a filesystem path that this module depends on
        void addFilePathDependency(String const& path);

            /// Set the AST for this module.
            ///
            /// This should only be called once, during creation of the module.
            ///
        void setModuleDecl(ModuleDecl* moduleDecl);// { m_moduleDecl = moduleDecl; }

            /// Set the IR for this module.
            ///
            /// This should only be called once, during creation of the module.
            ///
        void setIRModule(IRModule* irModule) { m_irModule = irModule; }

        Index getEntryPointCount() SLANG_OVERRIDE { return 0; }
        RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE { SLANG_UNUSED(index); return nullptr; }

        Index getShaderParamCount() SLANG_OVERRIDE { return m_shaderParams.getCount(); }
        GlobalShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE { return m_shaderParams[index]; }

        Index getSpecializationParamCount() SLANG_OVERRIDE { return m_specializationParams.getCount(); }
        SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE { return m_specializationParams[index]; }

        Index getRequirementCount() SLANG_OVERRIDE;
        RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

        List<Module*> const& getModuleDependencies() SLANG_OVERRIDE { return m_moduleDependencyList.getModuleList(); }
        List<String> const& getFilePathDependencies() SLANG_OVERRIDE { return m_filePathDependencyList.getFilePathList(); }

            /// Collect information on the shader parameters of the module.
            ///
            /// This method should only be called once, after the core
            /// structured of the module (its AST and IR) have been created,
            /// and before any of the `ComponentType` APIs are used.
            ///
            /// TODO: We might eventually consider a non-stateful approach
            /// to constructing a `Module`.
            ///
        void _collectShaderParams();

        class ModuleSpecializationInfo : public SpecializationInfo
        {
        public:
            struct GenericArgInfo
            {
                RefPtr<Decl> paramDecl;
                RefPtr<Val> argVal;
            };

            List<GenericArgInfo> genericArgs;
            List<ExpandedSpecializationArg> existentialArgs;
        };

    protected:
        void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo) SLANG_OVERRIDE;

        RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink) SLANG_OVERRIDE;

    private:
        // The AST for the module
        RefPtr<ModuleDecl>  m_moduleDecl;

        // The IR for the module
        RefPtr<IRModule> m_irModule = nullptr;

        List<GlobalShaderParamInfo> m_shaderParams;
        SpecializationParams m_specializationParams;

        List<Module*> m_requirements;

        // List of modules this module depends on
        ModuleDependencyList m_moduleDependencyList;

        // List of filesystem paths this module depends on
        FilePathDependencyList m_filePathDependencyList;
    };
    typedef Module LoadedModule;

        /// A request for the front-end to compile a translation unit.
    class TranslationUnitRequest : public RefObject
    {
    public:
        TranslationUnitRequest(
            FrontEndCompileRequest* compileRequest);

        // The parent compile request
        FrontEndCompileRequest* compileRequest = nullptr;

        // The language in which the source file(s)
        // are assumed to be written
        SourceLanguage sourceLanguage = SourceLanguage::Unknown;

        // The source file(s) that will be compiled to form this translation unit
        //
        // Usually, for HLSL or GLSL there will be only one file.
        List<SourceFile*> m_sourceFiles;

        List<SourceFile*> const& getSourceFiles() { return m_sourceFiles; }
        void addSourceFile(SourceFile* sourceFile);

        // The entry points associated with this translation unit
        List<RefPtr<EntryPoint>> entryPoints;

        // Preprocessor definitions to use for this translation unit only
        // (whereas the ones on `compileRequest` will be shared)
        Dictionary<String, String> preprocessorDefinitions;

            /// The name that will be used for the module this translation unit produces.
        Name* moduleName = nullptr;

            /// Result of compiling this translation unit (a module)
        RefPtr<Module> module;

        Module* getModule() { return module; }
        RefPtr<ModuleDecl> getModuleDecl() { return module->getModuleDecl(); }

        Session* getSession();
        NamePool* getNamePool();
        SourceManager* getSourceManager();
    };

    enum class FloatingPointMode : SlangFloatingPointMode
    {
        Default = SLANG_FLOATING_POINT_MODE_DEFAULT,
        Fast = SLANG_FLOATING_POINT_MODE_FAST,
        Precise = SLANG_FLOATING_POINT_MODE_PRECISE,
    };

    enum class WriterChannel : SlangWriterChannel
    {
        Diagnostic = SLANG_WRITER_CHANNEL_DIAGNOSTIC,
        StdOutput = SLANG_WRITER_CHANNEL_STD_OUTPUT,
        StdError = SLANG_WRITER_CHANNEL_STD_ERROR,
        CountOf = SLANG_WRITER_CHANNEL_COUNT_OF,
    };

    enum class WriterMode : SlangWriterMode
    {
        Text = SLANG_WRITER_MODE_TEXT,
        Binary = SLANG_WRITER_MODE_BINARY,
    };

        /// A request to generate output in some target format.
    class TargetRequest : public RefObject
    {
    public:
        Linkage*            linkage;
        CodeGenTarget       target;
        SlangTargetFlags    targetFlags = 0;
        Slang::Profile      targetProfile = Slang::Profile();
        FloatingPointMode   floatingPointMode = FloatingPointMode::Default;

        Linkage* getLinkage() { return linkage; }
        CodeGenTarget getTarget() { return target; }
        Profile getTargetProfile() { return targetProfile; }
        FloatingPointMode getFloatingPointMode() { return floatingPointMode; }

        Session* getSession();
        MatrixLayoutMode getDefaultMatrixLayoutMode();

        // TypeLayouts created on the fly by reflection API
        Dictionary<Type*, RefPtr<TypeLayout>> typeLayouts;

        Dictionary<Type*, RefPtr<TypeLayout>>& getTypeLayouts() { return typeLayouts; }

        TypeLayout* getTypeLayout(Type* type);
    };

        /// Are we generating code for a D3D API?
    bool isD3DTarget(TargetRequest* targetReq);

        /// Are we generating code for a Khronos API (OpenGL or Vulkan)?
    bool isKhronosTarget(TargetRequest* targetReq);

    // Compute the "effective" profile to use when outputting the given entry point
    // for the chosen code-generation target.
    //
    // The stage of the effective profile will always come from the entry point, while
    // the profile version (aka "shader model") will be computed as follows:
    //
    // - If the entry point and target belong to the same profile family, then take
    //   the latest version between the two (e.g., if the entry point specified `ps_5_1`
    //   and the target specifies `sm_5_0` then use `sm_5_1` as the version).
    //
    // - If the entry point and target disagree on the profile family, always use the
    //   profile family and version from the target.
    //
    Profile getEffectiveProfile(EntryPoint* entryPoint, TargetRequest* target);


    // A directory to be searched when looking for files (e.g., `#include`)
    struct SearchDirectory
    {
        SearchDirectory() = default;
        SearchDirectory(SearchDirectory const& other) = default;
        SearchDirectory(String const& path)
            : path(path)
        {}

        String  path;
    };

        /// A list of directories to search for files (e.g., `#include`)
    struct SearchDirectoryList
    {
        // A parent list that should also be searched
        SearchDirectoryList*    parent = nullptr;

        // Directories to be searched
        List<SearchDirectory>   searchDirectories;
    };

    /// Create a blob that will retain (a copy of) raw data.
    ///
    ComPtr<ISlangBlob> createRawBlob(void const* data, size_t size);


        /// Given a target returns the required downstream compiler
    PassThroughMode getDownstreamCompilerRequiredForTarget(CodeGenTarget target);

    PassThroughMode getPassThroughModeForCPPCompiler(CPPCompiler::CompilerType type);


        /// A context for loading and re-using code modules.
    class Linkage : public RefObject, public slang::ISession
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL

        ISlangUnknown* getInterface(const Guid& guid);

        SLANG_NO_THROW slang::IGlobalSession* SLANG_MCALL getGlobalSession() override;
        SLANG_NO_THROW slang::IModule* SLANG_MCALL loadModule(
            const char* moduleName,
            slang::IBlob**     outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::IComponentType* SLANG_MCALL createCompositeComponentType(
            slang::IComponentType* const*  componentTypes,
            SlangInt                componentTypeCount,
            ISlangBlob**            outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::TypeReflection* SLANG_MCALL specializeType(
            slang::TypeReflection*          type,
            slang::SpecializationArg const* specializationArgs,
            SlangInt                        specializationArgCount,
            ISlangBlob**                    outDiagnostics = nullptr) override;
        SLANG_NO_THROW slang::TypeLayoutReflection* SLANG_MCALL getTypeLayout(
            slang::TypeReflection* type,
            SlangInt               targetIndex = 0,
            slang::LayoutRules     rules = slang::LayoutRules::Default,
            ISlangBlob**    outDiagnostics = nullptr) override;
        SLANG_NO_THROW SlangResult SLANG_MCALL createCompileRequest(
            SlangCompileRequest**   outCompileRequest) override;

        void addTarget(
            slang::TargetDesc const& desc);
        SlangResult addSearchPath(
            char const* path);
        SlangResult addPreprocessorDefine(
            char const* name,
            char const* value);
        SlangResult setMatrixLayoutMode(
            SlangMatrixLayoutMode mode);

            /// Create an initially-empty linkage
        Linkage(Session* session);

            /// Get the parent session for this linkage
        Session* getSessionImpl() { return m_session; }

        // Information on the targets we are being asked to
        // generate code for.
        List<RefPtr<TargetRequest>> targets;

        // Directories to search for `#include` files or `import`ed modules
        SearchDirectoryList searchDirectories;

        SearchDirectoryList const& getSearchDirectories() { return searchDirectories; }

        // Definitions to provide during preprocessing
        Dictionary<String, String> preprocessorDefinitions;

        // Source manager to help track files loaded
        SourceManager m_defaultSourceManager;
        SourceManager* m_sourceManager = nullptr;

        // Name pool for looking up names
        NamePool namePool;

        NamePool* getNamePool() { return &namePool; }

        // Modules that have been dynamically loaded via `import`
        //
        // This is a list of unique modules loaded, in the order they were encountered.
        List<RefPtr<LoadedModule> > loadedModulesList;

        // Map from the path (or uniqueIdentity if available) of a module file to its definition
        Dictionary<String, RefPtr<LoadedModule>> mapPathToLoadedModule;

        // Map from the logical name of a module to its definition
        Dictionary<Name*, RefPtr<LoadedModule>> mapNameToLoadedModules;

        // The resulting specialized IR module for each entry point request
        List<RefPtr<IRModule>> compiledModules;

        /// File system implementation to use when loading files from disk.
        ///
        /// If this member is `null`, a default implementation that tries
        /// to use the native OS filesystem will be used instead.
        ///
        ComPtr<ISlangFileSystem> fileSystem;

        /// The extended file system implementation. Will be set to a default implementation
        /// if fileSystem is nullptr. Otherwise it will either be fileSystem's interface, 
        /// or a wrapped impl that makes fileSystem operate as fileSystemExt
        ComPtr<ISlangFileSystemExt> fileSystemExt;

        ISlangFileSystemExt* getFileSystemExt() { return fileSystemExt; }

        /// Load a file into memory using the configured file system.
        ///
        /// @param path The path to attempt to load from
        /// @param outBlob A destination pointer to receive the loaded blob
        /// @returns A `SlangResult` to indicate success or failure.
        ///
        SlangResult loadFile(String const& path, ISlangBlob** outBlob);


        RefPtr<Expr> parseTypeString(String typeStr, RefPtr<Scope> scope);

        Type* specializeType(
            Type*           unspecializedType,
            Int             argCount,
            Type* const*    args,
            DiagnosticSink* sink);

            /// Add a mew target amd return its index.
        UInt addTarget(
            CodeGenTarget   target);

        RefPtr<Module> loadModule(
            Name*               name,
            const PathInfo&     filePathInfo,
            ISlangBlob*         fileContentsBlob,
            SourceLoc const&    loc,
            DiagnosticSink*     sink);

        void loadParsedModule(
            RefPtr<TranslationUnitRequest>  translationUnit,
            Name*                           name,
            PathInfo const&                 pathInfo);

            /// Load a module of the given name.
        Module* loadModule(String const& name);

        RefPtr<Module> findOrImportModule(
            Name*               name,
            SourceLoc const&    loc,
            DiagnosticSink*     sink);

        SourceManager* getSourceManager()
        {
            return m_sourceManager;
        }

            /// Override the source manager for the linakge.
            ///
            /// This is only used to install a temporary override when
            /// parsing stuff from strings (where we don't want to retain
            /// full source files for the parsed result).
            ///
            /// TODO: We should remove the need for this hack.
            ///
        void setSourceManager(SourceManager* sourceManager)
        {
            m_sourceManager = sourceManager;
        }

        void setFileSystem(ISlangFileSystem* fileSystem);

        /// The layout to use for matrices by default (row/column major)
        MatrixLayoutMode defaultMatrixLayoutMode = kMatrixLayoutMode_ColumnMajor;
        MatrixLayoutMode getDefaultMatrixLayoutMode() { return defaultMatrixLayoutMode; }

        DebugInfoLevel debugInfoLevel = DebugInfoLevel::None;

        OptimizationLevel optimizationLevel = OptimizationLevel::Default;

        bool m_useFalcorCustomSharedKeywordSemantics = false;

    private:
        Session* m_session = nullptr;

            /// Tracks state of modules currently being loaded.
            ///
            /// This information is used to diagnose cases where
            /// a user tries to recursively import the same module
            /// (possibly along a transitive chain of `import`s).
            ///
        struct ModuleBeingImportedRAII
        {
        public:
            ModuleBeingImportedRAII(
                Linkage*    linkage,
                Module*     module)
                : linkage(linkage)
                , module(module)
            {
                next = linkage->m_modulesBeingImported;
                linkage->m_modulesBeingImported = this;
            }

            ~ModuleBeingImportedRAII()
            {
                linkage->m_modulesBeingImported = next;
            }

            Linkage* linkage;
            Module* module;
            ModuleBeingImportedRAII* next;
        };

        // Any modules currently being imported will be listed here
        ModuleBeingImportedRAII* m_modulesBeingImported = nullptr;

            /// Is the given module in the middle of being imported?
        bool isBeingImported(Module* module);

        List<RefPtr<Type>> m_specializedTypes;
    };

        /// Shared functionality between front- and back-end compile requests.
        ///
        /// This is the base class for both `FrontEndCompileRequest` and
        /// `BackEndCompileRequest`, and allows a small number of parts of
        /// the compiler to be easily invocable from either front-end or
        /// back-end work.
        ///
    class CompileRequestBase : public RefObject
    {
        // TODO: We really shouldn't need this type in the long run.
        // The few places that rely on it should be refactored to just
        // depend on the underlying information (a linkage and a diagnostic
        // sink) directly.
        //
        // The flags to control dumping and validation of IR should be
        // moved to some kind of shared settings/options `struct` that
        // both front-end and back-end requests can store.

    public:
        Session* getSession();
        Linkage* getLinkage() { return m_linkage; }
        DiagnosticSink* getSink() { return m_sink; }
        SourceManager* getSourceManager() { return getLinkage()->getSourceManager(); }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }
        ISlangFileSystemExt* getFileSystemExt() { return getLinkage()->getFileSystemExt(); }
        SlangResult loadFile(String const& path, ISlangBlob** outBlob) { return getLinkage()->loadFile(path, outBlob); }

        bool shouldDumpIR = false;
        bool shouldValidateIR = false;

    protected:
        CompileRequestBase(
            Linkage*        linkage,
            DiagnosticSink* sink);

    private:
        Linkage* m_linkage = nullptr;
        DiagnosticSink* m_sink = nullptr;
    };

        /// A request to compile source code to an AST + IR.
    class FrontEndCompileRequest : public CompileRequestBase
    {
    public:
        FrontEndCompileRequest(
            Linkage*        linkage,
            DiagnosticSink* sink);

        int addEntryPoint(
            int                     translationUnitIndex,
            String const&           name,
            Profile                 entryPointProfile);

        // Translation units we are being asked to compile
        List<RefPtr<TranslationUnitRequest> > translationUnits;

        RefPtr<TranslationUnitRequest> getTranslationUnit(UInt index) { return translationUnits[index]; }

        // Compile flags to be shared by all translation units
        SlangCompileFlags compileFlags = 0;

        // If true then generateIR will serialize out IR, and serialize back in again. Making 
        // serialization a bottleneck or firewall between the front end and the backend
        bool useSerialIRBottleneck = false; 

        // If true will serialize and de-serialize with debug information
        bool verifyDebugSerialization = false;

        List<RefPtr<FrontEndEntryPointRequest>> m_entryPointReqs;

        List<RefPtr<FrontEndEntryPointRequest>> const& getEntryPointReqs() { return m_entryPointReqs; }
        UInt getEntryPointReqCount() { return m_entryPointReqs.getCount(); }
        FrontEndEntryPointRequest* getEntryPointReq(UInt index) { return m_entryPointReqs[index]; }

        // Directories to search for `#include` files or `import`ed modules
        // NOTE! That for now these search directories are not settable via the API
        // so the search directories on Linkage is used for #include as well as for modules.
        SearchDirectoryList searchDirectories;

        SearchDirectoryList const& getSearchDirectories() { return searchDirectories; }

        // Definitions to provide during preprocessing
        Dictionary<String, String> preprocessorDefinitions;

        void parseTranslationUnit(
            TranslationUnitRequest* translationUnit);

        // Perform primary semantic checking on all
        // of the translation units in the program
        void checkAllTranslationUnits();

        void generateIR();

        SlangResult executeActionsInner();

            /// Add a translation unit to be compiled.
            ///
            /// @param language The source language that the translation unit will use (e.g., `SourceLanguage::Slang`
            /// @param moduleName The name that will be used for the module compile from the translation unit.
            /// @return The zero-based index of the translation unit in this compile request.
        int addTranslationUnit(SourceLanguage language, Name* moduleName);

            /// Add a translation unit to be compiled.
            ///
            /// @param language The source language that the translation unit will use (e.g., `SourceLanguage::Slang`
            /// @return The zero-based index of the translation unit in this compile request.
            ///
            /// The module name for the translation unit will be automatically generated.
            /// If all translation units in a compile request use automatically generated
            /// module names, then they are guaranteed not to conflict with one another.
            ///
        int addTranslationUnit(SourceLanguage language);

        void addTranslationUnitSourceFile(
            int             translationUnitIndex,
            SourceFile*     sourceFile);

        void addTranslationUnitSourceBlob(
            int             translationUnitIndex,
            String const&   path,
            ISlangBlob*     sourceBlob);

        void addTranslationUnitSourceString(
            int             translationUnitIndex,
            String const&   path,
            String const&   source);

        void addTranslationUnitSourceFile(
            int             translationUnitIndex,
            String const&   path);

            /// Get a component type that represents the global scope of the compile request.
        ComponentType* getGlobalComponentType()  { return m_globalComponentType; }

            /// Get a component type that represents the global scope of the compile request, plus the requested entry points.
        ComponentType* getGlobalAndEntryPointsComponentType() { return m_globalAndEntryPointsComponentType; }

    private:
            /// A component type that includes only the global scopes of the translation unit(s) that were compiled.
        RefPtr<ComponentType> m_globalComponentType;

            /// A component type that extends the global scopes with all of the entry points that were specified.
        RefPtr<ComponentType> m_globalAndEntryPointsComponentType;
    };

        /// A "legacy" program composes multiple translation units from a single compile request,
        /// and takes care to treat global declarations of the same name from different translation
        /// units as representing the "same" parameter.
        ///
        /// TODO: This type only exists to support a single requirement: that multiple translation
        /// units can be compiled in one pass and be guaranteed that the "same" parameter declared
        /// in different translation units (hence different modules) will get the same layout.
        /// This feature should be deprecated and removed as soon as possible, since the complexity
        /// it creates in the codebase is not justified by its limited utility.
        ///
    class LegacyProgram : public ComponentType
    {
    public:
        LegacyProgram(
            Linkage*                                    linkage,
            List<RefPtr<TranslationUnitRequest>> const& translationUnits,
            DiagnosticSink*                             sink);

        Index getTranslationUnitCount() { return m_translationUnits.getCount(); }
        RefPtr<TranslationUnitRequest> getTranslationUnit(Index index) { return m_translationUnits[index]; }

        Index getEntryPointCount() SLANG_OVERRIDE { return 0; }
        RefPtr<EntryPoint> getEntryPoint(Index index) SLANG_OVERRIDE { SLANG_UNUSED(index); return nullptr; }

        Index getShaderParamCount() SLANG_OVERRIDE { return m_shaderParams.getCount(); }
        GlobalShaderParamInfo getShaderParam(Index index) SLANG_OVERRIDE { return m_shaderParams[index]; }

        Index getSpecializationParamCount() SLANG_OVERRIDE { return m_specializationParams.getCount(); }
        SpecializationParam const& getSpecializationParam(Index index) SLANG_OVERRIDE { return m_specializationParams[index]; }

        Index getRequirementCount() SLANG_OVERRIDE;
        RefPtr<ComponentType> getRequirement(Index index) SLANG_OVERRIDE;

        List<Module*> const& getModuleDependencies() SLANG_OVERRIDE { return m_moduleDependencies.getModuleList(); }
        List<String> const& getFilePathDependencies() SLANG_OVERRIDE { return m_fileDependencies.getFilePathList(); }

    protected:
        void acceptVisitor(ComponentTypeVisitor* visitor, SpecializationInfo* specializationInfo) SLANG_OVERRIDE;

        RefPtr<SpecializationInfo> _validateSpecializationArgsImpl(
            SpecializationArg const*    args,
            Index                       argCount,
            DiagnosticSink*             sink) SLANG_OVERRIDE;

    private:
        void _collectShaderParams(DiagnosticSink* sink);

        List<RefPtr<TranslationUnitRequest>> m_translationUnits;

        List<EntryPoint*> m_entryPoints;
        List<GlobalShaderParamInfo> m_shaderParams;
        List<ComponentType*> m_requirements;
        SpecializationParams m_specializationParams;
        ModuleDependencyList m_moduleDependencies;
        FilePathDependencyList m_fileDependencies;
    };

        /// A visitor for use with `ComponentType`s, allowing dispatch over the concrete subclasses.
    class ComponentTypeVisitor
    {
    public:
        // The following methods should be overriden in a concrete subclass
        // to customize how it acts on each of the concrete types of component.
        //
        // In cases where the application wants to simply "recurse" on a
        // composite, specialized, or legacy component type it can use
        // the `visitChildren` methods below.
        //
        virtual void visitEntryPoint(EntryPoint* entryPoint, EntryPoint::EntryPointSpecializationInfo* specializationInfo) = 0;
        virtual void visitModule(Module* module, Module::ModuleSpecializationInfo* specializationInfo) = 0;
        virtual void visitComposite(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) = 0;
        virtual void visitSpecialized(SpecializedComponentType* specialized) = 0;
        virtual void visitLegacy(LegacyProgram* legacy, CompositeComponentType::CompositeSpecializationInfo* specializationInfo) = 0;

    protected:
        // These helpers can be used to recurse into the logical children of a
        // component type, and are useful for the common case where a visitor
        // only cares about a few leaf cases.
        //
        // Note that for a `LegacyProgram` the "children" in this case are the
        // `Module`s of the translation units that make up the legacy program.
        // In some cases this is what is desired, but in others it is incorrect
        // to treat a legacy program as a composition of modules, and instead
        // it should be treated directly as a leaf case. Clients should make
        // an informed decision based on an understanding of what `LegacyProgram` is used for.
        //
        void visitChildren(CompositeComponentType* composite, CompositeComponentType::CompositeSpecializationInfo* specializationInfo);
        void visitChildren(SpecializedComponentType* specialized);
        void visitChildren(LegacyProgram* legacy, CompositeComponentType::CompositeSpecializationInfo* specializationInfo);
    };

        /// A `TargetProgram` represents a `ComponentType` specialized for a particular `TargetRequest`
        ///
        /// TODO: This should probably be renamed to `TargetComponentType`.
        ///
        /// By binding a component type to a specific target, a `TargetProgram` allows
        /// for things like layout to be computed, that fundamentally depend on
        /// the choice of target.
        ///
        /// A `TargetProgram` handles request for compiled kernel code for
        /// entry point functions. In practice, kernel code can only be
        /// correctly generated when the underlying `ComponentType` is "fully linked"
        /// (has no remaining unsatisfied requirements).
        ///
    class TargetProgram : public RefObject
    {
    public:
        TargetProgram(
            ComponentType*  componentType,
            TargetRequest*  targetReq);

            /// Get the underlying program
        ComponentType* getProgram() { return m_program; }

            /// Get the underlying target
        TargetRequest* getTargetReq() { return m_targetReq; }

            /// Get the layout for the program on the target.
            ///
            /// If this is the first time the layout has been
            /// requested, report any errors that arise during
            /// layout to the given `sink`.
            ///
        ProgramLayout* getOrCreateLayout(DiagnosticSink* sink);

            /// Get the layout for the program on the taarget.
            ///
            /// This routine assumes that `getOrCreateLayout`
            /// has already been called previously.
            ///
        ProgramLayout* getExistingLayout()
        {
            SLANG_ASSERT(m_layout);
            return m_layout;
        }

            /// Get the compiled code for an entry point on the target.
            ///
            /// If this is the first time that code generation has
            /// been requested, report any errors that arise during
            /// code generation to the given `sink`.
            ///
        CompileResult& getOrCreateEntryPointResult(Int entryPointIndex, DiagnosticSink* sink);

            /// Get the compiled code for an entry point on the target.
            ///
            /// This routine assumes that `getOrCreateEntryPointResult`
            /// has already been called previously.
            ///
        CompileResult& getExistingEntryPointResult(Int entryPointIndex)
        {
            return m_entryPointResults[entryPointIndex];
        }


            /// Internal helper for `getOrCreateEntryPointResult`.
            ///
            /// This is used so that command-line and API-based
            /// requests for code can bottleneck through the same place.
            ///
            /// Shouldn't be called directly by most code.
            ///
        CompileResult& _createEntryPointResult(
            Int                     entryPointIndex,
            BackEndCompileRequest*  backEndRequest,
            EndToEndCompileRequest* endToEndRequest);

    private:
        // The program being compiled or laid out
        ComponentType* m_program;

        // The target that code/layout will be generated for
        TargetRequest* m_targetReq;

        // The computed layout, if it has been generated yet
        RefPtr<ProgramLayout> m_layout;

        // Generated compile results for each entry point
        // in the parent `Program` (indexing matches
        // the order they are given in the `Program`)
        List<CompileResult> m_entryPointResults;
    };

        /// A request to generate code for a program
    class BackEndCompileRequest : public CompileRequestBase
    {
    public:
        BackEndCompileRequest(
            Linkage*        linkage,
            DiagnosticSink* sink,
            ComponentType*  program = nullptr);

        // Should we dump intermediate results along the way, for debugging?
        bool shouldDumpIntermediates = false;

        // How should `#line` directives be emitted (if at all)?
        LineDirectiveMode lineDirectiveMode = LineDirectiveMode::Default;

        LineDirectiveMode getLineDirectiveMode() { return lineDirectiveMode; }

        ComponentType* getProgram() { return m_program; }
        void setProgram(ComponentType* program) { m_program = program; }

        // Should R/W images without explicit formats be assumed to have "unknown" format?
        //
        // The default behavior is to make a best-effort guess as to what format is intended.
        //
        bool useUnknownImageFormatAsDefault = false;

    private:
        RefPtr<ComponentType> m_program;
    };

        /// A compile request that spans the front and back ends of the compiler
        ///
        /// This is what the command-line `slangc` uses, as well as the legacy
        /// C API. It ties together the functionality of `Linkage`,
        /// `FrontEndCompileRequest`, and `BackEndCompileRequest`, plus a small
        /// number of additional features that primarily make sense for
        /// command-line usage.
        ///
    class EndToEndCompileRequest : public RefObject
    {
    public:
        EndToEndCompileRequest(
            Session* session);

        EndToEndCompileRequest(
            Linkage* linkage);

        // What container format are we being asked to generate?
        //
        // Note: This field is unused except by the options-parsing
        // logic; it exists to support wriiting out binary modules
        // once that feature is ready.
        //
        ContainerFormat containerFormat = ContainerFormat::None;

        // Path to output container to
        //
        // Note: This field exists to support wriiting out binary modules
        // once that feature is ready.
        //
        String containerOutputPath;

        // Should we just pass the input to another compiler?
        PassThroughMode passThrough = PassThroughMode::None;

            /// Source code for the specialization arguments to use for the global specialization parameters of the program.
        List<String> globalSpecializationArgStrings;

        bool shouldSkipCodegen = false;

        // Are we being driven by the command-line `slangc`, and should act accordingly?
        bool isCommandLineCompile = false;

        String mDiagnosticOutput;

            /// A blob holding the diagnostic output
        ComPtr<ISlangBlob> diagnosticOutputBlob;

            /// Per-entry-point information not tracked by other compile requests
        class EntryPointInfo : public RefObject
        {
        public:
                /// Source code for the specialization arguments to use for the specialization parameters of the entry point.
            List<String> specializationArgStrings;
        };
        List<EntryPointInfo> entryPoints;

            /// Per-target information only needed for command-line compiles
        class TargetInfo : public RefObject
        {
        public:
            // Requested output paths for each entry point.
            // An empty string indices no output desired for
            // the given entry point.
            Dictionary<Int, String> entryPointOutputPaths;
        };
        Dictionary<TargetRequest*, RefPtr<TargetInfo>> targetInfos;

        Linkage* getLinkage() { return m_linkage; }

        int addEntryPoint(
            int                     translationUnitIndex,
            String const&           name,
            Profile                 profile,
            List<String> const &    genericTypeNames);

        void setWriter(WriterChannel chan, ISlangWriter* writer);
        ISlangWriter* getWriter(WriterChannel chan) const { return m_writers[int(chan)]; }

        SlangResult executeActionsInner();
        SlangResult executeActions();

        Session* getSession() { return m_session; }
        DiagnosticSink* getSink() { return &m_sink; }
        NamePool* getNamePool() { return getLinkage()->getNamePool(); }

        FrontEndCompileRequest* getFrontEndReq() { return m_frontEndReq; }
        BackEndCompileRequest* getBackEndReq() { return m_backEndReq; }

        ComponentType* getUnspecializedGlobalComponentType() { return getFrontEndReq()->getGlobalComponentType(); }
        ComponentType* getUnspecializedGlobalAndEntryPointsComponentType()
        {
            return getFrontEndReq()->getGlobalAndEntryPointsComponentType();
        }

        ComponentType* getSpecializedGlobalComponentType() { return m_specializedGlobalComponentType; }
        ComponentType* getSpecializedGlobalAndEntryPointsComponentType() { return m_specializedGlobalAndEntryPointsComponentType; }

    private:
        void init();

        Session*                        m_session = nullptr;
        RefPtr<Linkage>                 m_linkage;
        DiagnosticSink                  m_sink;
        RefPtr<FrontEndCompileRequest>  m_frontEndReq;
        RefPtr<ComponentType>           m_specializedGlobalComponentType;
        RefPtr<ComponentType>           m_specializedGlobalAndEntryPointsComponentType;
        RefPtr<BackEndCompileRequest>   m_backEndReq;

        // For output
        ComPtr<ISlangWriter> m_writers[SLANG_WRITER_CHANNEL_COUNT_OF];
    };

    void generateOutput(
        BackEndCompileRequest* compileRequest);

    void generateOutput(
        EndToEndCompileRequest* compileRequest);

    // Helper to dump intermediate output when debugging
    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        void const*     data,
        size_t          size,
        CodeGenTarget   target);
    void maybeDumpIntermediate(
        BackEndCompileRequest* compileRequest,
        char const*     text,
        CodeGenTarget   target);

    /* Returns SLANG_OK if a codeGen target is available. */
    SlangResult checkCompileTargetSupport(Session* session, CodeGenTarget target);
    /* Returns SLANG_OK if pass through support is available */
    SlangResult checkExternalCompilerSupport(Session* session, PassThroughMode passThrough);

    /* Report an error appearing from external compiler to the diagnostic sink error to the diagnostic sink.
    @param compilerName The name of the compiler the error came for (or nullptr if not known)
    @param res Result associated with the error. The error code will be reported. (Can take HRESULT - and will expand to string if known)
    @param diagnostic The diagnostic string associated with the compile failure
    @param sink The diagnostic sink to report to */
    void reportExternalCompileError(const char* compilerName, SlangResult res, const UnownedStringSlice& diagnostic, DiagnosticSink* sink);

    /* Determines a suitable filename to identify the input for a given entry point being compiled.
    If the end-to-end compile is a pass-through case, will attempt to find the (unique) source file
    pathname for the translation unit containing the entry point at `entryPointIndex.
    If the compilation is not in a pass-through case, then always returns `"slang-generated"`.
    @param endToEndReq The end-to-end compile request which might be using pass-through copmilation
    @param entryPointIndex The index of the entry point to compute a filename for.
    @return the appropriate source filename */
    String calcSourcePathForEntryPoint(EndToEndCompileRequest* endToEndReq, UInt entryPointIndex);

    struct TypeCheckingCache;
    //

    class Session : public RefObject, public slang::IGlobalSession
    {
    public:
        SLANG_REF_OBJECT_IUNKNOWN_ALL

        ISlangUnknown* getInterface(const Guid& guid);

            /** Create a new linkage.
            */
        SLANG_NO_THROW SlangResult SLANG_MCALL createSession(
            slang::SessionDesc const&  desc,
            slang::ISession**          outSession) override;

        SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(
            char const*     name) override;

        SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPath(
            SlangPassThrough passThrough,
            char const* path) override;

        SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPrelude(
            SlangPassThrough inPassThrough,
            char const* prelude) override;

        SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() override;

        enum class SharedLibraryFuncType
        {
            Glslang_Compile,
            Fxc_D3DCompile,
            Fxc_D3DDisassemble,
            Dxc_DxcCreateInstance,
            CountOf,
        };

        //

        RefPtr<Scope>   baseLanguageScope;
        RefPtr<Scope>   coreLanguageScope;
        RefPtr<Scope>   hlslLanguageScope;
        RefPtr<Scope>   slangLanguageScope;

        List<RefPtr<ModuleDecl>> loadedModuleCode;

        SourceManager   builtinSourceManager;

        SourceManager* getBuiltinSourceManager() { return &builtinSourceManager; }

        // Name pool stuff for unique-ing identifiers

        RootNamePool rootNamePool;
        NamePool namePool;

        RootNamePool* getRootNamePool() { return &rootNamePool; }
        NamePool* getNamePool() { return &namePool; }
        Name* getNameObj(String name) { return namePool.getName(name); }
        Name* tryGetNameObj(String name) { return namePool.tryGetName(name); }
        //

        // Generated code for stdlib, etc.
        String stdlibPath;
        String coreLibraryCode;
        String slangLibraryCode;
        String hlslLibraryCode;
        String glslLibraryCode;

        String getStdlibPath();
        String getCoreLibraryCode();
        String getHLSLLibraryCode();

        // Basic types that we don't want to re-create all the time
        RefPtr<Type> errorType;
        RefPtr<Type> initializerListType;
        RefPtr<Type> overloadedType;
        RefPtr<Type> constExprRate;
        RefPtr<Type> irBasicBlockType;

        RefPtr<Type> stringType;
        RefPtr<Type> enumTypeType;

        RefPtr<CPPCompilerSet> cppCompilerSet;                                          ///< Information about available C/C++ compilers. null unless information is requested (because slow)

        ComPtr<ISlangSharedLibraryLoader> sharedLibraryLoader;                          ///< The shared library loader (never null)
        ComPtr<ISlangSharedLibrary> sharedLibraries[int(SharedLibraryType::CountOf)];   ///< The loaded shared libraries
        SlangFuncPtr sharedLibraryFunctions[int(SharedLibraryFuncType::CountOf)];

        Dictionary<int, RefPtr<Type>> builtinTypes;
        Dictionary<String, Decl*> magicDecls;

        void initializeTypes();

        Type* getBoolType();
        Type* getHalfType();
        Type* getFloatType();
        Type* getDoubleType();
        Type* getIntType();
        Type* getInt64Type();
        Type* getUIntType();
        Type* getUInt64Type();
        Type* getVoidType();
        Type* getBuiltinType(BaseType flavor);

        Type* getInitializerListType();
        Type* getOverloadedType();
        Type* getErrorType();
        Type* getStringType();

        Type* getEnumTypeType();

        // Construct the type `Ptr<valueType>`, where `Ptr`
        // is looked up as a builtin type.
        RefPtr<PtrType> getPtrType(RefPtr<Type> valueType);

        // Construct the type `Out<valueType>`
        RefPtr<OutType> getOutType(RefPtr<Type> valueType);

        // Construct the type `InOut<valueType>`
        RefPtr<InOutType> getInOutType(RefPtr<Type> valueType);

        // Construct the type `Ref<valueType>`
        RefPtr<RefType> getRefType(RefPtr<Type> valueType);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the actual type name for the pointer type is given by `ptrTypeName`
        RefPtr<PtrTypeBase> getPtrType(RefPtr<Type> valueType, char const* ptrTypeName);

        // Construct a pointer type like `Ptr<valueType>`, but where
        // the generic declaration for the pointer type is `genericDecl`
        RefPtr<PtrTypeBase> getPtrType(RefPtr<Type> valueType, GenericDecl* genericDecl);

        RefPtr<ArrayExpressionType> getArrayType(
            Type*   elementType,
            IntVal* elementCount);

        RefPtr<VectorExpressionType> getVectorType(
            RefPtr<Type>    elementType,
            RefPtr<IntVal>  elementCount);

        SyntaxClass<RefObject> findSyntaxClass(Name* name);

        Dictionary<Name*, SyntaxClass<RefObject> > mapNameToSyntaxClass;

        // cache used by type checking, implemented in check.cpp
        TypeCheckingCache* typeCheckingCache = nullptr;
        TypeCheckingCache* getTypeCheckingCache();
        void destroyTypeCheckingCache();
        //

            /// Will try to load the library by specified name (using the set loader), if not one already available.
        ISlangSharedLibrary* getOrLoadSharedLibrary(SharedLibraryType type, DiagnosticSink* sink);
            /// Will unload the specified shared library if it's currently loaded 
        void setSharedLibrary(SharedLibraryType type, ISlangSharedLibrary* library);

            /// Gets a shared library by type, or null if not loaded
        ISlangSharedLibrary* getSharedLibrary(SharedLibraryType type) const { return sharedLibraries[int(type)]; }

        SlangFuncPtr getSharedLibraryFunc(SharedLibraryFuncType type, DiagnosticSink* sink);

            /// Get the downstream compiler prelude
        const String& getDownstreamCompilerPrelude(PassThroughMode mode) { return m_downstreamCompilerPreludes[int(mode)]; }

            /// Finds out what compilers are present and caches the result
        CPPCompilerSet* requireCPPCompilerSet();

        Session();

        void addBuiltinSource(
            RefPtr<Scope> const&    scope,
            String const&           path,
            String const&           source);
        ~Session();

    private:
            /// Linkage used for all built-in (stdlib) code.
        RefPtr<Linkage> m_builtinLinkage;

        String m_downstreamCompilerPaths[int(PassThroughMode::CountOf)];              ///< Paths for each pass through
        String m_downstreamCompilerPreludes[int(PassThroughMode::CountOf)];             ///< Prelude for each type of target
    };


//
// The following functions are utilties to convert between
// matching "external" (public API) and "internal" (implementation)
// types. They are favored over explicit casts because they
// help avoid making incorrect conversions (e.g., when using
// `reinterpret_cast` or C-style casts), and because they
// abstract over the conversion required for each pair of types.
//

SLANG_FORCE_INLINE slang::IGlobalSession* asExternal(Session* session)
{
    return static_cast<slang::IGlobalSession*>(session);
}

SLANG_FORCE_INLINE Session* asInternal(slang::IGlobalSession* session)
{
    return static_cast<Session*>(session);
}

SLANG_FORCE_INLINE slang::ISession* asExternal(Linkage* linkage)
{
    return static_cast<slang::ISession*>(linkage);
}

SLANG_FORCE_INLINE Module* asInternal(slang::IModule* module)
{
    return static_cast<Module*>(module);
}

SLANG_FORCE_INLINE slang::IModule* asExternal(Module* module)
{
    return static_cast<slang::IModule*>(module);
}

SLANG_FORCE_INLINE ComponentType* asInternal(slang::IComponentType* componentType)
{
    return static_cast<ComponentType*>(componentType);
}

SLANG_FORCE_INLINE slang::IComponentType* asExternal(ComponentType* componentType)
{
    return static_cast<slang::IComponentType*>(componentType);
}

SLANG_FORCE_INLINE slang::ProgramLayout* asExternal(ProgramLayout* programLayout)
{
    return (slang::ProgramLayout*) programLayout;
}

SLANG_FORCE_INLINE Type* asInternal(slang::TypeReflection* type)
{
    return reinterpret_cast<Type*>(type);
}

SLANG_FORCE_INLINE slang::TypeReflection* asExternal(Type* type)
{
    return reinterpret_cast<slang::TypeReflection*>(type);
}

SLANG_FORCE_INLINE TypeLayout* asInternal(slang::TypeLayoutReflection* type)
{
    return reinterpret_cast<TypeLayout*>(type);
}

SLANG_FORCE_INLINE slang::TypeLayoutReflection* asExternal(TypeLayout* type)
{
    return reinterpret_cast<slang::TypeLayoutReflection*>(type);
}

SLANG_FORCE_INLINE SlangCompileRequest* asExternal(EndToEndCompileRequest* request)
{
    return reinterpret_cast<SlangCompileRequest*>(request);
}

SLANG_FORCE_INLINE EndToEndCompileRequest* asInternal(SlangCompileRequest* request)
{
    return reinterpret_cast<EndToEndCompileRequest*>(request);
}

}

#endif
