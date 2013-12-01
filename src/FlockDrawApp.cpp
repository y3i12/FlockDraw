#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Filesystem.h"
#include "ParticleEmitter.h"
#include "cinder/app/FileDropEvent.h"

using namespace ci;
using namespace ci::app;
using namespace std;


class CinderApp : public AppNative 
{
public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
	void prepareSettings( Settings *settings );
	void fileDrop (ci::app::FileDropEvent e);
  void updateOutputArea( Vec2i& _imageSize );

  Surface                     m_surface;
  gl::Texture                 m_texture;
  Area                        m_outputArea;
  ParticleEmitter             m_particleEmitter;
  double                      m_lastTime;
  gl::Fbo                     m_frameBufferObject;
  std::vector< ci::fs::path > m_files;
  double                      m_cycleImageEvery;

private:
  double                      m_cycleCounter;
};

void CinderApp::fileDrop (ci::app::FileDropEvent e)
{
  m_files        = e.getFiles();
  m_cycleCounter = m_cycleImageEvery;
}

void CinderApp::setup()
{
  m_cycleImageEvery = 15.0;

  try
  {
    if ( getArgs().size() > 1 )
    {
      const std::vector< std::string >& args = getArgs();

      for ( size_t i = 1; i < args.size(); ++i )
      {
        m_files.push_back( fs::canonical( fs::path( args[ i ] ) ) );
      }

      m_cycleCounter = m_cycleImageEvery;
    }
    else
    {
      m_cycleCounter  = -1.0;
    }


  }
  catch ( ... )
  {
    console() << "Unable to load the image" << std::endl;
  }
  
  m_frameBufferObject = gl::Fbo( 800, 600, true );
  m_frameBufferObject.bindFramebuffer();
  gl::enableAlphaBlending();
  gl::clear( Color( 0.0f, 0.0f, 0.0f ) ); 
  m_frameBufferObject.unbindFramebuffer();

  m_particleEmitter.m_maxLifeTime        = 0.0;
  m_particleEmitter.m_minLifeTime        = 10.0;
  m_particleEmitter.m_fadeInTime         = 0.5f;
  m_particleEmitter.m_fadeOutTime        = 3.0f;
  m_particleEmitter.m_referenceSurface   = &m_surface;
  m_particleEmitter.m_screenTexture      = &m_frameBufferObject.getTexture();
  m_particleEmitter.m_particlesPerSecond = 0;
  
  m_lastTime        = ci::app::getElapsedSeconds();
}


void CinderApp::mouseDown( MouseEvent event )
{
  m_cycleCounter = m_cycleImageEvery;
}


void CinderApp::updateOutputArea( Vec2i& _imageSize )
{
    m_outputArea.x1 = m_outputArea.x2 = static_cast< int >( getWindowCenter().x );
    m_outputArea.y1 = m_outputArea.y2 = static_cast< int >( getWindowCenter().y );
    
    m_outputArea.x1 -= _imageSize.x / 2;
    m_outputArea.x2 += _imageSize.x / 2;
    m_outputArea.y1 -= _imageSize.y / 2;
    m_outputArea.y2 += _imageSize.y / 2;

    m_particleEmitter.m_position = Vec2f( static_cast< float >( m_outputArea.x1 ), static_cast< float >( m_outputArea.y1 ) );
}

void CinderApp::update()
{
  if ( m_cycleCounter == -1.0 )
  {
    return;
  }

  double currentTime = ci::app::getElapsedSeconds();
  double delta       = currentTime - m_lastTime ;

  if ( m_cycleCounter != -1.0 )
  {
    m_cycleCounter += delta;

    if ( m_cycleCounter >= m_cycleImageEvery )
    {
      m_cycleCounter -= m_cycleImageEvery;

      ci::fs::path aPath = m_files.front();
      m_files.erase( m_files.begin() );
      m_files.push_back( aPath );

      m_surface = loadImage( aPath );
      m_texture = m_surface;

      updateOutputArea( m_surface.getSize() );

      m_particleEmitter.killAll( currentTime );

      m_particleEmitter.addParticles( 300 );
#if !defined _DEBUG
      m_particleEmitter.addParticles( 300 );
      m_particleEmitter.addParticles( 300 );
      m_particleEmitter.addParticles( 300 );
      m_particleEmitter.addParticles( 300 );
#endif
    }
  }

  m_particleEmitter.update( currentTime, delta );

  m_lastTime = currentTime;
  //m_particleEmitter.addParticles( 50 );
}

void CinderApp::draw()
{
  if ( m_cycleCounter == -1.0 )
  {
    return;
  }

	// clear out the window with black
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) ); 

  m_frameBufferObject.blitToScreen( getWindowBounds(), getWindowBounds() );

  gl::color( 0.0f, 0.0f, 0.0f, 0.01f );
  gl::drawSolidRect( getWindowBounds() );

  m_particleEmitter.draw();
    
  m_frameBufferObject.blitFromScreen( getWindowBounds(), getWindowBounds() );
}

void CinderApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( 800, 600 );
  settings->setFrameRate( 60.0f );
}

CINDER_APP_NATIVE( CinderApp, RendererGl )
