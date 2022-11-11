#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/Camera.h>
#include <ngl/Light.h>
#include <ngl/Material.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <array>
#include <iostream>

NGLScene::NGLScene()
{
  setTitle("Point Light Shadows with PCF");
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_cam.setShape(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

constexpr float znear = 1.0f;
constexpr float zfar = 25.0f;

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();
  m_pointLight.reset(new PointLightShadow(m_lightPosition, ngl::Vec3(1.0f, 1.0f, 1.0f), 1024 * 4, 1024 * 4));
  m_pointLight->setNearFar(znear, zfar);
  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();

  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0, 2, 6);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  // now load to our new camera
  m_cam.set(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape(45, 720.0f / 576.0f, znear, zfar);

  // in this case I'm only using the light to hold the position
  // it is not passed to the shader directly
  m_lightAngle = 0.0f;

  shader->loadShader("Texture", "shaders/TextureVert.glsl", "shaders/TextureFrag.glsl");
  // we are creating a shader called Shadow
  shader->loadShader("Shadow", "shaders/ShadowVert.glsl", "shaders/ShadowFrag.glsl");
  shader->use("Shadow");
  shader->setUniform("far_plane", zfar);
  // create the primitives to draw
  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();
  prim->createSphere("sphere", 0.5, 50);
  prim->createTorus("torus", 0.15f, 0.4f, 40, 40);

  prim->createTrianglePlane("plane", 14, 14, 80, 80, ngl::Vec3(0, 1, 0));
  // we need to enable depth testing
  glEnable(GL_DEPTH_TEST);
  // set the depth comparison mode
  glDepthFunc(GL_LEQUAL);
  // set the bg to black
  glClearColor(0, 0, 0, 1.0f);
  m_lightTimer = startTimer(40);

  shader->use(ngl::nglColourShader);
  shader->setUniform("Colour", 1.0f, 0.0f, 0.0f, 1.0f);
}

void NGLScene::loadMatricesToShadowShader()
{
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->use("Shadow");
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_mouseGlobalTX * m_transform.getMatrix();
  MV = m_cam.getViewMatrix() * M;
  MVP = m_cam.getVPMatrix() * M;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("lightPos", m_pointLight->pos());

  shader->setUniform("MVP", MVP);
  shader->setUniform("model", m_transform.getMatrix());
  shader->setUniform("viewPos", m_cam.getEye().toVec3());
  shader->setUniform("normalMatrix", normalMatrix);
}

void NGLScene::loadToPointDepthShader()
{
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();
  shader->use(m_pointLight->c_shaderName);
  shader->setUniform("M", m_transform.getMatrix());
  shader->setUniform("lightPos", m_pointLight->pos());
  shader->setUniform("far_plane", zfar);
}

void NGLScene::drawScene(std::function<void()> _shaderFunc)
{
  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_win.spinXFace);
  rotY.rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();

  m_transform.reset();
  m_transform.setScale(0.1f, 0.1f, 0.1f);
  m_transform.setPosition(0.0f, -0.5f, 0.0f);
  _shaderFunc();
  prim->draw("dragon");
  m_transform.reset();
  m_transform.setPosition(-3.0f, 0.0f, 0.0f);
  _shaderFunc();
  prim->draw("sphere");
  m_transform.reset();
  m_transform.setPosition(3.0f, 0.0f, 0.0f);
  _shaderFunc();
  prim->draw("cube");
  m_transform.reset();
  m_transform.setPosition(0.0f, 0.0f, 2.0f);
  _shaderFunc();
  prim->draw("teapot");
  m_transform.reset();
  m_transform.setScale(0.1f, 0.1f, 0.1f);
  m_transform.setPosition(0.0f, -0.5f, -2.0f);
  _shaderFunc();
  prim->draw("buddah");
  m_transform.reset();
  m_transform.setPosition(2.0f, 0.0f, -2.0f);
  _shaderFunc();
  prim->draw("torus");
  // floor
  m_transform.reset();
  m_transform.setPosition(0.0f, -0.5f, 0.0f);
  _shaderFunc();
  prim->draw("plane");
  // back wall
  m_transform.reset();
  m_transform.setPosition(0.0f, 6.5, -7.0f);
  m_transform.setRotation(90.0f, 0.f, 0.0f);
  _shaderFunc();
  prim->draw("plane");
  // left wall
  m_transform.reset();
  m_transform.setPosition(-7.0f, 6.5, 0.0f);
  m_transform.setRotation(0.0f, 0.f, -90.0f);
  _shaderFunc();
  prim->draw("plane");
  // right wall
  m_transform.reset();
  m_transform.setPosition(7.0f, 6.5, 0.0f);
  m_transform.setRotation(0.0f, 0.f, 90.0f);
  _shaderFunc();
  prim->draw("plane");
  m_transform.reset();
}

