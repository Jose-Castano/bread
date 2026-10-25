#pragma once
#include "Core/Header.hpp"


namespace Collections
{

template<typename T, typename E>
struct ResultStorage
{
	union
	{
		T u_value;
		E u_error;
	};

	ResultStorage(T&& v)
	: u_value(Core::Forward<T>(v))
	{}

	ResultStorage(const E& e)
	: u_error(e)
	{}
};

template<typename E>
struct ResultStorage<void, E>
{
	E u_error;

	ResultStorage() {}

	ResultStorage(const E& e)
	: u_error(e)
	{}
};

template<typename T, typename E>
struct [[nodiscard]] Result
{
	using Type = T;
	using Error = E;

	ResultStorage<Type, Error> storage;
	bool has_value;

	template<typename V>
	constexpr Result(V&& v)
	requires(!Core::IsSame<V, void> && Core::IsSame<V, Type>)
	: storage(Core::Forward<V>(v)), has_value(true)
	{}

	constexpr Result()
	requires(Core::IsSame<Type, void>)
	: has_value(true)
	{}

	constexpr Result(const Error& e)
	: storage(e), has_value(false)
	{}

	constexpr bool fail() const { return !has_value; }

	constexpr auto value() const;
	constexpr auto error() const;

	template<typename F>
	requires(Core::Returns<F, void>)
	constexpr auto or_else(F&& f) const
	{
		if constexpr(!Core::IsSame<T, void>)
		{
			if (has_value)
			{
				return storage.u_value;
			}
		}
		
		return f(storage.u_error);
	}
	
};

template<Core::Enum E>
using ResultOnly = Result<void, E>;

}

#include "Debug/Fail.hpp"

namespace Collections
{

template<typename T, typename E>
constexpr auto Result<T, E>::value() const
{
	if constexpr (!Core::IsSame<Type, void>)
	{
		if (has_value)
		{
			return storage.u_value;
		}
	}

	FailOn(true, "the result hasn't a value");
	return T();
}

template<typename T, typename E>
constexpr auto Result<T, E>::error() const
{
	FailOn(has_value, "the result has a value");
	return storage.u_error;
}

}
