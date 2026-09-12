// unit-test-replay-playback.cpp
// Unit tests for playback dispatcher and handler infrastructure

#include "unit-test-replay-common.h"

// =============================================================================
// Test REPLAY_REGISTER macro - using a simple test proxy
// =============================================================================

// Simple test interface for replay macro testing
struct ITestCalculator : public ISlangUnknown
{
    SLANG_COM_INTERFACE(
        0x12345678,
        0x1234,
        0x1234,
        {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0});

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
    TestCalculatorProxy(ITestCalculator* actual)
        : m_actual(actual), m_refCount(1)
    {
    }
    virtual ~TestCalculatorProxy() = default;

    // ISlangUnknown
    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) override
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
        if (count == 0)
            delete this;
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

        if (m_actual)
            m_actual->setOffset(offset);
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
    TestCalculatorImpl()
        : m_offset(0), m_refCount(1)
    {
    }
    virtual ~TestCalculatorImpl() = default;

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) override
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
        if (count == 0)
            delete this;
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
    // TestCalculatorImpl's constructor starts the refcount at 1, so attach
    // rather than add a second reference the test never releases (#11936).
    Slang::ComPtr<ITestCalculator> impl(Slang::INIT_ATTACH, new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    // The constructor starts the refcount at 1; adopt that reference with attach
    // semantics rather than adding a second one the test never releases (#11936).
    Slang::ComPtr<ITestCalculator> proxyPtr(Slang::INIT_ATTACH, proxy);

    // Build a recorded stream manually with known signatures
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Register the proxy and get its handle
    uint64_t proxyHandle = ctx().testsOnlyRegisterProxy(proxyPtr.get());
    SLANG_CHECK(proxyHandle >= kFirstValidHandle);

    // Record a call manually with a simple signature we control
    const char* addSignature = "TestCalculatorProxy::add";
    ctx().record(RecordFlag::Input, addSignature); // signature

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
    ctx().testsOnlyRegisterProxy(proxyPtr.get()); // Same handle value

    // Register a handler using the replayHandler template (what REPLAY_REGISTER does internally)
    auto addHandler = [](ReplayContext& ctxRef)
    {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::add);
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
    // TestCalculatorImpl's constructor starts the refcount at 1, so attach
    // rather than add a second reference the test never releases (#11936).
    Slang::ComPtr<ITestCalculator> impl(Slang::INIT_ATTACH, new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    // The constructor starts the refcount at 1; adopt that reference with attach
    // semantics rather than adding a second one the test never releases (#11936).
    Slang::ComPtr<ITestCalculator> proxyPtr(Slang::INIT_ATTACH, proxy);

    // ========== RECORDING PHASE ==========
    ctx().reset();
    ctx().setMode(Mode::Record);

    // Register the proxy (simulates what happens during createSession)
    uint64_t proxyHandle = ctx().testsOnlyRegisterProxy(proxyPtr.get());
    SLANG_CHECK(proxyHandle >= kFirstValidHandle);

    // Call methods through proxy - this uses RECORD_CALL() which normalizes the signature
    int32_t result1 = proxy->add(10, 20);
    SLANG_CHECK(result1 == 30); // Implementation adds the values
    SLANG_CHECK(s_testCalcAddCalls == 1);
    SLANG_CHECK(s_testCalcLastA == 10);
    SLANG_CHECK(s_testCalcLastB == 20);

    proxy->setOffset(5);
    SLANG_CHECK(s_testCalcSetOffsetCalls == 1);
    SLANG_CHECK(s_testCalcOffset == 5);

    int32_t result2 = proxy->add(100, 200);
    SLANG_CHECK(result2 == 305); // 100 + 200 + 5 (offset)
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
    // TestCalculatorImpl's constructor starts the refcount at 1, so attach
    // rather than add a second reference the test never releases (#11936).
    Slang::ComPtr<ITestCalculator> impl2(Slang::INIT_ATTACH, new TestCalculatorImpl());
    TestCalculatorProxy* proxy2 = new TestCalculatorProxy(impl2.get());
    // The constructor starts the refcount at 1; adopt that reference with attach
    // semantics rather than adding a second one the test never releases (#11936).
    Slang::ComPtr<ITestCalculator> proxyPtr2(Slang::INIT_ATTACH, proxy2);

    // Switch to playback mode
    ctx().switchToPlayback();

    // Re-register with same handle - during real playback, this happens
    // when the creation methods are replayed
    ctx().testsOnlyRegisterProxy(proxyPtr2.get());

    // Register handlers - this is what REPLAY_REGISTER does
    // We need to use the signature that parseSignature produces from __FUNCSIG__
    // For TestCalculatorProxy::add, parseSignature extracts "TestCalculatorProxy::add"
    auto addHandler = [](ReplayContext& ctxRef)
    {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::add);
    };
    auto setOffsetHandler = [](ReplayContext& ctxRef)
    {
        SlangRecord::replayHandler<ITestCalculator, TestCalculatorProxy>(
            ctxRef,
            &TestCalculatorProxy::setOffset);
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
        const char* msvcSig =
            "SlangResult __cdecl SlangRecord::GlobalSessionProxy::createSession(struct "
            "slang::SessionDesc const &,struct slang::ISession **)";
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
        const char* virtualSig = "virtual SlangProfileID __cdecl "
                                 "SlangRecord::GlobalSessionProxy::findProfile(char const *)";
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
        SLANG_CHECK(SLANG_SUCCEEDED(
            slang_createGlobalSession2(&globalDesc, baselineGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = baselineGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        SLANG_CHECK(SLANG_SUCCEEDED(
            baselineGlobalSession->createSession(sessionDesc, baselineSession.writeRef())));
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
        SLANG_CHECK(SLANG_SUCCEEDED(
            slang_createGlobalSession2(&globalDesc, recordedGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = recordedGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        SLANG_CHECK(SLANG_SUCCEEDED(
            recordedGlobalSession->createSession(sessionDesc, recordedSession.writeRef())));
        SLANG_CHECK(baselineProfile == targetDesc.profile);
    }

    // Get the handle for the session so we can look it up after playback
    uint64_t recordedSessionHandle = ctx().getProxyHandle(recordedSession.get());
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
    ctx().disable(); // Stop playback

    // Look up the session by its handle - it should exist after playback
    ISlangUnknown* playedBackSessionUnk = ctx().getProxy(recordedSessionHandle);
    SLANG_CHECK(playedBackSessionUnk != nullptr);

    // Replaying createSession wrapped the recreated session on the real
    // recordInterfaceImpl isOutput path, which notes the orphaned creation
    // reference (#11936). Pin that the production dispatcher-to-bookkeeping wiring
    // actually fired here -- the other orphan tests drive the bookkeeping through
    // testsOnly* stand-ins, so without this a regression that stopped the
    // dispatcher noting orphans would pass every unit assertion and only surface
    // as a sanitizer leak. (The teardown drain is covered by the sweep tests.)
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(playedBackSessionUnk) > 0);

    // Verify we can query the ISession interface
    Slang::ComPtr<slang::ISession> playedBackSession;
    SLANG_CHECK(SLANG_SUCCEEDED(playedBackSessionUnk->queryInterface(
        slang::ISession::getTypeGuid(),
        (void**)playedBackSession.writeRef())));
}

// A test-only proxy that owns another ITestCalculator, so that releasing it
// cascade-destroys the one it holds. It mirrors the two things ProxyBase does
// that the orphan sweep depends on: it unregisters itself from the context when
// it is destroyed, and it drops its owned reference at the same time.
class TestOwningProxy : public ITestCalculator
{
public:
    TestOwningProxy(ITestCalculator* owned)
        : m_owned(owned), m_refCount(1)
    {
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) override
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
        if (count == 0)
            delete this;
        return count;
    }

    int32_t SLANG_MCALL add(int32_t a, int32_t b) override { return a + b; }
    void SLANG_MCALL setOffset(int32_t offset) override { SLANG_UNUSED(offset); }

    virtual ~TestOwningProxy()
    {
        s_owningProxyDestroyed++;
        // tryGet(), mirroring ~ProxyBase: a proxy destroyed by the teardown
        // sweep must not construct a context to unregister into.
        if (ReplayContext* context = ReplayContext::tryGet())
        {
            context->unregisterProxy(
                static_cast<ISlangUnknown*>(static_cast<ITestCalculator*>(this)));
        }
        m_owned = nullptr;
    }

    /// Counts destructions, so a test can tell that each proxy was destroyed
    /// exactly once rather than inferring it from a refcount it can no longer
    /// safely read.
    static int s_owningProxyDestroyed;

private:
    Slang::ComPtr<ITestCalculator> m_owned;
    uint32_t m_refCount;
};

int TestOwningProxy::s_owningProxyDestroyed = 0;

// The orphan bookkeeping added for #11936 keeps the replay path from leaking a
// proxy the dispatcher created. The production noter is the replay dispatcher,
// which no unit test can drive, so the accounting is pinned directly here.
SLANG_UNIT_TEST(replayContextOrphanedProxyAccounting)
{
    // REPLAY_TEST declares the fixture that resets the context; this test drives
    // the context through ctx() and never needs unitTestContext itself.
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    Slang::ComPtr<ITestCalculator> impl(Slang::INIT_ATTACH, new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    Slang::ComPtr<ITestCalculator> proxyPtr(Slang::INIT_ATTACH, proxy);
    ISlangUnknown* key = static_cast<ISlangUnknown*>(proxyPtr.get());

    ctx().reset();
    // The sweep only releases proxies that are in the handle registry, so a test
    // proxy has to be registered the way wrapObject() would have registered it.
    ctx().testsOnlyRegisterProxy(proxy);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 0);

    // A proxy can be handed back more than once in one replay, so notes
    // accumulate rather than overwrite -- otherwise the second orphaned
    // reference would never be released.
    ctx().testsOnlyNoteOrphanedProxy(key);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 1);
    ctx().testsOnlyNoteOrphanedProxy(key);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 2);

    // Nulls are ignored rather than tracked.
    ctx().testsOnlyNoteOrphanedProxy(nullptr);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(nullptr) == 0);

    // Teardown releases exactly as many references as were noted. Two notes were
    // recorded above, so take two references here for the sweep to consume; the
    // proxy must survive with the single reference proxyPtr owns.
    proxyPtr->addRef();
    proxyPtr->addRef();
    ctx().testsOnlyReleaseOrphanedProxies();
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 0);
    // Still alive and owned solely by proxyPtr: releasing once more would
    // destroy it, so query instead and check the count came back to one.
    SLANG_CHECK(proxyPtr->addRef() == 2);
    SLANG_CHECK(proxyPtr->release() == 1);

    ctx().unregisterProxy(key);
}

