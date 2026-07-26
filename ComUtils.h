#pragma once

#include <d3d9.h>
#include <cstring>

template <typename T>
void SafeRelease(T*& resource) noexcept
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}

template <typename TBuffer>
HRESULT UploadBufferData(TBuffer* buffer, const void* sourceData, UINT dataSize)
{
	if (buffer == nullptr ||
		sourceData == nullptr ||
		dataSize == 0)
	{
		return E_INVALIDARG;
	}

	void* destination = nullptr;

	const HRESULT lockResult = buffer->Lock(0, dataSize, &destination, 0);

	if (FAILED(lockResult))
		return lockResult;

	std::memcpy(destination, sourceData, dataSize);

	return buffer->Unlock();
}

inline HRESULT CreateManagedVertexBuffer(
	LPDIRECT3DDEVICE9 device,
	const void* sourceData,
	UINT dataSize,
	DWORD fvf,
	LPDIRECT3DVERTEXBUFFER9* vertexBuffer)
{
	if (device == nullptr || vertexBuffer == nullptr)
		return E_INVALIDARG;

	SafeRelease(*vertexBuffer);

	const HRESULT createResult = device->CreateVertexBuffer(
		dataSize,
		0,
		fvf,
		D3DPOOL_MANAGED,
		vertexBuffer,
		nullptr);

	if (FAILED(createResult))
		return createResult;

	const HRESULT uploadResult = UploadBufferData(*vertexBuffer, sourceData, dataSize);

	if (FAILED(uploadResult))
		SafeRelease(*vertexBuffer);

	return uploadResult;
}

inline HRESULT CreateManagedIndexBuffer(
	LPDIRECT3DDEVICE9 device,
	const void* sourceData,
	UINT dataSize,
	D3DFORMAT format,
	LPDIRECT3DINDEXBUFFER9* indexBuffer)
{
	if (device == nullptr || indexBuffer == nullptr)
		return E_INVALIDARG;

	SafeRelease(*indexBuffer);

	const HRESULT createResult = device->CreateIndexBuffer(
		dataSize,
		0,
		format,
		D3DPOOL_MANAGED,
		indexBuffer,
		nullptr);

	if (FAILED(createResult))
		return createResult;

	const HRESULT uploadResult = UploadBufferData(*indexBuffer, sourceData, dataSize);

	if (FAILED(uploadResult))
		SafeRelease(*indexBuffer);

	return uploadResult;
}