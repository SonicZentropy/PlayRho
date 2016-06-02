/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef B2_STACK_ALLOCATOR_H
#define B2_STACK_ALLOCATOR_H

#include <Box2D/Common/Settings.h>

namespace box2d {

/// Stack allocator.
/// @detail
/// This is a stack allocator used for fast per step allocations.
/// You must nest allocate/free pairs. The code will assert
/// if you try to interleave multiple allocate/free pairs.
class StackAllocator
{
public:
	using size_type = size_t;

	static constexpr auto StackSize = unsigned{100 * 1024};	///< Stack size (100k).
	static constexpr auto MaxStackEntries = unsigned{32};
	
	StackAllocator() noexcept;
	~StackAllocator() noexcept;

	void* Allocate(size_type size) noexcept;
	void Free(void* p) noexcept;

	template <typename T>
	T* Allocate(size_type size) noexcept
	{
		return static_cast<T*>(Allocate(size * sizeof(T)));
	}

	/// Functional operator for freeing memory allocated by this object.
	/// @detail This method frees memory (like called Free) and allows this object
	///   to be used as deleter to std::unique_ptr.
	void operator()(void *p) noexcept;

	size_type GetMaxAllocation() const noexcept
	{
		return m_maxAllocation;
	}

private:

	struct StackEntry
	{
		using size_type = size_t;
		
		void* data;
		size_type size;
		bool usedMalloc;
	};
	
	char m_data[StackSize];
	size_type m_index = 0;

	size_type m_allocation = 0;
	size_type m_maxAllocation = 0;

	StackEntry m_entries[MaxStackEntries];
	std::remove_cv<decltype(MaxStackEntries)>::type m_entryCount = 0;
};

} // namespace box2d

#endif
