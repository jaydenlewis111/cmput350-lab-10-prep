#version 120 // glsl version 1.2 for OpenGL 2.1

uniform vec2 windowSize;  // render target size in pixels
uniform vec2 mousePos;    // cursor position in window coords (origin bottom-left)
uniform float time;       // seconds since program start
uniform bool animate;     // toggles Problem 3 animation

const vec4 BLACK = vec4(0.0, 0.0, 0.0, 1.0);
const vec4 RED = vec4(1.0, 0.0, 0.0, 1.0);
const vec4 GREEN = vec4(0.0, 1.0, 0.0, 1.0);
const vec4 YELLOW = vec4(1.0, 1.0, 0.0, 1.0);
const vec4 WHITE = vec4(1.0);
const float PI = 3.14159265358979;
const float CIRCLE_RADIUS = 10.0;

void main(void) {
    // Problem 1: normalize the pixel center to [0, 1]^2 and bilinearly interpolate corners.
    vec2 uv = gl_FragCoord.xy / windowSize;

    // Problem 3: slide the gradient horizontally over time. Using a cosine makes the
    // pattern periodic and mirrored, so it wraps around without a visible seam.
    // At time = 0, u goes from 0 to 1 across the screen.
    float u = uv.x;
    if (animate) {
        u = 0.5 - 0.5 * cos(PI * (uv.x - 0.25 * time));
    }

    vec4 bottom = mix(BLACK, RED, u);
    vec4 top = mix(GREEN, YELLOW, u);
    vec4 color = mix(bottom, top, uv.y);

    // Problem 2: analytic white circle of radius 10 around the cursor.
    if (distance(gl_FragCoord.xy, mousePos) <= CIRCLE_RADIUS) {
        color = WHITE;
    }

    gl_FragColor = color;
}
