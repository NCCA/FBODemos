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
NGLScene::NGLScene()
{
  setTitle("Deferred Render");
  m_lights.resize(m_numLights);
}

NGLScene::~NGLScene()
{
  std::cout<<"Shutting down NGL, removing VAO's and Shaders\n";
  // clear out our buffers
  glDeleteTextures(1,&m_textureID);
  glDeleteFramebuffers(1,&m_fboID);
}


void NGLScene::resizeGL( int _w, int _h )
{
  m_project=ngl::perspective(45.0f, static_cast<float>( _w ) / _h, 0.05f, 50.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}

auto GeometryPassShader="GeometryPassShader";
auto GeometryPassCheckerShader="GeometryPassCheckerShader";
auto LightingPassShader="LightingPassShader";
auto LightingPassFragment="LightingPassFragment";
void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::instance();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f);			   // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  m_from.set(0,2,10);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  m_win.width  = static_cast<int>( width() * devicePixelRatio() );
  m_win.height = static_cast<int>( height() * devicePixelRatio() );

  // now load to our new camera
  m_view=ngl::lookAt(m_from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45.0f,720.0f/576.0f,0.05f,10.0f);
  // now to load the shader and set the values
  // grab an instance of shader manager
  auto *shader=ngl::ShaderLib::instance();
  // load geometry pass shader and setup uniforms
  shader->loadShader(GeometryPassShader,"shaders/GeometryPassVertex.glsl","shaders/GeometryPassFragment.glsl");
  shader->use(GeometryPassShader);
  shader->setUniform("albedoSampler",0);
  shader->setUniform("specularSampler",1);
  shader->loadShader(GeometryPassCheckerShader,"shaders/GeometryPassVertex.glsl","shaders/CheckGeoPassFragment.glsl");
  shader->use(GeometryPassCheckerShader);
  shader->setUniform("colour1",0.9f,0.9f,0.9f,1.0f);
  shader->setUniform("colour2",0.6f,0.6f,0.6f,1.0f);
  shader->setUniform("checkSize",60.0f);

  // need to load this one manually as it will be dynamically editied
  shader->createShaderProgram(LightingPassShader);

  shader->attachShader("LightingPassVertex",ngl::ShaderType::VERTEX);
  shader->attachShader(LightingPassFragment,ngl::ShaderType::FRAGMENT);
  shader->loadShaderSource("LightingPassVertex","shaders/LightingPassVertex.glsl");
  shader->loadShaderSource(LightingPassFragment,"shaders/LightingPassFragment.glsl");
  // the shader has a tag called @numLights, edit this and set to 8
  shader->editShader(LightingPassFragment,"@numLights","64");
  shader->compileShader("LightingPassVertex");
  shader->compileShader(LightingPassFragment);
  shader->attachShaderToProgram(LightingPassShader,"LightingPassVertex");
  shader->attachShaderToProgram(LightingPassShader,LightingPassFragment);

 shader->linkProgramObject(LightingPassShader);
  (*shader)[LightingPassShader]->use();



//  shader->loadShader(LightingPassShader,"shaders/LightingPassVertex.glsl","shaders/LightingPassFragment.glsl");
//  shader->use(LightingPassShader);
  shader->setUniform("positionSampler",0);
  shader->setUniform("normalSampler",1);
  shader->setUniform("albedoSpecSampler",2);

  ngl::VAOPrimitives::instance()->createTrianglePlane("floor",20,20,1,1,ngl::Vec3::up());
  ngl::VAOPrimitives::instance()->createSphere("sphere",0.5f,20);

  m_renderFBO=FrameBufferObject::create(1024*devicePixelRatio(),720*devicePixelRatio());
  m_renderFBO->bind();
  m_renderFBO->addColourAttachment("position",GLAttatchment::_0,GLTextureFormat::RGB,GLTextureInternalFormat::RGB16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST,GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE,true);
  m_renderFBO->addColourAttachment("normal",GLAttatchment::_1,GLTextureFormat::RGB,GLTextureInternalFormat::RGB16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST,GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE,true);

  m_renderFBO->addColourAttachment("albedoSpec",GLAttatchment::_2,GLTextureFormat::RGBA,GLTextureInternalFormat::RGBA16F,
                                   GLTextureDataType::FLOAT,
                                   GLTextureMinFilter::NEAREST,GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE,true);


  m_renderFBO->addDepthBuffer(GLTextureDepthFormats::DEPTH_COMPONENT16,
                              GLTextureMinFilter::NEAREST,GLTextureMagFilter::NEAREST,
                              GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE,
                              true
                              );
  // setup draw buffers whilst still bound
  GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
  glDrawBuffers(3, attachments);

  if(!m_renderFBO->isComplete())
  {
   ngl::msg->addWarning("FrameBuffer incomplete");
   m_renderFBO->print();
  }
  createScreenQuad();

  // now create the primitives to draw
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createTrianglePlane("plane",2,2,20,20,ngl::Vec3(0,1,0));
  prim->createSphere("sphere",0.1f,80);
  startTimer(10);
  GLint maxAttach = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
  std::cout << maxAttach<<"\n";
  glEnable(GL_DEPTH_TEST);

  // load textures for render pass (only using 1 for all teapots)
  ngl::Texture t;
  t.loadImage("textures/albedo.png");
  m_albedoTextureID=t.setTextureGL();
  t.loadImage("textures/metallic.png");
  m_specularTextureID=t.setTextureGL();
  createLights();
  m_randomUpdateTimer=startTimer(400);

}


