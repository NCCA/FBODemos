#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)


layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

out vec2 uv;
void main() 
{
  gl_Position = vec4( 1.0, 1.0, 0.5, 1.0 );
     uv = vec2( 1.0, 1.0 );
     EmitVertex();

     gl_Position = vec4(-1.0, 1.0, 0.5, 1.0 );
     uv = vec2( 0.0, 1.0 );
     EmitVertex();

     gl_Position = vec4( 1.0,-1.0, 0.5, 1.0 );
     uv = vec2( 1.0, 0.0 );
     EmitVertex();

     gl_Position = vec4(-1.0,-1.0, 0.5, 1.0 );
     uv = vec2( 0.0, 0.0 );
     EmitVertex();

     EndPrimitive();
}
