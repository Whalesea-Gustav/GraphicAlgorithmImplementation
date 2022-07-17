#pragma once

#include <GL/glew.h>
#include <string>
#include <vector>

class Texture2D
{
public:
	Texture2D();
public:
	GLuint ID;
	GLuint Width, Height;
	GLuint Internal_Format;
	GLuint Image_Format;
	GLuint Wrap_S;
	GLuint Wrap_T;
	GLuint Filter_Min;
	GLuint Filter_Max;
    GLuint DataType;

    void Generate(GLuint width, GLuint height, float* data);
    void Generate(GLuint width, GLuint height, char* data);
	void Bind() const;
};

struct TextureConfig
{
    enum class m_TextureType
    {
        Texture2D = GL_TEXTURE_2D,
        Texture3D = GL_TEXTURE_3D,
        Texture2DArray = GL_TEXTURE_2D_ARRAY,
        TextureCubeMap = GL_TEXTURE_CUBE_MAP,
        TextureCubeArray = GL_TEXTURE_CUBE_MAP_ARRAY,
        DepthCubeMap = GL_DEPTH_COMPONENT,
    };
    enum class m_TextureAttachmentType
    {
        ColorTexture,
        DepthTexture,
        CubeDepthTexture,
        StencilTexture,
        DepthAndStencilTexture,
        DepthArrayTexture
    };
    GLint					InternalFormat;
    GLenum					ExternalFormat;
    GLenum					DataType;
    GLsizei					Width;
    GLsizei					Height;
    GLsizei					Depth;
    std::vector<GLvoid*>	pDataSet;
    GLint					Type4WrapS;
    GLint					Type4WrapT;
    GLint					Type4WrapR;
    GLint					Type4MinFilter;
    GLint					Type4MagFilter;
    GLboolean				isMipmap;
    GLboolean				isSRGBSpace;
    GLint					TextureID;
    GLint					ImageBindUnit;
    GLint					FrameBufferID;
    GLint					AttachmentID;
    std::string				TextureName;
    m_TextureType			TextureType;
    m_TextureAttachmentType	TextureAttachmentType;
    std::vector<float>		BorderColor;

    TextureConfig() : TextureID(-1), InternalFormat(GL_RGBA), ExternalFormat(GL_RGBA), DataType(GL_UNSIGNED_BYTE), Width(1024),
                 Height(1024), Depth(1), Type4WrapS(GL_REPEAT), Type4WrapT(GL_REPEAT), Type4WrapR(GL_REPEAT), Type4MinFilter(GL_LINEAR),
                 Type4MagFilter(GL_LINEAR), isMipmap(GL_FALSE), isSRGBSpace(GL_FALSE), TextureType(m_TextureType::Texture2D), TextureAttachmentType(m_TextureAttachmentType::ColorTexture),
                 BorderColor{ 1,1,1,1 }, ImageBindUnit(-1), FrameBufferID(-1), AttachmentID(-1)
    {
    }
};

