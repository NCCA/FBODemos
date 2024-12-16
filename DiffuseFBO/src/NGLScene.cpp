#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Util.h>
#include <ngl/Random.h>
#include <QGuiApplication>
#include <QMouseEvent>

constexpr size_t TextureWidth=1024;
constexpr size_t TextureHeight=1024;

NGLScene::NGLScene()
{
  setTitle("Render to Screen Triangle");
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{

  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
  m_projection = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.1f, 200.0f);

}
constexpr auto ScreenTri = "ScreenTri";
constexpr auto GeometryPass = "GeometryPass";

void NGLScene::initializeGL()
{
  ngl::NGLInit::initialize();
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  ngl::ShaderLib::loadShader(ScreenTri,"shaders/ScreenTriVertex.glsl","shaders/ScreenTriFragment.glsl");
  ngl::ShaderLib::use(ScreenTri);
  ngl::ShaderLib::setUniform("lightColour",1.0f,1.0f,1.0f);
  ngl::ShaderLib::setUniform("lightPos",m_lightPos);
  // Need a vertex array to call draw arrays
  // this will have no buffers
  glGenVertexArrays(1,&m_vao);
  // Now generate a texture for our final image
  glGenTextures(1, &m_textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TextureWidth, TextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

  generateFBO();

  ngl::ShaderLib::loadShader(GeometryPass,"shaders/GeometryPassVertex.glsl","shaders/GeometryPassFragment.glsl");
  ngl::ShaderLib::use(GeometryPass);
  ngl::ShaderLib::setUniform("colour",1.0f,0.0f,0.0f);

  // We now create our view matrix for a static camera
  ngl::Vec3 from{0.0f, 2.0f, 2.0f};
  ngl::Vec3 to{0.0f, 0.0f, 0.0f};
  ngl::Vec3 up{0.0f, 1.0f, 0.0f};
  // now load to our new camera
  m_view = ngl::lookAt(from, to, up);
}


void NGLScene::generateFBO()
{
  // Generate framebuffer and textures
    glGenFramebuffers(1, &m_fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

    glGenTextures(1, &m_position);
    glBindTexture(GL_TEXTURE_2D, m_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, TextureWidth, TextureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_position, 0);

    glGenTextures(1, &m_normal);
    glBindTexture(GL_TEXTURE_2D, m_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, TextureWidth, TextureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normal, 0);

    glGenTextures(1, &m_albedo);
    glBindTexture(GL_TEXTURE_2D, m_albedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TextureWidth, TextureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_albedo, 0);

    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TextureWidth, TextureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!\n";
        exit(EXIT_FAILURE);
        
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void NGLScene::paintGL()
{

  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);

  // multiply the rotations
  m_mouseGlobalTX = rotX * rotY;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  // render pass 1 geometry pass
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  glViewport(0, 0,TextureWidth,TextureHeight);
  // clear the screen and depth buffer
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw a teapot
  ngl::ShaderLib::use(GeometryPass);
  ngl::ShaderLib::setUniform("MVP", m_projection * m_view * m_mouseGlobalTX);
  auto M = m_view * m_mouseGlobalTX;
  ngl::Mat3 normalMatrix = M;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
  ngl::ShaderLib::setUniform("model",M);    
  ngl::VAOPrimitives::draw("teapot");

  // now bind back to the default framebuffer and draw 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0,m_win.width,m_win.height);
  // clear the screen and depth buffer
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // grab an instance of the shader manager
  ngl::ShaderLib::use(ScreenTri);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_position);
  ngl::ShaderLib::setUniform("position",0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_normal);
  ngl::ShaderLib::setUniform("normal",1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_albedo);
  ngl::ShaderLib::setUniform("albedo",2);
  ngl::ShaderLib::setUniform("lightPos",m_lightPos);
  // Draw screen Tri with bound Texture
  glBindVertexArray(m_vao);
  glDrawArrays(GL_TRIANGLES,0,3);
  glBindVertexArray(0);

}

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quit
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
    case Qt::Key_Left:
    m_lightPos.m_x -= 0.5;
    break;
  case Qt::Key_Right:
    m_lightPos.m_x += 0.5;
    break;
  case Qt::Key_Up:
    m_lightPos.m_y += 0.5;
    break;
  case Qt::Key_Down:
    m_lightPos.m_y -= 0.5;
    break;
  case Qt::Key_I:
    m_lightPos.m_z -= 0.5;
    break;
  case Qt::Key_O:
    m_lightPos.m_z += 0.5;
    break;

  default:
    break;
  }
  update();
}



void NGLScene::timerEvent(QTimerEvent *)
{
  if(!m_animate) return;
update();
}