#ifdef GL_ES
// Set default precision to medium
precision mediump int;
precision highp float;
#endif

varying vec2 position;
uniform sampler2D renderedTexture;

void main()
{
    gl_FragColor = texture2D(renderedTexture, position);
}
