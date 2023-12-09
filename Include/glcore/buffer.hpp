#ifndef  GLCORE_BUFFER_HPP
#define  GLCORE_BUFFER_HPP

#include <glcore/globj.hpp>
#include <vector>
#include <utilities/data/vecs.hpp>
#include <gsl/span>

namespace nitros::glcore
{
    class GLCORE_EXPORT Buffer : public GLobj
    {
        public:
        enum class buffer_type
        {
            ARRAY_BUFFER,
            ELEMENT_BUFFER
        };

        enum class value_type
        {
            FLOAT, UINT
        };

        explicit Buffer();
        Buffer(const Buffer&) = delete;
        Buffer(Buffer&& ) = default;
        ~Buffer();

        Buffer& operator=(const Buffer&) = delete;

        void bind(buffer_type type) const;

        template <class type, std::size_t N>
        void write_data(const std::vector<std::array<type, N>>  &data);

        template <class type, std::size_t N>
        void read_data(std::vector<std::array<type, N>>  &data) const;

        [[nodiscard]] const std::size_t get_elements_count() const noexcept;
        [[nodiscard]] const std::size_t vec_length() const noexcept;
        [[nodiscard]] const std::size_t row_stride() const noexcept;
        [[nodiscard]] const value_type  get_value_type() const noexcept;

        private:
        
        void write_data(const gsl::span<const std::uint8_t>  data, bool is_integral);
        void read_data(gsl::span<std::uint8_t>  vec, std::size_t stride, bool is_integral) const;

        std::size_t num_elements, vec_components, stride;
        value_type   _value_type;
    };
}

#include "buffer.inl"

#endif