void NGLScene::loadMatricesToShader(const ngl::Mat4 &_mouse)
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4  MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M=_mouse*m_transform.getMatrix();
  MV=  m_view*M;
  MVP= m_project*MV;
  normalMatrix=MV;
  normalMatrix.inverse().transpose();
  shader->setUniform("MVP",MVP);
  shader->setUniform("normalMatrix",normalMatrix);
  shader->setUniform("M",M);
}

void NGLScene::paintGL()
{
  //----------------------------------------------------------------------------------------------------------------------
  // draw to our FBO first
  //----------------------------------------------------------------------------------------------------------------------
  // grab an instance of the shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // set the background colour (using blue to show it up)

  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX(m_win.spinXFace);
  rotY.rotateY(m_win.spinYFace);
  // multiply the rotations
  m_mouseGlobalTX=rotY*rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;


   // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  //----------------------------------------------------------------------------------------------------------------------
  // Pass 1
  // we are now going to draw to our FBO
  // set the rendering destination to FBO
  //----------------------------------------------------------------------------------------------------------------------
  // set this to be the actual screen size
  //glViewport(0, 0, m_win.width, m_win.height);
  m_renderFBO->bind();
  m_renderFBO->setViewport();
//  GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
//  glDrawBuffers(3, attachments);
  // bind textures for teapots
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D,m_albedoTextureID);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D,m_specularTextureID);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader->use(GeometryPassShader);
  // if we want a different camera we wouldset this here
  // rotate the teapot
  m_transform.reset();
  static float s_rot=0.1f;
  for (float z=-10.0f; z<10.0f; z+=1.8f)
  {
    for(float x=-10.0f; x<10.0f; x+=1.8f)
    {
      m_transform.setRotation(0,s_rot,0);
      m_transform.setPosition(x,0.0f,z);
      loadMatricesToShader(m_mouseGlobalTX);
      prim->draw("teapot");
    }
  }
  s_rot+=1.0f;
  shader->use(GeometryPassCheckerShader);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.45f,0.0f);
//  ngl::Mat4 MVP=m_project*m_view*m_mouseGlobalTX*m_transform.getMatrix();
//  ngl::Mat3 normalMatrix=m_view*m_mouseGlobalTX;
//  normalMatrix.inverse().transpose();
//  shader->setUniform("MVP",MVP);
//  shader->setUniform("normalMatrix",normalMatrix);
  loadMatricesToShader(m_mouseGlobalTX);
  prim->draw("floor");
  //----------------------------------------------------------------------------------------------------------------------
  // if in debug mode draw gbuffer values for 2nd pass
  //
  m_renderFBO->unbind();
  if(m_debugOn==true)
  {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER,defaultFramebufferObject());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderFBO->getID());
    glReadBuffer(GL_COLOR_ATTACHMENT0+m_debugAttachment);
    glBlitFramebuffer(0,0,m_win.width,m_win.height,0,0,m_win.width,m_win.height,GL_COLOR_BUFFER_BIT,GL_NEAREST);
  }
  else // do deferred render
  {
    shader->use(LightingPassShader);
    glBindFramebuffer(GL_FRAMEBUFFER,defaultFramebufferObject());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0,0,m_win.width,m_win.height);
    // bind textures for FBO
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("position"));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("normal"));
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("albedoSpec"));
    int i=0;
    for(auto l : m_lights)
    {
      shader->setUniform(fmt::format("lights[{0}].position",i),l.position);
      shader->setUniform(fmt::format("lights[{0}].colour",i),l.colour);
      shader->setUniform(fmt::format("lights[{0}].linear",i),l.linear);
      shader->setUniform(fmt::format("lights[{0}].quadratic",i),l.quadratic);

      ++i;
    }
    shader->setUniform("viewPos",m_from);
    shader->setUniform("screenResolution",ngl::Vec2(m_win.width,m_win.height));
    m_screenQuad->bind();
    m_screenQuad->draw();
    m_screenQuad->unbind();
    if(m_showLights==true)
    {
      /// now to do a foward render pass of light Geo
      // first we copy the depth buffer from our render buffer to the defaultFBO
      glBindFramebuffer(GL_READ_FRAMEBUFFER, m_renderFBO->getID());
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebufferObject()); // write to default framebuffer
      glBlitFramebuffer(0, 0, m_win.width, m_win.width, 0, 0, m_win.width, m_win.width, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
      glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
      glViewport(0,0,m_win.width,m_win.height);
      shader->use(ngl::nglColourShader);
      for(auto l : m_lights)
      {
        m_transform.setPosition(l.position);
        shader->setUniform("Colour",ngl::Vec4(l.colour.m_r,l.colour.m_g,l.colour.m_b,1.0f));
        ngl::Mat4 MVP =m_project* m_view* m_mouseGlobalTX*m_transform.getMatrix();
        shader->setUniform("MVP",MVP);
        prim->draw("sphere");
      }

    } // end show lights / forward pass
  }

}

