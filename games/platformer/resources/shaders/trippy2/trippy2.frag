#version 330 core
precision highp float;

in vec2 uv;
out vec4 color;

uniform sampler2D image;

uniform float time;

void main (void)
{
    vec2 resolution = vec2(200.0, 200.0);
    vec2 position = (gl_FragCoord.xy / resolution.xy);
    float ss = 0.1;
    vec2 gg = gl_FragCoord.xy;
    gg = ceil(gg / ss) * ss;

    float t = time * .1;
    float alpha = time * 1.25;

    vec3 ro = vec3(0, 0, -1);
        vec3 lookat = vec3(0.0);
        float zoom = .1 + abs( sin(t))/5.;

        vec3 f = normalize(lookat-ro),
        r = normalize(cross(vec3(0,1,0), f)),
        u = cross(r, f),
        c = ro + f * zoom,
        i = c + uv.x * r + uv.y * u,
        rd = normalize(i-ro);

        float radius = mix(.3, 1.5, .5+.5);

        float dS, dO;
        vec3 p;

        for(int i=0; i<2000; i++) 
        {
            if (alpha > 0.5)
                alpha -= 0.01;

            p = ro + rd * dO;
            dS = -(length(vec2(length(p.xz)-1., p.y)) - .15);
            if(dS<.0001) break;
            dO += dS;
        }

    vec3 col = vec3(0);
    vec3 col2 = vec3(0);
  
         
    if(dS<.001) {
        float x = atan(p.x, p.z)+t*.5;          // -pi to pi
        float y = atan(length(p.xz)-1., p.y);
         
        float ripples = sin((x*10.-y*10.)*3.)*.5+.5;
        float waves = sin(x*2.-y*4.+t*20.);
        float custom = sin(x*5.-y*9.+t*40.);

        float b1 = smoothstep(-.4, .2, custom);
        float b2 = smoothstep(-.2, .4, custom-.5);
        float b3 = smoothstep(-.5, .3, custom);

        float m = b1*(1.-b2);
        m = max(m, ripples*b2*max(0., waves));
        m += max(0., waves*.3*b2);

        float z = b1*(1.-b2);
        z = max(m, ripples*b2*max(0., custom));
        z += max(0., custom*.3*b2);

        col += m;
        col.rb *= 2.5;
        col.z *= 2.5*abs(cos(t));

        col2 += m;
        col2.rb *= 5.5;
        col2.z *= 7.2*abs(cos(t));
    }

    color = vec4( col2, alpha ) * texture(image, uv);
}

