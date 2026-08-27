// unit-test-nvvm-compiler.cpp

#include "unit-test-nvvm-support.h"

// Exercise the public lazy-discovery path, not just the locator in isolation. This catches a new
// pass-through enum being added without registering its default downstream compiler locator.
SLANG_UNIT_TEST(nvvmPassThroughDiscoversInjectedLibrary)
{
    gFakeNVVM.reset();
    {
        ComPtr<slang::IGlobalSession> globalSession;
        SLANG_CHECK_ABORT(
            slang_createGlobalSession(SLANG_API_VERSION, globalSession.writeRef()) == SLANG_OK);
        ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
        globalSession->setSharedLibraryLoader(loader);

        SLANG_CHECK(globalSession->checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM) == SLANG_OK);
        int major = -1;
        int minor = -1;
        SLANG_CHECK(
            globalSession->getDownstreamCompilerVersion(SLANG_PASS_THROUGH_NVVM, &major, &minor) ==
            SLANG_OK);
        SLANG_CHECK(major == 2);
        SLANG_CHECK(minor == 0);
        SLANG_CHECK(gFakeNVVM.successfulLoadCount == 1);
        SLANG_CHECK(gFakeNVVM.loadedPath == "nvvm");
    }
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmLocatorRejectsMissingRequiredSymbols)
{
    gFakeNVVM.reset();
    for (const char* missingSymbol : kRequiredSymbols)
    {
        gFakeNVVM.missingSymbol = missingSymbol;
        {
            ComPtr<ISlangSharedLibraryLoader> loader(new FakeNVVMLoader);
            RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
            SlangResult result = NVVMDownstreamCompilerUtil::locateCompilers(String(), loader, set);
            SLANG_CHECK(SLANG_FAILED(result));
            SLANG_CHECK(result != SLANG_E_NOT_FOUND);
            SLANG_CHECK(!set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
        }
        SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    }
}

SLANG_UNIT_TEST(nvvmLocatorAcceptsMissingOptionalSymbols)
{
    gFakeNVVM.reset();
    gFakeNVVM.omitOptionalSymbols = true;
    {
        RefPtr<DownstreamCompilerSet> set;
        IDownstreamCompiler* compiler = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
        SLANG_CHECK(compiler != nullptr);
        SLANG_CHECK(compiler->getDesc().type == SLANG_PASS_THROUGH_NVVM);
        SLANG_CHECK(compiler->getDesc().version == SemanticVersion(2, 0));
    }
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
}

SLANG_UNIT_TEST(nvvmLocatorRanksNumericCandidates)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    gFakeNVVM.reset();
    TempDirectory tempDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(tempDirectory)));

#if SLANG_WINDOWS_FAMILY
    const String lowerPath = Path::combine(tempDirectory.path, "nvvm64_90_0.dll");
    const String higherPath = Path::combine(tempDirectory.path, "nvvm64_100_0.dll");
    const String expectedLoadPath = Path::getPathWithoutExt(higherPath);
#else
    const String lowerPath = Path::combine(tempDirectory.path, "libnvvm.so.9");
    const String higherPath = Path::combine(tempDirectory.path, "libnvvm.so.10");
    const String expectedLoadPath = higherPath;
#endif
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(lowerPath, String())));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(higherPath, String())));

    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        NVVMDownstreamCompilerUtil::locateCompilers(tempDirectory.path, loader, set)));
    SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    SLANG_CHECK(recordingLoader->loadRequests[0] == expectedLoadPath);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmLocatorNormalizesDecoratedExplicitFile)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY || SLANG_APPLE_FAMILY
    gFakeNVVM.reset();
    TempDirectory tempDirectory;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(tempDirectory)));

#if SLANG_WINDOWS_FAMILY
    const String decoratedPath = Path::combine(tempDirectory.path, "nvvm64_100_0.dll");
    const String expectedLoadPath = Path::getPathWithoutExt(decoratedPath);
#elif SLANG_LINUX_FAMILY
    const String decoratedPath = Path::combine(tempDirectory.path, "libnvvm.so");
    const String expectedLoadPath = Path::combine(tempDirectory.path, "nvvm");
#else
    const String decoratedPath = Path::combine(tempDirectory.path, "libnvvm.dylib");
    const String expectedLoadPath = Path::combine(tempDirectory.path, "nvvm");
