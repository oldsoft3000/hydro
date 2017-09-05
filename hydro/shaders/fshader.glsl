#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

varying vec3 v_vertex;
varying vec3 v_normal;
varying vec4 v_color;

//! [0]
void main()
{
    gl_FragColor = v_color;
}
//! [0]