// A proxy that the replayed release stream drives to refcount 0 destroys itself
// through ~ProxyBase -> unregisterProxy. That has to scrub the orphan note too,
// or the teardown sweep would release an object that is already freed.
SLANG_UNIT_TEST(replayContextUnregisterScrubsOrphanNote)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    Slang::ComPtr<ITestCalculator> impl(Slang::INIT_ATTACH, new TestCalculatorImpl());
    TestCalculatorProxy* proxy = new TestCalculatorProxy(impl.get());
    Slang::ComPtr<ITestCalculator> proxyPtr(Slang::INIT_ATTACH, proxy);
    ISlangUnknown* key = static_cast<ISlangUnknown*>(proxyPtr.get());

    ctx().reset();
    ctx().testsOnlyRegisterProxy(proxy);
    ctx().testsOnlyNoteOrphanedProxy(key);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 1);

    // Stands in for ~ProxyBase running because the replayed stream released the
    // last reference.
    ctx().unregisterProxy(key);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 0);

    // With the note scrubbed the sweep has nothing to do, so the reference
    // proxyPtr still owns is untouched -- without the scrub this would release
    // it and the ComPtr below would release freed memory.
    ctx().testsOnlyReleaseOrphanedProxies();
    SLANG_CHECK(proxyPtr->addRef() == 2);
    SLANG_CHECK(proxyPtr->release() == 1);
}

