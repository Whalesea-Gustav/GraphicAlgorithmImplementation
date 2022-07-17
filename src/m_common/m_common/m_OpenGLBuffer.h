#pragma once
#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>

class OpenGLBuffer
{
public:
    OpenGLBuffer() {};
    virtual ~OpenGLBuffer() {};

    virtual void Bind()=0;
    virtual void UnBind()=0;

    static std::shared_ptr<OpenGLBuffer> Create();
};