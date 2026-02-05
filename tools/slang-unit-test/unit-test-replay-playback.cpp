// unit-test-replay-playback.cpp
// Unit tests for playback dispatcher and handler infrastructure

#include "unit-test-replay-common.h"

// =============================================================================
// Playback Dispatcher Tests
// =============================================================================

// Track calls made during playback
static int s_playbackCallCount = 0;
static const char* s_lastProfileName = nullptr;

// Handler for findProfile playback
static void playbackFindProfile(ReplayContext& ctx)
{
    s_playbackCallCount++;

    // executeNextCall seeks back to the start of the command, so we must
    // first consume the signature and 'this' handle (just like RECORD_CALL does)
    const char* signature = nullptr;
    ctx.record(RecordFlag::Input, signature);
    
    ISlangBlob* thisPtr = nullptr;
    ctx.record(RecordFlag::Input, thisPtr);

    // Now read the profile name input
    const char* profileName = nullptr;
    ctx.record(RecordFlag::Input, profileName);
    s_lastProfileName = profileName;

    // In a real playback, we'd call the actual function here:
    // auto* globalSession = ctx.getCurrentThis<slang::IGlobalSession>();
    // SlangProfileID result = globalSession->findProfile(profileName);

    // For testing, just read and discard the return value
    int32_t returnValue = 0;
    ctx.record(RecordFlag::None, returnValue);
}

SLANG_UNIT_TEST(replayContextPlaybackDispatcher)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Reset test state
    s_playbackCallCount = 0;
    s_lastProfileName = nullptr;

    // First, record a call - we'll manually construct the stream format
    // Format: signature (string), thisHandle (ObjectHandle), input args..., return value
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Record signature
    const char* signature = "findProfile_test_signature";
    ctx().record(RecordFlag::Input, signature);

    // Record 'this' pointer as a handle - use a blob as a tracked object since we can record those
    // Actually, let's just write the handle bytes directly since recorder is in Record mode
    // We need to register an object first, then record its handle
    
    // Create a fake blob to use as "this"
    Slang::ComPtr<ISlangBlob> fakeBlob = Slang::RawBlob::create("fake", 4);
    uint64_t thisHandle = ctx().registerInterface(fakeBlob.get());
    SLANG_UNUSED(thisHandle);
    
    // Write the handle directly (ObjectHandle TypeId + handle value)
    ISlangBlob* blobPtr = fakeBlob.get();
    ctx().record(RecordFlag::Input, blobPtr);

    // Record the profile name input
    const char* profileName = "sm_6_0";
    ctx().record(RecordFlag::Input, profileName);

    // Record return value
    int32_t profileId = 42;
    ctx().record(RecordFlag::ReturnValue, profileId);

    // Now set up playback
    ctx().switchToPlayback();
    
    // Register the handler
    ctx().registerHandler("findProfile_test_signature", playbackFindProfile);

    // Also need to register the fake object so getCurrentThis works
    // Use the same handle value for consistency
    ctx().registerInterface(fakeBlob.get());

    // Execute the call
    bool executed = ctx().executeNextCall();
    SLANG_CHECK(executed);
    SLANG_CHECK(s_playbackCallCount == 1);
    SLANG_CHECK(s_lastProfileName != nullptr);
    SLANG_CHECK(strcmp(s_lastProfileName, "sm_6_0") == 0);

    // No more calls
    SLANG_CHECK(!ctx().hasMoreCalls());
    SLANG_CHECK(!ctx().executeNextCall());
}