#endif
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(File::writeAllText(decoratedPath, String())));

    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(NVVMDownstreamCompilerUtil::locateCompilers(decoratedPath, loader, set)));
    SLANG_CHECK(set->hasCompiler(SLANG_PASS_THROUGH_NVVM));
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    SLANG_CHECK(recordingLoader->loadRequests[0] == expectedLoadPath);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmCompilerOwnsLibrary)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* foundCompiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, foundCompiler)));
    ComPtr<IDownstreamCompiler> compiler(foundCompiler);
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 1);

    set.setNull();
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 1);
    compiler.setNull();
    SLANG_CHECK(gFakeNVVM.liveLibraryCount == 0);
    SLANG_CHECK(gFakeNVVM.destroyedLibraryCount == 1);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsInvalidInputs)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    ComPtr<IArtifact> validArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> wrongArtifact =
        ArtifactUtil::createArtifactForCompileTarget(SLANG_HOST_LLVM_IR);
    wrongArtifact->addRepresentationUnknown(StringBlob::create(UnownedStringSlice(kMinimalNVVMIR)));
    ComPtr<IArtifact> hostBitcodeArtifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::LLVMIR, ArtifactStyle::Host));
    hostBitcodeArtifact->addRepresentationUnknown(
        RawBlob::create(kMinimalNVVMBitcode, SLANG_COUNT_OF(kMinimalNVVMBitcode)));

    IArtifact* oneValidSource[] = {validArtifact};
    IArtifact* twoValidSources[] = {validArtifact, validArtifact};
    IArtifact* oneWrongSource[] = {wrongArtifact};
    IArtifact* oneHostBitcodeSource[] = {hostBitcodeArtifact};
    DownstreamCompileOptions::CapabilityVersion validCapability;
    validCapability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    validCapability.version.set(7, 5);
    DownstreamCompileOptions::CapabilityVersion malformedCapability;
    malformedCapability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    malformedCapability.version.set(7, 10);

    DownstreamCompileOptions baseOptions;
    baseOptions.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    baseOptions.targetType = SLANG_PTX;
    baseOptions.debugInfoType = DownstreamCompileOptions::DebugInfoType::None;

    // No source artifacts.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // More than one source artifact.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(twoValidSources, SLANG_COUNT_OF(twoValidSources));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // LLVM IR for the host has the right payload but the wrong artifact style.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneWrongSource, SLANG_COUNT_OF(oneWrongSource));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // Binary LLVM IR is accepted only when it carries the kernel style.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts =
            makeSlice(oneHostBitcodeSource, SLANG_COUNT_OF(oneHostBitcodeSource));
        options.requiredCapabilityVersions = makeSlice(&validCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // A valid source still requires an explicit CUDA architecture capability.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneValidSource, SLANG_COUNT_OF(oneValidSource));
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }

    // CUDA architecture minor versions contain one decimal digit.
    {
        gFakeNVVM.resetCalls();
        DownstreamCompileOptions options = baseOptions;
        options.sourceArtifacts = makeSlice(oneValidSource, SLANG_COUNT_OF(oneValidSource));
        options.requiredCapabilityVersions = makeSlice(&malformedCapability, 1);
        ComPtr<IArtifact> outputArtifact;
        SlangResult result = compiler->compile(options, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
    }
}

SLANG_UNIT_TEST(nvvmCompilerAcceptsLLVMBitcodeArtifact)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    // This deliberately contains several embedded NULs. The artifact descriptor identifies the
    // bytes as bitcode; Slang must forward the complete buffer without treating it as a string.
    static const uint8_t bitcode[] = {0x42, 0x43, 0xc0, 0xde, 0x00, 0x11, 0x00, 0x22};
    ComPtr<IArtifact> sourceArtifact = ArtifactUtil::createArtifact(
        ArtifactDesc::make(
            ArtifactKind::ObjectCode,
            ArtifactPayload::LLVMIR,
            ArtifactStyle::Kernel));
    sourceArtifact->addRepresentationUnknown(RawBlob::create(bitcode, SLANG_COUNT_OF(bitcode)));

    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_OK);

    ComPtr<ISlangBlob> outputBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, outputBlob.writeRef())));
    SLANG_CHECK(outputBlob->getBufferSize() == ::strlen(kFakePTX));
    SLANG_CHECK(::memcmp(outputBlob->getBufferPointer(), kFakePTX, ::strlen(kFakePTX)) == 0);

    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.addedModule.getLength() == SLANG_COUNT_OF(bitcode));
    SLANG_CHECK(::memcmp(gFakeNVVM.addedModule.getBuffer(), bitcode, SLANG_COUNT_OF(bitcode)) == 0);
    SLANG_CHECK(gFakeNVVM.addedModuleName == "slang-nvvm-input");
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
}

SLANG_UNIT_TEST(nvvmCompilerNegotiatesCUDADeviceLibraryOption)
{
    DownstreamCompileOptions defaults;
    SLANG_CHECK(sizeof(void*) == 4 || sizeof(void*) == 8);
    const uint32_t oldSize = sizeof(void*) == 8 ? 240u : 148u;
    SLANG_CHECK(offsetof(DownstreamCompileOptions, requiresCUDADeviceLibrary) == oldSize);
    SLANG_CHECK(oldSize % alignof(void*) == 0);
    SLANG_CHECK(
        sizeof(DownstreamCompileOptions) >= oldSize + sizeof(defaults.requiresCUDADeviceLibrary));
    SLANG_CHECK(sizeof(defaults.requiresCUDADeviceLibrary) == sizeof(void*));

    SLANG_CHECK(defaults.version.size == sizeof(DownstreamCompileOptions));
    SLANG_CHECK(!defaults.requiresCUDADeviceLibrary);

    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<slang::IBlob> rootlessVersionBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compiler->getVersionString(rootlessVersionBlob.writeRef())));
    SLANG_CHECK(_getBlobText(rootlessVersionBlob).indexOf("libdevice=") < 0);
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    IArtifact* sourceArtifacts[] = {sourceArtifact};
    DownstreamCompileOptions::CapabilityVersion capability;
    capability.kind = DownstreamCompileOptions::CapabilityVersion::Kind::CUDASM;
    capability.version.set(7, 5);

    DownstreamCompileOptions oldOptions;
    oldOptions.version.size = oldSize;
    oldOptions.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM;
    oldOptions.targetType = SLANG_PTX;
    oldOptions.sourceArtifacts = makeSlice(sourceArtifacts, SLANG_COUNT_OF(sourceArtifacts));
    oldOptions.requiredCapabilityVersions = makeSlice(&capability, 1);
    oldOptions.requiresCUDADeviceLibrary = true;
    ComPtr<IArtifact> outputArtifact;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compiler->compile(oldOptions, outputArtifact.writeRef())));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_findDiagnostics(outputArtifact)->getResult() == SLANG_OK);
    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 1);
}

