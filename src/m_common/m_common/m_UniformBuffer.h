#pragma once

#include <stdint.h>
#include <memory>
#include <GL/glew.h>

class UniformBuffer
{
public:
    UniformBuffer(const void* data, uint32_t size)
    {
        glGenBuffers(1, &UBO_id);

        glBindBuffer(GL_UNIFORM_BUFFER, UBO_id);
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
    ~UniformBuffer()
    {

    }

    void Bind()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, UBO_id);
    }

    void BindBufferRange(uint32_t index, uint32_t offset, uint32_t size)
    {
        glBindBufferRange(GL_UNIFORM_BUFFER, index, this->UBO_id, offset, size);
    }

    void BindBufferBase(uint32_t index)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, index, this->UBO_id);
    }

    void UnBind()
    {
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }

    void Update(const void* data, uint32_t size)
    {
        this->Bind();
        glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW);
        //learn OpenGL use glBufferSubData to bind different part of data
        this->UnBind();
    }

    static std::shared_ptr<UniformBuffer> Create(const void* data, uint32_t size)
    {
        return std::make_shared<UniformBuffer>(data, size);
    }

private:
    uint32_t UBO_id;
};