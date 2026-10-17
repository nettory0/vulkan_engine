#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
namespace en_ecs
{
template<typename T>
class CComponentArray
{
public:
	void insertData(uint64_t entity, T&& component)
	{
		if(hasComponent(entity))
		{
			m_aComponent[m_aEntityToIndex[entity]] = std::forward<T>(component);
			return;
		}

		m_aEntityToIndex[entity] = size;
		m_aIndexToEntity[size] = entity;
		m_aComponent.emplace(size, std::forward<T>(component));
		size++;
	}

	void clear()
	{
		m_aComponent.clear();
		m_aIndexToEntity.clear();
		m_aEntityToIndex.clear();
		size = 0;
	}

	void removeData(uint64_t entity)
	{
		if(!hasComponent(entity))
			return;

		size_t index = m_aEntityToIndex[entity];
		size_t lastIndex = size - 1;

		if(index != lastIndex)
		{
			m_aComponent[index] = std::move(m_aComponent[lastIndex]);

			uint64_t lastEntity = m_aIndexToEntity[lastIndex];
			m_aEntityToIndex[lastEntity] = index;
			m_aIndexToEntity[index] = lastEntity;
		}

		m_aComponent.erase(lastIndex);
		m_aIndexToEntity.erase(lastIndex);
		m_aEntityToIndex.erase(entity);

		size--;
	}

	T& addData(uint64_t entity)
	{
		if(!hasComponent(entity))
		{
			m_aEntityToIndex[entity] = size;
			m_aIndexToEntity[size] = entity;
			size++;
		}

		return m_aComponent[m_aEntityToIndex[entity]];
	}

	T* getData(uint64_t entity)
	{
		if(!hasComponent(entity))
			return nullptr;
		return &m_aComponent.at(m_aEntityToIndex.at(entity));
	}

	const T* getData(uint64_t entity) const
	{
		if(!hasComponent(entity))
			return nullptr;
		return &m_aComponent.at(m_aEntityToIndex.at(entity));
	}

	bool hasComponent(uint64_t entity) const
	{
		return m_aEntityToIndex.find(entity) != m_aEntityToIndex.end();
	}
	const auto& getIndexToEntity() const
	{
		return m_aIndexToEntity;
	}
private:
	std::unordered_map<size_t, T> m_aComponent;
	std::unordered_map<size_t, uint64_t> m_aIndexToEntity;
	std::unordered_map<uint64_t, size_t> m_aEntityToIndex;
	size_t size = 0;
};

class CComponentManager
{

public:
	CComponentManager(const CComponentManager&) = delete;
	CComponentManager& operator=(const CComponentManager&) = delete;
	CComponentManager() = default;
	template<typename T>
	void addComponent(uint64_t entity, T&& component)
	{
		getArray<T>()->data.insertData(entity, std::forward<T>(component));
	}

	template<typename T>
	void removeAllComponents()
	{
		auto it = arrays.find(typeid(T).hash_code());
		if(it == arrays.end())
			return;

		auto* array = static_cast<Array<T>*>(it->second.get());
		array->data.clear();
	}

	template<typename T>
	const T* getComponentConst(uint64_t entity) const
	{
		const auto* array = getArray<T>();
		if(!array || !array->data.hasComponent(entity))
			return nullptr;

		return array->data.getData(entity); // const T&
	}

	template<typename T>
	T* getComponent(uint64_t entity)
	{
		auto* array = getArray<T>();
		if(!array || !array->data.hasComponent(entity))
			return nullptr;

		return array->data.getData(entity);
	}

	template<typename T>
	T& addComponent(uint64_t entity)
	{
		return getArray<T>()->data.addData(entity);
	}

	template<typename T>
	bool hasComponent(uint64_t entity) const
	{
		auto it = arrays.find(typeid(T).hash_code());
		if(it == arrays.end())
			return false;

		auto array = static_cast<Array<T>*>(it->second.get());
		return array->data.hasComponent(entity);
	}

	template<typename T>
	void removeComponent(uint64_t entity)
	{
		auto* array = getArray<T>();
		if(!array)
			return;

		array->data.removeData(entity);
	}

	template<typename... Components>
	auto getEntitiesWithData()
	{
		using ResultTuple = std::tuple<uint64_t, Components&...>;
		std::vector<ResultTuple> result;

		if constexpr(sizeof...(Components) == 0)
			return result;

		using First = std::tuple_element_t<0, std::tuple<Components...>>;
		auto* arrFirst = getArray<First>();
		if(!arrFirst)
			return result;

		for(const auto& [index, entity] : arrFirst->data.getIndexToEntity())
		{
			if((hasComponent<Components>(entity) && ...))
			{
				result.emplace_back(entity, addComponent<Components>(entity)...);
			}
		}

		return result;
	}

private:
	struct IArray
	{
		virtual ~IArray() = default;
	};

	template<typename T>
	struct Array: IArray
	{
		CComponentArray<T> data;
	};

	std::unordered_map<std::size_t, std::unique_ptr<IArray>> arrays;

	template<typename T>
	Array<T>* getArray()
	{
		static const std::size_t id = typeid(T).hash_code();

		if(!arrays.contains(id))
			arrays[id] = std::make_unique<Array<T>>();

		return static_cast<Array<T>*>(arrays[id].get());
	}
	template<typename T>
	const Array<T>* getArray() const
	{
		std::size_t id = typeid(T).hash_code();
		auto it = arrays.find(id);
		if(it == arrays.end())
			return nullptr;
		return static_cast<const Array<T>*>(it->second.get());
	}
};
}