SLANG_UNIT_TEST(nvvmCompilerUsesSelectedToolkitLibdevice)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kSelectedLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x00, 0x18};
    static const uint8_t kConflictingLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0xff, 0xee};
    TempDirectory selectedToolkit;
    TempDirectory conflictingToolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(selectedToolkit)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(conflictingToolkit)));
    String selectedCandidate;
    String selectedLibdevicePath;
    String conflictingCandidate;
    String conflictingLibdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        selectedToolkit.path,
        kSelectedLibdevice,
        sizeof(kSelectedLibdevice),
        selectedCandidate,
        selectedLibdevicePath)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        conflictingToolkit.path,
        kConflictingLibdevice,
        sizeof(kConflictingLibdevice),
        conflictingCandidate,
        conflictingLibdevicePath)));

    gFakeNVVM.reset();
    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SlangUnitTest::ScopedEnvVar conflictingCUDAPath(
        "CUDA_PATH",
        conflictingToolkit.path.getBuffer());
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        NVVMDownstreamCompilerUtil::locateCompilers(selectedToolkit.path, loader, set)));
    IDownstreamCompiler* compiler = _findNVVMCompiler(set);
    SLANG_CHECK_ABORT(compiler != nullptr);
    SLANG_CHECK(recordingLoader->loadRequests.getCount() == 1);
    ComPtr<slang::IBlob> versionBlob;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compiler->getVersionString(versionBlob.writeRef())));
    StringBuilder expectedLibdeviceIdentity;
    expectedLibdeviceIdentity << "libdevice="
                              << SharedLibraryUtils::getFileTimestamp(selectedLibdevicePath);
    SLANG_CHECK(
        _getBlobText(versionBlob).indexOf(expectedLibdeviceIdentity.getUnownedSlice()) >= 0);

    CompileSettings settings;
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 1);

    gFakeNVVM.resetCalls();
    settings.requiresCUDADeviceLibrary = true;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
    SLANG_CHECK(gFakeNVVM.moduleAddKinds[0] == FakeModuleAddKind::Normal);
    SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Lazy);
    SLANG_CHECK(gFakeNVVM.moduleAddNames[0] == "slang-nvvm-input");
    SLANG_CHECK(gFakeNVVM.moduleAddNames[1] == "libdevice.10.bc");
    SLANG_CHECK(gFakeNVVM.addedModule == kMinimalNVVMIR);
    SLANG_CHECK(gFakeNVVM.addedLibraryModuleName == "libdevice.10.bc");
    SLANG_CHECK(gFakeNVVM.addedLibraryModule.getLength() == sizeof(kSelectedLibdevice));
    SLANG_CHECK(
        ::memcmp(
            gFakeNVVM.addedLibraryModule.getBuffer(),
            kSelectedLibdevice,
            sizeof(kSelectedLibdevice)) == 0);
    SLANG_CHECK(
        ::memcmp(
            gFakeNVVM.addedLibraryModule.getBuffer(),
            kConflictingLibdevice,
            sizeof(kConflictingLibdevice)) != 0);
    SLANG_CHECK(gFakeNVVM.verifyProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmCompilerRejectsUnavailableRequestedLibdevice)
{
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;
    settings.requiresCUDADeviceLibrary = true;

    gFakeNVVM.reset();
    {
        RefPtr<DownstreamCompilerSet> set;
        IDownstreamCompiler* compiler = nullptr;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
        ComPtr<IArtifact> outputArtifact;
        const SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
        SLANG_CHECK(_diagnosticsContain(_findDiagnostics(outputArtifact), "toolkit root"));
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
    }

#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    gFakeNVVM.reset();
    TempDirectory incompleteToolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(incompleteToolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _createFakeNVVMToolkit(incompleteToolkit.path, nullptr, 0, candidatePath, libdevicePath)));
    auto recordingLoader = new RecordingFakeNVVMLoader;
    ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
    RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        NVVMDownstreamCompilerUtil::locateCompilers(incompleteToolkit.path, loader, set)));
    IDownstreamCompiler* compiler = _findNVVMCompiler(set);
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> outputArtifact;
    CompileSettings noLibrarySettings;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, noLibrarySettings, outputArtifact.writeRef())));
    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);

    gFakeNVVM.resetCalls();
    outputArtifact.setNull();
    const SlangResult result =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    _checkRejectedInputResult(result, outputArtifact);
    SLANG_CHECK(_diagnosticsContain(_findDiagnostics(outputArtifact), "libdevice.10.bc"));
    SLANG_CHECK(gFakeNVVM.addModuleCallCount == 0);
    SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
