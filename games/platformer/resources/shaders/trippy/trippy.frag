#version 330 core
precision highp float;

out vec4 color;

uniform float time;

void main(void) {

    vec2 resolution = vec2(200.0, 200.0);

    vec2 position = (gl_FragCoord.xy / resolution.xy);

    float val = 100.0;

    val *= cos( position.x * sin( time / 15.0 ) * 80.0 ) + cos( position.y * cos( time / 15.0 ) * 10.0 );
    val *= cos( position.y * cos( time / 10.0 ) * 40.0 ) - cos( position.x * sin( time / 25.0 ) * 40.0 );
    val -= cos( position.x * sin( time / 5.0 ) * 20.0 ) - cos( position.y * sin( time / 35.0 ) * 80.0 );
    val *= cos( position.y * sin( time / 5.0 ) * 20.0 ) + cos( position.x * sin( time / 45.0 ) * 120.0 );

    color = vec4( vec3( sin( val * time / 69.0 ) * 0.75, sin( val * time / 3.0 ) * 0.75, sin( val + time / 3.0 ) * 0.75 ), 0.5 );

} 
