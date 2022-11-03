#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/SimpleVAO.h>
#include <ngl/Random.h>
#include <ngl/VAOFactory.h>
#include <ngl/Texture.h>
#include "ScopedBind.h"
#include "TexturePack.h"
#include "imgui.h"
#include "QtImGui.h"
#include "ImGuizmo.h"

NGLScene::NGLScene()
{
  setTitle("Deferred Render");
  m_lights.resize(m_numLights);
  m_timer.start();
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  // clear out our buffers
  glDeleteTextures(1, &m_textureID);
  glDeleteFramebuffers(1, &m_fboID);
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_cam.setProjection(45.0f, static_cast<float>(_w) / _h, 0.05f, 50.0f);
  m_win.width = static_cast<int>(_w * devicePixelRatio());
  m_win.height = static_cast<int>(_h * devicePixelRatio());
}

auto GeometryPassShader = "GeometryPassShader";
auto GeometryPassCheckerShader = "GeometryPassCheckerShader";
auto LightingPassShader = "LightingPassShader";
auto LightingPassFragment = "LightingPassFragment";
auto BloomPassShader = "BloomPassShader";
auto BloomPassFinalShader = "BloomPassFinalShader";
auto DebugShader = "DebugShader";
auto ColourShader = "ColourShader";
auto SSAOPassShader = "SSAOPassShader";
auto SSAOBlurShader = "SSAOBlurShader";
auto DOFShader = "DOFShader";
void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0, 2, 10);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_win.width = static_cast<int>(width() * devicePixelRatio());
  m_win.height = static_cast<int>(height() * devicePixelRatio());

  // now load to our new camera
  m_cam.set(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setProjection(45.0f, 720.0f / 576.0f, 0.05f, 10.0f);
  // load all the shaders from a json file
  bool loaded = ngl::ShaderLib::loadFromJson("shaders/shaders.json");
  if (!loaded)
  {
    std::cerr << "problem loading shaders\n";
    exit(EXIT_FAILURE);
  }

  ngl::VAOPrimitives::createTrianglePlane("floor", 30, 30, 10, 10, ngl::Vec3::up());
  ngl::VAOPrimitives::createSphere("sphere", 1.0f, 20);
  ngl::NGLMessage::addMessage("Creating m_renderFBO");
  FrameBufferObject::setDefaultFBO(defaultFramebufferObject());
  m_renderFBO = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
  m_renderFBO->bind();
  m_renderFBO->addColourAttachment("position", GLAttatchment::_0, GLTextureFormat::RGB, GLTextureInternalFormat::RGB16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);
  m_renderFBO->addColourAttachment("normal", GLAttatchment::_1, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  m_renderFBO->addColourAttachment("albedoMetallic", GLAttatchment::_2, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  m_renderFBO->addColourAttachment("positionViewSpace", GLAttatchment::_3, GLTextureFormat::RGB, GLTextureInternalFormat::RGB16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  m_renderFBO->addDepthBuffer(GLTextureDepthFormats::DEPTH_COMPONENT24,
                              GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                              GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE,
                              true);
  // setup draw buffers whilst still bound
  GLuint attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, attachments);

  if (!m_renderFBO->isComplete())
  {
    ngl::NGLMessage::addWarning("FrameBuffer incomplete");
    m_renderFBO->print();
  }
  m_renderFBO->unbind();

  ngl::NGLMessage::addMessage("Creating m_lightingFBO");
  m_lightingFBO = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
  m_lightingFBO->bind();
  m_lightingFBO->addColourAttachment("fragColour", GLAttatchment::_0, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                     GLTextureDataType::FLOAT,
                                     GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                     GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);
  m_lightingFBO->addColourAttachment("brightness", GLAttatchment::_1, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                     GLTextureDataType::FLOAT,
                                     GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                     GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  m_lightingFBO->addDepthBuffer(GLTextureDepthFormats::DEPTH_COMPONENT24,
                                GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE,
                                true);
  GLuint attachmentsLighting[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachmentsLighting);

  ngl::NGLMessage::addMessage("Creating m_dofTarget");
  m_dofTarget = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
  m_dofTarget->bind();
  m_dofTarget->addColourAttachment("fragColour", GLAttatchment::_0, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  m_dofTarget->addDepthBuffer(GLTextureDepthFormats::DEPTH_COMPONENT24,
                              GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                              GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE,
                              true);
  GLuint attachmentsDOF[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachmentsDOF);

  ngl::NGLMessage::addMessage("Creating m_forwardPass");

  m_forwardPass = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
  m_forwardPass->bind();
  m_forwardPass->addColourAttachment("fragColour", GLAttatchment::_0, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                     GLTextureDataType::FLOAT,
                                     GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                     GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);
  m_forwardPass->addColourAttachment("brightness", GLAttatchment::_1, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                     GLTextureDataType::FLOAT,
                                     GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                     GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  m_forwardPass->addDepthBuffer(GLTextureDepthFormats::DEPTH_COMPONENT24,
                                GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE,
                                true);

  // setup draw buffers whilst still bound
  GLuint forwardPass[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(1, forwardPass);

  if (!m_forwardPass->isComplete())
  {
    ngl::NGLMessage::addWarning("FrameBuffer incomplete");
    m_forwardPass->print();
  }

  m_ssaoPass = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
  m_ssaoPass->bind();
  m_ssaoPass->addColourAttachment("ssao", GLAttatchment::_0, GLTextureFormat::RED, GLTextureInternalFormat::RED,
                                  GLTextureDataType::FLOAT,
                                  GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                                  GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, false);

  // setup draw buffers whilst still bound
  GLuint ssaoPass[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, ssaoPass);

  if (!m_ssaoPass->isComplete())
  {
    ngl::NGLMessage::addWarning("FrameBuffer SSAO incomplete");
    m_ssaoPass->print();
  }
  m_ssaoPass->unbind();

  m_dofPass = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
  m_dofPass->bind();
  m_dofPass->addColourAttachment("blurTarget", GLAttatchment::_0, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                                 GLTextureDataType::FLOAT,
                                 GLTextureMinFilter::LINEAR, GLTextureMagFilter::LINEAR,
                                 GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

  if (!m_dofPass->isComplete())
  {
    ngl::NGLMessage::addWarning("FrameBuffer DOF incomplete");
    m_dofPass->print();
  }
  m_dofPass->unbind();

  ngl::NGLMessage::addMessage("Creating PingPoing");
  for (auto &b : m_pingPongBuffer)
  {
    b = FrameBufferObject::create(1024 * devicePixelRatio(), 720 * devicePixelRatio());
    b->bind();
    b->addColourAttachment("fragColour", GLAttatchment::_0, GLTextureFormat::RGBA, GLTextureInternalFormat::RGBA16F,
                           GLTextureDataType::FLOAT,
                           GLTextureMinFilter::NEAREST, GLTextureMagFilter::NEAREST,
                           GLTextureWrap::CLAMP_TO_EDGE, GLTextureWrap::CLAMP_TO_EDGE, true);

    if (!b->isComplete())
    {
      ngl::NGLMessage::addWarning("FrameBuffer incomplete");
      b->print();
      std::exit(EXIT_FAILURE);
    }
    b->unbind();
  }

  createScreenQuad();

  // now create the primitives to draw
  startTimer(10);
  GLint maxAttach = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
  std::cout << maxAttach << "\n";
  glEnable(GL_DEPTH_TEST);

  // load textures for render pass (only using 1 for all teapots)
  ngl::Texture t;
  t.loadImage("textures/floorNormal.png");
  m_floorNormalTexture = t.setTextureGL();

  // m_specularTextureID=t.setTextureGL();
  createLights();
  m_randomUpdateTimer = startTimer(400);
  createSSAOKernel();
  TexturePack tp;
  tp.loadJSON("textures/textures.json");
  QtImGui::initialize(this);
}

void NGLScene::createSSAOKernel()
{
  for (unsigned int i = 0; i < 64; ++i)
  {
    ngl::Vec3 sample(ngl::Random::randomNumber(), ngl::Random::randomNumber(), ngl::Random::randomPositiveNumber());
    sample.normalize();
    float scale = static_cast<float>(i) / 64.0f;
    scale = ngl::lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;
    m_ssaoKernel.push_back(sample);
  }
  std::vector<ngl::Vec3> ssaoNoise;

  for (unsigned int i = 0; i < 16; i++)
  {
    ngl::Vec3 noise(ngl::Random::randomNumber(), ngl::Random::randomNumber(), 0.0f);
    ssaoNoise.push_back(noise);
  }

  glGenTextures(1, &m_noiseTexture);
  glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  ngl::ShaderLib::use(SSAOPassShader);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));

  // Send kernel + rotation
  for (unsigned int i = 0; i < 64; ++i)
    ngl::ShaderLib::setUniform(fmt::format("samples[{0}]", i), m_ssaoKernel[i]);
  ngl::ShaderLib::use(SSAOBlurShader);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
}

void NGLScene::loadDOFUniforms()
{
  ngl::ShaderLib::use(DOFShader);

  float magnification = m_focalLenght / abs(m_focalDistance - m_focalLenght);
  float blur = m_focalLenght * magnification / m_fstop;
  float ppm = sqrtf(m_win.width * m_win.width + m_win.height * m_win.height) / 35;
  ngl::ShaderLib::setUniform("depthRange", ngl::Vec2(0.1f, 50.0f));
  ngl::ShaderLib::setUniform("blurCoefficient", blur);
  ngl::ShaderLib::setUniform("PPM", ppm);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  ngl::ShaderLib::setUniform("focusDistance", m_focusDistance);
}

void NGLScene::loadMatricesToShader(const ngl::Mat4 &_mouse)
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M = m_transform.getMatrix();
  MV = m_cam.getView() * M;
  MVP = m_cam.getProjection() * MV;
  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
  ngl::ShaderLib::setUniform("M", M);
  ngl::ShaderLib::setUniform("MV", MV);
}

void NGLScene::geometryPass()
{
  ScopedBind<FrameBufferObject> t(m_renderFBO.get());
  m_renderFBO->setViewport();
  ngl::Random::setSeed(1235);
  TexturePack tp;
  static std::string textures[] =
      {
          "copper",
          "greasy",
          "panel",
          "rusty",
          "wood"};
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ngl::ShaderLib::use(GeometryPassShader);
  // if we want a different camera we wouldset this here
  // rotate the teapot
  m_transform.reset();
  static float s_rot = 0.1f;
  for (float z = -14.0f; z < 14.0f; z += 1.8f)
  {
    for (float x = -14.0f; x < 14.0f; x += 1.8f)
    {
      tp.activateTexturePack(textures[static_cast<int>(ngl::Random::randomPositiveNumber(5))]);

      m_transform.setRotation(0, s_rot, 0);
      m_transform.setPosition(x, 0.0f, z);
      loadMatricesToShader(m_mouseGlobalTX);
      ngl::VAOPrimitives::draw("teapot");
    }
  }
  s_rot += 1.0f;
  ngl::ShaderLib::use(GeometryPassCheckerShader);
  ngl::ShaderLib::setUniform("normalMapSampler", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_floorNormalTexture);

  m_transform.reset();
  m_transform.setPosition(0.0f, -0.45f, 0.0f);
  loadMatricesToShader(m_mouseGlobalTX);
  ngl::VAOPrimitives::draw("floor");
}

void NGLScene::lightingPass()
{
  ngl::ShaderLib::use(LightingPassShader);
  m_lightingFBO->bind();

  glClear(/*L_COLOR_BUFFER_BIT |*/ GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);

  int i = 0;
  for (auto l : m_lights)
  {
    ngl::ShaderLib::setUniform(fmt::format("lights[{0}].position", i), l.position);
    ngl::ShaderLib::setUniform(fmt::format("lights[{0}].colour", i), l.colour);
    ngl::ShaderLib::setUniform(fmt::format("lights[{0}].linear", i), l.linear);
    ngl::ShaderLib::setUniform(fmt::format("lights[{0}].quadratic", i), l.quadratic);

    ++i;
  }
  // bind textures for FBO
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("position"));
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("normal"));
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("albedoMetallic"));
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, m_ssaoPass->getTextureID("ssao"));
  ngl::ShaderLib::setUniform("viewPos", m_cam.getEye());
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  ngl::ShaderLib::setUniform("useAO", bool(m_useAO));
  m_screenQuad->bind();
  m_screenQuad->draw();
  m_screenQuad->unbind();
}

