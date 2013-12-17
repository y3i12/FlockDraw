#include "Particle.h"
#include "cinder/gl/gl.h"
#include "cinder/app/App.h"
#include "ParticleEmitter.h"

#include <SimpleGUI.h>

#define DEG_TO_RAD( x ) ( ( x ) * 0.017453292519943295769236907684886f )
#define LUMINANCE( r, g, b ) ( 0.299f * ( r ) + 0.587f * ( g ) + 0.114f * ( b ) )

float  Particle::s_maxRadius          = 5.0f;
float  Particle::s_particleSizeRatio  = 1.0f;
float  Particle::s_particleSpeedRatio = 1.0f;
float  Particle::s_dampness           = 0.9f;
float  Particle::s_colorRedirection   = 1.0f;
size_t Particle::s_idGenerator        = 0;

Particle::Particle( ParticleEmitter* _owner, ci::Vec2f& _position, ci::Vec2f& _direction ) :
  m_position( _position ),
  m_stablePosition( _position ),
  m_direction( _direction ),
  m_color( 1.0f, 1.0f, 1.0f ),
  m_velocity( 0.0f, 0.0f ),
  m_maxSpeedSquared( 0.0f ),
  m_minSpeedSquared( 0.0f ),
  m_spawnTime( ci::app::getElapsedSeconds() ),
  m_timeOfDeath( -1.0 ),
  m_owner( _owner ),
  m_group( -1 ),
  m_id( s_idGenerator++ )
{
}

Particle::~Particle( void )
{
}

void Particle::update( double _currentTime, double _delta )
{
  // update the speed
  m_velocity += m_acceleration;
  m_acceleration.set( 0.0f, 0.0f );
  m_direction = m_velocity.normalized();
  limitSpeed();

  // update the position
  m_position += m_velocity * static_cast< float >( _delta ) * Particle::s_particleSpeedRatio;
  m_velocity *= Particle::s_dampness;

  // wrap the particle 
  ci::Vec2f wrapSize = m_referenceSurface->getSize();
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

  if ( m_referenceSurface )
  {
    t_tempDir = m_direction * 2.0f;
    t_angle   = DEG_TO_RAD( 45 );
    t_currentColor = m_referenceSurface->getPixel( m_position );

    t_nextPos[ 0 ] = m_position + t_tempDir;
    t_tempDir.rotate( t_angle );
    t_nextPos[ 1 ] = m_position + t_tempDir;
    t_tempDir.rotate( t_angle * -2.0f );
    t_nextPos[ 2 ] = m_position + t_tempDir;
    

    for ( int i = 0; i < 3; ++i )
    {
      // to guide thru color
      t_c      = t_currentColor - m_referenceSurface->getPixel( t_nextPos[ i ] );
      t_l[ i ] = t_c.lengthSquared();
      
      // to guide thru luminance
      // ci::ColorA c  = m_referenceSurface->getPixel( nextPos[ i ] );
      // l[ i ] = LUMINANCE( c.r, c.g, c.b );
    }
    
    t_angle = DEG_TO_RAD( Particle::s_colorRedirection );
    
    if ( t_l[ 1 ] < t_l[ 0 ] )
    {
      m_velocity.rotate( static_cast< float >( t_angle * _delta ) );
    }
    else if ( t_l[ 2 ] < t_l[ 0 ] )
    {
      m_velocity.rotate( static_cast< float >( t_angle * -2.0f * _delta ) );
    }
  }
}

void Particle::draw( void )
{
  t_sourceArea.x1 = static_cast< int >( m_position.x - Particle::s_maxRadius );
  t_sourceArea.y1 = static_cast< int >( m_position.y - Particle::s_maxRadius );
  t_sourceArea.x2 = static_cast< int >( m_position.x + Particle::s_maxRadius );
  t_sourceArea.y2 = static_cast< int >( m_position.y + Particle::s_maxRadius );

  t_color      = m_referenceSurface->areaAverage( t_sourceArea );
  //t_color      = m_referenceSurface->getPixel( m_position );

  t_color      = m_referenceSurface->getPixel( m_position );
  float radius = ( 1.0f + Particle::s_maxRadius * LUMINANCE( t_color.r, t_color.g, t_color.b ) ) * Particle::s_particleSizeRatio;

  ci::gl::color( t_color );
  ci::gl::drawSolidCircle( m_position + m_owner->m_position, radius );
}

void Particle::debugDraw( void )
{
  if ( ParticleEmitter::s_debugDraw )
  {
    float zoneRadius = sqrt( m_owner->m_zoneRadiusSqrd );
    ci::Vec2f pos    = m_position + m_owner->m_position;

    ci::gl::color( 1.0f, 1.0f, 1.0f, 0.5f );
    ci::gl::drawStrokedCircle( pos, zoneRadius );

    ci::gl::color( 1.0f, 1.0f, 0.0f, 0.5f );
    ci::gl::drawStrokedCircle( pos, zoneRadius * m_owner->m_highThresh );

    ci::gl::color( 1.0f, 0.0f, 1.0f, 0.5f );
    ci::gl::drawStrokedCircle( pos, zoneRadius * m_owner->m_lowThresh );

    ci::gl::color( 1.0f, 1.0f, 1.0f, 1.0f );
    sgui::SimpleGUI::textureFont->drawString( boost::lexical_cast< std::string >( m_id ), pos + ci::Vec2f( 5.0f, 5.0f ) );
  }
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

