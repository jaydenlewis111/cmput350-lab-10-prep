#version 120 // glsl version 1.2 for OpenGL 2.1

uniform sampler2D texture;
uniform vec2 textureSize;   // size in pixels

const int RADIUS = 2;

void main() {
    vec2 uv = gl_TexCoord[0].xy;
    // One pixel expressed in UV units
    vec2 texel = 1.0 / textureSize;

    vec4 sum = vec4(0.0);
    for (int dy = -RADIUS; dy <= RADIUS; ++dy) {
        for (int dx = -RADIUS; dx <= RADIUS; ++dx) {
            sum += texture2D(texture, uv + vec2(float(dx), float(dy)) * texel);
        }
    }
    float count = float((2 * RADIUS + 1) * (2 * RADIUS + 1));
    gl_FragColor = sum / count;
}