// destroySingleton() has to drain the orphan set itself: it deletes the context
// outright and so reaches neither reset() nor switchTo*(), and without a drain
// the noted references are dropped while still live. It also has to keep the
// context reachable while draining, since the proxy destructors it runs
// unregister through it -- which is what s_contextDraining is for.
SLANG_UNIT_TEST(replayContextDestroySingletonDrainsWithoutResurrecting)
{
    SLANG_UNUSED(unitTestContext);

    // Deliberately not REPLAY_TEST: that fixture calls reset() on scope exit,
    // and reset() drains the orphan set, so the set would be empty before
    // destroySingleton() ever ran and there would be nothing to observe.
    ctx().reset();

    // TestOwningProxy is the proxy here whose destructor reaches for the context
    // to unregister itself, the way ~ProxyBase does. That is what makes the
    // ordering observable: a proxy that never touches the context on the way out
    // cannot tell the two implementations apart.
    TestOwningProxy* proxy = new TestOwningProxy(nullptr);
    ISlangUnknown* key = static_cast<ISlangUnknown*>(static_cast<ITestCalculator*>(proxy));

    // One reference, registered and noted, so the sweep in destroySingleton()
    // releases it and runs that destructor during teardown.
    ctx().testsOnlyRegisterProxy(proxy);
    ctx().testsOnlyNoteOrphanedProxy(key);

    const int destroyedBefore = TestOwningProxy::s_owningProxyDestroyed;
    ReplayContext::destroySingleton();

    // The drain ran: the orphaned reference was released and the proxy
    // destroyed, rather than dropped on the floor with the map.
    SLANG_CHECK(TestOwningProxy::s_owningProxyDestroyed == destroyedBefore + 1);

    // And nothing recreated the singleton on the way out.
    SLANG_CHECK(ReplayContext::tryGet() == nullptr);

    // Destroying the singleton took the default playback handlers with it, so
    // put them back for the rest of the suite (same restore the shutdown-leak
    // test does after resetHandlers()).
    ctx().registerDefaultHandlers();
    SLANG_CHECK(ctx().getHandlerCount() > 0);
}

