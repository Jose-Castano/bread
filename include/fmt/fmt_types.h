#pragma once
#include "Core/Header.hpp"
#include "Collections/StringView.hpp"




namespace Format
{

enum class FormatType
{
	Unknown = 0,
	Bool,
	Signed,
	Unsigned,
	Float32,
	Float64,
	Pointer,
	StringView,
	CString,
	Slice
};

template<typename T>
struct FormatArgument
{
	FormatType type;
};

template<typename T>
constexpr FormatType __GetFormatType()
{
	if constexpr(Core::IsSame<T, bool>)
	{
		return FormatType::Bool;
	}
	else if constexpr(Core::IsSigned<T>)
	{
		return FormatType::Signed;
	}
	else if constexpr(Core::IsUnsigned<T>)
	{
		return FormatType::Unsigned;
	}
	else if constexpr(Core::IsSame<T, f32>)
	{
		return FormatType::Float32;
	}
	else if constexpr(Core::IsSame<T, f64>)
	{
		return FormatType::Float64;
	}
	else if constexpr(Core::IsPointer<T>)
	{
		return FormatType::Pointer;
	}
	else if constexpr(Core::IsSame<T, Collections::StringView>)
	{
		return FormatType::StringView;
	}
	else if constexpr(Core::IsArrayOf<T, char>)
	{
		return FormatType::CString;
	}
	else if constexpr(Core::IsSlice<T>)
	{
		return FormatType::Slice;
	}

	return FormatType::Unknown;
}

}
