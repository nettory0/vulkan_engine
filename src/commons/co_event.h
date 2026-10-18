#pragma once
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

using Fn = std::function<void(const void*)>;
using eventHandle = std::uint64_t;

struct SSlot
{
	eventHandle id;
	std::shared_ptr<Fn> fn;
};

struct SChannel
{
	std::vector<SSlot> slots;
	int  m_depth = 0;
	bool m_bDirty = false;
};

class CEventBus
{
public:
	static constexpr eventHandle kInvalid = 0;
	template <class E, class F>
	eventHandle subscribe(F&& fn)
	{
		const std::type_index type(typeid(E));
		SChannel& ch = m_channels[type];
		const eventHandle id = ++m_next;

		ch.slots.push_back(SSlot   {id,std::make_shared<Fn>([f = std::forward<F>(fn)](const void* p){f(*static_cast<const E*>(p));})   });
		m_owner.emplace(id, type);
		return id;
	}

	void unsubscribe(eventHandle id)
	{
		auto o = m_owner.find(id);
		if(o == m_owner.end()) return;

		SChannel& ch = m_channels.at(o->second);
		for(SSlot& s : ch.slots)
		{
			if(s.id == id)
			{
				s.fn.reset(); 
				break;
			}
		}
		ch.m_bDirty = true;
		if(ch.m_depth == 0)
			compact(ch);
		m_owner.erase(o);
	}

	template <class E>
	void emit(const E& e)
	{
		auto it = m_channels.find(std::type_index(typeid(E)));
		if(it == m_channels.end()) return;

		SChannel& ch = it->second;
		++ch.depth;
		for(std::size_t i = 0, n = ch.slots.size(); i < n; ++i)
		{
			if(auto fn = ch.slots[i].fn)
				(*fn)(&e);
			--ch.depth;
		}
		if(ch.depth == 0 && ch.dirty)
			compact(ch);
	}

	template <class E>
	void queue(E e)
	{
		m_deferred.push_back([this, ev = std::move(e)]() mutable { emit(ev); });
	}

	void dispatch()
	{
		std::vector<std::function<void()>> batch;
		batch.swap(m_deferred);
		for(auto& f : batch)
			f();
	}

	void clear()
	{
		m_channels.clear(); m_owner.clear(); m_deferred.clear();
	}

private:
	static void compact(SChannel& ch)
	{
		ch.slots.erase(std::remove_if(ch.slots.begin(), ch.slots.end(), [](const SSlot& s) { return !s.fn; }), ch.slots.end());
		ch.m_bDirty = false;
	}

	std::unordered_map<std::type_index, SChannel> m_channels;
	std::unordered_map<eventHandle, std::type_index>  m_owner;
	std::vector<std::function<void()>> m_deferred;
	eventHandle m_next = kInvalid;
};

class CScopedSub
{
public:
	CScopedSub() = default;
	CScopedSub(CEventBus& bus, eventHandle h): m_bus(&bus), m_h(h)
	{}
	CScopedSub(CScopedSub&& o) noexcept: m_bus(o.m_bus), m_h(o.m_h)
	{
		o.m_h = 0;
	}
	CScopedSub& operator=(CScopedSub&& o) noexcept
	{
		if(this != &o)
		{
			reset();
			m_bus = o.m_bus; 
			m_h = o.m_h; 
			o.m_h = 0;
		}
		return *this;
	}
	CScopedSub(const CScopedSub&) = delete;
	CScopedSub& operator=(const CScopedSub&) = delete;
	~CScopedSub()
	{
		reset();
	}

	void reset()
	{
		if(m_bus && m_h) 
			m_bus->unsubscribe(m_h);
		m_h = 0;
	}

private:
	CEventBus* m_bus = nullptr;
	eventHandle m_h = 0;
};