// Releasing one orphan can cascade-destroy another that is also in the sweep's
// snapshot. The sweep must notice the second one is already gone -- it checks
// the handle registry, which the cascaded destructor removed itself from --
// instead of releasing freed memory.
//
// This is the same shape as the real dependency the sweep exists to survive: a
// SessionProxy holds the Linkage whose teardown writes through to the global
// Session that a GlobalSessionProxy holds. So the owner has to be released
// first, which is what the descending-handle order in the sweep guarantees;
// registering `owned` before `owner` gives the owner the higher handle.
SLANG_UNIT_TEST(replayContextOrphanSweepStopsAtCascadeDestroy)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().reset();
    TestOwningProxy::s_owningProxyDestroyed = 0;

    // `owned` is deliberately left with exactly one reference, held by `owner`.
    // That is what makes the owner's destruction destroy it outright, so the
    // sweep reaches an entry whose object is already gone.
    TestOwningProxy* owned = new TestOwningProxy(nullptr);
    ISlangUnknown* ownedKey = static_cast<ISlangUnknown*>(static_cast<ITestCalculator*>(owned));
    ctx().testsOnlyRegisterProxy(owned);

    TestOwningProxy* owner = new TestOwningProxy(owned); // takes a reference
    ISlangUnknown* ownerKey = static_cast<ISlangUnknown*>(static_cast<ITestCalculator*>(owner));
    ctx().testsOnlyRegisterProxy(owner);
    owned->release(); // owner is now the sole owner

    ctx().testsOnlyNoteOrphanedProxy(ownedKey);
    ctx().testsOnlyNoteOrphanedProxy(ownerKey);

    // Releasing `owner` destroys it, which drops the last reference to `owned`
    // and destroys that too. The sweep then reaches `owned`'s entry; without the
    // registry check it would release an object that no longer exists.
    ctx().testsOnlyReleaseOrphanedProxies();

    SLANG_CHECK(TestOwningProxy::s_owningProxyDestroyed == 2);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(ownerKey) == 0);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(ownedKey) == 0);
}