#endif
}

SLANG_UNIT_TEST(nvvmCompilerHandlesLibdeviceModuleAddition)
{
#if SLANG_WINDOWS_FAMILY || SLANG_LINUX_FAMILY
    static const uint8_t kLibdevice[] = {0x42, 0x43, 0xc0, 0xde, 0x18, 0x00};
    TempDirectory toolkit;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createTempDirectory(toolkit)));
    String candidatePath;
    String libdevicePath;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_createFakeNVVMToolkit(
        toolkit.path,
        kLibdevice,
        sizeof(kLibdevice),
        candidatePath,
        libdevicePath)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;
    settings.requiresCUDADeviceLibrary = true;

    gFakeNVVM.reset();
    {
        auto recordingLoader = new RecordingFakeNVVMLoader;
        ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            NVVMDownstreamCompilerUtil::locateCompilers(toolkit.path, loader, set)));
        IDownstreamCompiler* compiler = _findNVVMCompiler(set);
        SLANG_CHECK_ABORT(compiler != nullptr);
        gFakeNVVM.failure = FakeFailure::LazyAddModule;
        ComPtr<IArtifact> outputArtifact;
        const SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK(SLANG_FAILED(result));
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        SLANG_CHECK(_diagnosticsContain(_findDiagnostics(outputArtifact), "device-library"));
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 1);
        SLANG_CHECK(gFakeNVVM.verifyProgramCallCount == 0);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 0);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    }

    gFakeNVVM.reset();
    gFakeNVVM.omitOptionalSymbols = true;
    {
        auto recordingLoader = new RecordingFakeNVVMLoader;
        ComPtr<ISlangSharedLibraryLoader> loader(recordingLoader);
        RefPtr<DownstreamCompilerSet> set = new DownstreamCompilerSet;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            NVVMDownstreamCompilerUtil::locateCompilers(toolkit.path, loader, set)));
        IDownstreamCompiler* compiler = _findNVVMCompiler(set);
        SLANG_CHECK_ABORT(compiler != nullptr);
        ComPtr<IArtifact> outputArtifact;
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 2);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds.getCount() == 2);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[0] == FakeModuleAddKind::Normal);
        SLANG_CHECK(gFakeNVVM.moduleAddKinds[1] == FakeModuleAddKind::Normal);
        SLANG_CHECK(gFakeNVVM.moduleAddNames[0] == "slang-nvvm-input");
        SLANG_CHECK(gFakeNVVM.moduleAddNames[1] == "libdevice.10.bc");
        SLANG_CHECK(gFakeNVVM.addedLibraryModule.getLength() == sizeof(kLibdevice));
        SLANG_CHECK(
            ::memcmp(gFakeNVVM.addedLibraryModule.getBuffer(), kLibdevice, sizeof(kLibdevice)) ==
            0);
        SLANG_CHECK(gFakeNVVM.verifyProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);

        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = FakeFailure::EagerAddModule;
        outputArtifact.setNull();
        const SlangResult eagerFailureResult =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK(SLANG_FAILED(eagerFailureResult));
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        SLANG_CHECK(_diagnosticsContain(_findDiagnostics(outputArtifact), "device-library"));
        SLANG_CHECK(gFakeNVVM.addModuleCallCount == 2);
        SLANG_CHECK(gFakeNVVM.lazyAddModuleCallCount == 0);
        SLANG_CHECK(gFakeNVVM.verifyProgramCallCount == 0);
        SLANG_CHECK(gFakeNVVM.compileProgramCallCount == 0);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
        gFakeNVVM.failure = FakeFailure::None;
    }
#else
    SLANG_IGNORE_TEST;
#endif
}

