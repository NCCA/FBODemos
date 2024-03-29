#include <QColorDialog>
#include "NGLScene.h"
#include <iostream>
#include <ngl/Mat4.h>
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/MultiBufferVAO.h>
#include <ngl/ShaderLib.h>
#include <array>
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
// in this ctor we need to call the CreateCoreGLContext class, this is mainly for the MacOS Lion version as
// we need to init the OpenGL 3.2 sub-system which is different than other platforms
//----------------------------------------------------------------------------------------------------------------------
NGLScene::NGLScene(QWidget *_parent) : QOpenGLWidget(_parent)

{

  // set this widget to have the initial keyboard focus
  setFocus();
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  this->resize(_parent->size());
  m_animate = true;
  m_lightPosition.set(8, 4, 8);
  m_lightYPos = 4.0f;
  m_lightXoffset = 8.0f;
  m_lightZoffset = 8.0f;
  m_drawDebugQuad = true;
  m_textureSize = 512;
  m_zNear = 0.5;
  ;
  m_zfar = 20.0f;
  m_fov = 45.0f;
  m_polyOffsetFactor = 0.0f;
  m_polyOffsetScale = 0.0f;
  m_textureMinFilter = GL_NEAREST;
  m_textureMagFilter = GL_NEAREST;
  m_colour.set(1.0f, 1.0f, 1.0f);
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  glDeleteTextures(1, &m_textureID);
  glDeleteFramebuffers(1, &m_fboID);
}