// The entry-point double-reference case, the reason a note is deliberately kept
// rather than unnoted. On the playback path a RECORD_ENTRYPOINT_OUTPUT proxy
// sits at refcount 2: the orphaned creation reference the sweep must release,
// plus the reference m_returnedEntryPoints holds so the entry point outlives the
// sweep and is destroyed only when its owning component proxy is torn down. The
// retention does not balance the creation reference, so the sweep has to take
// the proxy 2 -> 1 and leave the retained reference holding the last one.
SLANG_UNIT_TEST(replayContextOrphanSweepKeepsEntryPointRetention)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    ctx().reset();
    TestOwningProxy::s_owningProxyDestroyed = 0;

    // Refcount 1: the orphaned creation reference the dispatcher never handed out.
    TestOwningProxy* entryPoint = new TestOwningProxy(nullptr);
    ISlangUnknown* key = static_cast<ISlangUnknown*>(static_cast<ITestCalculator*>(entryPoint));
    ctx().testsOnlyRegisterProxy(entryPoint);
    ctx().testsOnlyNoteOrphanedProxy(key);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 1);

    // Stands in for the m_returnedEntryPoints retention: a second reference held
    // by the owning component, taking the proxy to refcount 2.
    entryPoint->addRef();

    // The sweep releases the one noted creation reference (2 -> 1). The entry
    // point must survive on the retained reference rather than be destroyed.
    ctx().testsOnlyReleaseOrphanedProxies();
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(key) == 0);
    SLANG_CHECK(TestOwningProxy::s_owningProxyDestroyed == 0);

    // Dropping the retained reference -- the owning component being torn down --
    // is what finally destroys the entry point, exactly once.
    entryPoint->release();
    SLANG_CHECK(TestOwningProxy::s_owningProxyDestroyed == 1);
}

// =============================================================================
// Custom-file-system createSession playback tests
// =============================================================================

// A minimal user-supplied ISlangFileSystem, used to drive createSession down the
// custom-file-system arms of GlobalSessionProxy::createSession that the default
// tests never reach. loadFile is stubbed because these tests create a session but
// compile nothing.
//
// The double deliberately denies the extended file-system interfaces
// (ISlangFileSystemExt / ISlangMutableFileSystem) on *both* probe paths that the
// wrap machinery consults, and the two must agree: tryWrap() selects the proxy
// overload by queryInterface (proxy-base.cpp), and the chosen
// MutableFileSystemProxy(ISlangFileSystem*) constructor then re-probes via castAs
// (proxy-mutable-file-system.h) to decide whether it may forward extended
// operations. Denying on only one path would be internally inconsistent, so both
// queryInterface and castAs report a plain read-only ISlangFileSystem here.
//
// s_liveCount lets a test observe that every reference the createSession
// record/playback paths take on a user file system is also released: a residual
// reference leaves the count above zero after teardown, which a leak sanitizer
// would flag but a Debug run would otherwise miss.
class TestFileSystem : public ISlangFileSystem
{
public:
    TestFileSystem() { ++s_liveCount; }
    virtual ~TestFileSystem() { --s_liveCount; }

