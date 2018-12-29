#include "Emitter.h"
#include <ngl/Random.h>
#include <ngl/Transformation.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/NGLStream.h>
#include <ngl/VAOPrimitives.h>
#include <array>
/// @brief ctor
/// @param _pos the position of the emitter
/// @param _numParticles the number of particles to create
Emitter::Emitter(ngl::Vec3 _pos, unsigned int _numParticles )
{
  Particle p;
  GLParticle g;
  ngl::Random *rand=ngl::Random::instance();
  m_pos=_pos;
  m_particles.reset(  new Particle[_numParticles]);
  std::unique_ptr<GLParticle []> glparticles=std::make_unique<GLParticle []>(_numParticles);
//  glparticles.reset( new GLParticle[_numParticles]);

  m_vao=ngl::VAOFactory::createVAO(ngl::simpleVAO,GL_POINTS);


//#pragma omp parallel for ordered schedule(dynamic)
  for (unsigned int i=0; i< _numParticles; ++i)
  {
    g.px=p.m_px=m_pos.m_x;
    g.py=p.m_py=m_pos.m_y;
    g.pz=p.m_pz=m_pos.m_z;
    g.pw=0.1f;
    ngl::Vec3 c=rand->getRandomColour3()*10;
    g.pr=0.5f+c.m_r;
    g.pg=0.5f+c.m_g;
    g.pb=0.5f+c.m_b;
    p.m_maxLife=rand->randomPositiveNumber(50);
    p.m_dx=rand->randomNumber(5)+0.5f;
    p.m_dy=rand->randomPositiveNumber(10.0f)+0.5f;
    p.m_dz=rand->randomNumber(5)+0.5f;
    p.m_gravity=-9.0f;
    p.m_currentLife=0.0f;
    m_particles[i]=p;
    glparticles[i]=g;
  }
  m_numParticles=_numParticles;
  // create the VAO and stuff data

  m_vao->bind();
  // now copy the data
  m_vao->setData(ngl::SimpleVAO::VertexData(m_numParticles*sizeof(GLParticle),glparticles[0].px));
  m_vao->setVertexAttributePointer(0,4,GL_FLOAT,sizeof(GLParticle),0);
  m_vao->setVertexAttributePointer(1,4,GL_FLOAT,sizeof(GLParticle),4);
  m_vao->setNumIndices(_numParticles);
  m_vao->unbind();


}


Emitter::~Emitter()
{
}

/// @brief a method to update each of the particles contained in the system
void Emitter::update()
{
  m_vao->bind();
  ngl::Real *glPtr=m_vao->mapBuffer();
  unsigned int glIndex=0;
  static float time=0.0;
  time+=m_time;
// / ngl::msg->drawLine();

 // #pragma omp parallel for
  for(unsigned int i=0; i<m_numParticles; ++i)
  {
    m_particles[i].m_currentLife+=0.005f;
    // use projectile motion equation to calculate the new position
    // x(t)=Ix+Vxt
    // y(t)=Iy+Vxt-1/2gt^2
    // z(t)=Iz+Vzt
    ngl::Vec3 wind(1,1,1);
    m_particles[i].m_px=m_pos.m_x+(wind.m_x*m_particles[i].m_dx*m_particles[i].m_currentLife);
    m_particles[i].m_py= m_pos.m_y+(wind.m_y*m_particles[i].m_dy*m_particles[i].m_currentLife)+m_particles[i].m_gravity*(m_particles[i].m_currentLife*m_particles[i].m_currentLife);
    m_particles[i].m_pz=m_pos.m_z+(wind.m_z*m_particles[i].m_dz*m_particles[i].m_currentLife);

    glPtr[glIndex]=m_particles[i].m_px;
    glPtr[glIndex+1]=m_particles[i].m_py;
    glPtr[glIndex+2]=m_particles[i].m_pz;
    glPtr[glIndex+3]+=0.01f;
    // if we go below the origin re-set
    if(m_particles[i].m_py <=m_pos.m_y-0.01f || m_particles[i].m_currentLife > m_particles[i].m_maxLife)
    {

      m_particles[i].m_px=m_pos.m_x;
      m_particles[i].m_pz=m_pos.m_y;
      m_particles[i].m_px=m_pos.m_z;

      m_particles[i].m_currentLife=0.0;
      ngl::Random *rand=ngl::Random::instance();
      m_particles[i].m_dx=rand->randomNumber(2)+0.5f;
      m_particles[i].m_dy=rand->randomPositiveNumber(10)+0.5f;
      m_particles[i].m_dz=rand->randomNumber(2)+0.5f;
      m_particles[i].m_maxLife=rand->randomPositiveNumber(50);

      glPtr[glIndex]=m_particles[i].m_px;
      glPtr[glIndex+1]=m_particles[i].m_py;
      glPtr[glIndex+2]=m_particles[i].m_pz;
      glPtr[glIndex+3]=0.5f;

      ngl::Vec3 c=rand->getRandomColour3()*10.0f;
      glPtr[glIndex+4]=0.5f+c.m_r;
      glPtr[glIndex+5]=0.5f+c.m_g;
      glPtr[glIndex+6]=0.5f+c.m_b;

    }
  //  #pragma omp atomic
    glIndex+=8;

  }
  m_vao->unmapBuffer();


  m_vao->unbind();


}
/// @brief a method to draw all the particles contained in the system
void Emitter::draw(const ngl::Mat4 &_VP)
{
  auto shader=ngl::ShaderLib::instance();
  shader->use("ParticleShader");
  shader->setUniform("MVP",_VP);
  glEnable(GL_PROGRAM_POINT_SIZE);
  //glPointSize(60);
  m_vao->bind();
  m_vao->draw();
/*
  ngl::Real *glPtr=m_vao->mapBuffer();

    ngl::Transformation tx;
  size_t glIndex=0;
  for(size_t i=0; i<m_numParticles; ++i)
  {

     tx.setScale(0.01,0.01,0.01);
     tx.setPosition(glPtr[glIndex],glPtr[glIndex+1],glPtr[glIndex+2]);
     ngl::ShaderLib::instance()->setUniform("MVP",_VP*tx.getMatrix());
     glIndex+=7;
     ngl::VAOPrimitives::instance()->draw("sphere");
   }
  m_vao->unmapBuffer();*/

  m_vao->unbind();
}