void m_generate_texture(TextureConfig& textureConfig)
{
    GLint TextureID;
    glGenTextures(1, &(GLuint&)TextureID);

    GLuint TextureType = -1;
    switch (textureConfig.TextureType)
    {
        case TextureConfig::m_TextureType::Texture2D:
            TextureType = GL_TEXTURE_2D;
            glBindTexture(TextureType, TextureID);
            glTexImage2D(TextureType, 0, textureConfig.InternalFormat,
                         textureConfig.Width, textureConfig.Height, 0,
                         textureConfig.ExternalFormat, textureConfig.DataType,
                         textureConfig.pDataSet.size() > 0 ? textureConfig.pDataSet[0] : nullptr);
            break;
        case TextureConfig::m_TextureType::Texture2DArray:
            TextureType = GL_TEXTURE_2D_ARRAY;
            glBindTexture(TextureType, TextureID);
            glTexImage3D(TextureType, 0, textureConfig.InternalFormat, textureConfig.Width, textureConfig.Height, textureConfig.Depth, 0, textureConfig.ExternalFormat, textureConfig.DataType, nullptr);
            for (int i = 0; i < static_cast<int>(textureConfig.pDataSet.size()); ++i)
                glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, textureConfig.Width, textureConfig.Height, 1, textureConfig.ExternalFormat, textureConfig.DataType, textureConfig.pDataSet[i]);
            break;
        case TextureConfig::m_TextureType::Texture3D:
            TextureType = GL_TEXTURE_3D;
            glBindTexture(TextureType, TextureID);
            glTexImage3D(TextureType, 0, textureConfig.InternalFormat, textureConfig.Width, textureConfig.Height, textureConfig.Depth, 0, textureConfig.ExternalFormat, textureConfig.DataType, textureConfig.pDataSet.size() > 0 ? textureConfig.pDataSet[0] : nullptr);
            break;
        case TextureConfig::m_TextureType::TextureCubeMap:
            TextureType = GL_TEXTURE_CUBE_MAP;
            glBindTexture(TextureType, TextureID);
            for (int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, textureConfig.InternalFormat, textureConfig.Width, textureConfig.Height, 0, textureConfig.ExternalFormat, textureConfig.DataType, int(textureConfig.pDataSet.size()) > i ? textureConfig.pDataSet[i] : nullptr);
            break;
        case TextureConfig::m_TextureType::TextureCubeArray:
            TextureType = GL_TEXTURE_CUBE_MAP_ARRAY;
            glBindTexture(TextureType, TextureID);
            glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, textureConfig.InternalFormat, textureConfig.Width, textureConfig.Height, textureConfig.Depth * 6, 0, textureConfig.ExternalFormat, textureConfig.DataType, textureConfig.pDataSet.size() > 0 ? textureConfig.pDataSet[0] : nullptr);
            break;
        case TextureConfig::m_TextureType::DepthCubeMap:
            TextureType = GL_TEXTURE_CUBE_MAP;
            glBindTexture(TextureType, TextureID);
            for (int i = 0; i < 6; ++i)
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, textureConfig.Width, textureConfig.Height, 0, GL_DEPTH_COMPONENT, textureConfig.DataType, int(textureConfig.pDataSet.size()) > i ? textureConfig.pDataSet[i] : nullptr);
            break;
        default:
            break;
    }
    glTexParameterfv(TextureType, GL_TEXTURE_BORDER_COLOR, textureConfig.BorderColor.data());
    glTexParameteri(TextureType, GL_TEXTURE_WRAP_S, textureConfig.Type4WrapS);
    glTexParameteri(TextureType, GL_TEXTURE_WRAP_T, textureConfig.Type4WrapT);
    glTexParameteri(TextureType, GL_TEXTURE_WRAP_R, textureConfig.Type4WrapR);
    if (textureConfig.isMipmap && textureConfig.Type4MinFilter == GL_LINEAR)
        textureConfig.Type4MinFilter = GL_LINEAR_MIPMAP_LINEAR;
    glTexParameteri(TextureType, GL_TEXTURE_MIN_FILTER, textureConfig.Type4MinFilter);
    glTexParameteri(TextureType, GL_TEXTURE_MAG_FILTER, textureConfig.Type4MagFilter);
    //glTexParameteri(TextureType, GL_TEXTURE_MAX_LEVEL, 4);
    if (textureConfig.isMipmap)
        glGenerateMipmap(TextureType);
    glBindTexture(TextureType, 0);
    if (textureConfig.ImageBindUnit != -1)
        glBindImageTexture(textureConfig.ImageBindUnit, TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, textureConfig.InternalFormat);
    textureConfig.TextureID = TextureID;
}

void m_bind_image_texture(TextureConfig& textureConfig)
{
    if (textureConfig.ImageBindUnit != -1)
        glBindImageTexture(textureConfig.ImageBindUnit, textureConfig.TextureID, 0, GL_FALSE, 0, GL_READ_WRITE, textureConfig.InternalFormat);
}

void m_clear_texture(TextureConfig& textureConfig, GLuint TextureType)
{
    std::vector<GLfloat> emptyData(textureConfig.Width * textureConfig.Height * textureConfig.Depth * sizeof(float), 0);
    glBindTexture(TextureType, textureConfig.TextureID);
    glTexSubImage3D(TextureType, 0, 0, 0, 0,
                    textureConfig.Width, textureConfig.Height, textureConfig.Depth,
                    textureConfig.ExternalFormat, textureConfig.DataType, &emptyData[0]);
    glBindTexture(TextureType, 0);
}