void NGLScene::forwardPass()
{
  // copy depth buffer from the deferred render pass to our forward pass
  m_renderFBO->bind(FrameBufferObject::Target::READ);
  m_forwardPass->bind(FrameBufferObject::Target::DRAW);
  glBlitFramebuffer(0, 0, m_win.width, m_win.height, 0, 0, m_win.width, m_win.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  // now the textures
  FrameBufferObject::copyFrameBufferTexture(m_lightingFBO->getTextureID("fragColour"),
                                            m_forwardPass->getTextureID("fragColour"),
                                            m_lightingFBO->width(), m_lightingFBO->height());
  FrameBufferObject::copyFrameBufferTexture(m_lightingFBO->getTextureID("brightness"),
                                            m_forwardPass->getTextureID("brightness"),
                                            m_lightingFBO->width(), m_lightingFBO->height());

  // now bind for writing
  m_forwardPass->bind();
  GLuint attachmentsLighting[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachmentsLighting);

  // glClear(  GL_DEPTH_BUFFER_BIT);
  ngl::ShaderLib::use(ColourShader);
  m_transform.setScale(0.05f, 0.05f, 0.05f);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  for (auto l : m_lights)
  {
    m_transform.setPosition(l.position);
    ngl::ShaderLib::setUniform("colour", ngl::Vec4(l.colour.m_r, l.colour.m_g, l.colour.m_b, 1.0f));
    ngl::Mat4 MVP = m_cam.getVP() * m_mouseGlobalTX * m_transform.getMatrix();
    ngl::ShaderLib::setUniform("MVP", MVP);
    ngl::VAOPrimitives::draw("sphere");
  }
  m_forwardPass->unbind();
}

void NGLScene::bloomBlurPass()
{
  size_t amount = 10;
  ngl::ShaderLib::use(BloomPassShader);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));

  m_screenQuad->bind();
  m_pingPongBuffer[0]->setViewport();
  //  ngl::ShaderLib::setUniform("screenResolution",ngl::Vec2(m_win.width,m_win.height));

  bool firstTime = true;
  bool horizontal = true;
  for (size_t i = 0; i < amount; i++)
  {
    m_pingPongBuffer[horizontal]->bind();
    ngl::ShaderLib::setUniform("horizontal", bool(horizontal));
    ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,
                  firstTime ? m_forwardPass->getTextureID("brightness") : m_pingPongBuffer[!horizontal]->getTextureID("fragColour"));

    firstTime = false;
    m_screenQuad->draw();
    m_pingPongBuffer[horizontal]->unbind();
    horizontal ^= true; //! horizontal;
  }
  m_screenQuad->unbind();
}

