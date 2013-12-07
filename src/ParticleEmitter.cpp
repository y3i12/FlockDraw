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
  m_particlesPerSecond( 0.0f ),
  m_zoneRadiusSqrd( 75.0f * 75.0f ),
  m_repelStrength( 0.04f ),
  m_alignStrength( 0.04f ),
  m_attractStrength( 0.02f ),
  m_groupRepelStrength( 0.1f ),
  m_lowThresh( 0.125f ),
  m_highThresh( 0.65f ),
  m_referenceSurface( 0 ),
  m_particlesPerSecondLeftOver( 0.0f )
{
}

ParticleEmitter::~ParticleEmitter(void)
{
}

#define EMISSION_AREA_PERCENTAGE 0.3f
void ParticleEmitter::addParticles( int _aumont, int _group )
{
  ci::Vec2f refSize;
  ci::Area  emissionArea( m_position, m_position );
  
  if ( m_referenceSurface )
  {
    refSize = m_referenceSurface->getSize();
    emissionArea.x1 = static_cast< int >( ci::randFloat( refSize.x - refSize.x * EMISSION_AREA_PERCENTAGE ) );
    emissionArea.y1 = static_cast< int >( ci::randFloat( refSize.y - refSize.y * EMISSION_AREA_PERCENTAGE ) );
    emissionArea.x2 = static_cast< int >( emissionArea.x1 + refSize.x * EMISSION_AREA_PERCENTAGE );
    emissionArea.y2 = static_cast< int >( emissionArea.y1 + refSize.y * EMISSION_AREA_PERCENTAGE );
  }
  
  
  float angle = ci::Rand::randFloat( 0.0f, 2 * PI );

  for ( int i = 0; i < _aumont; ++i )
  {
    
    float angleVar  = ci::Rand::randFloat( 0.0f, 0.8f * PI );
    float u         = sin( angle + angleVar );
    float v         = cos( angle + angleVar );

    Particle* p = 0;
    if ( m_referenceSurface )
    {
      ci::Vec2f pos;
      pos.x = ci::Rand::randFloat( static_cast< float >( emissionArea.x1 ), static_cast< float >( emissionArea.x2 ) );
      pos.y = ci::Rand::randFloat( static_cast< float >( emissionArea.y1 ), static_cast< float >( emissionArea.y2 ) );

      p = new Particle( this, pos, ci::Vec2f( u, v ) );
      p->m_referenceSurface = m_referenceSurface;
    }
    else
    {
      p = new Particle( this, m_position, ci::Vec2f( u, v ) );
    }

    p->m_maxSpeedSquared = ci::Rand::randFloat( 10, 50 );
    p->m_minSpeedSquared = ci::Rand::randFloat( 1, 10 );
    
    p->m_acceleration         = p->m_direction;
    p->m_acceleration.normalize();
    p->m_acceleration        *= 2.5f;
    p->m_group                = _group;
    
    m_particles.push_back( p );
  }
}

void ParticleEmitter::draw( void )
{
  std::vector< Particle* >::iterator itr     = m_particles.begin();
  std::vector< Particle* >::iterator itr_end = m_particles.end();

  for ( ; itr != itr_end; ++itr )
  {
    ( *itr )->draw();
  }
}

void ParticleEmitter::update( double _currentTime, double _delta )
{
  if ( m_particlesPerSecond )
  { 
    float particlesToEmit     = static_cast< float >( _delta ) * m_particlesPerSecond + m_particlesPerSecondLeftOver;
    int   particlesToEmmitInt = static_cast< int >( particlesToEmit );
    
    if ( particlesToEmmitInt )
    {
      addParticles( particlesToEmmitInt );
    }

    m_particlesPerSecondLeftOver = particlesToEmit - particlesToEmmitInt;
  }

  updateParticlesQuadratic( _currentTime, _delta );

}

void ParticleEmitter::updateParticlesQuadratic( double _currentTime, double _delta )
{
  std::vector< Particle* >::iterator itr     = m_particles.begin();
  std::vector< Particle* >::iterator itr_end = m_particles.end();
  std::vector< Particle* >::iterator itr2;
  ci::Vec2f dir;

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
    
    for( ++itr2; itr2 != itr_end; ++itr2 )
    {
      Particle* p2 = *itr2;
      dir = p1->m_position - p2->m_position;
      float distSqrd = dir.lengthSquared();
			
			if ( distSqrd < m_zoneRadiusSqrd ) // Neighbor is in the zone
      {			
				float percent = distSqrd/m_zoneRadiusSqrd;
	      
        if ( p1->m_group == p2->m_group )
        {
				  if( percent < m_lowThresh )			// Separation
          {
					  float F = ( m_lowThresh / percent - 1.0f ) * m_repelStrength;
					  dir = dir.normalized() * F;
			
					  p1->m_acceleration += dir;
					  p2->m_acceleration -= dir;
				  } 
          else if( percent < m_highThresh ) // Alignment
          {	
					  float threshDelta     = m_highThresh - m_lowThresh;
					  float adjustedPercent	= ( percent - m_lowThresh )/threshDelta;
					  float F               = ( 1.0f - ( cos( adjustedPercent * PI2 ) * -0.5f + 0.5f ) ) * m_alignStrength;
					
					  p1->m_acceleration += p2->m_direction * F;
					  p2->m_acceleration += p1->m_direction * F;
					
				  } 
          else 								// Cohesion
          {
					  float threshDelta     = 1.0f - m_highThresh;
					  float adjustedPercent	= ( percent - m_highThresh )/threshDelta;
					  float F               = ( 1.0f - ( cos( adjustedPercent * PI2 ) * -0.5f + 0.5f ) ) * m_attractStrength;
										
					  dir.normalize();
					  dir *= F;
			
					  p1->m_acceleration -= dir;
					  p2->m_acceleration += dir;
				  }
        }
        else
        {
					float F = ( m_highThresh / percent - 1.0f ) * m_groupRepelStrength;
					dir = dir.normalized() * F;
			
					p1->m_acceleration += dir;
					p2->m_acceleration -= dir;
        }
			}
    }
    
    p1->update( _currentTime, _delta );

    ++itr;
  }
}



void ParticleEmitter::killAll( double _currentTime )
{
  std::vector< Particle* >::iterator itr     = m_particles.begin();
  std::vector< Particle* >::iterator itr_end = m_particles.end();
  std::vector< Particle* >::iterator itr2;

  // update the flocking routine
  while ( itr != itr_end )
  {
    ( *itr )->m_timeOfDeath = _currentTime + 3.0f;
    ++itr;
  }
}