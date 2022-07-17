#include <m_common/m_IndexBuffer.h>

IndexBuffer::IndexBuffer(const uint32_t* indices, uint32_t count)
	:m_Count(count)
{
	glGenBuffers(1, &IBO_id);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_id);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * count, indices, GL_STATIC_DRAW);
}

IndexBuffer::~IndexBuffer()
{
	glDeleteBuffers(1, &IBO_id);
}

void IndexBuffer::Bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO_id);
}

void IndexBuffer::UnBind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

std::shared_ptr<IndexBuffer> IndexBuffer::Create(const uint32_t* indices, uint32_t count)
{

	return std::make_shared<IndexBuffer>(indices, count);
}
void IndexBuffer::Update(const uint32_t* indices, uint32_t count)
{
	Bind();
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * count, indices, GL_STATIC_DRAW);
	UnBind();
}