SLANG_UNIT_TEST(nvvmCompilerEnforcesFloatingPointPolicy)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();

    static const DownstreamCompileOptions::FloatingPointMode kModes[] = {
        DownstreamCompileOptions::FloatingPointMode::Default,
        DownstreamCompileOptions::FloatingPointMode::Precise,
        DownstreamCompileOptions::FloatingPointMode::Fast,
    };
    static const DownstreamCompileOptions::FloatingPointDenormalMode kDenormalModes[] = {
        DownstreamCompileOptions::FloatingPointDenormalMode::Any,
        DownstreamCompileOptions::FloatingPointDenormalMode::Preserve,
        DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero,
    };
    for (auto mode : kModes)
    {
        for (auto denormalMode : kDenormalModes)
        {
            gFakeNVVM.resetCalls();
            CompileSettings settings;
            settings.floatingPointMode = mode;
            settings.denormalModeFp32 = denormalMode;
            ComPtr<IArtifact> outputArtifact;
            SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
                _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
            const Index expectedOptionCount =
                2 + (mode == DownstreamCompileOptions::FloatingPointMode::Default ? 0 : 3) +
                (denormalMode == DownstreamCompileOptions::FloatingPointDenormalMode::Any ? 0 : 1);
            SLANG_CHECK(gFakeNVVM.compileOptions.getCount() == expectedOptionCount);
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-prec-div=1") ==
                (mode == DownstreamCompileOptions::FloatingPointMode::Precise));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-prec-sqrt=1") ==
                (mode == DownstreamCompileOptions::FloatingPointMode::Precise));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-fma=0") ==
                (mode == DownstreamCompileOptions::FloatingPointMode::Precise));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-prec-div=0") ==
                (mode == DownstreamCompileOptions::FloatingPointMode::Fast));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-prec-sqrt=0") ==
                (mode == DownstreamCompileOptions::FloatingPointMode::Fast));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-fma=1") ==
                (mode == DownstreamCompileOptions::FloatingPointMode::Fast));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-ftz=0") ==
                (denormalMode == DownstreamCompileOptions::FloatingPointDenormalMode::Preserve));
            SLANG_CHECK(
                _hasOption(gFakeNVVM.compileOptions, "-ftz=1") ==
                (denormalMode == DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero));
            SLANG_CHECK(gFakeNVVM.verifyOptions.getCount() == gFakeNVVM.compileOptions.getCount());
            for (Index i = 0; i < gFakeNVVM.compileOptions.getCount(); ++i)
                SLANG_CHECK(gFakeNVVM.verifyOptions[i] == gFakeNVVM.compileOptions[i]);
        }
    }

    static const char* kManagedOverrides[] = {
        "-ftz",
        "-ftz=1",
        "-prec-div",
        "-prec-div=0",
        "-prec-sqrt",
        "-prec-sqrt=0",
        "-fma",
        "-fma=1",
    };
    for (const char* managedOverride : kManagedOverrides)
    {
        gFakeNVVM.resetCalls();
        CompileSettings settings;
        settings.compilerSpecificArgument = managedOverride;
        ComPtr<IArtifact> outputArtifact;
        const SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
        SLANG_CHECK(_diagnosticsContain(_findDiagnostics(outputArtifact), "managed"));
    }

    struct UnsupportedDenormalCase
    {
        bool fp16;
        DownstreamCompileOptions::FloatingPointDenormalMode mode;
        const char* diagnostic;
    };
    static const UnsupportedDenormalCase kUnsupportedDenormalCases[] = {
        {true, DownstreamCompileOptions::FloatingPointDenormalMode::Preserve, "fp16"},
        {true, DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero, "fp16"},
        {false, DownstreamCompileOptions::FloatingPointDenormalMode::Preserve, "fp64"},
        {false, DownstreamCompileOptions::FloatingPointDenormalMode::FlushToZero, "fp64"},
    };
    for (const auto& unsupportedCase : kUnsupportedDenormalCases)
    {
        gFakeNVVM.resetCalls();
        CompileSettings settings;
        if (unsupportedCase.fp16)
            settings.denormalModeFp16 = unsupportedCase.mode;
        else
            settings.denormalModeFp64 = unsupportedCase.mode;
        ComPtr<IArtifact> outputArtifact;
        const SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        _checkRejectedInputResult(result, outputArtifact);
        SLANG_CHECK(
            _diagnosticsContain(_findDiagnostics(outputArtifact), unsupportedCase.diagnostic));
    }

    gFakeNVVM.resetCalls();
    CompileSettings invalidFloatingPointSettings;
    invalidFloatingPointSettings.floatingPointMode =
        static_cast<DownstreamCompileOptions::FloatingPointMode>(0xff);
    ComPtr<IArtifact> invalidFloatingPointOutput;
    SlangResult invalidResult = _compileNVVM(
        compiler,
        sourceArtifact,
        invalidFloatingPointSettings,
        invalidFloatingPointOutput.writeRef());
    _checkRejectedInputResult(invalidResult, invalidFloatingPointOutput);
    SLANG_CHECK(
        _diagnosticsContain(_findDiagnostics(invalidFloatingPointOutput), "floating-point mode"));
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);

    gFakeNVVM.resetCalls();
    CompileSettings invalidDenormalSettings;
    invalidDenormalSettings.denormalModeFp32 =
        static_cast<DownstreamCompileOptions::FloatingPointDenormalMode>(0xff);
    ComPtr<IArtifact> invalidDenormalOutput;
    invalidResult = _compileNVVM(
        compiler,
        sourceArtifact,
        invalidDenormalSettings,
        invalidDenormalOutput.writeRef());
    _checkRejectedInputResult(invalidResult, invalidDenormalOutput);
    SLANG_CHECK(_diagnosticsContain(_findDiagnostics(invalidDenormalOutput), "fp32 denormal"));
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

