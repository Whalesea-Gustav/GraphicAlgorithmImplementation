#include <m_common/m_VertexBuffer.h>


VertexBuffer::VertexBuffer(const void* data, uint32_t size)
{
		glGenBuffers(1, &VBO_id);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_id);

		glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

VertexBuffer::~VertexBuffer()
{
	glDeleteBuffers(1, &VBO_id);
}


void VertexBuffer::Bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO_id);
}


void VertexBuffer::UnBind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::Update(const void* data, uint32_t size)
{
	this->Bind();

	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	this->UnBind();
}

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const void* data, uint32_t size)
{
	return std::make_shared<VertexBuffer>(data, size);
}
