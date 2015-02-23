#ifndef OGL_H
#define OGL_H

#include <exception>

#include <GL/glew.h>

namespace OGL {
	
	enum : GLenum {
		BUFFER_TYPE_UNUSED,
		BUFFER_TYPE_ARRAY =               GL_ARRAY_BUFFER,
		BUFFER_TYPE_ATOMIC_COUNTER =      GL_ATOMIC_COUNTER_BUFFER,
		BUFFER_TYPE_COPY_READ =           GL_COPY_READ_BUFFER,
		BUFFER_TYPE_COPY_WRITE =          GL_COPY_WRITE_BUFFER,
		BUFFER_TYPE_DISPATCH_INDIRECT =   GL_DISPATCH_INDIRECT_BUFFER,
		BUFFER_TYPE_DRAW_INDIRECT =       GL_DRAW_INDIRECT_BUFFER,
		BUFFER_TYPE_ELEMENT_ARRAY =       GL_ELEMENT_ARRAY_BUFFER,
		BUFFER_TYPE_PIXEL_PACK =          GL_PIXEL_PACK_BUFFER,
		BUFFER_TYPE_PIXEL_UNPACK =        GL_PIXEL_UNPACK_BUFFER,
		BUFFER_TYPE_QUERY =               GL_QUERY_BUFFER,
		BUFFER_TYPE_SHADER_STORAGE =      GL_SHADER_STORAGE_BUFFER,
		BUFFER_TYPE_TEXTURE =             GL_TEXTURE_BUFFER,
		BUFFER_TYPE_TRANSFORM_FEEDBACK =  GL_TRANSFORM_FEEDBACK_BUFFER,
		BUFFER_TYPE_UNIFORM =             GL_UNIFORM_BUFFER
	};
	
}

namespace ogl {
	
	using namespace OGL;
	
	using buffer_type = decltype(OGL::BUFFER_TYPE_UNUSED);
	
	/** Use this instead of glewInit() */
	inline GLenum init();
	
	inline void* get_current_context();
	inline bool is_current_context(void*);
	
	template<typename T, T defaultValue = static_cast<T>(*(T*)0)>
	class context_local {
	private:
		static std::map<void*, T> _perContextMap;
	public:
		
		T get() const;
		void set(const T& value);
	};
	
	template<typename T, T defaultValue>
	T context_local<T, defaultValue>::get() const {
		auto it = _perContextMap.find(get_current_context());
		if(it == _perContextMap.end()) {
			return defaultValue;
		}
		
		return *it;
	}
	
	template<typename T, T defaultValue>
	void context_local<T, defaultValue>::set(const T& value) const {
		void* currentContext = get_current_context();
		auto it = _perContextMap.find(currentContext);
		if(it == _perContextMap.end()) {
			_perContextMap.insert({currentContext, value});
			return;
		}
		
		*it = value;
	}
	
	class _object {
	protected:
		GLuint _obj = 0;
	};
	
	template<buffer_type bufferType>
	class buffer;
	
	template<typename T>
	class mapped_buffer_data {
	private:
		void* _rawData = nullptr;
		GLsizei _length = 0;
		buffer_type _bufferType;
	public:
		
		mapped_buffer_data(buffer_type, void* data, GLsizei length);
		~mapped_buffer_data();
		
		T& operator[](int32_t n);
		const T& operator[](int32_t n) const;
	};
	
	template<typename T>
	mapped_buffer_data::mapped_buffer_data(buffer_type bufferType, void* rawData, GLsizei length)
		: _bufferType(bufferType)
		, _rawData(rawData)
		, _length(length) {
		
	}
	
	template<typename T>
	mapped_buffer_data<T>::~mapped_buffer_data() {
		glUnmapBuffer(_bufferType);
	}
	
	template<typename T>
	T& mapped_buffer_data::operator&[](int32_t n) {
		if(n > _length) {
			
		}
		
		return static_cast<T&>(*(reinterpret_cast<T*>(_rawData)+n));
	}
	
	template<typename T>
	const T& mapped_buffer_data::operator&[](int32_t n) const {
		if(n > _length) {
			
		}
		
		return static_cast<const T&>(*(reinterpret_cast<T*>(_rawData)+n));
	}
	
	class _buffer
		: public _object {
	protected:
		friend void ogl::init();
		
		_buffer() = default;
		_buffer(const _buffer&) = default;
		_buffer(_buffer&&) = default;
		
		_buffer& operator=(const _buffer&) = default;
		_buffer& operator=(_buffer&&) = default;
		
		static void* (*_map_func_ptr)(ogl::buffer_type bufferType, GLuint bufObj, GLenum access);
		static void* _map_func_4_5(ogl::buffer_type bufferType, GLuint bufObj, GLenum access);
		static void* _map_func_ext(ogl::buffer_type bufferType, GLuint bufObj, GLenum access);
		static void* _map_func_fallback(ogl::buffer_type bufferType, GLuint bufObj, GLenum access);
	};
	
	template<buffer_type bufferType>
	class buffer
		: public _buffer {
	private:
		
		GLuint _lastBound = 0;
		void _bind();
		void _unbind();
	public:
		
		buffer() {
			glGenBuffers(1, &_obj);
			glBindBuffer(bufferType, _obj);
		}
		
		bool is_mapped() const;
		
		const void* raw_map_read();
		void* raw_map_write();
		void* raw_map();
		
		void unmap();
		
		template<typename T>
		const mapped_buffer_data<T> map_read();
		
		template<typename T>
		mapped_buffer_data<T> map_write();
		
		template<typename T>
		mapped_buffer_data<T> map();
		
		template<typename T>
		mapped_buffer_data<T> map_range(GLintptr offset, GLsizei length);
	};
	
