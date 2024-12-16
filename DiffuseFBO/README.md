# DiffuseFBO 

This demo shows how we can generate a full screen rendering area by using a simple triangle.

The triangle will be much larger than the viewport area but only a part of it will be visible as the rest will be clipped.  This is a very efficient way of rendering what used to be called a "Screen Quad"

There is a good article [here](https://rauwendaal.net/2014/06/14/rendering-a-screen-covering-triangle-in-opengl/) showing this and this demo is a good starting point for any project that requires rendering some form of buffer / texture to the screen.

## Process

1. We first load the shaders which will be called when drawing our triangle.
2. Next we need a VertexArrayObject to enable drawing. This will not contain any buffers or data but still needs to be bound.
3. call glDrawArrays(GL_TRIANGLES,0,3); This calls the vertex shader 3 times, where gl_VertexID is used to determine which of the triangle vertices to generate and also the texture co-ordinates
4. These are passed onto the fragment shader.

Next we are going to Render to a Frame Buffer Object (FBO) and then render the texture to the screen.
Passes are as follows:
1.	Geometry Pass:
    - Outputs position (gPosition), normal (gNormal), and color (gAlbedo) to the G-buffer.
2.	Shading Pass:
    - Uses G-buffer textures to compute diffuse shading in screen space.
3.	Framebuffer:
    - Contains attachments for position, normal, and color.
4.	Shaders:
    - Vertex/fragment shaders handle data transformation and storage in the geometry pass.
    - The shading pass applies lighting based on the data.
5.	Light Source:
    - A single point light is used for simplicity.

