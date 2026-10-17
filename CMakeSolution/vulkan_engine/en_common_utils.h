#pragma once
#include <unordered_map>
#include <format>
#include "co_logging.h"

namespace en_util
{

template<typename T>
class CResourceKeeper
{
public:
	uint32_t add(T&& resource)
	{
		auto freeId = m_resourceCounter;
		m_resourceCounter++;
		m_resource[freeId] = std::make_unique <T>(std::move(resource));
		return freeId;
	}
	uint32_t add(std::unique_ptr<T> pResource)
	{
		auto freeId = m_resourceCounter++;
		m_resource[freeId] = std::move(pResource);
		return freeId;
	}

	void remove(uint32_t id)
	{
		if(auto it = m_resource.find(id); it != m_resource.end())
			m_resource.erase(it);
	}
	T* get(uint32_t id) const
	{
		if(auto it = m_resource.find(id); it != m_resource.end())
			return it->second.get();
		else
			common::logError(std::format("get Can not find id {}\n", id));
		return nullptr;
	}
	const std::unordered_map<uint32_t, std::unique_ptr<T>>& resource() const
	{
		return m_resource;
	}
	void clear()
	{
		m_resource.clear();
	}
private:
	std::unordered_map<uint32_t, std::unique_ptr<T>> m_resource;
	uint32_t m_resourceCounter{0};
};
}