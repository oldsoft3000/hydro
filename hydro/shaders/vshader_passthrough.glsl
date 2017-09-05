#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

varying vec2 position;

void main()
{
    gl_Position = ftransform();
    position = gl_Vertex.xy;
}
