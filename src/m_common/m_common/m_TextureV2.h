#include "m_GLBase.h"
#include "m_Debug.h"

class texture_base : public gl_object_base
{
public:
    using gl_object_base::gl_object_base;

    virtual ~texture_base()
    {
        destroy();
    }

    void destroy()
    {
        if(handle){
            GL_EXPR(glDeleteTextures(1,&handle));
            handle = 0;
        }
    }

    virtual void initialize_handle() = 0;

};