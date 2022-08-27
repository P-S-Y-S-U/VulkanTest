#ifndef UTILS_MEMORY_HPP
#define UTILS_MEMORY_HPP

#include <memory>

namespace utils
{
	template<class _Type>
	using Sptr = std::shared_ptr<_Type>;
	
	template<class _Type>
	using Uptr = std::unique_ptr< _Type, std::default_delete<_Type> >;
} // namespace utils

#endif