    SLANG_NO_THROW SlangResult SLANG_MCALL
    queryInterface(SlangUUID const& uuid, void** outObject) override
    {
        // Single-inheritance chain (ISlangFileSystem : ISlangCastable : ISlangUnknown),
        // so `this` is the canonical identity for all three; the extended file-system
        // interfaces are denied here -- this is the first of the two probe paths the
        // wrapper consults (see the class comment on why both must agree).
        if (uuid == ISlangFileSystem::getTypeGuid() || uuid == ISlangCastable::getTypeGuid() ||
            uuid == ISlangUnknown::getTypeGuid())
        {
            *outObject = static_cast<ISlangFileSystem*>(this);
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
        if (count == 0)
            delete this;
        return count;
    }

    SLANG_NO_THROW void* SLANG_MCALL castAs(SlangUUID const& uuid) override
    {
        // The second probe path (see the class comment): returning null keeps castAs
        // consistent with queryInterface's denial of the extended/mutable file-system
        // interfaces, so the MutableFileSystemProxy wrapper treats this double as a
        // plain read-only file system.
        SLANG_UNUSED(uuid);
        return nullptr;
    }

    SLANG_NO_THROW SlangResult SLANG_MCALL loadFile(char const* path, ISlangBlob** outBlob) override
    {
        SLANG_UNUSED(path);
        SLANG_UNUSED(outBlob);
        return SLANG_E_NOT_IMPLEMENTED;
    }

    /// Number of live instances, so a test can assert the createSession paths leak
    /// no reference onto a user-supplied file system.
    static std::atomic<int> s_liveCount;

private:
    std::atomic<uint32_t> m_refCount{1};
};

std::atomic<int> TestFileSystem::s_liveCount{0};

// Record and play back a single createSession that supplies a custom
// ISlangFileSystem on SessionDesc::fileSystem. This drives the not-yet-registered
// write arm (records kCustomFileSystemHandle) and, on playback, the matching
// kCustomFileSystemHandle branch that wraps a fresh per-call ReplayNullFileSystem
// as the stand-in file system and takes the guarded owning-reference release() in
// GlobalSessionProxy::createSession -- an arm no default-file-system test reaches.
//
// The test pins three things independently:
//   * TestFileSystem::s_liveCount back to 0 at teardown tracks the user-supplied
//     file system (the write side): the write kCustomFileSystemHandle arm balances
//     the reference it takes on that object.
//   * ReplayNullFileSystem's live count (testsOnlyReplayNullFileSystemLiveCount)
//     tracks the playback stand-in: it is created during executeAll and self-deleted
//     when its wrapper proxy dies. This gives the #12865 stand-in leak fix a
//     deterministic guard in every build, not only under the leak sanitizer.
//   * The orphan-count check confirms the recreated session was noted as an orphaned
//     playback proxy, so a regression that stopped the dispatcher noting orphans
//     fails here rather than only surfacing as a sanitizer leak.
//
// replayContextCustomFileSystemRegisteredSessionPlayback below is the dedicated
// regression check for the registered-reuse leak (#12470).
SLANG_UNIT_TEST(replayContextCustomFileSystemSessionPlayback)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Deliberate reset for test isolation: TestFileSystem is used only by these three
    // tests and each asserts the count back to 0 at teardown, so start from a known 0.
    TestFileSystem::s_liveCount = 0;

    // The constructor starts the refcount at 1; adopt that reference rather than
    // adding a second one the test never releases.
    TestFileSystem* fileSystem = new TestFileSystem();
    Slang::ComPtr<ISlangFileSystem> fileSystemPtr(Slang::INIT_ATTACH, fileSystem);
    SLANG_CHECK(TestFileSystem::s_liveCount == 1);

    ctx().enable();
    ctx().reset();
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> recordedGlobalSession;
    Slang::ComPtr<slang::ISession> recordedSession;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(
            slang_createGlobalSession2(&globalDesc, recordedGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = recordedGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        // The custom file system routes createSession down the kCustomFileSystemHandle
        // path instead of the default-file-system path every other test exercises.
        sessionDesc.fileSystem = fileSystemPtr.get();
        SLANG_CHECK(SLANG_SUCCEEDED(
            recordedGlobalSession->createSession(sessionDesc, recordedSession.writeRef())));
    }

    uint64_t recordedSessionHandle = ctx().getProxyHandle(recordedSession.get());
    SLANG_CHECK(recordedSessionHandle >= kFirstValidHandle);

    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());
    ctx().executeAll();
    ctx().disable();

    // Playback took the reading kCustomFileSystemHandle arm, so a stand-in
    // ReplayNullFileSystem was created and is still held by the recreated session's
    // file-system proxy. This assertion directly guards the #12865 fix: reverting
    // the arm to the old, untracked new NULLFileSystem() would leave this counter at
    // 0 and fail here.
    SLANG_CHECK(SlangRecord::testsOnlyReplayNullFileSystemLiveCount() >= 1);

    // The session was recreated by playback and noted as an orphaned proxy.
    ISlangUnknown* playedBackSessionUnk = ctx().getProxy(recordedSessionHandle);
    SLANG_CHECK(playedBackSessionUnk != nullptr);
    SLANG_CHECK(ctx().testsOnlyGetOrphanedRefCount(playedBackSessionUnk) > 0);

