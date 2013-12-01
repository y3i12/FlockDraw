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


class HodginTourApp : public AppNative 
{
  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
  void prepareSettings( Settings *settings );
  void fileDrop (ci::app::FileDropEvent e);

  Surface                     m_surface;
  gl::Texture                 m_texture;
  Area                        outputArea;
  ParticleEmitter             m_particleEmitter;
  double                      m_lastTime;
  gl::Fbo                     m_frameBufferObject;
  std::vector< ci::fs::path > m_files;
  double                      m_cycleImageEvery;

private:
  double                      m_cycleCounter;
};

void HodginTourApp::fileDrop (ci::app::FileDropEvent e)
{
  m_files        = e.getFiles();
  m_cycleCounter = m_cycleImageEvery;
}

void HodginTourApp::setup()
{
  try
  {
   // fs::path appPath = getAppPath() + fs::path( "/assets" );
    if ( getArgs().size() > 1 )
    {
      const std::vector< std::string >& args = getArgs();

      for ( size_t i = 1; i < args.size(); ++i )
      {
        m_files.push_back( fs::path( args[ i ] ) );
      }

      m_cycleCounter = 0.0;
    }
    else
    {
      m_surface       = loadImage( loadAsset( "DSC_8489.jpg" ) );
      m_cycleCounter  = -1.0;
    }

    m_texture = m_surface;

    Vec2i imageSize = m_surface.getSize();
  
    outputArea.x1 = outputArea.x2 = static_cast< int >( getWindowCenter().x );
    outputArea.y1 = outputArea.y2 = static_cast< int >( getWindowCenter().y );
    
    outputArea.x1 -= imageSize.x / 2;
    outputArea.x2 += imageSize.x / 2;
    outputArea.y1 -= imageSize.y / 2;
    outputArea.y2 += imageSize.y / 2;
  }
  catch ( ... )
  {
    console() << "Unable to load the image" << std::endl;
  }
  
  gl::Fbo::Format hdrFormat;
  hdrFormat.setColorInternalFormat( GL_RGBA32F );
  m_frameBufferObject = gl::Fbo( 800, 600, hdrFormat );
  m_frameBufferObject.bindFramebuffer();
  gl::enableAlphaBlending();
  gl::clear( Color( 0.0f, 0.0f, 0.0f ) ); 
  m_frameBufferObject.unbindFramebuffer();

  m_particleEmitter.m_maxLifeTime        = 0.0;
  m_particleEmitter.m_minLifeTime        = 10.0;
  m_particleEmitter.m_fadeInTime         = 0.5f;
  m_particleEmitter.m_fadeOutTime        = 3.0f;
  m_particleEmitter.m_position           = Vec2f( static_cast< float >( outputArea.x1 ), static_cast< float >( outputArea.y1 ) );
  m_particleEmitter.m_referenceSurface   = &m_surface;
  m_particleEmitter.m_screenTexture      = &m_frameBufferObject.getTexture();
  m_particleEmitter.m_particlesPerSecond = 0;
  
  m_particleEmitter.addParticles( 300 );
#if !defined _DEBUG
  m_particleEmitter.addParticles( 300 );
  m_particleEmitter.addParticles( 300 );
  m_particleEmitter.addParticles( 300 );
  m_particleEmitter.addParticles( 300 );
#endif

  m_lastTime        = ci::app::getElapsedSeconds();
  m_cycleImageEvery = 15.0;
}

void HodginTourApp::mouseDown( MouseEvent event )
{
}

void HodginTourApp::update()
{
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

void HodginTourApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) ); 

  m_frameBufferObject.blitToScreen( getWindowBounds(), getWindowBounds() );

  gl::color( 0.0f, 0.0f, 0.0f, 0.01f );
  gl::drawSolidRect( getWindowBounds() );

  m_particleEmitter.draw();
    
  m_frameBufferObject.blitFromScreen( getWindowBounds(), getWindowBounds() );
}

void HodginTourApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( 800, 600 );
  settings->setFrameRate( 60.0f );
}

CINDER_APP_NATIVE( HodginTourApp, RendererGl )
