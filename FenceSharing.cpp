#include <iostream>
#include <Unknwn.h>
#include <winrt/base.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include <d3d11_4.h>

using namespace winrt;

constexpr UINT64 FenceTestValue = 42;

void Fence11To12Test(com_ptr<ID3D11Device5> d3d11Device, com_ptr<ID3D12Device> d3d12Device) {
	com_ptr<ID3D11Fence> fence11;
	check_hresult(d3d11Device->CreateFence(FenceTestValue, D3D11_FENCE_FLAG_SHARED, IID_PPV_ARGS(&fence11)));

	winrt::handle fence11Handle;
	check_hresult(fence11->CreateSharedHandle(nullptr, GENERIC_ALL, nullptr, fence11Handle.put()));

	com_ptr<ID3D12Fence1> fence12;
	check_hresult(d3d12Device->OpenSharedHandle(fence11Handle.get(), IID_PPV_ARGS(&fence12)));

	if (fence12->GetCompletedValue() != FenceTestValue) {
		printf("Fence11To12Test failed\n");
	}
}

void Fence12To11Test(com_ptr<ID3D11Device5> d3d11Device, com_ptr<ID3D12Device> d3d12Device) {
	com_ptr<ID3D12Fence1> fence12;
	check_hresult(d3d12Device->CreateFence(FenceTestValue, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&fence12)));

	winrt::handle fence12Handle;
	check_hresult(d3d12Device->CreateSharedHandle(fence12.get(), nullptr, GENERIC_ALL, nullptr, fence12Handle.put()));

	com_ptr<ID3D11Fence> fence11;
	check_hresult(d3d11Device->OpenSharedFence(fence12Handle.get(), IID_PPV_ARGS(&fence11)));

	if (fence11->GetCompletedValue() != FenceTestValue) {
		printf("Fence12To11Test failed\n");
	}
}

void Fence12To12Test(com_ptr<ID3D12Device> d3d12Device1, com_ptr<ID3D12Device> d3d12Device2) {
	com_ptr<ID3D12Fence1> fence12;
	check_hresult(d3d12Device1->CreateFence(FenceTestValue, D3D12_FENCE_FLAG_SHARED, IID_PPV_ARGS(&fence12)));

	winrt::handle fence12Handle;
	check_hresult(d3d12Device1->CreateSharedHandle(fence12.get(), nullptr, GENERIC_ALL, nullptr, fence12Handle.put()));

	com_ptr<ID3D12Fence> fence12b;
	check_hresult(d3d12Device2->OpenSharedHandle(fence12Handle.get(), IID_PPV_ARGS(&fence12b)));

	if (fence12b->GetCompletedValue() != FenceTestValue) {
		printf("Fence12To12Test failed\n");
	}
}