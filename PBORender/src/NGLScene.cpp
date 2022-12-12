#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/Random.h>
#include <iostream>
NGLScene::NGLScene()
{
  setTitle("PBORender");
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  // clear out our buffers
  glDeleteTextures(1, &m_texture);
}

void NGLScene::createTextureObject(int _width, int _height)
{
  // create a texture object for the FBO
  glGenTextures(1, &m_texture);
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  // Now generate the PixelBufferObject for mapping later
  glGenBuffers(1, &m_pbo);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, 1024*1024*3, 0, GL_STREAM_DRAW);
  // Now bind defaults 
  glBindTexture(GL_TEXTURE_2D,0);
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  std::cout<<"Texture created ID" <<m_texture<<" PBO "<<m_pbo<<"\n";
}

void NGLScene::createFramebufferObject()
{
  // create a framebuffer object 
  glGenFramebuffers(1, &m_fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  // This has one attachment which is the texture we are going to blit to the screen.
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D, m_texture, 0);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! \n";
    std::exit(EXIT_FAILURE);
  }
  

  // now got back to the default render context use defaultFramebufferObject in Qt
  // as it uses it's own buffers internally
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  std::cout<<"FBO created ID "<<m_fboID<<'\n';

}

void NGLScene::resizeGL(int _w, int _h)
{
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
  // glBindTexture(GL_TEXTURE_2D, m_texture);
  // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_win.width, m_win.height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  m_win.width = static_cast<int>(width() * devicePixelRatio());
  m_win.height = static_cast<int>(height() * devicePixelRatio());

  // now create our texture object
  createTextureObject(m_win.width,m_win.height);
  // now the fbo
  createFramebufferObject();
  // Add generator for the colours
  auto dist=std::uniform_int_distribution<>(0,255);
  ngl::Random::addIntGenerator("colour",dist);
  startTimer(0);
}

void NGLScene::paintGL()
{
  glViewport(0, 0, m_win.width, m_win.height);
  glClear(GL_COLOR_BUFFER_BIT );
  // Bind the default FBO to draw to
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
  // Bind the texture to read from
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
  // Now blit to screen note source and destination must be the same size
  glBlitFramebuffer(0, 0, m_win.width, m_win.height, 0, 0, m_win.width, m_win.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}


void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  default:
    break;
  }
  // finally update the GLWindow and re-draw
  update();
}
void NGLScene::timerEvent(QTimerEvent *_event)
{
  static size_t pixel=0;
  // grab a random colour
  unsigned char r=static_cast<unsigned char>(ngl::Random::getIntFromGeneratorName("colour"));
  unsigned char g=static_cast<unsigned char>(ngl::Random::getIntFromGeneratorName("colour"));
  unsigned char b=static_cast<unsigned char>(ngl::Random::getIntFromGeneratorName("colour"));
  // bind the texture buffer in the FBO
  glBindTexture(GL_TEXTURE_2D, m_texture);
  // bind the pixel buffer so we can map to this texuture
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbo);
  glBufferData(GL_PIXEL_UNPACK_BUFFER, m_win.width* m_win.height*3, 0, GL_STREAM_DRAW);
  // Now grab a pointer into this buffer so we can write to it
  GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
  if(ptr)
  {
    // loop for the single scanline and set the colour
    for(int i=0; i<m_win.width; ++i)
    {
      ptr[pixel]=r;
      ptr[pixel+1]=g;
      ptr[pixel+2]=b;
      pixel+=3;
    }
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER); // release the mapped buffer
  }
  // if we are at top move to bottom
  if(pixel >=m_win.width* m_win.height*3)
  {
    pixel=0;
  }
  // We need to call this to let copy the new data into the buffer
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_win.width, m_win.height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  // bind back to default buffers
  glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  update();
}