SLANG_UNIT_TEST(replayContextPlaybackMultipleCalls)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    s_playbackCallCount = 0;

    // Record multiple calls
    ctx().reset();
    ctx().setMode(Mode::Record);

    Slang::ComPtr<ISlangBlob> fakeBlob = Slang::RawBlob::create("fake", 4);
    ctx().registerInterface(fakeBlob.get());

    for (int i = 0; i < 3; i++)
    {
        const char* sig = "findProfile_test_signature";
        ctx().record(RecordFlag::Input, sig);
        
        ISlangBlob* blobPtr = fakeBlob.get();
        ctx().record(RecordFlag::Input, blobPtr);
        
        const char* name = "sm_5_0";
        ctx().record(RecordFlag::Input, name);
        
        int32_t result = 10 + i;
        ctx().record(RecordFlag::ReturnValue, result);
    }

    // Playback all
    ctx().switchToPlayback();
    ctx().registerHandler("findProfile_test_signature", playbackFindProfile);
    ctx().registerInterface(fakeBlob.get());

    ctx().executeAll();

    SLANG_CHECK(s_playbackCallCount == 3);
    SLANG_CHECK(!ctx().hasMoreCalls());
}

// =============================================================================
// Test REPLAY_REGISTER macro - using a simple test proxy
// =============================================================================

// Simple test interface for replay macro testing
struct ITestCalculator : public ISlangUnknown
{
    SLANG_COM_INTERFACE(0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0});
    
    virtual int32_t SLANG_MCALL add(int32_t a, int32_t b) = 0;
    virtual void SLANG_MCALL setOffset(int32_t offset) = 0;
};

// Track what gets called during playback
static int32_t s_testCalcLastA = 0;
static int32_t s_testCalcLastB = 0;
static int32_t s_testCalcOffset = 0;
static int s_testCalcAddCalls = 0;
static int s_testCalcSetOffsetCalls = 0;

// Simple proxy for ITestCalculator that uses our recording macros
class TestCalculatorProxy : public ITestCalculator
{
public:
    TestCalculatorProxy(ITestCalculator* actual) : m_actual(actual), m_refCount(1) {}

    // ISlangUnknown
    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == ITestCalculator::getTypeGuid() || uuid == ISlangUnknown::getTypeGuid())
        {
            *outObject = this;
            addRef();
            return SLANG_OK;
        }
        *outObject = nullptr;
        return SLANG_E_NO_INTERFACE;
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ++m_refCount; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override 
    { 
        uint32_t count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }

    // ITestCalculator - with recording
    int32_t SLANG_MCALL add(int32_t a, int32_t b) override
    {
        RECORD_CALL();
        RECORD_INPUT(a);
        RECORD_INPUT(b);

        // Track for test verification
        s_testCalcLastA = a;
        s_testCalcLastB = b;
        s_testCalcAddCalls++;

        int32_t result = m_actual ? m_actual->add(a, b) : (a + b);
        RECORD_RETURN(result);
    }

    void SLANG_MCALL setOffset(int32_t offset) override
    {
        RECORD_CALL();
        RECORD_INPUT(offset);

        s_testCalcOffset = offset;
        s_testCalcSetOffsetCalls++;

        if (m_actual) m_actual->setOffset(offset);
    }

    ITestCalculator* getActual() { return m_actual; }

private:
    ITestCalculator* m_actual;
    std::atomic<uint32_t> m_refCount;
};

// Simple implementation that just does the math
class TestCalculatorImpl : public ITestCalculator
{
public:
    TestCalculatorImpl() : m_offset(0), m_refCount(1) {}

    SLANG_NO_THROW SlangResult SLANG_MCALL queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        if (uuid == ITestCalculator::getTypeGuid() || uuid == ISlangUnknown::getTypeGuid())
        {
            *outObject = this;
            addRef();
            return SLANG_OK;
        }
        *outObject = nullptr;
        return SLANG_E_NO_INTERFACE;
    }

    SLANG_NO_THROW uint32_t SLANG_MCALL addRef() override { return ++m_refCount; }
    SLANG_NO_THROW uint32_t SLANG_MCALL release() override 
    { 
        uint32_t count = --m_refCount;
        if (count == 0) delete this;
        return count;
    }

    int32_t SLANG_MCALL add(int32_t a, int32_t b) override { return a + b + m_offset; }
    void SLANG_MCALL setOffset(int32_t offset) override { m_offset = offset; }

private:
    int32_t m_offset;
    std::atomic<uint32_t> m_refCount;
};