SLANG_UNIT_TEST(nvvmCompilerCompilesTrivialIR)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));

    ComPtr<slang::IBlob> versionString;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compiler->getVersionString(versionString.writeRef())));
    String version(UnownedStringSlice(
        (const char*)versionString->getBufferPointer(),
        versionString->getBufferSize()));
    SLANG_CHECK(version.indexOf("2.0") >= 0);
    SLANG_CHECK(version.indexOf("nvvm-ir=2.0") >= 0);
    SLANG_CHECK(version.indexOf("debug=3.0") >= 0);

    CompileSettings settings;
    settings.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::None;
    settings.debugInfoType = DownstreamCompileOptions::DebugInfoType::Maximal;
    settings.floatingPointMode = DownstreamCompileOptions::FloatingPointMode::Precise;
    settings.denormalModeFp32 = DownstreamCompileOptions::FloatingPointDenormalMode::Preserve;
    settings.addFakeCompilerArgument = true;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef())));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);

    SLANG_CHECK(
        outputArtifact->getDesc() ==
        ArtifactDesc::make(ArtifactKind::ObjectCode, ArtifactPayload::PTX, ArtifactStyle::Kernel));
    IArtifactDiagnostics* diagnostics =
        findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_OK);

    ComPtr<ISlangBlob> outputBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, outputBlob.writeRef())));
    SLANG_CHECK(outputBlob->getBufferSize() == ::strlen(kFakePTX));
    SLANG_CHECK(::memcmp(outputBlob->getBufferPointer(), kFakePTX, ::strlen(kFakePTX)) == 0);
    if (outputBlob->getBufferSize())
    {
        const char* bytes = (const char*)outputBlob->getBufferPointer();
        SLANG_CHECK(bytes[outputBlob->getBufferSize() - 1] != 0);
    }

    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    SLANG_CHECK(gFakeNVVM.addedModule == kMinimalNVVMIR);
    SLANG_CHECK(gFakeNVVM.addedModuleName == "slang-nvvm-input");
    SLANG_CHECK(gFakeNVVM.verifyOptions.getCount() == 8);
    SLANG_CHECK(gFakeNVVM.compileOptions.getCount() == 8);
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-arch=compute_75"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-g"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-opt=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-ftz=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-prec-div=1"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-prec-sqrt=1"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-fma=0"));
    SLANG_CHECK(_hasOption(gFakeNVVM.compileOptions, "-fake-nvvm-option"));
    for (Index i = 0; i < gFakeNVVM.compileOptions.getCount(); ++i)
        SLANG_CHECK(gFakeNVVM.verifyOptions[i] == gFakeNVVM.compileOptions[i]);

    // Maximal debug metadata is only valid for unoptimized code. Reject the combination before
    // creating a libNVVM program so the policy cannot be silently weakened by option ordering.
    gFakeNVVM.resetCalls();
    settings.optimizationLevel = DownstreamCompileOptions::OptimizationLevel::High;
    ComPtr<IArtifact> invalidOutput;
    SlangResult invalidResult =
        _compileNVVM(compiler, sourceArtifact, settings, invalidOutput.writeRef());
    SLANG_CHECK(invalidResult == SLANG_E_INVALID_ARG);
    SLANG_CHECK_ABORT(invalidOutput != nullptr);
    diagnostics = findAssociatedRepresentation<IArtifactDiagnostics>(invalidOutput);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(diagnostics->getResult() == SLANG_E_INVALID_ARG);
    SLANG_CHECK(_diagnosticsContain(diagnostics, "requires optimization to be disabled"));
    SLANG_CHECK(gFakeNVVM.createProgramCallCount == 0);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsTerminatorOnlyResult)
{
    _checkRejectedCompiledResult(FakeResultMode::TerminatorOnly);
}

SLANG_UNIT_TEST(nvvmCompilerRejectsUnterminatedResult)
{
    _checkRejectedCompiledResult(FakeResultMode::Unterminated);
}

SLANG_UNIT_TEST(nvvmCompilerDestroysProgramsOnFailure)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;

    static const FakeFailure kFailures[] = {
        FakeFailure::CreateProgram,
        FakeFailure::AddModule,
        FakeFailure::VerifyProgram,
        FakeFailure::CompileProgram,
        FakeFailure::GetResultSize,
        FakeFailure::GetResult,
        FakeFailure::GetLogSize,
        FakeFailure::GetLog,
    };
    for (FakeFailure failure : kFailures)
    {
        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = failure;
        ComPtr<IArtifact> outputArtifact;
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        IArtifactDiagnostics* diagnostics =
            findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
        SLANG_CHECK(
            gFakeNVVM.destroyProgramCallCount == (failure == FakeFailure::CreateProgram ? 0 : 1));
    }
    gFakeNVVM.failure = FakeFailure::None;
}

SLANG_UNIT_TEST(nvvmCompilerClassifiesVerificationAndCompilationFailures)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    CompileSettings settings;

    struct FailureCase
    {
        FakeFailure operation;
        TestNVVMResult nvvmResult;
        SlangResult callResult;
        SlangResult diagnosticResult;
    };
    static const FailureCase kCases[] = {
        {FakeFailure::VerifyProgram, TestNVVMResult::Compilation, SLANG_OK, SLANG_FAIL},
        {FakeFailure::CompileProgram, TestNVVMResult::Compilation, SLANG_OK, SLANG_FAIL},
        {FakeFailure::VerifyProgram,
         TestNVVMResult::OutOfMemory,
         SLANG_E_OUT_OF_MEMORY,
         SLANG_E_OUT_OF_MEMORY},
        {FakeFailure::CompileProgram,
         TestNVVMResult::OutOfMemory,
         SLANG_E_OUT_OF_MEMORY,
         SLANG_E_OUT_OF_MEMORY},
        {FakeFailure::VerifyProgram, TestNVVMResult::Cancelled, SLANG_E_ABORT, SLANG_E_ABORT},
        {FakeFailure::CompileProgram, TestNVVMResult::Cancelled, SLANG_E_ABORT, SLANG_E_ABORT},
    };

    for (const auto& failureCase : kCases)
    {
        gFakeNVVM.resetCalls();
        gFakeNVVM.failure = failureCase.operation;
        gFakeNVVM.failureResult = failureCase.nvvmResult;
        ComPtr<IArtifact> outputArtifact;
        SlangResult result =
            _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
        SLANG_CHECK(result == failureCase.callResult);
        SLANG_CHECK_ABORT(outputArtifact != nullptr);
        IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
        SLANG_CHECK_ABORT(diagnostics != nullptr);
        SLANG_CHECK(diagnostics->getResult() == failureCase.diagnosticResult);
        SLANG_CHECK(diagnostics->getCount() >= 1);
        SLANG_CHECK(gFakeNVVM.destroyProgramCallCount == 1);
    }
    gFakeNVVM.failure = FakeFailure::None;
    gFakeNVVM.failureResult = TestNVVMResult::Compilation;
}

