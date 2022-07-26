#version 460 core
layout (location = 0) in vec2 _Position;
layout (location = 1) in vec2 _TexCoords;

layout(location = 0) out vec2 oScreenCoord;

void main()
{
    gl_Position = vec4(_Position, 0, 1.0);
    oScreenCoord = _TexCoords;
}