// Test the REPLAY_REGISTER infrastructure by using the replayHandler template directly
// with a known signature
SLANG_UNIT_TEST(replayContextReplayRegisterMacro)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Reset test state
    s_testCalcLastA = 0;
    s_testCalcLastB = 0;
    s_testCalcAddCalls = 0;
    s_testCalcSetOffsetCalls = 0;
    s_testCalcOffset = 0;

    // Create implementation and proxy
    Slang::ComPtr<ITestCalculator> impl(new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    Slang::ComPtr<ITestCalculator> proxyPtr(proxy);

    // Build a recorded stream manually with known signatures
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Register the proxy and get its handle
    uint64_t proxyHandle = ctx().registerInterface(proxyPtr.get());
    SLANG_CHECK(proxyHandle >= kFirstValidHandle);

    // Record a call manually with a simple signature we control
    const char* addSignature = "TestCalculator::add";
    ctx().record(RecordFlag::Input, addSignature);  // signature
    
    // Record 'this' handle with proper TypeId (what beginCall does via recordHandle)
    ctx().recordHandle(RecordFlag::Input, proxyHandle);
    
    int32_t arg_a = 10;
    int32_t arg_b = 20;
    ctx().record(RecordFlag::Input, arg_a);
    ctx().record(RecordFlag::Input, arg_b);
    
    int32_t returnVal = 30;
    ctx().record(RecordFlag::ReturnValue, returnVal);

    // Verify we recorded something
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // Switch to playback
    ctx().switchToPlayback();
    ctx().registerInterface(proxyPtr.get());  // Same handle value
    
    // Register a handler using the replayHandler template (what REPLAY_REGISTER does internally)
    auto addHandler = [](ReplayContext& ctxRef) {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::add
        );
    };
    ctx().registerHandler(addSignature, addHandler);

    // Execute playback - this should:
    // 1. Read signature "TestCalculator::add"
    // 2. Read thisHandle and set m_currentThisHandle
    // 3. Call addHandler which calls replayHandler
    // 4. replayHandler gets 'this' via getCurrentThis and calls proxy->add(default, default)
    // 5. Proxy's add method uses RECORD_* macros which read from stream in Playback mode
    
    // But wait - the proxy's RECORD_CALL uses ctx() singleton, not 'player'
    // We need to test differently - verify the template infrastructure compiles and works
    
    // For this test, just verify the handler dispatch works
    bool executed = ctx().executeNextCall();
    SLANG_CHECK(executed);
    
    // In this test, the proxy's add() was called with default args (0, 0)
    // because we're testing the dispatch, not full bidirectional record/replay
    SLANG_CHECK(s_testCalcAddCalls == 1);
    
    // No more calls
    SLANG_CHECK(!ctx().hasMoreCalls());
}

// Test the MemberFunctionTraits template
SLANG_UNIT_TEST(replayContextMemberFunctionTraits)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);
    
    // Test arity detection
    using AddTraits = MemberFunctionTraits<decltype(&TestCalculatorProxy::add)>;
    static_assert(AddTraits::Arity == 2, "add should have 2 args");
    static_assert(std::is_same_v<AddTraits::ReturnType, int32_t>, "add returns int32_t");
    
    using SetOffsetTraits = MemberFunctionTraits<decltype(&TestCalculatorProxy::setOffset)>;
    static_assert(SetOffsetTraits::Arity == 1, "setOffset should have 1 arg");
    static_assert(std::is_void_v<SetOffsetTraits::ReturnType>, "setOffset returns void");
    
    // Test DefaultValue
    int32_t defInt = DefaultValue<int32_t>::get();
    SLANG_CHECK(defInt == 0);
    
    int32_t* defPtr = DefaultValue<int32_t*>::get();
    SLANG_CHECK(defPtr == nullptr);
    
    // All checks passed
    SLANG_CHECK(true);
}

// =============================================================================
// Test full round-trip: record through proxy, playback through proxy
// =============================================================================

