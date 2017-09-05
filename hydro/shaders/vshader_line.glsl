#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

uniform mat4 mvp_matrix;
attribute vec4 vertex;
attribute vec3 normal;

varying vec4 v_vertex;
varying vec3 v_normal;
varying vec4 v_color;

//! [0]
void main()
{
    v_vertex = vertex;
    vec3 n_normal = normalize(normal);
    v_normal = n_normal;
    v_color = gl_Color;

    vec4 position = mvp_matrix * vertex;
    gl_Position = position;
    gl_FrontColor = gl_Color;


}
//! [0]
