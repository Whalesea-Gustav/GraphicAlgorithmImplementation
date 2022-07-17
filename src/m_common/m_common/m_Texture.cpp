#include <m_common/m_Texture.h>
#include <iostream>

Texture2D::Texture2D()
{
	Width = Height = 0;
	Internal_Format = GL_RGB;
	Image_Format = GL_RGB;
	Wrap_S = GL_REPEAT;
	Wrap_T = GL_REPEAT;
	Filter_Min = GL_LINEAR;
	Filter_Max = GL_LINEAR;
    DataType = GL_UNSIGNED_BYTE;
	glGenTextures(1, &this->ID);
}


void Texture2D::Generate(GLuint width, GLuint height, float* data)
{
	this->Width = width;
	this->Height = height;

	glBindTexture(GL_TEXTURE_2D, this->ID);
	glTexImage2D(GL_TEXTURE_2D, 0, this->Internal_Format, width, height, 0, this->Image_Format, this->DataType, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, this->Wrap_S);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, this->Wrap_T);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, this->Filter_Min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, this->Filter_Max);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::Bind() const
{
	glBindTexture(GL_TEXTURE_2D, this->ID);
}
