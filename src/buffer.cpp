

#include <glcore/buffer.hpp>
#include "platform/gl.hpp"
#include <type_traits>
#include <cstring>
#include <stdexcept>

namespace nitros::glcore
{
    constexpr  auto get_GLType(Buffer::buffer_type  type)    {
        switch(type)
        {
            case Buffer::buffer_type::ARRAY_BUFFER : return GL_ARRAY_BUFFER;
            case Buffer::buffer_type::ELEMENT_BUFFER : return GL_ELEMENT_ARRAY_BUFFER;

            default:
                throw std::invalid_argument("Buffer Type not Handled");
        }
    }

    template<class type_>
    auto get_valueType();

    template<>
    auto get_valueType<float>(){
        return Buffer::value_type::FLOAT;
    }

    template<>
    auto get_valueType<std::uint32_t>(){
        return Buffer::value_type::UINT;
    }

    Buffer::Buffer()
        :_value_type{value_type::FLOAT}
    {
    #if OPENGL_CORE >= 40500
        glCreateBuffers(1, &_id);
    #else
        glGenBuffers(1, &_id);
    #endif
    }

    Buffer::~Buffer()
    {
        glDeleteBuffers(1, &_id);
    }

    void Buffer::bind(buffer_type type) const
    {
        glBindBuffer(get_GLType(type), _id);
    }
    
    void Buffer::write_data(const gsl::span<const std::uint8_t>  data, bool is_integral)
    {
    #if OPENGL_CORE >= 40500
        glNamedBufferData(_id, data.size(), data.data(), GL_STATIC_DRAW);
    #else
        auto array_type = is_integral ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        glBindBuffer(array_type, _id);
        glBufferData(array_type, data.size(), data.data(), GL_STATIC_DRAW);
    #endif
    }

    void Buffer::read_data(gsl::span<std::uint8_t>  vec, std::size_t  stride_, bool is_integral) const
    {
        if(stride_ != stride){
            throw std::runtime_error("Write Stride and Read Stride doesn't match");
        }
        const auto size = vec.size_bytes();

    #if OPENGL_CORE >= 40500
        glGetNamedBufferSubData(_id, 0, size, vec.data() );
    #elif OPENGL_CORE >= 40300
        auto array_type = is_integral ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        glBindBuffer(array_type, _id);
        glGetBufferSubData(array_type, 0, size, vec.data());
    #else
        auto array_type = is_integral ? GL_ELEMENT_ARRAY_BUFFER : GL_ARRAY_BUFFER;
        glBindBuffer(array_type, _id);
        void*  _map_data = glMapBufferRange(array_type, 0, size, GL_MAP_READ_BIT);
        if(_map_data != nullptr){
            std::memcpy(vec.data(), _map_data, size);
        }
        glUnmapBuffer(array_type);
    #endif
    }

    const std::size_t Buffer::get_elements_count() const noexcept
    {
        return num_elements;
    }

    const std::size_t Buffer::vec_length() const noexcept
    {
        return vec_components;
    }

    const std::size_t Buffer::row_stride() const noexcept
    {
        return stride;
    }

    const Buffer::value_type  Buffer::get_value_type() const noexcept
    {
        return _value_type;
    }
}