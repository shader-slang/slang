#include "slang-tint-compiler.h"

#include "slang-artifact-associated-impl.h"

#include "../slang-tint/slang-tint.h"

namespace Slang
{

    class TintDownstreamCompiler : public DownstreamCompilerBase
    {

    public:

        // IDownstreamCompiler
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
        compile(const CompileOptions& options, IArtifact** outResult) SLANG_OVERRIDE;
        virtual SLANG_NO_THROW bool SLANG_MCALL
        canConvert(const ArtifactDesc& from, const ArtifactDesc& to) SLANG_OVERRIDE;
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
        convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact) SLANG_OVERRIDE;
        virtual SLANG_NO_THROW bool SLANG_MCALL
        isFileBased() SLANG_OVERRIDE { return false; }
        virtual SLANG_NO_THROW SlangResult SLANG_MCALL
        getVersionString(slang::IBlob** outVersionString) SLANG_OVERRIDE;

        SlangResult compile(IArtifact *const sourceArtifact, IArtifact** outArtifact);

        SlangResult init(ISlangSharedLibrary* library);

    protected:

        ComPtr<ISlangSharedLibrary> m_sharedLibrary;

    private:

        tint_CompileFunc m_compile;
    };

    SlangResult TintDownstreamCompiler::init(ISlangSharedLibrary* library)
    {
        m_compile = (tint_CompileFunc)library->findFuncByName("tint_compile");
        if(m_compile == nullptr)
        {
            return SLANG_FAIL;
        }

        m_sharedLibrary = library;

        m_desc = Desc(SLANG_PASS_THROUGH_TINT);

        return SLANG_OK;
    }

    SlangResult
    TintDownstreamCompilerUtil::locateCompilers(
        const String& path,
        ISlangSharedLibraryLoader* loader,
        DownstreamCompilerSet* set
    )
    {
        ComPtr<ISlangSharedLibrary> library;
        SLANG_RETURN_ON_FAIL(
            DownstreamCompilerUtil::loadSharedLibrary(path, loader, nullptr, "slang-tint", library)
        );
        SLANG_ASSERT(library);

        TintDownstreamCompiler *const compiler = new TintDownstreamCompiler();
        SLANG_RETURN_ON_FAIL(compiler->init(library));

        set->addCompiler(ComPtr<IDownstreamCompiler>(compiler));
        return SLANG_OK;
    }

    SlangResult TintDownstreamCompiler::compile(const CompileOptions& options, IArtifact** outArtifact)
    {
        IArtifact *const sourceArtifact {options.sourceArtifacts[0]};
        return compile(sourceArtifact, outArtifact);
    }

    SlangResult TintDownstreamCompiler::compile(IArtifact *const sourceArtifact, IArtifact** outArtifact)
    {
        tint_CompileRequest req {};

        if(sourceArtifact == nullptr)
            return SLANG_FAIL;

        ComPtr<ISlangBlob> sourceBlob;
        SLANG_RETURN_FALSE_ON_FAIL(sourceArtifact->loadBlob(ArtifactKeep::Yes, sourceBlob.writeRef()));

        size_t const spirvCodeSize {sourceBlob->getBufferSize()};
        SLANG_ASSERT((spirvCodeSize % sizeof(uint32_t)) == size_t{0});
        size_t const spirvCodeLength {spirvCodeSize/sizeof(uint32_t)};

        req.spirvCode = static_cast<uint32_t const*>(sourceBlob->getBufferPointer());
        req.spirvCodeLength = spirvCodeLength;

        int const result {m_compile(&req)};
        if(result != 0)
            return SLANG_FAIL;

        ComPtr<ISlangBlob> stringBlob {
            StringBlob::create(String(req.resultBuffer, req.resultBuffer + req.resultBufferSize))
        };
        ::free((void*)req.resultBuffer);
        req.resultBuffer = nullptr;

        ComPtr<IArtifact> resultArtifact =
            ArtifactUtil::createArtifactForCompileTarget(SlangCompileTarget::SLANG_WGSL);
        resultArtifact->addRepresentationUnknown((ISlangBlob*)stringBlob.get());

        *outArtifact = resultArtifact.detach();
        return SLANG_OK;
    }

    bool TintDownstreamCompiler::canConvert(const ArtifactDesc& from, const ArtifactDesc& to)
    {
        return (from.payload == ArtifactPayload::SPIRV) && (to.payload == ArtifactPayload::WGSL);
    }

    SlangResult TintDownstreamCompiler::convert(IArtifact* from, const ArtifactDesc& to, IArtifact** outArtifact)
    {
        if(!canConvert(from->getDesc(), to))
            return SLANG_FAIL;
        return compile(from, outArtifact);
    }

    SlangResult TintDownstreamCompiler::getVersionString(slang::IBlob** /* outVersionString */)
    {
        // We just use Tint at whatever version is in Dawn ToT, so nobody should depend on the particular version
        // at the moment.
        return SLANG_FAIL;
    }

}
