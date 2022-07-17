#pragma once

#include <m_common/m_OpenGLBuffer.h>

class FrameBuffer : public OpenGLBuffer
{
public:
    FrameBuffer()
    {
        glGenFramebuffers(1, &FBO_id);
    }

    ~FrameBuffer() {}

    //depth map use GenerateTextureAttachment(width, height, GL_DEPTH_COMPONENT, GL_FLOAT)
    void GenerateTexture2DAttachment(uint32_t width, uint32_t height, GLenum texture_interalformat, GLenum texture_format, GLenum texture_type, GLenum attachment_type)
    {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_interalformat,
                     width, height, 0, texture_format, texture_type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        this->textures.push_back(texture_id);

        //bind texture to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, this->FBO_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, texture_id, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GenerateTexture2DAttachment(uint32_t width, uint32_t height, GLenum texture_format, GLenum texture_type, GLenum attachment_type)
    {
        GenerateTexture2DAttachment(width, height, GL_DEPTH_COMPONENT, texture_format, texture_type, attachment_type);
    }

    void GenerateShadowMap(uint32_t width, uint32_t height, GLenum texture_interalformat, GLenum texture_format, GLenum texture_type, GLenum attachment_type, bool alone)
    {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, texture_interalformat,
                     width, height, 0, texture_format, texture_type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        this->textures.push_back(texture_id);

        //bind texture to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, this->FBO_id);
        glFramebufferTexture2D(GL_FRAMEBUFFER, attachment_type, GL_TEXTURE_2D, texture_id, 0);

        //explicit tell OpenGL not render to color attachment
        if (alone)
        {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    std::vector<uint32_t> GetTextureAttachment() const
    {
        return this->textures;
    }

    void Bind()
    {
       glBindFramebuffer(GL_FRAMEBUFFER, this->FBO_id);
    }

    void UnBind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Update(const void* data, uint32_t size)
    {
    }

    static std::shared_ptr<FrameBuffer> Create()
    {
        return std::make_shared<FrameBuffer>();
    }

private:
    uint32_t FBO_id;
    std::vector<uint32_t> textures;
};