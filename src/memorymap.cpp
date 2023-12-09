

#include "glcore/memorymap.hpp"
#include "glcore/commands.hpp"
#include "platform/gl.hpp"
#include <gsl/gsl>

namespace nitros::glcore
{
#if defined(OPENGL_CORE)
    constexpr auto return_type(const _memorymap::type type_){
        if ( type_ == _memorymap::type::READ_WRITE )
            return GL_READ_WRITE;
        else if ( type_ == _memorymap::type::READ )
            return GL_READ_ONLY;
        else
            return GL_WRITE_ONLY;
    }
#else
    constexpr auto return_type(const _memorymap::type type_){
        if ( type_ == _memorymap::type::READ_WRITE )
            return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
        else if ( type_ == _memorymap::type::READ )
            return GL_MAP_READ_BIT;
        else
            return GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT;
    }
#endif

    template <_memorymap::type type_>
    MemoryMap<type_>::MemoryMap(const Buffer  &buffer)
        :_buffer{buffer}
        ,_buffer_ptr{nullptr}
    {
#if OPENGL_CORE >= 40500
        _buffer_ptr = glMapNamedBuffer(_buffer.get_id(), return_type(type_) );
        //buffer.bind(Buffer::buffer_type::ARRAY_BUFFER);
        //_buffer_ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, _buffer.get_elements_count() * _buffer.row_stride() / _buffer.vec_length(), return_type(type_));
        //glGetNamedBufferPointerv(_buffer.get_id(), GL_BUFFER_MAP_POINTER, &_buffer_ptr);
#else
    if(buffer.get_value_type() == Buffer::value_type::UINT){
        buffer.bind(Buffer::buffer_type::ELEMENT_BUFFER);
        _buffer_ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, _buffer.get_elements_count() * _buffer.row_stride() / _buffer.vec_length(), return_type(type_));
    }else{
        buffer.bind(Buffer::buffer_type::ARRAY_BUFFER);
        _buffer_ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, _buffer.get_elements_count() * _buffer.row_stride() / _buffer.vec_length(), return_type(type_));
    }
#endif
    if(_buffer_ptr == NULL){
        auto errors = command::error();
        throw std::runtime_error("Memory Map error");
    }
    }

    template <_memorymap::type type_>
    MemoryMap<type_>::~MemoryMap() {
#if OPENGL_CORE >= 40500
        glUnmapNamedBuffer(_buffer.get_id());
#else
        auto gl_array_type = [buffer_type = _buffer.get_value_type()](){
            if(buffer_type == glcore::Buffer::value_type::UINT)
                return GL_ELEMENT_ARRAY_BUFFER;
            else
                return GL_ARRAY_BUFFER;
        }();
        if constexpr(type_ == _memorymap::type::WRITE){
            glFlushMappedBufferRange(gl_array_type, 0, _buffer.get_elements_count() * _buffer.row_stride() / _buffer.vec_length());
        }
        auto errors = command::error();
        glUnmapBuffer(gl_array_type);
#endif
    }

    template <_memorymap::type type_>
    gsl::span<std::uint8_t>  MemoryMap<type_>::get_data() const {
        return { static_cast<std::uint8_t*>(_buffer_ptr), gsl::narrow_cast<std::ptrdiff_t>( _buffer.get_elements_count() * _buffer.row_stride() / _buffer.vec_length()) };
    }

    template class MemoryMap<_memorymap::type::READ_WRITE>;
    template class MemoryMap<_memorymap::type::READ>;
    template class MemoryMap<_memorymap::type::WRITE>;
    
} // nitros::glcore
