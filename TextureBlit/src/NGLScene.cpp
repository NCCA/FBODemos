#include <QMouseEvent>
#include <QGuiApplication>
#include <algorithm>
#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/SimpleVAO.h>
#include <ngl/VAOFactory.h>
#include <ngl/Texture.h>
#include <iostream>

NGLScene::NGLScene(std::vector<std::string> _files)
{
  setTitle("Blit Framebuffer use 1-8 for full frame, A for all");
  m_files = _files;
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  // clear out our buffers
  glDeleteTextures(m_textures.size(), &m_textures[0]);
  glDeleteFramebuffers(1, &m_fboID);
}

void NGLScene::createTextureObjects()
{
  // create a texture object
  for (auto f : m_files)
  {
    ngl::Texture t(f);
    m_textures.push_back(t.setTextureGL());
  }
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

auto ScreenQuad = "ScreenQuad";
void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  // get some setup stuff
  GLint maxAttach = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
  GLint maxDrawBuf = 0;
  glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuf);
  std::cout << "Max attach " << maxAttach << " Max Draw " << maxDrawBuf << '\n';

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  m_win.width = static_cast<int>(width() * devicePixelRatio());
  m_win.height = static_cast<int>(height() * devicePixelRatio());

  ngl::ShaderLib::loadShader(ScreenQuad, "shaders/ScreenQuadVertex.glsl", "shaders/ScreenQuadFragment.glsl");

  // now create our texture object
  createTextureObjects();
  // now the fbo
  createScreenQuad();
}

void NGLScene::paintGL()
{
  //----------------------------------------------------------------------------------------------------------------------
  // draw to our FBO first
  //----------------------------------------------------------------------------------------------------------------------
  ngl::ShaderLib::use(ScreenQuad);
  glViewport(0, 0, m_win.width, m_win.height);
  ngl::ShaderLib::setUniform("width", m_win.width);
  ngl::ShaderLib::setUniform("height", m_win.height);
  std::string title = fmt::format("{} ", m_files[m_activeTexture]);
  setTitle(title.c_str());

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glBindTexture(GL_TEXTURE_2D, m_textures[m_activeTexture]);
  m_screenQuad->bind();
  m_screenQuad->draw();
  m_screenQuad->unbind();
}

void NGLScene::createScreenQuad()
{
  m_screenQuad = ngl::VAOFactory::createVAO(ngl::simpleVAO, GL_TRIANGLES);
  m_screenQuad->bind();
  std::array<GLfloat, 12> quad = {{-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f}};
  m_screenQuad->setData(ngl::AbstractVAO::VertexData(quad.size() * sizeof(GLfloat), quad[0]));
  m_screenQuad->setVertexAttributePointer(0, 2, GL_FLOAT, 0, 0);
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
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  case Qt::Key_Left:
    m_activeTexture = std::clamp(--m_activeTexture, 0ul, m_files.size() - 1);
    break;

  case Qt::Key_Right:
    m_activeTexture = std::clamp(++m_activeTexture, 0ul, m_files.size() - 1);
    break;

  default:
    break;
  }
  // finally update the GLWindow and re-draw
  update();
}
void NGLScene::timerEvent(QTimerEvent *_event)
{
  NGL_UNUSED(_event);
  update();
}
