#if !defined __PARTICLE_H__
#define __PARTICLE_H__

#include "cinder/Vector.h"
#include "cinder/Surface.h"

class ParticleEmitter;

class Particle
{
public:
  Particle( ParticleEmitter* _owner, ci::Vec2f& _position, ci::Vec2f& _direction );

  virtual ~Particle( void );

  virtual void update( double _currentTime, double _delta );
  virtual void draw( void );

  virtual void setup( );

  ci::Vec2f&   position() { return m_position; }

protected:
  void         updateColorByRef();
  void         limitSpeed();


public:
  ci::Vec2f           m_position;
  ci::Vec2f           m_direction;
  ci::Vec2f           m_velocity;
  ci::Vec2f           m_acceleration;

  ci::ColorA          m_color;

  
  float               m_radius;

  float               m_maxSpeedSquared;
  float               m_minSpeedSquared;
  
  double              m_spawnTime;
  double              m_timeOfDeath;

  float               m_fadeInTime;
  float               m_fadeOutTime;
  
  ci::Surface*        m_referenceSurface;

  ParticleEmitter*    m_owner;

  int                 m_group;
  
private:
  float               m_fadeFactor;
  float               m_maxRadius;

};

#endif // __PARTICLE_H__