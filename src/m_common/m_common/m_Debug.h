#include <GL/glew.h>

#include <spdlog/spdlog.h>

#define LOG_DEBUG(str, ...) spdlog::debug(str, ##__VA_ARGS__)
#define LOG_INFO(str, ...) spdlog::info(str, ##__VA_ARGS__)
#define LOG_ERROR(str, ...) spdlog::error(str, ##__VA_ARGS__)
#define LOG_CRITICAL(str, ...) spdlog::critical(str, ##__VA_ARGS__)

#define SET_LOG_LEVEL_DEBUG spdlog::set_level(spdlog::level::debug);

#define SET_LOG_LEVEL_INFO spdlog::set_level(spdlog::level::info);

#define SET_LOG_LEVEL_ERROR spdlog::set_level(spdlog::level::err);

#define SET_LOG_LEVEL_CRITICAL spdlog::set_level(spdlog::level::critical);

#define NOT_IMPL LOG_ERROR("this method is not imply yet!");

std::string GetGLErrorStr(GLenum gl_error)
{
    std::string error;
    switch (gl_error)
    {
        case GL_INVALID_ENUM:
            error = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_INVALID_INDEX:
            error = "GL_INVALID_INDEX";
            break;
        default:
            error = "UNKNOWN_ERROR";
            break;
    }
    return error;
}

void PrintGLErrorType(GLenum gl_error)
{
    LOG_ERROR("{}", GetGLErrorStr(gl_error));
}

GLenum PrintGLErrorMsg(const char *file, int line)
{
    GLenum error_code;
    while ((error_code = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (error_code)
        {
            case GL_INVALID_ENUM:
                error = "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "GL_INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "GL_STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "GL_STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "GL_OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_INVALID_INDEX:
                error = "GL_INVALID_INDEX";
                break;
        }
        LOG_ERROR("{} at line {} in file {}", error, line, file);
    }
    return error_code;
}

#define GL_REPORT PrintGLErrorMsg(__FILE__, __LINE__);

#define GL_ASSERT assert(glGetError() == GL_NO_ERROR);

#define GL_EXPR(expr)                                                                                                  \
    do                                                                                                                 \
    {                                                                                                                  \
        GLenum gl_error;                                                                                               \
        int __count = 0;                                                                                                 \
        while ((gl_error = glGetError()) != GL_NO_ERROR)                                                               \
        {                                                                                                              \
            LOG_ERROR("GL error {} before call {} at line {} in file {}", GetGLErrorStr(gl_error), #expr, __LINE__,    \
                      __FILE__);                                                                                       \
            __count++;                                                                                                   \
            if (__count > 10)                                                                                            \
                break;                                                                                                 \
        }                                                                                                              \
        expr;                                                                                                          \
        __count = 0;                                                                                                     \
        while ((gl_error = glGetError()) != GL_NO_ERROR)                                                               \
        {                                                                                                              \
            LOG_ERROR("Calling {} caused GL error {} at line {} in file {}", #expr, GetGLErrorStr(gl_error), __LINE__, \
                      __FILE__);                                                                                       \
            if (++__count > 10)                                                                                          \
                break;                                                                                                 \
        }                                                                                                              \
    } while (false);

#define GL_CHECK                                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        GLenum gl_error;                                                                                               \
        int __count = 0;                                                                                                 \
        while ((gl_error = glGetError()) != GL_NO_ERROR)                                                               \
        {                                                                                                              \
            LOG_ERROR("GL error {} before line {} in file {}", GetGLErrorStr(gl_error), __LINE__, __FILE__);           \
            if (++__count > 10)                                                                                          \
                break;                                                                                                 \
        }                                                                                                              \
    } while (0);
