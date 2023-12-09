

#include "buffer.hpp"

namespace nitros::glcore
{
    namespace internal
    {
        template <class T>
        struct always_false : std::false_type {};
    }
    
    template <class type, std::size_t N>
    void Buffer::write_data(const std::vector<std::array<type, N>>  &data)
    {
        static_assert(N > 0 && N <= 4);
        vec_components = N;
        num_elements = data.size() * N;
        stride = sizeof(std::array<type, N>);
        bool is_integral = false;
        
        if constexpr( std::is_floating_point_v<type> && std::is_same_v<type, float> ) {
            is_integral = false;
            _value_type = value_type::FLOAT;
        }
        else if constexpr( std::is_integral_v<type> && std::is_unsigned_v<type> && std::is_same_v<type, std::uint32_t> ) {
            is_integral = true;
            _value_type = value_type::UINT;
        }
        else {
            static_assert( internal::always_false<type>{}, "Array Type not supported" );
        }

        write_data( gsl::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(data.data()), gsl::narrow_cast<std::ptrdiff_t>(data.size() * stride) } , is_integral );    
    }

    template <class type, std::size_t N>
    void Buffer::read_data(std::vector<std::array<type, N>>  &data) const
    {
        static_assert(N > 0 && N <= 4);
        constexpr auto stride = sizeof(std::array<type, N>);
        data.resize(num_elements/vec_components);

        bool is_integral = false;
        if constexpr( std::is_floating_point_v<type> ) {
            is_integral = false;
        }
        else if constexpr( std::is_integral_v<type> && std::is_unsigned_v<type>) {
            is_integral = true;
        }
        else {
            static_assert( internal::always_false<type>{}, "Array Type not supported" );
        }

        auto data_span = gsl::span<std::uint8_t>{reinterpret_cast<std::uint8_t*>(data.data()), gsl::narrow_cast<std::ptrdiff_t>( num_elements * sizeof(type) ) };
        read_data( data_span, stride , is_integral );
    }
} // namespace nitros::glcore