    // Drop everything that could hold a reference on the user file system -- the
    // recorded session (whose real session holds the file-system proxy that in turn
    // holds our object) and the registries/orphan set that reset() drains -- then
    // release our own reference. If the createSession paths balanced their
    // references, the object is gone; a residual reference is a leak.
    recordedSession.setNull();
    recordedGlobalSession.setNull();
    ctx().reset();
    // Draining the context destroyed the recreated session's file-system proxy,
    // which released the stand-in to a zero refcount and freed it. A non-zero count
    // here means a ReplayNullFileSystem was created but never destroyed -- i.e. the
    // stand-in leaked.
    SLANG_CHECK(SlangRecord::testsOnlyReplayNullFileSystemLiveCount() == 0);
    SLANG_CHECK(TestFileSystem::s_liveCount == 1); // only fileSystemPtr remains
    fileSystemPtr.setNull();
    SLANG_CHECK(TestFileSystem::s_liveCount == 0);
}

// Record and play back two createSession calls that supply the *same* custom
// ISlangFileSystem object. The first call sees an unregistered file system and
// records kCustomFileSystemHandle (as above); the second sees it already registered
// (isInterfaceRegistered / getProxyHandle) and records the file-system proxy handle,
// so on playback the second call takes the `default:` branch --
// `toSlangInterface(getProxy(handle))`, a *borrowed* pointer for which
// ownsFileSystemWrapper stays false and no release() runs. That arm's failure mode
// is a double-release / use-after-free rather than a leak, which the leak
// suppression net structurally cannot catch, so covering it needs a test that
// executes it. The s_liveCount check additionally catches a residual reference on
// the registered-file-system write arm.
SLANG_UNIT_TEST(replayContextCustomFileSystemRegisteredSessionPlayback)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    // Deliberate reset for test isolation: TestFileSystem is used only by these three
    // tests and each asserts the count back to 0 at teardown, so start from a known 0.
    TestFileSystem::s_liveCount = 0;

    TestFileSystem* fileSystem = new TestFileSystem();
    Slang::ComPtr<ISlangFileSystem> fileSystemPtr(Slang::INIT_ATTACH, fileSystem);
    SLANG_CHECK(TestFileSystem::s_liveCount == 1);

    ctx().enable();
    ctx().reset();
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> recordedGlobalSession;
    Slang::ComPtr<slang::ISession> recordedSession1;
    Slang::ComPtr<slang::ISession> recordedSession2;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(
            slang_createGlobalSession2(&globalDesc, recordedGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = recordedGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        sessionDesc.fileSystem = fileSystemPtr.get();

        // First call: file system not yet registered -> kCustomFileSystemHandle.
        SLANG_CHECK(SLANG_SUCCEEDED(
            recordedGlobalSession->createSession(sessionDesc, recordedSession1.writeRef())));
        // Second call, same file-system object: now registered -> records its proxy
        // handle, so playback takes the `default:` branch.
        SLANG_CHECK(SLANG_SUCCEEDED(
            recordedGlobalSession->createSession(sessionDesc, recordedSession2.writeRef())));
    }

    uint64_t sessionHandle1 = ctx().getProxyHandle(recordedSession1.get());
    uint64_t sessionHandle2 = ctx().getProxyHandle(recordedSession2.get());
    SLANG_CHECK(sessionHandle1 >= kFirstValidHandle);
    SLANG_CHECK(sessionHandle2 >= kFirstValidHandle);
    SLANG_CHECK(sessionHandle1 != sessionHandle2);

    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());
    ctx().executeAll();
    ctx().disable();

    // Both sessions were recreated: the first via the kCustomFileSystemHandle branch,
    // the second via the borrowed `default:` branch.
    SLANG_CHECK(ctx().getProxy(sessionHandle1) != nullptr);
    SLANG_CHECK(ctx().getProxy(sessionHandle2) != nullptr);

    recordedSession1.setNull();
    recordedSession2.setNull();
    recordedGlobalSession.setNull();
    ctx().reset();
    SLANG_CHECK(TestFileSystem::s_liveCount == 1); // only fileSystemPtr remains
    fileSystemPtr.setNull();
    SLANG_CHECK(TestFileSystem::s_liveCount == 0);
}