// Test that recording via RECORD_CALL and playback via REPLAY_REGISTER work together
// This validates that parseSignature produces matching signatures in both directions
SLANG_UNIT_TEST(replayContextFullRoundTrip)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Reset test state
    s_testCalcLastA = 0;
    s_testCalcLastB = 0;
    s_testCalcAddCalls = 0;
    s_testCalcSetOffsetCalls = 0;
    s_testCalcOffset = 0;

    // Create implementation and proxy
    Slang::ComPtr<ITestCalculator> impl(new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    Slang::ComPtr<ITestCalculator> proxyPtr(proxy);

    // ========== RECORDING PHASE ==========
    ctx().reset();
    ctx().setMode(Mode::Record);
    
    // Register the proxy (simulates what happens during createSession)
    uint64_t proxyHandle = ctx().registerInterface(proxyPtr.get());
    SLANG_CHECK(proxyHandle >= kFirstValidHandle);

    // Call methods through proxy - this uses RECORD_CALL() which normalizes the signature
    int32_t result1 = proxy->add(10, 20);
    SLANG_CHECK(result1 == 30);  // Implementation adds the values
    SLANG_CHECK(s_testCalcAddCalls == 1);
    SLANG_CHECK(s_testCalcLastA == 10);
    SLANG_CHECK(s_testCalcLastB == 20);

    proxy->setOffset(5);
    SLANG_CHECK(s_testCalcSetOffsetCalls == 1);
    SLANG_CHECK(s_testCalcOffset == 5);

    int32_t result2 = proxy->add(100, 200);
    SLANG_CHECK(result2 == 305);  // 100 + 200 + 5 (offset)
    SLANG_CHECK(s_testCalcAddCalls == 2);

    // Verify we recorded something
    SLANG_CHECK(ctx().getStream().getSize() > 0);

    // ========== PLAYBACK PHASE ==========
    // Reset call tracking
    s_testCalcLastA = 0;
    s_testCalcLastB = 0;
    s_testCalcAddCalls = 0;
    s_testCalcSetOffsetCalls = 0;
    s_testCalcOffset = 0;

    // Create new implementation and proxy for playback
    Slang::ComPtr<ITestCalculator> impl2(new TestCalculatorImpl());
    TestCalculatorProxy* proxy2 = new TestCalculatorProxy(impl2.get());
    Slang::ComPtr<ITestCalculator> proxyPtr2(proxy2);

    // Switch to playback mode
    ctx().switchToPlayback();
    
    // Re-register with same handle - during real playback, this happens
    // when the creation methods are replayed
    ctx().registerInterface(proxyPtr2.get());

    // Register handlers - this is what REPLAY_REGISTER does
    // We need to use the signature that parseSignature produces from __FUNCSIG__
    // For TestCalculatorProxy::add, parseSignature extracts "TestCalculatorProxy::add"
    auto addHandler = [](ReplayContext& ctxRef) {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::add
        );
    };
    auto setOffsetHandler = [](ReplayContext& ctxRef) {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::setOffset
        );
    };
    
    // Use the exact signature that parseSignature will produce
    ctx().registerHandler("TestCalculatorProxy::add", addHandler);
    ctx().registerHandler("TestCalculatorProxy::setOffset", setOffsetHandler);

    // Execute all recorded calls
    ctx().executeAll();

    // Verify the calls were replayed
    // Note: the values should match what was recorded
    SLANG_CHECK(s_testCalcAddCalls == 2);
    SLANG_CHECK(s_testCalcSetOffsetCalls == 1);

    // The last recorded call was add(100, 200)
    SLANG_CHECK(s_testCalcLastA == 100);
    SLANG_CHECK(s_testCalcLastB == 200);
    SLANG_CHECK(s_testCalcOffset == 5);

    // No more calls
    SLANG_CHECK(!ctx().hasMoreCalls());
}

