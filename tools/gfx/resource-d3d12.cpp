// resource-d3d12.cpp
#include "resource-d3d12.h"

namespace gfx {
using namespace Slang;

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! D3D12BarrierSubmitter !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

void D3D12BarrierSubmitter::_flush()
{
	assert(m_numBarriers > 0);

	if (m_commandList)
	{
		m_commandList->ResourceBarrier(UINT(m_numBarriers), m_barriers);
	}
	m_numBarriers = 0;
}

D3D12_RESOURCE_BARRIER& D3D12BarrierSubmitter::_expandOne()
{
	_flush();
	return m_barriers[m_numBarriers++];
}

void D3D12BarrierSubmitter::transition(ID3D12Resource* resource, D3D12_RESOURCE_STATES prevState, D3D12_RESOURCE_STATES nextState)
{
    if (nextState != prevState)
    {
        D3D12_RESOURCE_BARRIER& barrier = expandOne();

        const UINT subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        const D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

        ::memset(&barrier, 0, sizeof(barrier));
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = flags;
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = prevState;
        barrier.Transition.StateAfter = nextState;
        barrier.Transition.Subresource = subresource;
    }
    else
    {
        if (nextState == D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        {
            D3D12_RESOURCE_BARRIER& barrier = expandOne();

            ::memset(&barrier, 0, sizeof(barrier));
            barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
            barrier.UAV.pResource = resource;
        }
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! D3D12ResourceBase !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */DXGI_FORMAT D3D12ResourceBase::calcFormat(D3DUtil::UsageType usage, ID3D12Resource* resource)
{
	return resource ? D3DUtil::calcFormat(usage, resource->GetDesc().Format) : DXGI_FORMAT_UNKNOWN;
}

void D3D12ResourceBase::transition(D3D12_RESOURCE_STATES nextState, D3D12BarrierSubmitter& submitter)
{
	// Transition only if there is a resource
	if (m_resource)
	{
        submitter.transition(m_resource, m_state, nextState);
        m_state = nextState;
    }
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! D3D12CounterFence !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

D3D12CounterFence::~D3D12CounterFence()
{
	if (m_event)
	{
		CloseHandle(m_event);
	}
}

Result D3D12CounterFence::init(ID3D12Device* device, uint64_t initialValue)
{
	m_currentValue = initialValue;

	SLANG_RETURN_ON_FAIL(device->CreateFence(m_currentValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.writeRef())));
	// Create an event handle to use for frame synchronization.
	m_event = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (m_event == nullptr)
	{
		Result res = HRESULT_FROM_WIN32(GetLastError());
		return SLANG_FAILED(res) ? res : SLANG_FAIL;
	}
	return SLANG_OK;
}

UInt64 D3D12CounterFence::nextSignal(ID3D12CommandQueue* commandQueue)
{
	// Increment the fence value. Save on the frame - we'll know that frame is done when the fence value >=
	m_currentValue++;
	// Schedule a Signal command in the queue.
	Result res = commandQueue->Signal(m_fence, m_currentValue);
	if (SLANG_FAILED(res))
	{
		assert(!"Signal failed");
	}
	return m_currentValue;
}

void D3D12CounterFence::waitUntilCompleted(uint64_t completedValue)
{
	// You can only wait for a value that is less than or equal to the current value
	assert(completedValue <= m_currentValue);

	// Wait until the previous frame is finished.
	while (m_fence->GetCompletedValue() < completedValue)
	{
		// Make it signal with the current value
		SLANG_ASSERT_VOID_ON_FAIL(m_fence->SetEventOnCompletion(completedValue, m_event));
		WaitForSingleObject(m_event, INFINITE);
	}
}

void D3D12CounterFence::nextSignalAndWait(ID3D12CommandQueue* commandQueue)
{
	waitUntilCompleted(nextSignal(commandQueue));
}

/* !!!!!!!!!!!!!!!!!!!!!!!!! D3D12Resource !!!!!!!!!!!!!!!!!!!!!!!! */

/* static */void D3D12Resource::setDebugName(ID3D12Resource* resource, const char* name)
{
	if (resource)
	{
		size_t len = ::strlen(name);
		List<wchar_t> buf;
		buf.setCount(len + 1);

		D3DUtil::appendWideChars(name, buf);
		resource->SetName(buf.begin());
	}
}

void D3D12Resource::setDebugName(const char* name)
{
	setDebugName(m_resource, name);
}

void D3D12Resource::setDebugName(const wchar_t* name)
{
	if (m_resource)
	{
		m_resource->SetName(name);
	}
}

void D3D12Resource::setResource(ID3D12Resource* resource, D3D12_RESOURCE_STATES initialState)
{
	if (resource != m_resource)
	{
		if (resource)
		{
			resource->AddRef();
		}
		if (m_resource)
		{
			m_resource->Release();
		}
		m_resource = resource;
	}
	m_prevState = initialState;
	m_state = initialState;
}

void D3D12Resource::setResourceNull()
{
	if (m_resource)
	{
		m_resource->Release();
		m_resource = nullptr;
	}
}

Result D3D12Resource::initCommitted(ID3D12Device* device, const D3D12_HEAP_PROPERTIES& heapProps, D3D12_HEAP_FLAGS heapFlags, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_RESOURCE_STATES initState, const D3D12_CLEAR_VALUE * clearValue)
{
	setResourceNull();
	ComPtr<ID3D12Resource> resource;
	SLANG_RETURN_ON_FAIL(device->CreateCommittedResource(&heapProps, heapFlags, &resourceDesc, initState, clearValue, IID_PPV_ARGS(resource.writeRef())));
	setResource(resource, initState);
	return SLANG_OK;
}

ID3D12Resource* D3D12Resource::detach()
{
	ID3D12Resource* resource = m_resource;
	m_resource = nullptr;
	return resource;
}

void D3D12Resource::swap(ComPtr<ID3D12Resource>& resourceInOut)
{
	ID3D12Resource* tmp = m_resource;
	m_resource = resourceInOut.detach();
	resourceInOut.attach(tmp);
}

void D3D12Resource::setState(D3D12_RESOURCE_STATES state)
{
	m_prevState = state;
	m_state = state;
}

} // renderer_test
