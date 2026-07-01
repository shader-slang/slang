#pragma once

#include "slang-support.h"

#include <mutex>
#include <slang-rhi.h>
#include <string>
#include <unordered_map>
#include <vector>

// Device Cache for preventing NVIDIA Tegra driver state corruption
// This cache reuses Vulkan instances and devices to avoid the VK_ERROR_INCOMPATIBLE_DRIVER
// issue that occurs after ~19 device creation/destruction cycles on Tegra platforms.
// Uses ComPtr for automatic device lifecycle management - devices are released when removed from
// cache.
class DeviceCache
{
public:
    struct DeviceCacheKey
    {
        rhi::DeviceType deviceType;
        bool enableValidation;
        bool enableRayTracingValidation;
        std::string profileName;
        SlangTargetFlags targetFlags;
        SlangMatrixLayoutMode defaultMatrixLayoutMode;
        uint32_t nvapiExtUavSlot;
        bool dx12ExperimentalFeatures;

        bool operator==(const DeviceCacheKey& other) const;
    };

    struct DeviceCacheKeyHash
    {
        std::size_t operator()(const DeviceCacheKey& key) const;
    };

    struct CachedDevice
    {
        Slang::ComPtr<rhi::IDevice> device;
        uint64_t creationOrder;

        CachedDevice();
    };

private:
    static constexpr int MAX_CACHED_DEVICES = 10;

    // Use function-local statics to control destruction order (Meyer's singleton pattern)
    static std::mutex& getMutex();
    static std::unordered_map<DeviceCacheKey, CachedDevice, DeviceCacheKeyHash>& getDeviceCache();
    static uint64_t& getNextCreationOrder();

    // Maps a device key to the debug bridge wired into that key's cached device. Unlike the device
    // cache it is never evicted: bridges are tiny and, like the ones from
    // createRetainedCoreToRHIDebugBridge(), must outlive any retained device state that could emit
    // late messages, so a device re-created for a key after eviction re-wires the same bridge.
    static std::unordered_map<
        DeviceCacheKey,
        Slang::RefPtr<renderer_test::CoreToRHIDebugBridge>,
        DeviceCacheKeyHash>&
    getBridgeCache();

    // Builds the cache key from a device descriptor. Shared by acquireDevice and acquireDebugBridge
    // so a device and its debug bridge are always keyed identically.
    static DeviceCacheKey makeKey(const rhi::DeviceDesc& desc);

    static void evictOldestDeviceIfNeeded();

public:
    static SlangResult acquireDevice(const rhi::DeviceDesc& desc, rhi::IDevice** outDevice);

    // Returns the debug bridge that the cached device for `desc` is (or will be) wired to, creating
    // and retaining one on first use. Callers set `desc.debugCallback` to it before acquireDevice
    // and bind their per-invocation ScopedCoreDebugCallback to it, so a cache hit rebinds the live
    // device's bridge instead of an unrelated fresh one (see #11856).
    static Slang::RefPtr<renderer_test::CoreToRHIDebugBridge> acquireDebugBridge(
        const rhi::DeviceDesc& desc);

    static void cleanCache();
};

// RAII wrapper for cached devices to ensure proper cleanup
class CachedDeviceWrapper
{
private:
    Slang::ComPtr<rhi::IDevice> m_device;

public:
    CachedDeviceWrapper() = default;

    CachedDeviceWrapper(Slang::ComPtr<rhi::IDevice> device)
        : m_device(device)
    {
    }

    ~CachedDeviceWrapper() {}

    // Move constructor
    CachedDeviceWrapper(CachedDeviceWrapper&& other) noexcept
        : m_device(std::move(other.m_device))
    {
    }

    // Move assignment
    CachedDeviceWrapper& operator=(CachedDeviceWrapper&& other) noexcept
    {
        if (this != &other)
        {
            m_device = std::move(other.m_device);
        }
        return *this;
    }

    // Delete copy constructor and assignment
    CachedDeviceWrapper(const CachedDeviceWrapper&) = delete;
    CachedDeviceWrapper& operator=(const CachedDeviceWrapper&) = delete;

    rhi::IDevice* get() const { return m_device.get(); }
    rhi::IDevice* operator->() const { return m_device.get(); }
    operator bool() const { return m_device != nullptr; }

    Slang::ComPtr<rhi::IDevice>& getComPtr() { return m_device; }
};