// Test parseSignature with various signature formats
SLANG_UNIT_TEST(replayContextParseSignature)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    char buffer[256];

    // Test MSVC-style __FUNCSIG__
    {
        const char* msvcSig = "SlangResult __cdecl SlangRecord::GlobalSessionProxy::createSession(struct slang::SessionDesc const &,struct slang::ISession **)";
        const char* result = ReplayContext::parseSignature(msvcSig, buffer, sizeof(buffer));
        SLANG_CHECK(strcmp(result, "GlobalSessionProxy::createSession") == 0);
    }

    // Test with void return type
    {
        const char* voidSig = "void __cdecl SlangRecord::SessionProxy::addSearchPath(char const *)";
        const char* result = ReplayContext::parseSignature(voidSig, buffer, sizeof(buffer));
        SLANG_CHECK(strcmp(result, "SessionProxy::addSearchPath") == 0);
    }

    // Test with virtual and other modifiers
    {
        const char* virtualSig = "virtual SlangProfileID __cdecl SlangRecord::GlobalSessionProxy::findProfile(char const *)";
        const char* result = ReplayContext::parseSignature(virtualSig, buffer, sizeof(buffer));
        SLANG_CHECK(strcmp(result, "GlobalSessionProxy::findProfile") == 0);
    }

    // Test simple signature (no namespace)
    {
        const char* simpleSig = "int MyClass::myMethod(int, int)";
        const char* result = ReplayContext::parseSignature(simpleSig, buffer, sizeof(buffer));
        // Should handle this case gracefully
        SLANG_CHECK(result != nullptr);
    }
}

// =============================================================================
// End-to-End Playback Test: Global Session and Session Creation
// =============================================================================

// This test validates the full playback process for creating a global session
// and a session within it. The test has 3 stages:
// 
// Stage 1: Without replay system, create objects to establish baseline behavior
// Stage 2: With recording enabled, create same objects and verify proxy wrapping
// Stage 3: Playback the recording and verify objects are recreated correctly

SLANG_UNIT_TEST(replayContextEndToEndSessionPlayback)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Start recording
    ctx().setMode(Mode::Record);

    // Create a global session without replay
    SlangProfileID baselineProfile;
    Slang::ComPtr<slang::IGlobalSession> baselineGlobalSession;
    Slang::ComPtr<slang::ISession> baselineSession;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, baselineGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = baselineGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        SLANG_CHECK(SLANG_SUCCEEDED(baselineGlobalSession->createSession(sessionDesc, baselineSession.writeRef())));
        baselineProfile = targetDesc.profile;
        SLANG_CHECK(baselineProfile != SLANG_PROFILE_UNKNOWN);
    }

    // =========================================================================
    // Stage 2: Create objects WITH recording enabled and verify proxy wrapping
    // =========================================================================

    // Enable recording
    ctx().enable();
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Pretty much identical process but recording this time. From outside perspective,
    // should have exactly the same data (albeit wrapped in proxys)
    Slang::ComPtr<slang::IGlobalSession> recordedGlobalSession;
    Slang::ComPtr<slang::ISession> recordedSession;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(slang_createGlobalSession2(&globalDesc, recordedGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = recordedGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        SLANG_CHECK(SLANG_SUCCEEDED(recordedGlobalSession->createSession(sessionDesc, recordedSession.writeRef())));
        SLANG_CHECK(baselineProfile == targetDesc.profile);
    }

    // Get the handle for the session so we can look it up after playback
    uint64_t recordedSessionHandle = ctx().getHandleForInterface(recordedSession.get());
    SLANG_CHECK(recordedSessionHandle >= kFirstValidHandle);

    // =========================================================================
    // Stage 3: Playback the recording and verify objects are recreated
    // =========================================================================

    // Switch to playback mode - this resets handles but keeps stream data
    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());

    // Execute all recorded calls
    // This should recreate the global session and session
    ctx().executeAll();

    // Look up the session by its handle - it should exist after playback
    ISlangUnknown* playedBackSessionUnk = ctx().getInterfaceForHandle(recordedSessionHandle);
    SLANG_CHECK(playedBackSessionUnk != nullptr);

    // Verify we can query the ISession interface
    Slang::ComPtr<slang::ISession> playedBackSession;
    SLANG_CHECK(SLANG_SUCCEEDED(playedBackSessionUnk->queryInterface(
        slang::ISession::getTypeGuid(), (void**)playedBackSession.writeRef())));

}
