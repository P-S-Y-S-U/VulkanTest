#ifndef APP_UTILS
#define APP_UTILS

#include <memory>

namespace app::utils
{
	template<class _Type>
	using Sptr = std::shared_ptr<_Type>;
	
	template<class _Type>
	using Uptr = std::unique_ptr< _Type, std::default_delete<_Type> >;
} // namespace app::utils

#endif