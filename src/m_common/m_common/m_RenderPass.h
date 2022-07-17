#pragma once
#include <string>
#include <memory>
#include <GL/glew.h>
#include <m_common/m_Shader.h>
class RenderPass
{
public:
    RenderPass() {};
    RenderPass(std::string& PassName) : m_PassName(PassName) {};

    virtual ~ RenderPass()
    {
    }

    void setShader(std::shared_ptr<Shader> pShader)
    {
        this->m_pShader = pShader;
    }
    std::shared_ptr<Shader> getShader()
    {
        return this->m_pShader;
    }

    const std::string& getPassName() const { return m_PassName; };
    void setPassName(const std::string& PassName) { this->m_PassName = PassName; };

    virtual void initV() = 0;
    virtual void updateV() = 0;

protected:
    std::shared_ptr<Shader> m_pShader;
    std::string m_PassName;
    std::string RenderPassType;
};

class RenderPassV2
{
public:
    RenderPassV2() = default;;
    RenderPassV2(std::string& PassName) : m_PassName(PassName) {};

    virtual ~RenderPassV2()
    = default;

    void SetShader(std::shared_ptr<Shader> pShader)
    {
        this->m_pShader = pShader;
    }

    std::shared_ptr<Shader> GetShader()
    {
        return this->m_pShader;
    }

    const std::string& GetPassName() const { return m_PassName; };

    void SetPassName(const std::string& PassName) { this->m_PassName = PassName; };

    virtual void InitPass() = 0;
    virtual void RenderPass() = 0;

protected:
    std::shared_ptr<Shader> m_pShader;
    std::string m_PassName;
    std::string RenderPassType;
};