void NGLScene::ssaoPass()
{
  ngl::ShaderLib::use(SSAOPassShader);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  ngl::ShaderLib::setUniform("positionSampler", 0);
  ngl::ShaderLib::setUniform("normalSampler", 1);
  ngl::ShaderLib::setUniform("texNoise", 2);
  m_ssaoPass->bind();
  ngl::ShaderLib::setUniform("projection", m_cam.getProjection());

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("positionViewSpace"));
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("normal"));
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_noiseTexture);
  m_screenQuad->bind();
  m_ssaoPass->setViewport();
  m_screenQuad->draw();
  // glClear(GL_COLOR_BUFFER_BIT );
  ngl::ShaderLib::use(SSAOBlurShader);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  ngl::ShaderLib::setUniform("ssaoInput", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_ssaoPass->getTextureID("ssao"));
  m_screenQuad->draw();
  m_screenQuad->unbind();
  // glEnable(GL_DEPTH_TEST);
}

void NGLScene::finalPass()
{
  // going to re-use the ping pong buffer here and use it in the DOF
  m_dofTarget->bind();
  m_screenQuad->bind();

  //  glBindFramebuffer(GL_FRAMEBUFFER,defaultFramebufferObject());
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ngl::ShaderLib::use(BloomPassFinalShader);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_forwardPass->getTextureID("fragColour"));
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_pingPongBuffer[0]->getTextureID("fragColour"));
  ngl::ShaderLib::setUniform("bloom", m_useBloom);
  ngl::ShaderLib::setUniform("exposure", m_exposure);
  ngl::ShaderLib::setUniform("gamma", m_gamma);
  ngl::ShaderLib::setUniform("scene", 0);
  ngl::ShaderLib::setUniform("bloomBlur", 1);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  m_screenQuad->draw();
  m_dofTarget->unbind();

  if (m_useDOF == true)
  {
    // first DOF
    m_dofPass->bind();
    ngl::ShaderLib::use(DOFShader);
    loadDOFUniforms();
    m_dofPass->setViewport();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_dofTarget->getTextureID("fragColour"));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_dofTarget->getDepthTextureID());

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_dofPass->getTextureID("blurTarget"));

    // HORIZONTAL BLUR
    ngl::ShaderLib::setUniform("colourSampler", 0);
    ngl::ShaderLib::setUniform("depthSampler", 1);
    ngl::ShaderLib::setUniform("uTexelOffset", 1.0f, 0.0f);
    m_screenQuad->draw();
    m_dofPass->unbind();

    // Vertical Blur to default FB
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glClear(GL_DEPTH_BUFFER_BIT);
    ngl::ShaderLib::setUniform("uTexelOffset", 0.0f, 1.0f);
    ngl::ShaderLib::setUniform("colourSampler", 2);
    glViewport(0, 0, m_win.width, m_win.height);
    ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
    m_screenQuad->draw();
    m_screenQuad->unbind();
  }
  else
  {
    debugBlit(m_dofTarget->getTextureID("fragColour"));
  }
  // debugBlit(m_dofPass->getTextureID("blurTarget"));
}