void NGLScene::paintGL()
{
  //----------------------------------------------------------------------------------------------------------------------
  // Pass 1 render our Depth texture to the FBO
  //----------------------------------------------------------------------------------------------------------------------
  // enable culling
  glEnable(GL_CULL_FACE);
  // Clear previous frame values
  glClear(GL_DEPTH_BUFFER_BIT);
  // as we are only rendering depth turn off the colour / alpha
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  m_pointLight->bindForRender();
  // render only the back faces so we don't get too much self shadowing
  glCullFace(GL_FRONT);
  drawScene(std::bind(&NGLScene::loadToPointDepthShader, this));
  m_pointLight->unbind();

  //----------------------------------------------------------------------------------------------------------------------
  // Pass two use the texture
  // now we have created the texture for shadows render the scene properly
  //----------------------------------------------------------------------------------------------------------------------
  // go back to our normal framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  // set the viewport to the screen dimensions
  glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
  // enable colour rendering again (as we turned it off earlier)
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  // clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // bind the shadow texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_pointLight->getCubeMapID());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  // now only cull back faces
  glCullFace(GL_BACK);
  // render our scene with the shadow shader
  drawScene(std::bind(&NGLScene::loadMatricesToShadowShader, this));
  //----------------------------------------------------------------------------------------------------------------------
  // now we draw a cube to visualise the light
  //----------------------------------------------------------------------------------------------------------------------
  ngl::VAOPrimitives *prim = ngl::VAOPrimitives::instance();
  ngl::ShaderLib *shader = ngl::ShaderLib::instance();

  shader->use(ngl::nglColourShader);
  m_transform.reset();
  m_transform.setPosition(m_pointLight->pos());
  ngl::Mat4 MVP = m_cam.getVPMatrix() * m_mouseGlobalTX * m_transform.getMatrix();
  shader->setUniform("MVP", MVP);
  prim->draw("cube");
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
  case Qt::Key_Space:
    toggleAnimation();
    break;
  case Qt::Key_Left:
    changeLightXOffset(-0.1f);
    break;
  case Qt::Key_Right:
    changeLightXOffset(0.1f);
    break;
  case Qt::Key_Up:
    changeLightYPos(0.1f);
    break;
  case Qt::Key_Down:
    changeLightYPos(-0.1f);
    break;
  case Qt::Key_I:
    changeLightZOffset(-0.1f);
    break;
  case Qt::Key_O:
    changeLightZOffset(0.1f);
    break;

  default:
    break;
  }
  update();
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::updateLight()
{
  // change the light angle
  m_lightAngle += 0.05;
  m_lightPosition.set(m_lightXoffset * cos(m_lightAngle), m_lightYPos, m_lightXoffset * sin(m_lightAngle));
  // now set this value and load to the shader
  m_pointLight->setPos(m_lightPosition);
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  // if the timer is the light timer call the update light method
  if (_event->timerId() == m_lightTimer && m_animate == true)
  {
    updateLight();
  }
  // re-draw GL
  update();
}
