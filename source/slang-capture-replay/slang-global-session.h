#ifndef SLANG_GLOBAL_SESSION_H
#define SLANG_GLOBAL_SESSION_H

#include "../../slang-com-ptr.h"
#include "../../slang.h"
#include "../../slang-com-helper.h"
#include "../core/slang-smart-pointer.h"

namespace SlangCapture
{
    using namespace Slang;

    class GlobalSessionCapture : public RefObject, public slang::IGlobalSession
    {
        public:
            explicit GlobalSessionCapture(slang::IGlobalSession* session);
            virtual ~GlobalSessionCapture();

            SLANG_REF_OBJECT_IUNKNOWN_ALL

            ISlangUnknown* getInterface(const Guid& guid);

            // slang::IGlobalSession
            SLANG_NO_THROW SlangResult SLANG_MCALL createSession(slang::SessionDesc const&  desc, slang::ISession** outSession) override;
            SLANG_NO_THROW SlangProfileID SLANG_MCALL findProfile(char const* name) override;
            SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPath(SlangPassThrough passThrough, char const* path) override;
            SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerPrelude(SlangPassThrough inPassThrough, char const* prelude) override;
            SLANG_NO_THROW void SLANG_MCALL getDownstreamCompilerPrelude(SlangPassThrough inPassThrough, ISlangBlob** outPrelude) override;
            SLANG_NO_THROW const char* SLANG_MCALL getBuildTagString() override;
            SLANG_NO_THROW SlangResult SLANG_MCALL setDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage, SlangPassThrough defaultCompiler) override;
            SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDefaultDownstreamCompiler(SlangSourceLanguage sourceLanguage) override;

            SLANG_NO_THROW void SLANG_MCALL setLanguagePrelude(SlangSourceLanguage inSourceLanguage, char const* prelude) override;
            SLANG_NO_THROW void SLANG_MCALL getLanguagePrelude(SlangSourceLanguage inSourceLanguage, ISlangBlob** outPrelude) override;

            SLANG_NO_THROW SlangResult SLANG_MCALL createCompileRequest(slang::ICompileRequest** outCompileRequest) override;

            SLANG_NO_THROW void SLANG_MCALL addBuiltins(char const* sourcePath, char const* sourceString) override;
            SLANG_NO_THROW void SLANG_MCALL setSharedLibraryLoader(ISlangSharedLibraryLoader* loader) override;
            SLANG_NO_THROW ISlangSharedLibraryLoader* SLANG_MCALL getSharedLibraryLoader() override;
            SLANG_NO_THROW SlangResult SLANG_MCALL checkCompileTargetSupport(SlangCompileTarget target) override;
            SLANG_NO_THROW SlangResult SLANG_MCALL checkPassThroughSupport(SlangPassThrough passThrough) override;

            SLANG_NO_THROW SlangResult SLANG_MCALL compileStdLib(slang::CompileStdLibFlags flags) override;
            SLANG_NO_THROW SlangResult SLANG_MCALL loadStdLib(const void* stdLib, size_t stdLibSizeInBytes) override;
            SLANG_NO_THROW SlangResult SLANG_MCALL saveStdLib(SlangArchiveType archiveType, ISlangBlob** outBlob) override;

            SLANG_NO_THROW SlangCapabilityID SLANG_MCALL findCapability(char const* name) override;

            SLANG_NO_THROW void SLANG_MCALL setDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target, SlangPassThrough compiler) override;
            SLANG_NO_THROW SlangPassThrough SLANG_MCALL getDownstreamCompilerForTransition(SlangCompileTarget source, SlangCompileTarget target) override;
            SLANG_NO_THROW void SLANG_MCALL getCompilerElapsedTime(double* outTotalTime, double* outDownstreamTime) override;

            SLANG_NO_THROW SlangResult SLANG_MCALL setSPIRVCoreGrammar(char const* jsonPath) override;

            SLANG_NO_THROW SlangResult SLANG_MCALL parseCommandLineArguments(
                int argc, const char* const* argv, slang::SessionDesc* outSessionDesc, ISlangUnknown** outAllocation) override;

            SLANG_NO_THROW SlangResult SLANG_MCALL getSessionDescDigest(slang::SessionDesc* sessionDesc, ISlangBlob** outBlob) override;

        private:
            SLANG_FORCE_INLINE slang::IGlobalSession* asExternal(GlobalSessionCapture* session)
            {
                return static_cast<slang::IGlobalSession*>(session);
            }

            Slang::ComPtr<slang::IGlobalSession> m_actualGlobalSession;
    };
} // namespace Slang

#endif
