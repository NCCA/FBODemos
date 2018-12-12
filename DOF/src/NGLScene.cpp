#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/SimpleVAO.h>
#include <ngl/VAOFactory.h>
#include <ngl/Image.h>
NGLScene::NGLScene()
{
  setTitle("Depth of Field");
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

auto ScreenQuad="ScreenQuad";
auto DOFShader="DOFShader";

void NGLScene::loadDOFUniforms()
{
  auto shader=ngl::ShaderLib::instance();
  shader->use(DOFShader);

  float magnification = m_focalLenght / abs(m_focalDistance - m_focalLenght);
  float blur = m_focalLenght * magnification / m_fstop;
  float ppm = sqrtf(m_win.width * m_win.width + m_win.height * m_win.height) / 35;
  shader->setUniform("depthRange",ngl::Vec2(0.1f,50.0f));
  shader->setUniform("blurCoefficient",blur);
  shader->setUniform("PPM",ppm);
  shader->setUniform("screenResolution",ngl::Vec2(m_win.width,m_win.height));
  shader->setUniform("focusDistance",m_focusDistance);

}

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
  ngl::Vec3 from(0,2,10);
  ngl::Vec3 to(0,0,0);
  ngl::Vec3 up(0,1,0);
  m_win.width  = static_cast<int>( width() * devicePixelRatio() );
  m_win.height = static_cast<int>( height() * devicePixelRatio() );

  // now load to our new camera
  m_view=ngl::lookAt(from,to,up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project=ngl::perspective(45.0f,720.0f/576.0f,0.05f,10.0f);
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  // we are creating a shader called Phong
  shader->loadShader("Phong","shaders/PhongVert.glsl","shaders/PhongFrag.glsl");
  // and make it active ready to load values
  (*shader)["Phong"]->use();
  ngl::Vec4 lightPos(-2.0f,5.0f,2.0f,0.0f);
  shader->setUniform("light.position",lightPos);
  shader->setUniform("light.ambient",0.0f,0.0f,0.0f,1.0f);
  shader->setUniform("light.diffuse",1.0f,1.0f,1.0f,1.0f);
  shader->setUniform("light.specular",0.8f,0.8f,0.8f,1.0f);
  // gold like phong material
  shader->setUniform("material.ambient",0.274725f,0.1995f,0.0745f,0.0f);
  shader->setUniform("material.diffuse",0.75164f,0.60648f,0.22648f,0.0f);
  shader->setUniform("material.specular",0.628281f,0.555802f,0.3666065f,0.0f);
  shader->setUniform("material.shininess",51.2f);
  shader->setUniform("viewerPos",from);

  // now load our texture shader
  shader->loadShader("TextureShader","shaders/TextureVertex.glsl","shaders/TextureFragment.glsl");

  shader->loadShader(ScreenQuad,"shaders/ScreenQuadVertex.glsl","shaders/ScreenQuadFragment.glsl");
  shader->loadShader(DOFShader,"shaders/DOFVertex.glsl","shaders/DOFFragment.glsl");
  loadDOFUniforms();
  m_renderFBO=FBO::create(1024*devicePixelRatio(),720*devicePixelRatio());
  m_renderFBO->bind();
  m_renderFBO->addColourAttachment("renderTarget",0,GLTextureFormat::RGBA,GLTextureInternalFormat::RGBA8,
                                   GLTextureDataType::UNSIGNED_BYTE,
                                   GLTextureMinFilter::NEAREST,GLTextureMagFilter::NEAREST,
                                   GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE);

  m_renderFBO->addDepthBuffer(GLTextureDepthFormats::DEPTH_COMPONENT16,GLTextureDataType::FLOAT,
                              GLTextureMinFilter::NEAREST,GLTextureMagFilter::NEAREST,
                              GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE
                              );

  ngl::msg->addMessage(fmt::format("RenderTargetID {0}",m_renderFBO->getTextureID("renderTarget")));
  m_renderFBO->print();
  ngl::msg->addMessage(fmt::format("Is complete {0}",m_renderFBO->isComplete()));


  m_blurFBO=FBO::create(1024*devicePixelRatio(),720*devicePixelRatio());
  m_blurFBO->bind();
  m_blurFBO->addColourAttachment("blurTarget",0,GLTextureFormat::RGBA,GLTextureInternalFormat::RGBA8,
                                   GLTextureDataType::UNSIGNED_BYTE,
                                   GLTextureMinFilter::LINEAR,GLTextureMagFilter::LINEAR,
                                   GLTextureWrap::CLAMP_TO_EDGE,GLTextureWrap::CLAMP_TO_EDGE);

  ngl::msg->addMessage(fmt::format("blurTarget {0}",m_blurFBO->getTextureID("blurTarget")));
  m_renderFBO->print();
  ngl::msg->addMessage(fmt::format("Is complete {0}",m_blurFBO->isComplete()));


  createScreenQuad();

  // now create the primitives to draw
  ngl::VAOPrimitives *prim=ngl::VAOPrimitives::instance();
  prim->createTrianglePlane("plane",2,2,20,20,ngl::Vec3(0,1,0));
  prim->createSphere("sphere",0.4f,80);
  startTimer(1);
  GLint maxAttach = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttach);
  std::cout << maxAttach<<"\n";
  glEnable(GL_DEPTH_TEST);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("renderTarget"));

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getDepthTextureID());

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_blurFBO->getTextureID("blurTarget"));




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
  shader->setUniform("MV",MV);
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
  (*shader)["Phong"]->use();
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
//  glActiveTexture(GL_TEXTURE0);
//  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getTextureID("renderTarget"));

