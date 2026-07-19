#pragma once

template <typename T>
void SafeRelease(T*& resource) noexcept
{
	if (resource != nullptr)
	{
		resource->Release();
		resource = nullptr;
	}
}