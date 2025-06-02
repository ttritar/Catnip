#version 450

vec2 positions[3] = vec2[](
    vec2(-1.0, -1.0), // bot-left
    vec2( 3.0, -1.0), // bot-right (is offscreen)
    vec2(-1.0,  3.0)  // top-left (is offscreen)
);