	template<buffer_type bufferType>
	void buffer<bufferType>::_bind() {
		switch(_bufferType) {
		#define case_setLastBoundByType(type) case BUFFER_TYPE_##type##: glGetIntegerv(GL_##type##_BUFFER_BINDING, &_lastBound)
		case_setLastBoundByType(ARRAY); break;
		case_setLastBoundByType(DISPATCH_INDIRECT); break;
		case_setLastBoundByType(ELEMENT_ARRAY); break;
		case_setLastBoundByType(PIXEL_PACK); break;
		case_setLastBoundByType(PIXEL_UNPACK); break;
		case_setLastBoundByType(SHADER_STORAGE); break;
		case_setLastBoundByType(TEXTURE); break;
		case_setLastBoundByType(TRANSFORM_FEEDBACK); break;
		case_setLastBoundByType(UNIFORM); break;
		#undef case_setLastBoundByType
		}
		glBindBuffer(bufferType, _obj);
	}
	
	void buffer<bufferType>::_unbind() {
		glBindBuffer(bufferType, _lastBound);
	}
	
	template<buffer_type bufferType>
	bool buffer<bufferType>::is_mapped() {
		GLint paramValue;
		
		if(GLEW_VERSION_4_5) {
			glGetNamedBufferParameteriv(_obj, GL_BUFFER_MAPPED, &paramValue);
		} else
		if(GLEW_EXT_direct_state_access) {
			glGetNamedBufferParameterivEXT(_obj, GL_BUFFER_MAPPED, &paramValue);
		} else {
			_bind();
			glGetBufferParameteriv(bufferType, GL_BUFFER_MAPPED, &paramValue);
			_unbind();
		}
		
		return paramValue != GL_FALSE;
	}
	
	template<buffer_type bufferType>
	void* buffer<bufferType>::raw_map() {
		if(is_mapped()) {
			return nullptr;
		}
		
		return _buffer::_map_func_ptr(bufferType, _obj, GL_READ_WRITE);
	}
	
	template<buffer_type bufferType>
	const void* buffer<bufferType>::raw_map_read() {
		if(is_mapped()) {
			return nullptr;
		}
		
		return _buffer::_map_func_ptr(bufferType, _obj, GL_READ_ONLY);
	}
	
	template<buffer_type bufferType>
	void* buffer<bufferType>::raw_map_write() {
		if(is_mapped()) {
			return nullptr;
		}
		
		return _buffer::_map_func_ptr(bufferType, _obj, GL_WRITE_ONLY);
	}
	
	template<buffer_type bufferType>
	void buffer<bufferType>::unmap() {
		_bind();
		
		glUnmapBuffer(bufferType);
		
		_unbind();
	}
	
	using array_buffer =               buffer<BUFFER_TYPE_ARRAY>;
	using atomic_counter_buffer =      buffer<BUFFER_TYPE_ATOMIC_COUNTER>;
	using copy_read_buffer =           buffer<BUFFER_TYPE_COPY_READ>;
	using copy_write_buffer =          buffer<BUFFER_TYPE_COPY_WRITE>;
	using dispatch_indirect_buffer =   buffer<BUFFER_TYPE_DISPATCH_INDIRECT>;
	using draw_indirect_buffer =       buffer<BUFFER_TYPE_DRAW_INDIRECT>;
	using element_array_buffer =       buffer<BUFFER_TYPE_ELEMENT_ARRAY>;
	using pixel_pack_buffer =          buffer<BUFFER_TYPE_PIXEL_PACK>;
	using pixel_unpack_buffer =        buffer<BUFFER_TYPE_PIXEL_UNPACK>;
	using query_buffer =               buffer<BUFFER_TYPE_QUERY>;
	using shader_storage_buffer =      buffer<BUFFER_TYPE_SHADER_STORAGE>;
	using texture_buffer =             buffer<BUFFER_TYPE_TEXTURE>;
	using transform_feedback_buffer =  buffer<BUFFER_TYPE_TRANSFORM_FEEDBACK>;
	using uniform_buffer =             buffer<BUFFER_TYPE_UNIFORM>;
}

GLenum ogl::init() {
	
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		return err;
	}
	
	if(GLEW_VERSION_4_5) {
		_buffer::_map_func_ptr = &_buffer::_map_func_4_5;
	} else
	if(GLEW_EXT_direct_state_access) {
		_buffer::_map_func_ptr = &_buffer::_map_func_ext;
	} else {
		_buffer::_map_func_ptr = &_buffer::_map_func_fallback;
	}
	
	return err;
}

#ifndef OGL_BINARY
#ifdef _WIN32
	
	#include <gl/wglew.h>
	
	void* ogl::get_current_context() {
		return (void*)wglGetCurrentContext();
	}
	
#else
	
	#include <gl/glxew.h>
	
	void* ogl::get_current_context() {
		return (void*)glXGetCurrentContext();
	}
	
#endif//WIN32 and X11 ogl::get_current_context definitions
#endif//OGL_BINARY

bool ogl::is_current_context(void* ctx) {
	return get_current_context() == ctx;
}

#endif//OGL_H