SLANG_UNIT_TEST(nvvmCompilerUsesErrorStringForEmptyLog)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.failure = FakeFailure::VerifyProgram;
    gFakeNVVM.programLog = String();

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    // Verification/compilation failures are represented on the artifact so the caller can consume
    // libNVVM's diagnostics through the same channel as other downstream compilers.
    SLANG_CHECK(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    IArtifactDiagnostics* diagnostics =
        findAssociatedRepresentation<IArtifactDiagnostics>(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));
    SLANG_CHECK(_diagnosticsContain(diagnostics, "libNVVM verification failed"));
    SLANG_CHECK(_diagnosticsContain(diagnostics, "fake NVVM compilation failure"));
    SLANG_CHECK(diagnostics->getCount() >= 1);
}

SLANG_UNIT_TEST(nvvmCompilerPreservesVerifierLogOnCompilationFailure)
{
    gFakeNVVM.reset();
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_locateFakeNVVM(set, compiler)));
    gFakeNVVM.usePhaseLogs = true;
    gFakeNVVM.verifierLog = "fake verifier success note";
    gFakeNVVM.compilerLog = "fake compiler failure detail";
    gFakeNVVM.failure = FakeFailure::CompileProgram;

    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SLANG_CHECK(
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef()) == SLANG_OK);
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK(SLANG_FAILED(diagnostics->getResult()));

    const TerminatedCharSlice raw = diagnostics->getRaw();
    const String rawText(UnownedStringSlice(raw.data, raw.count));
    const Index verifierLogIndex = rawText.indexOf(gFakeNVVM.verifierLog);
    const Index compilerLogIndex = rawText.indexOf(gFakeNVVM.compilerLog);
    SLANG_CHECK(verifierLogIndex >= 0);
    SLANG_CHECK(compilerLogIndex > verifierLogIndex);
    SLANG_CHECK(rawText.indexOf("libNVVM compilation failed") > verifierLogIndex);
}

SLANG_UNIT_TEST(nvvmCompilerCompilesSelfContainedLibdeviceSine)
{
    String cudaRoot;
    const SlangResult prerequisiteResult = _findLibdeviceNVVMToolkitFromCUDAPath(cudaRoot);
    if (prerequisiteResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real libdevice compile test because a complete CUDA_PATH toolkit is "
            "unavailable.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(prerequisiteResult));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMIRWithLibdevice(cudaRoot, outputArtifact);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kLibdeviceSineKernelName)));

    String ptx;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_loadPTXText(outputArtifact, ptx)));
    String signature;
    String body;
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_extractPTXEntry(
        ptx.getUnownedSlice(),
        toSlice(kLibdeviceSineKernelName),
        signature,
        body)));
    SLANG_CHECK(ptx.indexOf(".extern .func") < 0);
    SLANG_CHECK(body.getUnownedSlice().indexOf(toSlice("st.global")) >= 0);
}

SLANG_UNIT_TEST(nvvmCompilerLibdeviceSinePtxasAccepts)
{
    String cudaRoot;
    String ptxasPath;
    const SlangResult prerequisiteResult = _findLibdeviceNVVMToolkitFromCUDAPath(cudaRoot);
    const SlangResult ptxasResult = SLANG_SUCCEEDED(prerequisiteResult)
                                        ? _findPtxasInToolkit(cudaRoot, ptxasPath)
                                        : prerequisiteResult;
    if (prerequisiteResult == SLANG_E_NOT_FOUND || ptxasResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring libdevice ptxas test because one coherent CUDA toolkit is unavailable.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(prerequisiteResult));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(ptxasResult));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMIRWithLibdevice(cudaRoot, outputArtifact);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice(kLibdeviceSineKernelName)));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(outputArtifact, ptxasPath)));
}