void NGLScene::createScreenQuad()
{
  m_screenQuad=ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_TRIANGLES);
  m_screenQuad->bind();

  std::array<GLfloat,12> quad ={{-1.0f,1.0f,-1.0f,-1.0f, 1.0f,-1.0f,-1.0f,1.0f,1.0f,-1.0f,1.0f, 1.0f}};
  m_screenQuad->setData(ngl::AbstractVAO::VertexData(quad.size()*sizeof(GLfloat),quad[0]));
  m_screenQuad->setVertexAttributePointer(0,2,GL_FLOAT,0,0);
  m_screenQuad->setNumIndices(quad.size());
  m_screenQuad->unbind();

}

void NGLScene::editLightShader()
{
  auto *shader=ngl::ShaderLib::instance();
  auto editString=fmt::format("{0}",m_numLights);
  shader->editShader(LightingPassFragment,"@numLights",editString);
  shader->compileShader(LightingPassFragment);
  shader->linkProgramObject(LightingPassShader);
  shader->use(LightingPassShader);
  m_lights.resize(m_numLights);
  shader->setUniform("positionSampler",0);
  shader->setUniform("normalSampler",1);
  shader->setUniform("albedoSpecSampler",2);
  createLights();
  setTitle(QString("Deferred Renderer %0 Lights").arg(m_numLights));
}

void NGLScene::createLights()
{


//  for(float i=0; i<2.0f*static_cast<float>(M_PI); i+=2.0f*static_cast<float>(M_PI)/m_numl)

  float circleStep=2.0f*static_cast<float>(M_PI)/m_lights.size();
  auto rng=ngl::Random::instance();
  float i=0.0f;
  for(auto &l : m_lights)
  {
    float x=cosf(i)*m_lightRadius;
    float z=sinf(i)*m_lightRadius;
    float y=m_lightYOffset+sinf(i*m_freq);
    l.position.set(x,y,z);//=rng->getRandomVec3()*5.0f;
    l.colour=rng->getRandomColour3();
    l.colour.clamp(0.2f,1.0f);
    i+=circleStep;
  }
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
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;
  case Qt::Key_0 : m_showLights^=true; break;
  case Qt::Key_1 : m_debugAttachment=0; break;
  case Qt::Key_2 : m_debugAttachment=1; break;
  case Qt::Key_3 : m_debugAttachment=2; break;
  case Qt::Key_D : m_debugOn^=true; break;
  case Qt::Key_Left : m_lightRadius-=0.2; break;
  case Qt::Key_Right : m_lightRadius+=0.2; break;
  case Qt::Key_Up : m_lightYOffset+=0.2; break;
  case Qt::Key_Down : m_lightYOffset-=0.2; break;
  case Qt::Key_I : m_freq+=0.02f; break;
  case Qt::Key_O : m_freq-=0.02f; break;
  case Qt::Key_R : m_lightRandom^=true; break;
  case Qt::Key_4 :
    for(auto &l : m_lights)
    {
      l.colour.set(1.0f,1.0f,1.0f);
    }
  break;
  case Qt::Key_5 :
  {
    auto rng=ngl::Random::instance();
    for(auto &l : m_lights)
    {
      l.colour=rng->getRandomColour3();
      l.colour.clamp(0.2f,1.0f);
    }
  break;
  }
  case Qt::Key_Equal : m_numLights++; editLightShader(); break;
  case Qt::Key_Minus : m_numLights--; editLightShader(); break;
  default : break;
  }
  // finally update the GLWindow and re-draw
    update();
}
void NGLScene::timerEvent(QTimerEvent *_event)
{
  if(!m_lightRandom)
  {
    float circleStep=2.0f*static_cast<float>(M_PI)/m_lights.size();
    float i=0.0f;
    static float time=0.0f;
    for(auto &l : m_lights)
    {
      l.position.m_x=cosf(i)*m_lightRadius;
      l.position.m_z=sinf(i)*m_lightRadius;
      l.position.m_y=m_lightYOffset+abs(sinf(time*i*m_freq));
      i+=circleStep;
    }
    time+=0.01f;
  }
  else
  {
    if(_event->timerId()==m_randomUpdateTimer)
    {
    auto rng=ngl::Random::instance();
    for(auto &l : m_lights)
    {
      l.position=rng->getRandomPoint(10,5,10);

    }
    }
  }
  update();
}

