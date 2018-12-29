#ifndef EMITTER_H_
#define EMITTER_H_
#include <vector>
#include <ngl/Mat4.h>
#include <ngl/Vec3.h>
#include <ngl/AbstractVAO.h>
#include <memory>
#pragma pack(push,1)

struct Particle
{
	/// @brief the curent particle position
	GLfloat m_px;
	GLfloat m_py;
	GLfloat m_pz;
	/// @brief the direction vector of the particle
	GLfloat m_dx;
	GLfloat m_dy;
	GLfloat m_dz;
	/// @brief the current life value of the particle
	GLfloat m_currentLife;
  GLfloat m_maxLife;
  /// @brief gravity
	GLfloat m_gravity;


};
#pragma pack(pop)

#pragma pack(push,1)
struct GLParticle
{
  GLfloat px;
  GLfloat py;
  GLfloat pz;
  GLfloat pw;
  GLfloat pr;
  GLfloat pg;
  GLfloat pb;
  GLfloat ba;

};

#pragma pack(pop)





class Emitter
{
public :

	/// @brief ctor
	/// @param _pos the position of the emitter
	/// @param _numParticles the number of particles to create
  Emitter( ngl::Vec3 _pos, unsigned int _numParticles );
	/// @brief a method to update each of the particles contained in the system
	void update();
	/// @brief a method to draw all the particles contained in the system
  void draw(const ngl::Mat4 &_VP);
	~Emitter();
  void incTime(float _t){m_time+=_t;}
  void decTime(float _t){m_time-=_t;}
  private :
	/// @brief the position of the emitter
	ngl::Vec3 m_pos;
	/// @brief the number of particles
  unsigned int m_numParticles;
	/// @brief the container for the particles
  std::unique_ptr<Particle []> m_particles;
  /// @brief the name of the shader to use
  std::string m_shaderName;
  std::unique_ptr<ngl::AbstractVAO> m_vao;
  float m_time=0.8f;

};


#endif