SLANG_UNIT_TEST(nvvmCompilerLibdeviceSineRuns)
{
    String cudaRoot;
    const SlangResult prerequisiteResult = _findLibdeviceNVVMToolkitFromCUDAPath(cudaRoot);
    if (prerequisiteResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring libdevice runtime test because a complete CUDA_PATH toolkit is unavailable.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(prerequisiteResult));

    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMIRWithLibdevice(cudaRoot, outputArtifact);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    ComPtr<ISlangBlob> ptxBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())));

    CudaDriverApi cuda;
    if (!cuda.load() || cuda.cuInit(0) != 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring libdevice runtime test because the CUDA driver is unavailable.");
        SLANG_IGNORE_TEST;
    }
    int deviceCount = 0;
    if (cuda.cuDeviceGetCount(&deviceCount) != 0 || deviceCount == 0)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring libdevice runtime test because no CUDA device is available.");
        SLANG_IGNORE_TEST;
    }
    CudaDevice device = 0;
    SLANG_CHECK_ABORT(cuda.cuDeviceGet(&device, 0) == 0);
    int computeMajor = 0;
    int computeMinor = 0;
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMajor,
            kCudaDeviceAttributeComputeCapabilityMajor,
            device) == 0);
    SLANG_CHECK_ABORT(
        cuda.cuDeviceGetAttribute(
            &computeMinor,
            kCudaDeviceAttributeComputeCapabilityMinor,
            device) == 0);
    if (computeMajor < 7 || (computeMajor == 7 && computeMinor < 5))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring libdevice runtime test because the device is older than sm_75.");
        SLANG_IGNORE_TEST;
    }

    CudaContext context = nullptr;
    SLANG_CHECK_ABORT(cuda.cuDevicePrimaryCtxRetain(&context, device) == 0);
    CudaPrimaryContextGuard contextGuard{cuda, device};
    SLANG_CHECK_ABORT(cuda.cuCtxSetCurrent(context) == 0);

    static const float kInputs[] = {0.0f, 0.5f, -1.25f, 20.0f};
    for (float input : kInputs)
    {
        SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_runLibdeviceSineKernel(cuda, ptxBlob, input, 2.0e-6f)));
    }
}

SLANG_UNIT_TEST(nvvmCompilerCompilesEmptyKernel)
{
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SlangResult locateResult = _locateRealNVVM(String(), set, compiler);
    if (locateResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real libNVVM smoke test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> sourceArtifact = _createNVVMIRArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
    {
        _reportArtifactDiagnostics(outputArtifact);
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));

    ComPtr<ISlangBlob> ptxBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())));
    String ptx(
        UnownedStringSlice((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize()));
    SLANG_CHECK(ptx.indexOf(".visible .entry testEmpty") >= 0);
    SLANG_CHECK(ptxBlob->getBufferSize() > 0);
    if (ptxBlob->getBufferSize())
    {
        const char* bytes = (const char*)ptxBlob->getBufferPointer();
        SLANG_CHECK(bytes[ptxBlob->getBufferSize() - 1] != 0);
    }
}

SLANG_UNIT_TEST(nvvmCompilerCompilesEmptyKernelBitcode)
{
    RefPtr<DownstreamCompilerSet> set;
    IDownstreamCompiler* compiler = nullptr;
    SlangResult locateResult = _locateRealNVVM(String(), set, compiler);
    if (locateResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring real libNVVM bitcode test because no CUDA toolkit was discovered.");
        SLANG_IGNORE_TEST;
    }

    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(locateResult));
    SLANG_CHECK_ABORT(compiler != nullptr);
    ComPtr<IArtifact> sourceArtifact = _createNVVMBitcodeArtifact();
    ComPtr<IArtifact> outputArtifact;
    CompileSettings settings;
    SlangResult compileResult =
        _compileNVVM(compiler, sourceArtifact, settings, outputArtifact.writeRef());
    IArtifactDiagnostics* diagnostics = _findDiagnostics(outputArtifact);
    if (SLANG_FAILED(compileResult) || !diagnostics || SLANG_FAILED(diagnostics->getResult()))
        _reportArtifactDiagnostics(outputArtifact);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK_ABORT(diagnostics != nullptr);
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(diagnostics->getResult()));

    ComPtr<ISlangBlob> ptxBlob;
    SLANG_CHECK_ABORT(
        SLANG_SUCCEEDED(outputArtifact->loadBlob(ArtifactKeep::Yes, ptxBlob.writeRef())));
    String ptx(
        UnownedStringSlice((const char*)ptxBlob->getBufferPointer(), ptxBlob->getBufferSize()));
    SLANG_CHECK(ptx.indexOf(".visible .entry testEmpty") >= 0);
}

SLANG_UNIT_TEST(nvvmPtxasAcceptsEmptyKernel)
{
    String cudaRoot;
    String ptxasPath;
    if (SLANG_FAILED(_findPtxasFromCUDAPath(cudaRoot, ptxasPath)))
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH does not contain ptxas.");
        SLANG_IGNORE_TEST;
    }

    // Assemble PTX produced from bitcode so the compatibility fixture crosses the entire local
    // offline toolchain. The preceding real test keeps the textual bootstrap path covered.
    ComPtr<IArtifact> outputArtifact;
    const SlangResult compileResult = _compileRealNVVMBitcode(
        cudaRoot,
        kMinimalNVVMBitcode,
        SLANG_COUNT_OF(kMinimalNVVMBitcode),
        outputArtifact);
    if (compileResult == SLANG_E_NOT_FOUND)
    {
        getTestReporter()->message(
            TestMessageType::Info,
            "Ignoring ptxas smoke test because CUDA_PATH does not contain libNVVM.");
        SLANG_IGNORE_TEST;
    }
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(compileResult));
    SLANG_CHECK_ABORT(outputArtifact != nullptr);
    SLANG_CHECK(_ptxContainsEntry(outputArtifact, toSlice("testEmpty")));
    SLANG_CHECK_ABORT(SLANG_SUCCEEDED(_assemblePTX(outputArtifact, ptxasPath)));
}