void NGLScene::paintGL()
{
  std::chrono::steady_clock::time_point startPaint = std::chrono::steady_clock::now();

  float currentFrame = m_timer.elapsed() * 0.001f;
  m_deltaTime = currentFrame - m_lastFrame;
  m_lastFrame = currentFrame;
  //----------------------------------------------------------------------------------------------------------------------
  // draw to our FBO first
  //----------------------------------------------------------------------------------------------------------------------
  /// first we reset the movement values
  float xDirection = 0.0;
  float yDirection = 0.0;
  // now we loop for each of the pressed keys in the the set
  // and see which ones have been pressed. If they have been pressed
  // we set the movement value to be an incremental value
  foreach (Qt::Key key, m_keysPressed)
  {
    switch (key)
    {
    case Qt::Key_Left:
    {
      yDirection = -1.0f;
      break;
    }
    case Qt::Key_Right:
    {
      yDirection = 1.0f;
      break;
    }
    case Qt::Key_Up:
    {
      xDirection = 1.0f;
      break;
    }
    case Qt::Key_Down:
    {
      xDirection = -1.0f;
      break;
    }
    default:
      break;
    }
  }
  // if the set is non zero size we can update the ship movement
  // then tell openGL to re-draw
  if (m_keysPressed.size() != 0)
  {
    m_cam.move(xDirection, yDirection, m_deltaTime);
  }

  //----------------------------------------------------------------------------------------------------------------------
  // Pass 1
  //----------------------------------------------------------------------------------------------------------------------
  {
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    geometryPass();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    // ngl::NGLMessage::addMessage(fmt::format("Geometry Pass took {0} ms", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
    m_geoPassDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
  }
  //----------------------------------------------------------------------------------------------------------------------
  // if in debug mode draw gbuffer values for 2nd pass
  //----------------------------------------------------------------------------------------------------------------------
  if (m_debugOn == true)
  {
    // blit to the default FBO from the geo pass renderbuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderFBO->getID());
    // bind the colour attachment to read form so we can see all the buffers.
    glReadBuffer(GL_COLOR_ATTACHMENT0 + m_debugAttachment);
    glBlitFramebuffer(0, 0, m_win.width, m_win.height, 0, 0, m_win.width, m_win.height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }
  else // do deferred render
  {
    {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      ssaoPass();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      m_ssaoPassDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
      // ngl::NGLMessage::addMessage(fmt::format("SSAO Pass took {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
    }

    {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      lightingPass();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      // ngl::NGLMessage::addMessage(fmt::format("Lighting Pass took {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
      m_lightingPassDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    }

    {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      forwardPass();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      m_forwardPassDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
      // ngl::NGLMessage::addMessage(fmt::format("Forward Pass took {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
    }
    {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      bloomBlurPass();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      m_bloomBlurPassDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
      // ngl::NGLMessage::addMessage(fmt::format("Bloom blur Pass took {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
    }

    {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      finalPass();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      m_finalPassDuration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
      // ngl::NGLMessage::addMessage(fmt::format("Final Pass took {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()));
    }

    if (m_textureDebug == true)
    {
      debugBlit(m_debugTextureID);
    }
  }
  std::chrono::steady_clock::time_point endPaint = std::chrono::steady_clock::now();
  // ngl::NGLMessage::drawLine();
  // ngl::NGLMessage::addMessage(fmt::format("Total Rendertime {0} uS", std::chrono::duration_cast<std::chrono::microseconds>(endPaint - startPaint).count()));
  // ngl::NGLMessage::drawLine();
  m_totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(endPaint - startPaint).count();
  if (m_showUI == true)
    drawUI();
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

void NGLScene::editLightShader()
{
  m_numLights = std::min(m_numLights, std::max(1, 64));

  auto editString = fmt::format("{0}", m_numLights);
  ngl::ShaderLib::editShader(LightingPassFragment, "@numLights", editString);
  ngl::ShaderLib::compileShader(LightingPassFragment);
  ngl::ShaderLib::linkProgramObject(LightingPassShader);
  ngl::ShaderLib::use(LightingPassShader);

  m_lights.resize(m_numLights);
  ngl::ShaderLib::setUniform("positionSampler", 0);
  ngl::ShaderLib::setUniform("normalSampler", 1);
  ngl::ShaderLib::setUniform("albedoMetallicSampler", 2);
  // ngl::ShaderLib::setUniform("aoSampler",3);
  ngl::ShaderLib::setUniform("ssaoSampler", 4);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  createLights();
  setTitle(QString("Deferred Renderer %0 Lights").arg(m_numLights));
}

void NGLScene::createLights()
{

  //  for(float i=0; i<2.0f*static_cast<float>(M_PI); i+=2.0f*static_cast<float>(M_PI)/m_numl)

  float circleStep = 2.0f * static_cast<float>(M_PI) / m_lights.size();
  float i = 0.0f;
  for (auto &l : m_lights)
  {
    float x = cosf(i) * m_lightRadius;
    float z = sinf(i) * m_lightRadius;
    float y = m_lightYOffset + sinf(i * m_freq);
    l.position.set(x, y, z); //=ngl::Random::getRandomVec3()*5.0f;
    l.colour = ngl::Random::getRandomColour3() * m_lightMaxIntensity;
    l.colour.clamp(1.0f, m_lightMaxIntensity);
    l.linear = ngl::Random::randomPositiveNumber(0.5f) + 0.2f;
    l.quadratic = ngl::Random::randomNumber(1.0f) + 1.0f;
    i += circleStep;
  }
}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // add to our keypress set the values of any keys pressed
  m_keysPressed += static_cast<Qt::Key>(_event->key());

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
  case Qt::Key_N:
    showNormal();
    break;
  case Qt::Key_0:
    m_showLights ^= true;
    break;
  case Qt::Key_1:
    m_debugAttachment = 0;
    break;
  case Qt::Key_2:
    m_debugAttachment = 1;
    break;
  case Qt::Key_3:
    m_debugAttachment = 2;
    break;
  case Qt::Key_D:
    m_debugOn ^= true;
    break;
  case Qt::Key_F:
    m_lightRadius -= 0.2;
    break;
  case Qt::Key_H:
    m_lightRadius += 0.2;
    break;
  case Qt::Key_T:
    m_lightYOffset += 0.2;
    break;
  case Qt::Key_G:
    m_lightYOffset -= 0.2;
    break;
  case Qt::Key_I:
    m_freq += 0.02f;
    break;
  case Qt::Key_O:
    m_freq -= 0.02f;
    break;
  case Qt::Key_R:
    m_lightRandom ^= true;
    break;
  case Qt::Key_A:
    m_useAO ^= true;
    break;
  case Qt::Key_M:
    m_textureDebug ^= true;
    break;
  case Qt::Key_Comma:
    --m_debugTextureID;
    break;
  case Qt::Key_Period:
    ++m_debugTextureID;
    break;
  case Qt::Key_U:
    m_showUI ^= true;
    break;
  case Qt::Key_Space:
    m_cam.set({0, 2, 10}, {0, 0, 0}, {0, 1, 0});
    break;
  case Qt::Key_4:
    for (auto &l : m_lights)
    {
      l.colour.set(m_lightMaxIntensity, m_lightMaxIntensity, m_lightMaxIntensity);
    }
    break;
  case Qt::Key_5:
  {
    for (auto &l : m_lights)
    {
      l.colour = ngl::Random::getRandomColour3() * m_lightMaxIntensity;
      l.colour.clamp(1.0f, m_lightMaxIntensity);
    }
    break;
  }
  case Qt::Key_6:
    for (auto &l : m_lights)
    {
      l.colour.set(6.0f, 1.0f, 1.0f);
    }
    break;
  case Qt::Key_7:
    for (auto &l : m_lights)
    {
      l.colour.set(1.0f, 6.0f, 1.0f);
    }
    break;
  case Qt::Key_8:
    for (auto &l : m_lights)
    {
      l.colour.set(1.0f, 1.0f, 6.0f);
    }
    break;
  case Qt::Key_Equal:
    m_numLights++;
    editLightShader();
    break;
  case Qt::Key_Minus:
    m_numLights--;
    editLightShader();
    break;
  default:
    break;
  }
  // finally update the GLWindow and re-draw
  update();
}

