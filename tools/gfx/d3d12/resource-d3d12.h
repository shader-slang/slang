// resource-d3d12.h
#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

#include <dxgi1_4.h>
#include <d3d12.h>

#include "slang-com-ptr.h"
#include "../d3d/d3d-util.h"

namespace gfx {

// Enables more conservative barriers - restoring the state of resources after they are used.
// Should not need to be enabled in normal builds, as the barriers should correctly sync resources
// If enabling fixes an issue it implies regular barriers are not correctly used.
#define SLANG_ENABLE_CONSERVATIVE_RESOURCE_BARRIERS 0

struct D3D12BarrierSubmitter
{
	enum { MAX_BARRIERS = 8 };

	/// Expand one space to hold a barrier
	SLANG_FORCE_INLINE D3D12_RESOURCE_BARRIER& expandOne() { return (m_numBarriers < MAX_BARRIERS) ? m_barriers[m_numBarriers++] : _expandOne(); }
	/// Flush barriers to command list
	SLANG_FORCE_INLINE void flush() { if (m_numBarriers > 0) _flush(); }

        /// Transition resource from prevState to nextState
    void transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES nextState);

	/// Ctor
	SLANG_FORCE_INLINE D3D12BarrierSubmitter(ID3D12GraphicsCommandList* commandList) : m_numBarriers(0), m_commandList(commandList) { }
	/// Dtor
	SLANG_FORCE_INLINE ~D3D12BarrierSubmitter() { flush(); }

protected:
	D3D12_RESOURCE_BARRIER& _expandOne();
	void _flush();

	ID3D12GraphicsCommandList* m_commandList;
	int m_numBarriers;
	D3D12_RESOURCE_BARRIER m_barriers[MAX_BARRIERS];
};

/*! \brief A class to simplify using Dx12 fences.

A fence is a mechanism to track GPU work. This is achieved by having a counter that the CPU holds
called the current value. Calling nextSignal will increase the CPU counter, and add a fence
with that value to the commandQueue. When the GPU has completed all the work before the fence it will
update the completed value. This is typically used when
the CPU needs to know the GPU has finished some piece of work has completed. To do this the CPU
can check the completed value, and when it is greater or equal to the value returned by nextSignal the
CPU will know that all the work prior to when the nextSignal was added to the queue will have completed.

NOTE! This cannot be used across threads, as for amongst other reasons SetEventOnCompletion
only works with a single value.

Signal on the CommandQueue updates the fence on the GPU side. Signal on the fence object changes
the value on the CPU side (not used here).

Useful article describing how Dx12 synchronization works:
https://msdn.microsoft.com/en-us/library/windows/desktop/dn899217%28v=vs.85%29.aspx
*/
class D3D12CounterFence
{
public:
		/// Must be called before used
	SlangResult init(ID3D12Device* device, uint64_t initialValue = 0);
		/// Increases the counter, signals the queue and waits for the signal to be hit
	void nextSignalAndWait(ID3D12CommandQueue* queue);
		/// Signals with next counter value. Returns the value the signal was called on
	uint64_t nextSignal(ID3D12CommandQueue* commandQueue);
		/// Get the current value
	SLANG_FORCE_INLINE uint64_t getCurrentValue() const { return m_currentValue; }
		/// Get the completed value
	SLANG_FORCE_INLINE uint64_t getCompletedValue() const { return m_fence->GetCompletedValue(); }

		/// Waits for the the specified value
	void waitUntilCompleted(uint64_t completedValue);

		/// Ctor
	D3D12CounterFence() :m_event(nullptr), m_currentValue(0) {}
		/// Dtor
	~D3D12CounterFence();

protected:
	HANDLE m_event;
	Slang::ComPtr<ID3D12Fence> m_fence;
	UINT64 m_currentValue;
};

/** The base class for resource types allows for tracking of state. It does not allow for setting of the resource though, such that
an interface can return a D3D12ResourceBase, and a client cant manipulate it's state, but it cannot replace/change the actual resource */
struct D3D12ResourceBase
{
		/// Add a transition if necessary to the list
    void transition(
                    D3D12_RESOURCE_STATES currentState,
                    D3D12_RESOURCE_STATES nextState,
                    D3D12BarrierSubmitter& submitter);
		/// Get the associated resource
	SLANG_FORCE_INLINE ID3D12Resource* getResource() const { return m_resource; }

		/// True if a resource is set
	SLANG_FORCE_INLINE bool isSet() const { return m_resource != nullptr; }

		/// Coercible into ID3D12Resource
	SLANG_FORCE_INLINE operator ID3D12Resource*() const { return m_resource; }

		/// Given the usage, flags, and format will return the most suitable format. Will return DXGI_UNKNOWN if combination is not possible
	static DXGI_FORMAT calcFormat(D3DUtil::UsageType usage, ID3D12Resource* resource);

		/// Ctor
	SLANG_FORCE_INLINE D3D12ResourceBase() :
		m_resource(nullptr)
	{}

protected:
	/// This is protected so as clients cannot slice the class, and so state tracking is lost
	~D3D12ResourceBase() {}

	ID3D12Resource* m_resource;                 ///< The resource (ref counted)
};

struct D3D12Resource : public D3D12ResourceBase
{

	/// Dtor
	~D3D12Resource()
	{
		if (m_resource)
		{
			m_resource->Release();
		}
	}

		/// Initialize as committed resource
	Slang::Result initCommitted(ID3D12Device* device, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE * clearValue);

		/// Set a resource.
	void setResource(ID3D12Resource* resource);
		/// Make the resource null
	void setResourceNull();
		/// Returns the attached resource (with any ref counts) and sets to nullptr on this.
	ID3D12Resource* detach();

		/// Swaps the resource contents with the contents of the smart pointer
	void swap(Slang::ComPtr<ID3D12Resource>& resourceInOut);

		/// Set the debug name on a resource
	static void setDebugName(ID3D12Resource* resource, const char* name);

		/// Set the the debug name on the resource
	void setDebugName(const wchar_t* name);
		/// Set the debug name
	void setDebugName(const char* name);
};

} // renderer_test