//  glActiveTexture(GL_TEXTURE1);
//  glBindTexture(GL_TEXTURE_2D, m_renderFBO->getDepthTextureID());

//  glActiveTexture(GL_TEXTURE2);
//  glBindTexture(GL_TEXTURE_2D, m_blurFBO->getTextureID("blurTarget"));

  ngl::msg->addMessage(fmt::format("renderTarget {0} depth {1} blur {2}",m_renderFBO->getTextureID("renderTarget"),
                                   m_renderFBO->getDepthTextureID(),
                                   m_blurFBO->getTextureID("blurTarget")));
  m_renderFBO->bind();
  m_renderFBO->setViewport();
  glClearColor(0,0,0,1);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


  // if we want a different camera we wouldset this here
  // rotate the teapot
  m_transform.reset();
  static float s_rot=0.1f;
//  m_transform.setScale(0.8f,0.8f,0.8f);
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
  m_transform.reset();
  m_transform.setPosition(0,1.0f,m_focusDistance);
  m_transform.setScale(0.1f,0.1f,0.1f);
  loadMatricesToShader(m_mouseGlobalTX);
  prim->draw("cube");
 // ngl::Image::saveFrameBufferToFile("pass1.png",0,0,m_win.width,m_win.height);
  m_renderFBO->unbind();


 //----------------------------------------------------------------------------------------------------------------------
  /// BLUR PASSES
 //----------------------------------------------------------------------------------------------------------------------
  m_blurFBO->bind();
  m_screenQuad->bind();
 // m_blurFBO->setViewport();
  //glClear(GL_DEPTH_BUFFER_BIT);

  shader->use(DOFShader);
  loadDOFUniforms();
//  // HORIZONTAL BLUR
  shader->setUniform("colourSampler",0);
  shader->setUniform("depthSampler",1);
  shader->setUniform("uTexelOffset",1.0f,0.0f);
  m_screenQuad->draw();
  //ngl::Image::saveFrameBufferToFile("pass2.png",0,0,m_win.width,m_win.height);
  shader->setUniform("uTexelOffset",0.0f,1.0f);
  shader->setUniform("colourSampler",2);
  m_screenQuad->draw();

  //----------------------------------------------------------------------------------------------------------------------
  // VERTICAL BLUR
  //----------------------------------------------------------------------------------------------------------------------
  m_blurFBO->unbind();
  shader->use(ScreenQuad);
  glBindFramebuffer(GL_FRAMEBUFFER,defaultFramebufferObject());
  glClear(GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_win.width, m_win.height);
//  shader->setUniform("hTexelOffset",0.0f,1.0f);
  shader->setUniform("colour",2);
  m_screenQuad->draw();
  //ngl::Image::saveFrameBufferToFile("pass3.png",0,0,m_win.width,m_win.height);

  m_screenQuad->unbind();


  //----------------------------------------------------------------------------------------------------------------------
 }

void NGLScene::createScreenQuad()
{
  m_screenQuad=ngl::VAOFactory::createVAO(ngl::multiBufferVAO,GL_TRIANGLES);
  m_screenQuad->bind();

  std::array<GLfloat,12> quad ={{-1.0f,1.0f,-1.0f,-1.0f, 1.0f,-1.0f,-1.0f,1.0f,1.0f,-1.0f,1.0f, 1.0f}};
  std::array<GLfloat,12> uv=   {{ 0.0f,1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,1.0f,1.0f, 0.0f,1.0f, 1.0f}};
  m_screenQuad->setData(ngl::AbstractVAO::VertexData(quad.size()*sizeof(GLfloat),quad[0]));
  m_screenQuad->setVertexAttributePointer(0,2,GL_FLOAT,0,0);
  m_screenQuad->setData(ngl::AbstractVAO::VertexData(quad.size()*sizeof(GLfloat),uv[0]));
  m_screenQuad->setVertexAttributePointer(1,2,GL_FLOAT,0,0);
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
  // turn on wirframe rendering
  case Qt::Key_W : glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); break;
  // turn off wire frame
  case Qt::Key_S : glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); break;
  // show full screen
  case Qt::Key_F : showFullScreen(); break;
  // show windowed
  case Qt::Key_N : showNormal(); break;

  case Qt::Key_Left : m_av-=1; m_fstop=sqrtf(pow(2,m_av)); break;
  case Qt::Key_Right : m_av+=1; m_fstop=sqrtf(pow(2,m_av));  break;
  case Qt::Key_Up : m_focalDistance +=0.1f; break;
  case Qt::Key_Down : m_focalDistance -=0.1f; break;
  case Qt::Key_I : m_focalLenght +=0.1f; break;
  case Qt::Key_O : m_focalLenght -=0.1f; break;
  case Qt::Key_K : m_focusDistance +=0.1f; break;
  case Qt::Key_L : m_focusDistance -=0.1f; break;

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

