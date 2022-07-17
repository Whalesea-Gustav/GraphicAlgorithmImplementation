#include <m_common/m_VertexArrayBuffer.h>


VertexArrayBuffer::VertexArrayBuffer()
{
	glGenVertexArrays(1, &VAO_id);
}

VertexArrayBuffer::~VertexArrayBuffer()
{
	glDeleteVertexArrays(1, &VAO_id);
}

void VertexArrayBuffer::Bind()const
{
	glBindVertexArray(VAO_id);
}

void VertexArrayBuffer::UnBind()const
{
	glBindVertexArray(0);
}

uint32_t VertexArrayBuffer::GetVAOID() const
{
    return this->VAO_id;
}

void VertexArrayBuffer::SetVertexBuffer(std::shared_ptr<VertexBuffer>& vb)
{
	auto TypeCast = [](OpenGLDataType type)->GLenum {
		switch (type)
		{
		case OpenGLDataType::Float: return GL_FLOAT;
			break;
		case OpenGLDataType::Int: return GL_INT;
			break;
		case OpenGLDataType::Double: return GL_DOUBLE;
			break;
		case OpenGLDataType::Byte: return GL_BYTE;
			break;
		case OpenGLDataType::UInt32: return GL_UNSIGNED_INT;

        default : return GL_NONE;
		}
	};

	Bind();
	m_VertexBuffer = vb;
	m_VertexBuffer->Bind();
	auto& layout = m_VertexBuffer->GetLayout();
	auto& LayoutElements = layout.GetElements();
	uint32_t totalSize = layout.GetTotalSize();
	for (int i = 0; i < LayoutElements.size(); i++)
	{
		const LayoutElement& e = LayoutElements[i];

		glEnableVertexAttribArray(i);
		if (e.Type == OpenGLDataType::Int)
			glVertexAttribIPointer(i, e.Count, TypeCast(e.Type), totalSize, (void*)e.offset);
		else
			glVertexAttribPointer(i, e.Count, TypeCast(e.Type), (e.Normalized == true) ? GL_TRUE : GL_FALSE, totalSize, (void*)e.offset);
	}
	UnBind();
}

void VertexArrayBuffer::SetIndexBuffer(std::shared_ptr<IndexBuffer>& ib)
{
	Bind();
	ib->Bind();
	m_IndexBuffer = ib;
	UnBind();
}

std::shared_ptr<VertexArrayBuffer> VertexArrayBuffer::Create()
{
	return std::make_shared<VertexArrayBuffer>();
}

std::shared_ptr<VertexArrayBuffer> VertexArrayBuffer::Create(std::shared_ptr<VertexBuffer> VBO, std::shared_ptr<IndexBuffer> IBO)
{
	auto VAO = std::make_shared<VertexArrayBuffer>();
	VAO->SetVertexBuffer(VBO);
	VAO->SetIndexBuffer(IBO);
	return VAO;
}
