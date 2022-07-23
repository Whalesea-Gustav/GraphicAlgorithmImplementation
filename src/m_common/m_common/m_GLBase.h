#pragma once
#include <GL/glew.h>
#include <agz-utils/misc/uncopyable.h>
#include <cassert>

class gl_object_base : public agz::misc::uncopyable_t
{
public:
    explicit gl_object_base(GLuint _handle = 0) noexcept : handle(_handle) {};

    gl_object_base(gl_object_base&& other) noexcept
    :handle(other.handle)
    {
        other.handle = 0;
    }

    virtual ~gl_object_base() {};

    gl_object_base& operator=(gl_object_base&& other) noexcept
    {
        assert(!handle);
        handle = other.handle;
        other.handle = 0;
        return *this;
    }

    GLuint GetHandle() const noexcept
    {
        return handle;
    }

protected:
    GLuint handle;
};