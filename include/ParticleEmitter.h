#if !defined __PARTICLE_EMITTER_H__
#define __PARTICLE_EMITTER_H__

#include <list>
#include "cinder/Vector.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"

class Particle;
class b2World;

class ParticleEmitter
{
public:
  ParticleEmitter( void );
  virtual ~ParticleEmitter( void );

  void addParticles( int _aumont );

  virtual void draw( void );
  virtual void update( double _currentTime, double _delta );

  virtual void killAll( double _currentTime );

  std::list< Particle* > m_particles;
  ci::Vec2f              m_position;
  double                 m_maxLifeTime;
  double                 m_minLifeTime;

  double                 m_fadeInTime;
  double                 m_fadeOutTime;

  float                  m_particlesPerSecond;
  
  ci::Surface*           m_referenceSurface;
  ci::gl::Texture*       m_screenTexture;
  ci::Surface            m_screenSurface;

private:
  float                  m_particlesPerSecondLeftOver;
};

#endif //__PARTICLE_EMITTER_H__
