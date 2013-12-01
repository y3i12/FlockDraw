#include "ParticleEmitter.h"
#include "Particle.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include "cinder/Vector.h"

#define PI  3.14159265359f
#define PI2 6.28318530718f

ParticleEmitter::ParticleEmitter(void) :
  m_position( 0.0f, 0.0f ),
  m_maxLifeTime( 0.0f ),
  m_minLifeTime( 0.0f ),
  m_fadeInTime( 0.0f ),
  m_fadeOutTime( 0.0f ),
  m_particlesPerSecond( 0.0f ),
  m_referenceSurface( 0 ),
  m_particlesPerSecondLeftOver( 0.0f )
{
}

ParticleEmitter::~ParticleEmitter(void)
{
}

#define EMISSION_AREA_PERCENTAGE 0.01f
void ParticleEmitter::addParticles( int _aumont )
{
  ci::Vec2f refSize;
  ci::Area  emissionArea( m_position, m_position );
  
  if ( m_referenceSurface )
  {
    refSize = m_referenceSurface->getSize();
    emissionArea.x1 = ci::randFloat( refSize.x - refSize.x * EMISSION_AREA_PERCENTAGE );
    emissionArea.y1 = ci::randFloat( refSize.y - refSize.y * EMISSION_AREA_PERCENTAGE );
    emissionArea.x2 = emissionArea.x1 + refSize.x * EMISSION_AREA_PERCENTAGE;
    emissionArea.y2 = emissionArea.y1 + refSize.y * EMISSION_AREA_PERCENTAGE;
  }
  
  for ( int i = 0; i < _aumont; ++i )
  {
    float angle = ci::Rand::randFloat( 0.0f, 2 * PI );
    float u = sin( angle );
    float v = cos( angle );

    Particle* p = 0;
    if ( m_referenceSurface )
    {
      ci::Vec2f pos;
      pos.x = ci::Rand::randFloat( emissionArea.x1, emissionArea.x2 );
      pos.y = ci::Rand::randFloat( emissionArea.y1, emissionArea.y2 );

      p = new Particle( this, pos, ci::Vec2f( u, v ) );
      p->m_referenceSurface = m_referenceSurface;
    }
    else
    {
      p = new Particle( this, m_position, ci::Vec2f( u, v ) );
    }
    

    if ( m_minLifeTime < m_maxLifeTime )
    {
      p->m_timeOfDeath   = p->m_spawnTime   + ci::Rand::randFloat( m_minLifeTime, m_maxLifeTime );
      p->m_fadeOutTime   = p->m_timeOfDeath - m_fadeOutTime;
      p->m_fadeInTime    = p->m_spawnTime   + m_fadeInTime;
    }
    else
    {
      p->m_fadeInTime  = p->m_spawnTime   + m_fadeInTime;
      p->m_timeOfDeath = -1.0f;
    }

    p->m_maxSpeedSquared = ci::Rand::randFloat( 10, 50 );
    p->m_minSpeedSquared = ci::Rand::randFloat( 1, 10 );
    
    p->m_acceleration.x      = ci::Rand::randFloat( 0, 10 ) - 5.0f;
    p->m_acceleration.y      = ci::Rand::randFloat( 0, 10 ) - 5.0f;
    p->m_acceleration.normalize();
    p->m_acceleration       *= 2.5f;

    p->m_radius          = 5.0f;

    p->setup( );
    m_particles.push_back( p );
  }
}

void ParticleEmitter::draw( void )
{
  std::list< Particle* >::iterator itr     = m_particles.begin();
  std::list< Particle* >::iterator itr_end = m_particles.end();

  for ( ; itr != itr_end; ++itr )
  {
    ( *itr )->draw();
  }
}

void ParticleEmitter::update( double _currentTime, double _delta )
{
  if ( m_particlesPerSecond )
  { 
    float particlesToEmit     = _delta * m_particlesPerSecond + m_particlesPerSecondLeftOver;
    int   particlesToEmmitInt = static_cast< int >( particlesToEmit );
    
    if ( particlesToEmmitInt )
    {
      addParticles( particlesToEmmitInt );
    }

    m_particlesPerSecondLeftOver = particlesToEmit - particlesToEmmitInt;
  }
  
  float lowThresh      = 0.125f;
  float highThresh     = 0.65f;

  std::list< Particle* >::iterator itr     = m_particles.begin();
  std::list< Particle* >::iterator itr_end = m_particles.end();
  std::list< Particle* >::iterator itr2;

  // update the flocking routine
  while ( itr != itr_end )
  {
    Particle* p1 = *itr;
        
    if ( p1->m_timeOfDeath <= _currentTime && p1->m_timeOfDeath != -1.0f )
    {
      itr2 = itr;
      ++itr;
      delete p1;
      m_particles.erase( itr2 );
      continue;
    }
         
    itr2 = itr;

    //*
    for( ++itr2; itr2 != itr_end; ++itr2 ) 
    {
      Particle* p2             = *itr2;
      ci::Vec2f dir            = p1->m_position - p2->m_position;
      float     distSqrd       = dir.lengthSquared();
			float     zoneRadiusSqrd = 75.0f * 75.0f;//( p1->m_radius * p2->m_radius * p1->m_radius * p2->m_radius ) * 5.0f;
			
			if( distSqrd < zoneRadiusSqrd ) // Neighbor is in the zone
      {			
				float percent = distSqrd/zoneRadiusSqrd;
	
				if( percent < lowThresh )			// Separation
        {
					float F = ( lowThresh/percent - 1.0f ) * 0.04f;//repelStrength;
					dir = dir.normalized() * F;
			
					p1->m_acceleration += dir;
					p2->m_acceleration -= dir;
				} 
        else if( percent < highThresh ) // Alignment
        {	
					float threshDelta     = highThresh - lowThresh;
					float adjustedPercent	= ( percent - lowThresh )/threshDelta;
					float F               = ( 1.0 - ( cos( adjustedPercent * PI2 ) * -0.5f + 0.5f ) ) * 0.04f;//alignStrength;
					
					p1->m_acceleration += p2->m_direction * F;
					p2->m_acceleration += p1->m_direction * F;
					
				} 
        else 								// Cohesion
        {
					float threshDelta     = 1.0f - highThresh;
					float adjustedPercent	= ( percent - highThresh )/threshDelta;
					float F               = ( 1.0 - ( cos( adjustedPercent * PI2 ) * -0.5f + 0.5f ) ) * 0.02f;//attractStrength;
										
					dir.normalize();
					dir *= F;
			
					p1->m_acceleration -= dir;
					p2->m_acceleration += dir;
				}
			}
    }
    //*/
    
    p1->update( _currentTime, _delta );

    ++itr;
  }
}

void ParticleEmitter::killAll( double _currentTime )
{
  std::list< Particle* >::iterator itr     = m_particles.begin();
  std::list< Particle* >::iterator itr_end = m_particles.end();
  std::list< Particle* >::iterator itr2;

  // update the flocking routine
  while ( itr != itr_end )
  {
    ( *itr )->m_timeOfDeath = _currentTime + 3.0f;
    ++itr;
  }
}