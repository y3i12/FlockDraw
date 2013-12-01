#include "Particle.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "ParticleEmitter.h"

#define DEG_TO_RAD( x ) ( ( x ) * 0.017453292519943295769236907684886 )

Particle::Particle( ParticleEmitter* _owner, ci::Vec2f& _position, ci::Vec2f& _direction ) :
  m_position( _position ),
  m_direction( _direction ),
  m_color( 1.0f, 1.0f, 1.0f ),
  m_velocity( 0.0f, 0.0f ),
  m_radius( 0.0f ),
  m_maxSpeedSquared( 0.0f ),
  m_minSpeedSquared( 0.0f ),
  m_spawnTime( ci::app::getElapsedSeconds() ),
  m_timeOfDeath( 0.0 ),
  m_fadeInTime( 0.0f ),
  m_fadeOutTime( 0.0f ),
  m_owner( _owner ),
  m_group( -1 ),
  m_fadeFactor( 0.0f ),
  m_maxRadius( 0.0f )
{
}

Particle::~Particle( void )
{
}

void Particle::update( double _currentTime, double _delta )
{
  ci::Vec2f wrapSize = m_referenceSurface->getSize();
  // wrap the particle 
  if ( m_position.x < 0.0f )
  {
    m_position.x += wrapSize.x;
  }
  else if ( m_position.x >= wrapSize.x )
  {
    m_position.x -= wrapSize.x;
  }

  if ( m_position.y < 0.0f )
  {
    m_position.y += wrapSize.y;
  }
  else if ( m_position.y >= wrapSize.y )
  {
    m_position.y -= wrapSize.y;
  }
  
  // update the speed
  m_velocity += m_acceleration;
  m_direction = m_velocity.normalized();
  limitSpeed();
  m_position += m_velocity * _delta * m_owner->m_particleSpeedRatio;
  m_acceleration *= m_owner->m_dampness;


  if ( m_fadeInTime != 0.0f && _currentTime < m_fadeInTime )
  {
    m_fadeFactor = ( _currentTime - m_spawnTime ) / ( m_fadeInTime - m_spawnTime );
  }
  else if ( m_fadeOutTime != 0.0f && _currentTime > m_fadeOutTime ) 
  {
    m_fadeFactor = ( ( m_timeOfDeath - _currentTime ) / ( m_timeOfDeath - m_fadeOutTime ) );
  }
  else 
  {
    m_fadeFactor = 1.0f;
  }

  updateColorByRef();

  //*
  if ( m_referenceSurface )
  {
    ci::Vec2f tempDir = m_direction * 2.0f;
    float     angle   = DEG_TO_RAD( 30 );
    ci::Vec2f nextPos[ 3 ];
    float     l[ 3 ];
    
    nextPos[ 0 ] = m_position + tempDir;
    tempDir.rotate( angle );
    nextPos[ 1 ] = m_position + tempDir;
    tempDir.rotate( angle * -2.0f );
    nextPos[ 2 ] = m_position + tempDir;
    
    for ( int i = 0; i < 3; ++i )
    {
      ci::ColorA c = m_referenceSurface->getPixel( nextPos[ i ] );
      l[ i ] = ( 0.299f * c.r + 0.587f * c.g + 0.114 * c.b );
    }
    
    angle   = DEG_TO_RAD( 1 );
    
    if ( l[ 1 ] < l[ 0 ] )
    {
      m_velocity.rotate( angle );
    }
    else if ( l[ 2 ] < l[ 0 ] )
    {
      m_velocity.rotate( angle * -2.0f );
    }
  }
  //*/
}

void Particle::draw( void )
{
  float alpha = m_color.a;
  m_color.a *= m_fadeFactor;
  ci::gl::color( m_color );
  m_color.a = alpha;

  ci::gl::drawSolidCircle( m_position + m_owner->m_position, m_radius * m_owner->m_particleSizeRatio, 6 );
}


void Particle::setup( )
{
  if ( m_referenceSurface )
  {
    m_maxRadius = m_radius;
    updateColorByRef();
  }
}

void Particle::updateColorByRef()
{
  // random comment
  // dumb people should die
  
  ci::Area sourceArea;
  sourceArea.x1 = m_position.x - m_radius;
  sourceArea.y1 = m_position.y - m_radius;
  sourceArea.x2 = m_position.x + m_radius;
  sourceArea.y2 = m_position.y + m_radius;

  ci::Color c = m_referenceSurface->areaAverage( sourceArea );

  m_color.r = c.r;
  m_color.g = c.g;
  m_color.b = c.b;
  
  m_radius = std::max< float >( 1.5f, m_maxRadius * ( 0.299f * m_color.r + 0.587f * m_color.g + 0.114 * m_color.b ) );
}

void Particle::limitSpeed()
{
	float vLengthSqrd = m_velocity.lengthSquared();

	if ( vLengthSqrd > m_maxSpeedSquared )
  {
    m_velocity = m_direction * m_maxSpeedSquared;
		
	} 
  else if ( vLengthSqrd < m_minSpeedSquared )
  {
    m_velocity = m_direction * m_minSpeedSquared;
	}
}

