#pragma once
#include <m_common/m_Shader.h>

class ComputeShader : public Shader
{
public:
    ComputeShader(const char* computePath) : Shader()
    {
        std::string computeCode;
        std::ifstream cShaderFile;
        cShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);

        try
        {
            cShaderFile.open(computePath);
            std::stringstream cShaderStream;
            cShaderStream << cShaderFile.rdbuf();
            cShaderFile.close();
            computeCode = cShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        }
        const char* cShaderCode = computeCode.c_str();
        unsigned int compute;
        compute = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(compute, 1, &cShaderCode, NULL);
        glCompileShader(compute);
        this->checkCompileErrors(compute, "COMPUTE");
        this->ID = glCreateProgram();
        glAttachShader(ID, compute);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM");
        glValidateProgram(ID);
        glDeleteShader(compute);
    }
};