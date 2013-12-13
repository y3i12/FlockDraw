#if !defined __PARTICLE_H__
#define __PARTICLE_H__

#include "cinder/Vector.h"
#include "cinder/Surface.h"

class ParticleEmitter;

class Particle
{
public:
  struct PointAccessFunctor
  {
    static inline ci::Vec2f& position( Particle* p )
    {
      return p->m_position;
    }

    static inline ci::Vec2f& stable_position( Particle* p )
    {
      return p->m_stablePosition;
    }
  }; 

public:
  Particle( ParticleEmitter* _owner, ci::Vec2f& _position, ci::Vec2f& _direction );

  virtual ~Particle( void );

  virtual void update( double _currentTime, double _delta );
  virtual void draw( void );
  virtual void debugDraw( void );
  
  ci::Vec2f&   position() { return m_position; }

protected:
  void         limitSpeed();


public:
  ci::Vec2f           m_position;
  ci::Vec2f           m_stablePosition;
  ci::Vec2f           m_direction;
  ci::Vec2f           m_velocity;
  ci::Vec2f           m_acceleration;

  ci::ColorA          m_color;

  float               m_maxSpeedSquared;
  float               m_minSpeedSquared;
  
  double              m_spawnTime;
  double              m_timeOfDeath;
    
  ci::Surface*        m_referenceSurface;

  ParticleEmitter*    m_owner;

  int                 m_group;
  
public:
  static float        s_maxRadius;
  static float        s_particleSizeRatio;
  static float        s_particleSpeedRatio;
  static float        s_dampness;
  static float        s_colorRedirection;

private:
  // temporary variables to avoid construction every update
  ci::Area            t_sourceArea;
  ci::ColorA          t_color;
  ci::Vec2f           t_tempDir;
  float               t_angle;
  ci::Vec2f           t_nextPos[ 3 ];
  float               t_l[ 3 ];
  ci::ColorA          t_currentColor;
  ci::ColorA          t_c;

  size_t              m_id;
  static size_t       s_idGenerator;


};

#endif // __PARTICLE_H__