void NGLScene::debugBlit(GLuint _id)
{
  //  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  ngl::ShaderLib::use(DebugShader);
  ngl::ShaderLib::setUniform("screenResolution", ngl::Vec2(m_win.width, m_win.height));
  ngl::ShaderLib::setUniform("image", 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, _id);
  m_screenQuad->bind();
  m_screenQuad->draw();
  m_screenQuad->unbind();
}

void NGLScene::keyReleaseEvent(QKeyEvent *_event)
{
  // remove from our key set any keys that have been released
  m_keysPressed -= static_cast<Qt::Key>(_event->key());
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  if (!m_lightRandom)
  {
    float circleStep = 2.0f * static_cast<float>(M_PI) / m_lights.size();
    float i = 0.0f;
    static float time = 0.0f;
    for (auto &l : m_lights)
    {
      l.position.m_x = cosf(i) * m_lightRadius;
      l.position.m_z = sinf(i) * m_lightRadius;
      l.position.m_y = m_lightYOffset + abs(sinf(time * i * m_freq));
      i += circleStep;
    }
    time += 0.01f;
  }
  else
  {
    if (_event->timerId() == m_randomUpdateTimer)
    {
      ngl::Random::setSeed();

      for (auto &l : m_lights)
      {
        l.position = ngl::Random::getRandomPoint(10, 5, 10);
      }
    }
  }
  update();
}

