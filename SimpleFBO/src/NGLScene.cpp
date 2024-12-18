#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <iostream>

NGLScene::NGLScene()
{
  setTitle("Simple Framebuffer Object Demo");
  m_win.width = width();
  m_win.height = height();
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  // clear out our buffers
  glDeleteTextures(1, &m_textureID);
  glDeleteFramebuffers(1, &m_fboID);
}

const static int TEXTURE_WIDTH = 1024;
const static int TEXTURE_HEIGHT = 1024;

void NGLScene::createTextureObject()
{
  // create a texture object
  glGenTextures(1, &m_textureID);
  // bind it to make it active
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  // set params
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  // glGenerateMipmapEXT(GL_TEXTURE_2D);  // set the data size but just set the buffer to 0 as we will fill it with the FBO
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  // now turn the texture off for now
  glBindTexture(GL_TEXTURE_2D, 0);
}

void NGLScene::createFramebufferObject()
{
  // create a framebuffer object this is deleted in the dtor
  glGenFramebuffers(1, &m_fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  GLuint rboID;
  // create a renderbuffer object to store depth info
  glGenRenderbuffers(1, &rboID);
  glBindRenderbuffer(GL_RENDERBUFFER, rboID);

  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);
  // bind
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // attatch the texture we created earlier to the FBO
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);

  // now attach a renderbuffer to depth attachment point
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
  // now got back to the default render context
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // were finished as we have an attached RB so delete it
  glDeleteRenderbuffers(1, &rboID);
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(2, 2, 2);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  // now load to our new camera
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45.0f, 720.0f / 576.0f, 0.05f, 350.0f);
  // we are creating a shader called Phong
  ngl::ShaderLib::createShaderProgram("Phong");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("PhongVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("PhongFragment", ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("PhongVertex", "shaders/PhongVert.glsl");
  ngl::ShaderLib::loadShaderSource("PhongFragment", "shaders/PhongFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("PhongVertex");
  ngl::ShaderLib::compileShader("PhongFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("Phong", "PhongVertex");
  ngl::ShaderLib::attachShaderToProgram("Phong", "PhongFragment");

  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("Phong");
  // and make it active ready to load values
  ngl::ShaderLib::use("Phong");
  ngl::Vec4 lightPos(-2.0f, 5.0f, 2.0f, 0.0f);
  ngl::ShaderLib::setUniform("light.position", lightPos);
  ngl::ShaderLib::setUniform("light.ambient", 0.0f, 0.0f, 0.0f, 1.0f);
  ngl::ShaderLib::setUniform("light.diffuse", 1.0f, 1.0f, 1.0f, 1.0f);
  ngl::ShaderLib::setUniform("light.specular", 0.8f, 0.8f, 0.8f, 1.0f);
  // gold like phong material
  ngl::ShaderLib::setUniform("material.ambient", 0.274725f, 0.1995f, 0.0745f, 0.0f);
  ngl::ShaderLib::setUniform("material.diffuse", 0.75164f, 0.60648f, 0.22648f, 0.0f);
  ngl::ShaderLib::setUniform("material.specular", 0.628281f, 0.555802f, 0.3666065f, 0.0f);
  ngl::ShaderLib::setUniform("material.shininess", 51.2f);
  ngl::ShaderLib::setUniform("viewerPos", from);

  // now load our texture shader
  ngl::ShaderLib::createShaderProgram("TextureShader");

  ngl::ShaderLib::attachShader("TextureVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("TextureFragment", ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource("TextureVertex", "shaders/TextureVertex.glsl");
  ngl::ShaderLib::loadShaderSource("TextureFragment", "shaders/TextureFragment.glsl");

  ngl::ShaderLib::compileShader("TextureVertex");
  ngl::ShaderLib::compileShader("TextureFragment");
  ngl::ShaderLib::attachShaderToProgram("TextureShader", "TextureVertex");
  ngl::ShaderLib::attachShaderToProgram("TextureShader", "TextureFragment");

  // link the shader no attributes are bound
  ngl::ShaderLib::linkProgramObject("TextureShader");
  ngl::ShaderLib::use("TextureShader");

  // now create our texture object
  createTextureObject();
  // now the fbo
  createFramebufferObject();
  // now create the primitives to draw
  ngl::VAOPrimitives::createTrianglePlane("plane", 2, 2, 20, 20, ngl::Vec3(0, 1, 0));
  ngl::VAOPrimitives::createSphere("sphere", 0.4f, 80);
  startTimer(1);
  GLint maxAttach = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
}

void NGLScene::loadMatricesToShader()
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_transform.getMatrix();
  MV = m_view * M;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MV", MV);
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  ngl::ShaderLib::setUniform("M", M);
}

void NGLScene::paintGL()
{
  //----------------------------------------------------------------------------------------------------------------------
  // draw to our FBO first
  //----------------------------------------------------------------------------------------------------------------------
  // grab an instance of the shader manager
  ngl::ShaderLib::use("Phong");

  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotX * rotY;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  static float rot = 0.0;
  // we are now going to draw to our FBO
  // set the rendering destination to FBO
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  // set the background colour (using blue to show it up)
  glClearColor(0, 0.4f, 0.5f, 1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // set our viewport to the size of the texture
  // if we want a different camera we wouldset this here
  glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
  // rotate the teapot
  m_transform.reset();

  m_transform.setRotation(rot, rot, rot);
  loadMatricesToShader();
  ngl::VAOPrimitives::draw("teapot");
  rot += 0.5;

  //----------------------------------------------------------------------------------------------------------------------
  // now we are going to draw to the normal GL buffer and use the texture created
  // in the previous render to draw to our objects
  //----------------------------------------------------------------------------------------------------------------------
  // first bind the normal render buffer
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  // now enable the texture we just rendered to
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  // do any mipmap generation
  // glGenerateMipmap(GL_TEXTURE_2D);
  // set the screen for a different clear colour
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // clear this screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // get the new shader and set the new viewport size
  ngl::ShaderLib::use("TextureShader");
  // this takes into account retina displays etc
  glViewport(0, 0, static_cast<GLsizei>(width() * devicePixelRatio()), static_cast<GLsizei>(height() * devicePixelRatio()));
  ngl::Mat4 MVP;
  m_transform.reset();
  MVP = m_project * m_view * m_mouseGlobalTX;
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::VAOPrimitives::draw("plane");
  m_transform.setPosition(0, 1, 0);
  MVP = m_project * m_view * m_mouseGlobalTX * m_transform.getMatrix();
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::VAOPrimitives::draw("sphere");
  //----------------------------------------------------------------------------------------------------------------------
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
  // turn on wirframe rendering
  case Qt::Key_W:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  // turn off wire frame
  case Qt::Key_S:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
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
