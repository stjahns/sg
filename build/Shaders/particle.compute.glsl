#layout 330 core

layout (std140, binding=0) uniform attractor_block
{
	vec4 attractor[64]; // xyz = position, w = mass
}

//layout (local_size_x = 1024) in;

layout (rgba32f, binding = 0) uniform imageBuffer velocity_buffer;
layout (rgba32f, binding = 1) uniform imageBuffer position_buffer;

uniform float dt = 1.0;

void main(void)
{
    vec4 vel = imageLoad(velocity_buffer, int(gl_GlobalInvocationID.x));
    vec4 pos = imageLoad(position_buffer, int(gl_GlobalInvocationID.x));

    int i;

    pos.xyz += vel.xyz * dt;
    pos.w -= 0.0001 * dt;

    for (i = 0; i < 4; i++)
    {
        vec3 dist = (attractor[i].xyz - pos.xyz);
        vel.xyz += dt * dt * attractor[i].w * normalize(dist) / (dot(dist, dist) + 10.0);
    }

    if (pos.w <= 0.0)
    {
        pos.xyz = -pos.xyz * 0.01;
        vel.xyz *= 0.01;
        pos.w += 1.0f;
    }

    imageStore(position_buffer, int(gl_GlobalInvocationID.x), pos);
    imageStore(velocity_buffer, int(gl_GlobalInvocationID.x), vel);
}
