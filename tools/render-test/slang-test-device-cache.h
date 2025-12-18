#pragma once

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
        std::vector<std::string> requiredFeatures;

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

    static void evictOldestDeviceIfNeeded();

public:
    static SlangResult acquireDevice(const rhi::DeviceDesc& desc, rhi::IDevice** outDevice);
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