//----------------------------------------------------------------------------------------------------------------------
// This virtual function is called once before the first call to paintGL() or resizeGL(),
// and then once whenever the widget has been assigned a new QGLContext.
// This function should set up any required OpenGL context rendering flags, defining VBOs etc.
//----------------------------------------------------------------------------------------------------------------------
void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);

  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0.0f, 2.0f, 6.0f);
  ngl::Vec3 to(0.0f, 0.0f, 0.0f);
  ngl::Vec3 up(0.0f, 1.0f, 0.0f);
  // now load to our new camera
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45.0f, 720.0f / 576.0f, m_zNear, m_zfar);

  // now load to our light POV camera

  m_lightCameraView = ngl::lookAt(m_lightPosition, to, up);
  // here we set the light POV camera shape, the aspect is 1 as our
  // texture is square.
  // use the same clip plane as before but set the FOV a bit larger
  // to get slightly better shadows and the clip planes will help
  // to get rid of some of the artefacts
  m_lightCameraProject = ngl::perspective(m_fov, 1.0f, m_zNear, m_zfar);

  // in this case I'm only using the light to hold the position
  // it is not passed to the shader directly
  m_lightAngle = 0.0f;

  // we are creating a shader called Texture
  ngl::ShaderLib::createShaderProgram("Texture");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("TextureVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("TextureFragment", ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("TextureVertex", "shaders/TextureVert.glsl");
  ngl::ShaderLib::loadShaderSource("TextureFragment", "shaders/TextureFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("TextureVertex");
  ngl::ShaderLib::compileShader("TextureFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("Texture", "TextureVertex");
  ngl::ShaderLib::attachShaderToProgram("Texture", "TextureFragment");
  // now bind the shader attributes for most NGL primitives we use the following
  // layout attribute 0 is the vertex data (x,y,z)
  ngl::ShaderLib::bindAttribute("Texture", 0, "inVert");

  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("Texture");
  ngl::ShaderLib::use("Texture");
  ngl::ShaderLib::autoRegisterUniforms("Texture");
  // we are creating a shader called Colour
  ngl::ShaderLib::createShaderProgram("Colour");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("ColourVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("ColourFragment", ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("ColourVertex", "shaders/ColourVert.glsl");
  ngl::ShaderLib::loadShaderSource("ColourFragment", "shaders/ColourFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("ColourVertex");
  ngl::ShaderLib::compileShader("ColourFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("Colour", "ColourVertex");
  ngl::ShaderLib::attachShaderToProgram("Colour", "ColourFragment");

  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("Colour");
  ngl::ShaderLib::use("Colour");
  ngl::ShaderLib::setUniform("Colour", 1.0f, 0.0f, 0.0f, 1.0f);
  ngl::ShaderLib::autoRegisterUniforms("Colour");
  // we are creating a shader called Shadow
  ngl::ShaderLib::createShaderProgram("Shadow");
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader("ShadowVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("ShadowFragment", ngl::ShaderType::FRAGMENT);
  // attach the source
  ngl::ShaderLib::loadShaderSource("ShadowVertex", "shaders/ShadowVert.glsl");
  ngl::ShaderLib::loadShaderSource("ShadowFragment", "shaders/ShadowFrag.glsl");
  // compile the shaders
  ngl::ShaderLib::compileShader("ShadowVertex");
  ngl::ShaderLib::compileShader("ShadowFragment");
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram("Shadow", "ShadowVertex");
  ngl::ShaderLib::attachShaderToProgram("Shadow", "ShadowFragment");

  // now we have associated this data we can link the shader
  ngl::ShaderLib::linkProgramObject("Shadow");
  ngl::ShaderLib::use("Shadow");
  ngl::ShaderLib::setUniform("ShadowMap", 0);
  // create the primitives to draw
  ngl::VAOPrimitives::createSphere("sphere", 0.5, 50);
  ngl::VAOPrimitives::createTorus("torus", 0.15f, 0.4f, 40, 40);

  ngl::VAOPrimitives::createTrianglePlane("plane", 14, 14, 80, 80, ngl::Vec3(0, 1, 0));
  // now create our FBO and texture
  createFramebufferObject();
  // we need to enable depth testing
  glEnable(GL_DEPTH_TEST);
  // set the depth comparison mode
  glDepthFunc(GL_LEQUAL);
  // set the bg to black
  glClearColor(0, 0, 0, 1.0f);
  // enable face culling this will be switch to front and back when
  // rendering shadow or scene
  glEnable(GL_CULL_FACE);
  m_lightTimer = startTimer(40);
}

//----------------------------------------------------------------------------------------------------------------------
// This virtual function is called whenever the widget has been resized.
// The new size is passed in width and height.
void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, static_cast<float>(_w) / _h, 0.05f, 350.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}
//----------------------------------------------------------------------------------------------------------------------
// This virtual function is called whenever the widget needs to be painted.
// this is our main drawing routine

void NGLScene::paintGL()
{
  //----------------------------------------------------------------------------------------------------------------------
  // Pass 1 render our Depth texture to the FBO
  //----------------------------------------------------------------------------------------------------------------------
  // enable culling
  glEnable(GL_CULL_FACE);

  // bind the FBO and render offscreen to the texture
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  // bind the texture object to 0 (off )
  glBindTexture(GL_TEXTURE_2D, 0);
  // we need to render to the same size as the texture to avoid
  // distortions
  glViewport(0, 0, m_textureSize, m_textureSize);

  // Clear previous frame values
  glClear(GL_DEPTH_BUFFER_BIT);
  // as we are only rendering depth turn off the colour / alpha
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  // render only the back faces so we don't get too much self shadowing
  glCullFace(GL_FRONT);
  glEnable(GL_POLYGON_OFFSET_FILL);
  glPolygonOffset(m_polyOffsetFactor, m_polyOffsetScale);
  // set the shape of the camera before rendering
  m_lightCameraProject = ngl::perspective(m_fov, 1.0f, m_zNear, m_zfar);
  // draw the scene from the POV of the light
  drawScene(std::bind(&NGLScene::loadToLightPOVShader, this));
  //----------------------------------------------------------------------------------------------------------------------
  // Pass two use the texture
  // now we have created the texture for shadows render the scene properly
  //----------------------------------------------------------------------------------------------------------------------
  // go back to our normal framebuffer note as Qt uses FBO's internally the id
  // is not 0 (as we would usually use) so get the actual FBO value with
  // QOpenGLWidget::defaultFramebufferObject()
  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  // set the viewport to the screen dimensions
  glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
  // enable colour rendering again (as we turned it off earlier)
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  // clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // bind the shadow texture
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

  // now only cull back faces

  glDisable(GL_CULL_FACE);

  // render our scene with the shadow shader
  drawScene(std::bind(&NGLScene::loadMatricesToShadowShader, this));
  //----------------------------------------------------------------------------------------------------------------------
  // this draws the debug texture on the quad
  //----------------------------------------------------------------------------------------------------------------------
  if (m_drawDebugQuad)
  {
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    debugTexture(-0.6f, -1.0f, 0.6f, 1.0f);
  }
  //----------------------------------------------------------------------------------------------------------------------
  // now we draw a cube to visualise the light
  //----------------------------------------------------------------------------------------------------------------------

  ngl::ShaderLib::use("Colour");

  m_transform.reset();
  m_transform.setPosition(m_lightPosition);
  ngl::Mat4 MVP = m_project * m_view *
                  m_mouseGlobalTX *
                  m_transform.getMatrix();

  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::VAOPrimitives::draw("cube");
}

void NGLScene::createFramebufferObject()
{

  // Try to use a texture depth component
  glGenTextures(1, &m_textureID);
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  // glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_textureMinFilter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_textureMagFilter);
  // glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_textureSize, m_textureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

  glBindTexture(GL_TEXTURE_2D, 0);

  // create our FBO
  glGenFramebuffers(1, &m_fboID);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
  // disable the colour and read buffers as we only want depth
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);

  // attach our texture to the FBO
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_textureID, 0);

  // switch back to window-system-provided framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void NGLScene::loadMatricesToShadowShader()
{
  ngl::ShaderLib::use("Shadow");
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_mouseGlobalTX * m_transform.getMatrix();
  MV = m_view * M;
  MVP = m_project * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MV", MV);
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  ngl::ShaderLib::setUniform("inColour", m_colour.m_r, m_colour.m_g, m_colour.m_b, 1.0f);

  // x = x* 0.5 + 0.5
  // y = y* 0.5 + 0.5
  // z = z* 0.5 + 0.5
  // Moving from unit cube [-1,1] to [0,1]
  auto bias = ngl::Mat4::scale(0.5f, 0.5f, 0.5f);
  // do the translate
  bias.m_m[3][0] = 0.5f;
  bias.m_m[3][1] = 0.5f;
  bias.m_m[3][2] = 0.5f;

  ngl::Mat4 view = m_lightCameraView;
  ngl::Mat4 proj = m_lightCameraProject;
  ngl::Mat4 model = m_transform.getMatrix();

  ngl::Mat4 textureMatrix = bias * proj * view * model;

  ngl::ShaderLib::setUniform("textureMatrix", textureMatrix);
}

void NGLScene::loadToLightPOVShader()
{
  ngl::ShaderLib::use("Colour");
  ngl::Mat4 MVP = m_lightCameraProject * m_lightCameraView * m_transform.getMatrix();
  ngl::ShaderLib::setUniform("MVP", MVP);
}

void NGLScene::drawScene(std::function<void()> _shaderFunc)
{
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_win.spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  m_transform.reset();
  // this is the same as calling
  //  ((*this).*(_shaderFunc))(m_transformStack);
  // but a lot more readable as to the intent
  // see the c++ faq link in header for more details
  m_transform.setScale(0.1f, 0.1f, 0.1f);
  m_transform.setPosition(0, -0.5, 0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("dragon");
  m_transform.reset();
  m_transform.setPosition(-3, 0.0, 0.0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("sphere");

  m_transform.reset();

  m_transform.setPosition(3, 0.0, 0.0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("cube");

  m_transform.reset();
  m_transform.setPosition(0, 0.0, 2.0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("teapot");

  m_transform.reset();
  m_transform.setScale(0.1f, 0.1f, 0.1f);
  m_transform.setPosition(0, -0.5, -2.0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("buddah");

  m_transform.reset();
  m_transform.setPosition(2, 0, -2.0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("torus");

  m_transform.reset();
  m_transform.setPosition(0.0, -0.5, 0.0);
  _shaderFunc();
  ngl::VAOPrimitives::draw("plane");
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::updateLight()
{
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  // if the timer is the light timer call the update light method
  if (_event->timerId() == m_lightTimer && m_animate == true)
  {
    // change the light angle
    m_lightAngle += 0.05f;
  }

  m_lightPosition.set(m_lightXoffset * cosf(m_lightAngle), m_lightYPos, m_lightXoffset * sinf(m_lightAngle));
  // now set this value and load to the shader
  m_lightCameraView = ngl::lookAt(m_lightPosition, ngl::Vec3(0, 0, 0), ngl::Vec3(0, 1, 0));

  // re-draw GL
  update();
}

void NGLScene::debugTexture(float _t, float _b, float _l, float _r)
{
  ngl::ShaderLib::use("Texture");
  glBindTexture(GL_TEXTURE_2D, m_textureID);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

  std::unique_ptr<ngl::AbstractVAO> quad(ngl::VAOFactory::createVAO("multiBufferVAO", GL_TRIANGLES));
  std::array<GLfloat, 18> vert;
  std::array<GLfloat, 12> uv;

  vert[0] = _t;
  vert[1] = _l;
  vert[2] = 0.0;
  vert[3] = _t;
  vert[4] = _r;
  vert[5] = 0.0;
  vert[6] = _b;
  vert[7] = _l;
  vert[8] = 0.0;

  vert[9] = _b;
  vert[10] = _l;
  vert[11] = 0.0;
  vert[12] = _t;
  vert[13] = _r;
  vert[14] = 0.0;
  vert[15] = _b;
  vert[16] = _r;
  vert[17] = 0.0;

  uv[0] = 0.0;
  uv[1] = 1.0;
  uv[2] = 1.0;
  uv[3] = 1.0;
  uv[4] = 0.0;
  uv[5] = 0.0;

  uv[6] = 0.0;
  uv[7] = 0.0;
  uv[8] = 1.0;
  uv[9] = 1.0;
  uv[10] = 1.0;
  uv[11] = 0.0;

  quad->bind();

  quad->setData(ngl::MultiBufferVAO::VertexData(vert.size() * sizeof(GLfloat), vert[0]));
  quad->setVertexAttributePointer(0, 3, GL_FLOAT, 0, 0);
  quad->setData(ngl::MultiBufferVAO::VertexData(uv.size() * sizeof(GLfloat), uv[0]));
  quad->setVertexAttributePointer(1, 2, GL_FLOAT, 0, 0);
  quad->setNumIndices(vert.size());
  quad->draw();
  quad->unbind();
}

void NGLScene::changeTextureSize(int _i)
{
  switch (_i)
  {
  case 0:
    m_textureSize = 64;
    break;
  case 1:
    m_textureSize = 128;
    break;
  case 2:
    m_textureSize = 256;
    break;
  case 3:
    m_textureSize = 512;
    break;
  case 4:
    m_textureSize = 1024;
    break;
  case 5:
    m_textureSize = 2048;
    break;
  }

  glDeleteTextures(1, &m_textureID);
  glDeleteFramebuffers(1, &m_fboID);
  createFramebufferObject();
  update();
}

void NGLScene::changeTextureMinFilter(int _i)
{
  switch (_i)
  {
  case 0:
    m_textureMagFilter = GL_NEAREST;
    break;
  case 1:
    m_textureMagFilter = GL_LINEAR;
    break;
  case 2:
    m_textureMagFilter = GL_NEAREST_MIPMAP_NEAREST;
    break;
  case 3:
    m_textureMagFilter = GL_LINEAR_MIPMAP_NEAREST;
    break;
  case 4:
    m_textureMagFilter = GL_NEAREST_MIPMAP_LINEAR;
    break;
  case 5:
    m_textureMagFilter = GL_LINEAR_MIPMAP_LINEAR;
    break;
  }
  glDeleteTextures(1, &m_textureID);
  glDeleteFramebuffers(1, &m_fboID);
  createFramebufferObject();
  update();
}

void NGLScene::changeTextureMagFilter(int _i)
{

  switch (_i)
  {
  case 0:
    m_textureMagFilter = GL_NEAREST;
    break;
  case 1:
    m_textureMagFilter = GL_LINEAR;
    break;
  }
  glDeleteTextures(1, &m_textureID);
  glDeleteFramebuffers(1, &m_fboID);
  createFramebufferObject();
  update();
}

void NGLScene::setColour()
{
  QColor colour = QColorDialog::getColor();
  if (colour.isValid())
  {
    ngl::ShaderLib::use("Shadow");
    m_colour.m_r = colour.redF();
    m_colour.m_g = colour.greenF();
    m_colour.m_b = colour.blueF();
    update();
  }
}
