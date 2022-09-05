#ifndef UTILS_SINGLETON_HPP
#define UTILS_SINGLETON_HPP

#include <cassert>

namespace utils
{
    template<typename T>
    class Singleton
    {
    public:
        static T& getSingleton()
        {
            assert( m_pSingletonType );
            return *m_pSingletonType;
        }

        static T* getSingletonPtr()
        {
            return m_pSingletonType;
        }

    protected:
        Singleton()
        {
            assert( !m_pSingletonType );
            m_pSingletonType = static_cast< T* >( this );
        }
        Singleton( const Singleton<T>& ) = delete;
        Singleton<T>& operator=(const Singleton<T>& ) = delete;
        ~Singleton()
        {
            assert(m_pSingletonType);
            m_pSingletonType = nullptr;
        }

        static T* m_pSingletonType;
    };
} // namespace utils


#endif