#pragma once
#include <type_traits>
#include <variant>
#include <typeinfo>
#include <utility>
#include "vk_struct_builders.h"

namespace vk_base
{
template <typename VkDeviceType>
class CVkDeviceBase
{
public:
	CVkDeviceBase() = default;
	CVkDeviceBase(const CVkDeviceBase&) = delete;
	CVkDeviceBase& operator=(const CVkDeviceBase&) = delete;
	// Moved-from object must not keep the handle, otherwise both destructors destroy it.
	// Move assignment swaps, so the previously owned handle is released by `other`.
	CVkDeviceBase(CVkDeviceBase&& other) noexcept
		: m_pDevice(std::exchange(other.m_pDevice, VkDeviceType{}))
	{
	}
	CVkDeviceBase& operator=(CVkDeviceBase&& other) noexcept
	{
		std::swap(m_pDevice, other.m_pDevice);
		return *this;
	}
	virtual ~CVkDeviceBase() = default;
	void setDevice(const VkDeviceType& device)
	{
		m_pDevice = device;
	}
	void releaseDevice()
	{
		m_pDevice = VkDeviceType{};
	}

	const VkDeviceType& device() const
	{
		return m_pDevice;
	}

private:
	VkDeviceType m_pDevice{VkDeviceType{}};
};

template <typename VkObjectType, typename VkDeviceType, typename VkCreateInfoType>
class CVulkanObject: public CVkDeviceBase<VkDeviceType>
{
public:

	CVulkanObject() = default;
	virtual ~CVulkanObject() = default;

	CVulkanObject(const CVulkanObject&) = delete;
	CVulkanObject& operator=(const CVulkanObject&) = delete;
	CVulkanObject(CVulkanObject&& other) noexcept
		: CVkDeviceBase<VkDeviceType>(std::move(other))
		, m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
	{
	}
	CVulkanObject& operator=(CVulkanObject&& other) noexcept
	{
		CVkDeviceBase<VkDeviceType>::operator=(std::move(other));
		std::swap(m_handle, other.m_handle);
		return *this;
	}

	[[nodiscard]] VkObjectType& handle()
	{
		return m_handle;
	}
	[[nodiscard]] const VkObjectType& handle() const
	{
		return m_handle;
	}
protected:
	template <typename CreateFunction>
	void createVulkanObject(CreateFunction createFunc, const VkCreateInfoType* pCreateInfo, VkDeviceType device)
	{
		if(!pCreateInfo)
			vk_init::checkResult(VK_NOT_READY, {typeid(VkObjectType).name()}, {typeid(CreateFunction).name()});
		if constexpr(!std::is_same_v<VkDeviceType, std::monostate>)
		{
			this->setDevice(device);
			vk_init::checkResult(createFunc(device, const_cast<VkCreateInfoType*>(pCreateInfo), nullptr, &m_handle), {typeid(VkObjectType).name()}, {typeid(CreateFunction).name()});
		}
		else
		{
			vk_init::checkResult(createFunc(pCreateInfo, nullptr, &m_handle), {typeid(VkObjectType).name()}, {typeid(CreateFunction).name()});
		}
	}
	template <typename DestroyFunction>
	void destroyVulkanObject(DestroyFunction destroyFunc)
	{
		if(handle())
		{
			if constexpr(!std::is_same_v<VkDeviceType, std::monostate>)
				destroyFunc(this->device(), m_handle, nullptr);
			else
				destroyFunc(m_handle, nullptr);

			m_handle = VK_NULL_HANDLE;
			this->releaseDevice();
		}
	}

protected:
	VkObjectType m_handle{VK_NULL_HANDLE};
};

}