// Record and play back two createSession calls with *distinct* custom file-system
// objects. Both are unregistered at their call, so both record kCustomFileSystemHandle
// and both take the reading kCustomFileSystemHandle arm on playback. Recording wraps two
// distinct file systems into two distinct proxies (two handle allocations); playback must
// allocate a distinct stand-in proxy per occurrence too, or the handle counter drifts from
// recording and the later session's recorded handle no longer resolves. This pins that the
// stand-in on that arm keeps per-call identity (a shared, deduplicated instance would make
// the second occurrence reuse the first proxy and desync the handles).
SLANG_UNIT_TEST(replayContextTwoDistinctCustomFileSystemsPlayback)
{
    REPLAY_TEST;
    SLANG_UNUSED(unitTestContext);

    TestFileSystem::s_liveCount = 0;

    TestFileSystem* fileSystemA = new TestFileSystem();
    Slang::ComPtr<ISlangFileSystem> fileSystemAPtr(Slang::INIT_ATTACH, fileSystemA);
    TestFileSystem* fileSystemB = new TestFileSystem();
    Slang::ComPtr<ISlangFileSystem> fileSystemBPtr(Slang::INIT_ATTACH, fileSystemB);
    SLANG_CHECK(TestFileSystem::s_liveCount == 2);

    ctx().enable();
    ctx().reset();
    ctx().setMode(Mode::Record);

    Slang::ComPtr<slang::IGlobalSession> recordedGlobalSession;
    Slang::ComPtr<slang::ISession> recordedSessionA;
    Slang::ComPtr<slang::ISession> recordedSessionB;
    {
        SlangGlobalSessionDesc globalDesc = {};
        globalDesc.apiVersion = 0;
        SLANG_CHECK(SLANG_SUCCEEDED(
            slang_createGlobalSession2(&globalDesc, recordedGlobalSession.writeRef())));
        slang::SessionDesc sessionDesc = {};
        slang::TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = recordedGlobalSession->findProfile("spirv_1_5");
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;

        // First distinct custom file system -> kCustomFileSystemHandle.
        sessionDesc.fileSystem = fileSystemAPtr.get();
        SLANG_CHECK(SLANG_SUCCEEDED(
            recordedGlobalSession->createSession(sessionDesc, recordedSessionA.writeRef())));
        // Second, *different* custom file system: also unregistered, so also
        // kCustomFileSystemHandle -- the case that reuse of a shared stand-in would break.
        sessionDesc.fileSystem = fileSystemBPtr.get();
        SLANG_CHECK(SLANG_SUCCEEDED(
            recordedGlobalSession->createSession(sessionDesc, recordedSessionB.writeRef())));
    }

    uint64_t sessionHandleA = ctx().getProxyHandle(recordedSessionA.get());
    uint64_t sessionHandleB = ctx().getProxyHandle(recordedSessionB.get());
    SLANG_CHECK(sessionHandleA >= kFirstValidHandle);
    SLANG_CHECK(sessionHandleB >= kFirstValidHandle);
    SLANG_CHECK(sessionHandleA != sessionHandleB);

    ctx().switchToPlayback();
    SLANG_CHECK(ctx().isPlayback());
    ctx().executeAll();
    ctx().disable();

    // Both sessions must resolve at their recorded handles: a handle desync on the
    // second kCustomFileSystemHandle occurrence would leave sessionHandleB unresolved.
    ISlangUnknown* playedBackA = ctx().getProxy(sessionHandleA);
    ISlangUnknown* playedBackB = ctx().getProxy(sessionHandleB);
    SLANG_CHECK(playedBackA != nullptr);
    SLANG_CHECK(playedBackB != nullptr);
    SLANG_CHECK(playedBackA != playedBackB);

    recordedSessionA.setNull();
    recordedSessionB.setNull();
    recordedGlobalSession.setNull();
    ctx().reset();
    SLANG_CHECK(TestFileSystem::s_liveCount == 2); // only the two ComPtrs remain
    fileSystemAPtr.setNull();
    fileSystemBPtr.setNull();
    SLANG_CHECK(TestFileSystem::s_liveCount == 0);
}
