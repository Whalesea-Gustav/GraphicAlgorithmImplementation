#pragma once
#include <stdint.h>
#include <memory>
#include <GL/glew.h>

class IndexBuffer
{
public:
	IndexBuffer(const uint32_t* indices, uint32_t count);
	~IndexBuffer();

	void Bind();
	void UnBind();

	inline void SetCount(uint32_t count) { m_Count = count; }
	inline uint32_t GetCount()const { return m_Count; }

	static std::shared_ptr<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
	void Update(const uint32_t* indices, uint32_t count);

private:
	uint32_t IBO_id;
	uint32_t m_Count;
};