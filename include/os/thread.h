#pragma once
#include "Core/Header.hpp"
#include "Mem/Allocator.hpp"



struct [[nodiscard]] Thread
{
    using ThreadFn = void(*)(Core::Opaque*);
	
	Mem::Allocator& allocator;
	Core::Opaque* impl;

	Thread(Mem::Allocator& allocator, ThreadFn fn, Core::Opaque* arg);
	~Thread();

	[[nodiscard]] bool join() const;
};