void NGLScene::drawUI()
{
  QtImGui::newFrame();

  ImGui::Begin("Statistics");
  ImGui::Text("Geometry Pass %ld uS", m_geoPassDuration);
  ImGui::Text("SSAO Pass %ld uS ", m_ssaoPassDuration);
  ImGui::Text("Ligthing Pass %ld uS ", m_lightingPassDuration);
  ImGui::Text("Forward Pass %ld uS ", m_forwardPassDuration);
  ImGui::Text("Bloom Blur Pass %ld uS ", m_bloomBlurPassDuration);
  ImGui::Text("Final  Pass %ld uS ", m_finalPassDuration);
  ImGui::Text("Total Time %ld uS ", m_totalDuration);

  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

  ImGui::End();

  ImGui::Begin("Controls");
  ImGui::Checkbox("Use DOF", &m_useDOF);

  ImGui::SliderFloat("F-Stop", &m_fstop, 0.8f, 32.0f);
  ImGui::SliderInt("AV", &m_av, 0, 12);
  ImGui::SliderFloat("Focal Length", &m_focalLenght, 0.1f, 32.0f);
  ImGui::SliderFloat("Focal Distance", &m_focalDistance, 0.0f, 10.0f);
  ImGui::SliderFloat("Focus Distance", &m_focusDistance, 0.0f, 10.0f);
  ImGui::Separator();
  ImGui::Checkbox("Use AO", &m_useAO);
  ImGui::Checkbox("Use Bloom", &m_useBloom);
  ImGui::Separator();
  ImGui::SliderFloat("Exposure", &m_exposure, 0.0f, 5.0f);
  ImGui::SliderFloat("Gamma", &m_gamma, 0.1f, 4.0f);

  ImGui::End();

  ImGui::Begin("Lights");
  if (ImGui::SliderInt("Number of Lights", &m_numLights, 1, 64))
    editLightShader();
  ImGui::SliderFloat("Light intensity max", &m_lightMaxIntensity, 1.0f, 200.0f);
  ImGui::Separator();

  ImGui::SliderFloat("Light Radius", &m_lightRadius, 0.0f, 20.0f);
  ImGui::SliderFloat("Light Y", &m_lightYOffset, -20.0f, 20.0f);
  ImGui::Checkbox("Light Random", &m_lightRandom);
  ImGui::Separator();
  if (ImGui::Button("White"))
  {
    for (auto &l : m_lights)
    {
      l.colour.set(m_lightMaxIntensity, m_lightMaxIntensity, m_lightMaxIntensity);
    }
  }
  if (ImGui::Button("Rand"))
  {
    ngl::Random::setSeed();
    for (auto &l : m_lights)
    {
      l.colour = ngl::Random::getRandomColour3() * m_lightMaxIntensity;
      l.colour.clamp(1.0f, m_lightMaxIntensity);
    }
  }
  ImGui::End();

  ImGui::Render();
}
