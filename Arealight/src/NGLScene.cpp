#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Transformation.h>
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
  ngl::ShaderLib::setUniform("lightColour",5.0f,5.0f,5.0f);
  ngl::ShaderLib::setUniform("lightPos",m_lightPos);
  ngl::ShaderLib::setUniform("viewPos",ngl::Vec3(0.0f,2.0f,2.0f));
  // Need a vertex array to call draw arrays
  // this will have no buffers
  glGenVertexArrays(1,&m_vao);
  // generate the frame buffer object.
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
  ngl::VAOPrimitives::createTrianglePlane("light",1.0f,1.0f,1,1,ngl::Vec3(0.0f,1.0f,0.0f));
  ngl::VAOPrimitives::createTrianglePlane("floor",20.0f,20.0f,100,100,ngl::Vec3(0.0f,1.0f,0.0f));
}


void NGLScene::generateFBO()
{
    // 1. Generates and binds the framebuffer.
    glGenFramebuffers(1, &m_fboID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
    // 2. Creates and configures three textures for position, normal, and albedo data.
    // note could use an array here for the id's
    glGenTextures(1, &m_position);
    glBindTexture(GL_TEXTURE_2D, m_position);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, TextureWidth, TextureHeight, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // 4 Attaches the textures to the framebuffer.
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

   // 5. Attaches the renderbuffer to the framebuffer for depth
    GLuint rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TextureWidth, TextureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
  // 6. Specifies the draw buffers for the framebuffer.
    GLenum attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);
  // check it all worked and exit if not.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!\n";
        exit(EXIT_FAILURE);
        
    }
  // 8. Unbinds the framebuffer and activates the default framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("Colour",1.0,0.0,0.0,1.0);

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
  ngl::ShaderLib::setUniform("colour",1.0f,0.0f,0.0f);

  ngl::ShaderLib::setUniform("MVP", m_projection * m_view * m_mouseGlobalTX);
  ngl::Transformation tx;
  auto M = m_view * m_mouseGlobalTX;
  ngl::Mat3 normalMatrix = M;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("normalMatrix",normalMatrix);
  ngl::ShaderLib::setUniform("model",M);    
  ngl::VAOPrimitives::draw("teapot");
  
  tx.setPosition(0,-0.5,0);
  ngl::ShaderLib::setUniform("MVP", m_projection * m_view * m_mouseGlobalTX*tx.getMatrix());
  ngl::ShaderLib::setUniform("colour",0.8f,0.8f,0.8f);
  ngl::VAOPrimitives::draw("floor");

  // draw light
  ngl::ShaderLib::use(ngl::nglColourShader);
  tx.reset();
  tx.setPosition(m_lightPos);
  tx.setScale(m_lightScaleU,m_lightScaleV,1.0f);
  tx.setRotation(90.0f,0.0f,0.0f);
  ngl::ShaderLib::setUniform("MVP", m_projection * m_view *m_mouseGlobalTX* tx.getMatrix());
  //ngl::VAOPrimitives::draw("light");

  // now bind back to the default framebuffer and draw 
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0,m_win.width,m_win.height);
  // clear the screen and depth buffer
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // grab an instance of the shader manager
  ngl::ShaderLib::use(ScreenTri);
  ngl::ShaderLib::setUniform("displayMode",static_cast<int>(m_displayMode));  
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_position);
  ngl::ShaderLib::setUniform("position",0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_normal);
  ngl::ShaderLib::setUniform("normal",1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_albedo);
  ngl::ShaderLib::setUniform("albedo",2);
  ngl::Vec3 lightU(m_lightScaleU,0.0f,0.0f);
  ngl::Vec3 lightV(0.0f,m_lightScaleV,0.0f);

  ngl::Transformation lightTx;
  lightTx.setRotation(m_lightRotation.m_x,m_lightRotation.m_y,m_lightRotation.m_z);

  ngl::ShaderLib::setUniform("lightPos",m_lightPos);
  ngl::ShaderLib::setUniform("lightU",lightU*lightTx.getMatrix());
  ngl::ShaderLib::setUniform("lightV",lightV*lightTx.getMatrix());
  ngl::ShaderLib::setUniform("lightIntensity",10.0f,10.0f,10.0f);
  ngl::ShaderLib::setUniform("specularPower",12.0f);
  ngl::ShaderLib::setUniform("specularStrength",0.8f);
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
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
    case Qt::Key_Left:
    m_lightPos.m_x -= 0.1;
    break;
  case Qt::Key_Right:
    m_lightPos.m_x += 0.1;
    break;
  case Qt::Key_Up:
    m_lightPos.m_y += 0.1;
    break;
  case Qt::Key_Down:
    m_lightPos.m_y -= 0.1;
    break;
  case Qt::Key_I:
    m_lightPos.m_z -= 0.1;
    break;
  case Qt::Key_O:
    m_lightPos.m_z += 0.1;
    break;

  case Qt::Key_1 :
    m_displayMode = DisplayModes::Shaded;
    break;
 case Qt::Key_2 :
    m_displayMode = DisplayModes::Position;
    break;
 case Qt::Key_3 :
    m_displayMode = DisplayModes::Normal;
    break;
 case Qt::Key_4 :
    m_displayMode = DisplayModes::Albedo;
    break;

  case Qt::Key_5 : m_lightRotation.m_x+=1.0f; break;
  case Qt::Key_6 : m_lightRotation.m_x-=1.0f; break;
  case Qt::Key_7 : m_lightRotation.m_y+=1.0f; break;
  case Qt::Key_8 : m_lightRotation.m_y-=1.0f; break;
  case Qt::Key_9 : m_lightRotation.m_z+=1.0f; break;
  case Qt::Key_0 : m_lightRotation.m_z-=1.0f; break;


  case Qt::Key_A : m_lightScaleU+=0.1f; break;
  case Qt::Key_Z : m_lightScaleU-=0.1f; break;
  case Qt::Key_S : m_lightScaleV+=0.1f; break;
  case Qt::Key_X : m_lightScaleV-=0.1f; break;

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