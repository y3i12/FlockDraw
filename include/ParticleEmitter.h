#if !defined __PARTICLE_EMITTER_H__
#define __PARTICLE_EMITTER_H__

#include <vector>
#include <unordered_map>
#include "cinder/Vector.h"
#include "cinder/Surface.h"
#include "cinder/gl/Texture.h"

#include "Particle.h"


class b2World;

class ParticleEmitter
{
public:
  ParticleEmitter( void );
  virtual ~ParticleEmitter( void );

  void addParticles( int _aumont, int _group = -1 );
  
  virtual void draw( void );
  virtual void debugDraw( void );
  virtual void update( double _currentTime, double _delta );

  virtual void killAll( double _currentTime );

  std::unordered_map< int, std::vector< Particle* > > m_particles;
  ci::Vec2f                m_position;
  double                   m_maxLifeTime;
  double                   m_minLifeTime;
                           
  float                    m_particlesPerSecond;

  // flocking vars
  float                    m_zoneRadiusSqrd;
  float                    m_repelStrength;
  float                    m_alignStrength;
  float                    m_attractStrength;
  float                    m_lowThresh;
  float                    m_highThresh;
                           
  ci::Surface*             m_referenceSurface;
  ci::gl::Texture*         m_screenTexture;
  ci::Surface              m_screenSurface;

  static bool              s_debugDraw;
private:
  void updateParticles( double _currentTime, double _delta, std::vector< Particle* >& _particles );

  float                  m_particlesPerSecondLeftOver;
  double                 m_updateFlockEvery;
  double                 m_updateFlockTimer;
  double                 m_lastFlockUpdateTime;
};

#endif //__PARTICLE_EMITTER_H__
