#pragma once
#include <stdint.h>
#include <memory>
#include <m_common/m_IndexBuffer.h>
#include <m_common/m_VertexBuffer.h>

class VertexArrayBuffer
{
public:
	VertexArrayBuffer();
	~VertexArrayBuffer();

	void Bind() const;
	void UnBind() const;

    uint32_t GetVAOID() const;

	void SetVertexBuffer(std::shared_ptr<VertexBuffer>& vb);
	void SetIndexBuffer(std::shared_ptr<IndexBuffer>& ib);


	const std::shared_ptr<IndexBuffer>& GetIndexBuffer()const { return m_IndexBuffer; }
	const std::shared_ptr<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }
	static std::shared_ptr<VertexArrayBuffer> Create();
	static std::shared_ptr<VertexArrayBuffer> Create(std::shared_ptr<VertexBuffer>, std::shared_ptr<IndexBuffer>);


private:
	uint32_t VAO_id;
	std::shared_ptr<VertexBuffer> m_VertexBuffer;
	std::shared_ptr<IndexBuffer> m_IndexBuffer;
};