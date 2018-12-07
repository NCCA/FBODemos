# Blitting Demo

This demo uses a shader to write check patterns to 8 outputs in the fragment shader

```
layout(location=0)out vec4 fragColour0;
layout(location=1)out vec4 fragColour1;
layout(location=2)out vec4 fragColour2;
layout(location=3)out vec4 fragColour3;
layout(location=4)out vec4 fragColour4;
layout(location=5)out vec4 fragColour5;
layout(location=6)out vec4 fragColour6;
layout(location=7)out vec4 fragColour7;
```

These are bound as colour attachments to the FBO and the call to render the screen quad will generate a simple check pattern and fill it in.

We then do the following to bind the default framebuffer (0 in most cases but in Qt always use[defaultFramebufferObject](http://doc.qt.io/qt-5/qopenglcontext.html#defaultFramebufferObject)) as a Drawing taget, and the previous framebuffer as the read target. We also need to specify which colour attachment to read from which is done using glReadBuffer (use keys 1-8 to change the attachment). 

Finally we blit from one to the other.

```
glBindFramebuffer(GL_DRAW_FRAMEBUFFER,defaultFramebufferObject());
glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
glReadBuffer(GL_COLOR_ATTACHMENT0+m_bufferIndex);
glBlitFramebuffer(0,0,m_win.width,m_win.height,0,0,m_win.width,m_win.height,GL_COLOR_BUFFER_BIT,GL_NEAREST);
```
