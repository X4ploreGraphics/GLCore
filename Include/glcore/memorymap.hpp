

#ifndef GLCORE_MEMORY_MAP_HPP
#define GLCORE_MEMORY_MAP_HPP

#include "glcore/glcore_export.h"
#include "glcore/buffer.hpp"
#include <gsl/gsl>

namespace nitros::glcore
{
    namespace _memorymap {
        enum class type {
            READ, WRITE, READ_WRITE
        };
    }
    
    template <_memorymap::type  type_ = _memorymap::type::READ_WRITE>
    class MemoryMap
    {
        public:
        explicit MemoryMap(const Buffer  &buffer);
        MemoryMap(const MemoryMap &) = delete;
        MemoryMap(MemoryMap &&) = delete;
        ~MemoryMap();

        MemoryMap& operator=(const MemoryMap &) = delete;
        MemoryMap& operator=(MemoryMap &&) = delete;

        gsl::span<std::uint8_t>   get_data() const;

        template <class type, std::size_t N>
        void get_data(gsl::span<std::array<type, N>>  &data) const;
        
        private:
        const Buffer&  _buffer;
        void *_buffer_ptr;
    };

    template <_memorymap::type type_>
    template < class type, std::size_t N>
    void MemoryMap<type_>::get_data(gsl::span<std::array<type, N>>  &data) const
    {
        gsl::span<std::uint8_t> ptr = get_data();
        const auto count =  gsl::narrow_cast<std::ptrdiff_t>( ptr.size_bytes() / sizeof(std::array<type, N>) );
        data = gsl::span<std::array<type,N>>{ reinterpret_cast<std::array<type, N>*>(ptr.data()), count } ;
    }

    extern template class GLCORE_EXPORT MemoryMap<_memorymap::type::READ_WRITE>;
    extern template class GLCORE_EXPORT MemoryMap<_memorymap::type::READ>;
    extern template class GLCORE_EXPORT MemoryMap<_memorymap::type::WRITE>;
} // nitros::glcore


#endif