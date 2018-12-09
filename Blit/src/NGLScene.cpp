#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/SimpleVAO.h>
#include <ngl/VAOFactory.h>
NGLScene::NGLScene()
{
  setTitle("Blit Framebuffer use 1-8 for full frame, A for all");
}

NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  // clear out our buffers
  glDeleteTextures(m_textures.size(),&m_textures[0]);
  glDeleteFramebuffers(1,&m_fboID);
}


void NGLScene::createTextureObjects(GLuint _num)
{
  m_textures.resize(_num);
  // create a texture object
  glGenTextures(_num, &m_textures[0]);
  for(auto t : m_textures)
  {
    glBindTexture(GL_TEXTURE_2D, t);
    //  // set params
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexStorage2D(GL_TEXTURE_2D, 0, GL_RGB, m_win.width, m_win.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_win.width, m_win.height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);

  }
  glBindTexture(GL_TEXTURE_2D, 0);

}

void NGLScene::createFramebufferObject()
{

  // create a framebuffer object this is deleted in the dtor
  glGenFramebuffers(1, &m_fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  int attachment=0;
  for(auto t : m_textures)
  {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+attachment, GL_TEXTURE_2D, t, 0);
    ++attachment;
 }
  // add depth and stencil buffer
  GLuint depthStencil;
  glGenTextures(1,&depthStencil);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, m_win.width, m_win.height, 0,
    GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr
  );

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthStencil, 0);


  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete! \n";

  // now got back to the default render context use defaultFramebufferObject in Qt
  // as it uses it's own buffers internally
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

}



void NGLScene::resizeGL( int _w, int _h )
{
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

auto ScreenQuad="ScreenQuad";
void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  // get some setup stuff
  GLint maxAttach = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
  GLint maxDrawBuf = 0;
  glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
  std::cout<<"Max attach "<<maxAttach<<" Max Draw "<<maxDrawBuf<<'\n';

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  m_win.width  = static_cast<int>( width() * devicePixelRatio() );
  m_win.height = static_cast<int>( height() * devicePixelRatio() );

  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();


  shader->loadShader(ScreenQuad,"shaders/ScreenQuadVertex.glsl","shaders/ScreenQuadFragment.glsl");

  // now create our texture object
  createTextureObjects(maxAttach);
  // now the fbo
  createFramebufferObject();
  createScreenQuad();
}


void NGLScene::paintGL()
{
  //----------------------------------------------------------------------------------------------------------------------
  // draw to our FBO first
  //----------------------------------------------------------------------------------------------------------------------
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(ScreenQuad);
  shader->setUniform("checkSize",40.0f);
  shader->setUniform("width",m_win.width);
  shader->setUniform("height",m_win.height);
  glViewport(0, 0, m_win.width, m_win.height);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Pass 1 render to framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER,m_fboID);
  GLuint drawBuffers[] = { GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3,
                          GL_COLOR_ATTACHMENT4,GL_COLOR_ATTACHMENT5,GL_COLOR_ATTACHMENT6,GL_COLOR_ATTACHMENT7};
  glDrawBuffers (8,drawBuffers);
  m_screenQuad->bind();
  m_screenQuad->draw();
  m_screenQuad->unbind();

  // pass 2 bind default fbo


  glBindFramebuffer(GL_DRAW_FRAMEBUFFER,defaultFramebufferObject());
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID);
  if(m_bufferIndex!=8)
  {
    glReadBuffer(GL_COLOR_ATTACHMENT0+m_bufferIndex);
    glBlitFramebuffer(0,0,m_win.width,m_win.height,0,0,m_win.width,m_win.height,GL_COLOR_BUFFER_BIT,GL_NEAREST);

  }
  else
  {
    auto w4=m_win.width/4;
    auto h2=m_win.height/2;
    for(int i=0; i<8; ++i)
    {
      glReadBuffer(GL_COLOR_ATTACHMENT0+i);
      if(i<4)
      {

        glBlitFramebuffer(w4*i,0,w4*i+w4,h2,
                          w4*i,0,w4*i+w4,h2
              ,GL_COLOR_BUFFER_BIT,GL_NEAREST);

      }
      else
      {

        glBlitFramebuffer(w4*(i-4),h2,w4*(i-4)+w4,m_win.height,
                          w4*(i-4),h2,w4*(i-4)+w4,m_win.height
              ,GL_COLOR_BUFFER_BIT,GL_NEAREST);

      }
    }
  }
 }

void NGLScene::createScreenQuad()
{
  m_screenQuad=ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_TRIANGLES);
  m_screenQuad->bind();
  std::array<GLfloat,12> quad ={{-1.0f,1.0f,-1.0f,-1.0f,1.0f,-1.0f,-1.0f,1.0f,1.0f,-1.0f,1.0f, 1.0f}};
  m_screenQuad->setData(ngl::AbstractVAO::VertexData(quad.size()*sizeof(GLfloat),quad[0]));
  m_screenQuad->setVertexAttributePointer(0,2,GL_FLOAT,0,0);
  m_screenQuad->setNumIndices(quad.size());
  m_screenQuad->unbind();
}


//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape : QGuiApplication::exit(EXIT_SUCCESS); break;
  case Qt::Key_1 : m_bufferIndex=0; break;
  case Qt::Key_2 : m_bufferIndex=1; break;
  case Qt::Key_3 : m_bufferIndex=2; break;
  case Qt::Key_4 : m_bufferIndex=3; break;
  case Qt::Key_5 : m_bufferIndex=4; break;
  case Qt::Key_6 : m_bufferIndex=5; break;
  case Qt::Key_7 : m_bufferIndex=6; break;
  case Qt::Key_8 : m_bufferIndex=7; break;
  case Qt::Key_A : m_bufferIndex=8; break;
  default : break;
  }
  // finally update the GLWindow and re-draw
    update();
}
void NGLScene::timerEvent(QTimerEvent *_event)
{
  NGL_UNUSED(_event);
  update();
}

