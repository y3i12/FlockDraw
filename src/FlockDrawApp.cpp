#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Filesystem.h"
#include "cinder/app/FileDropEvent.h"
#include "cinder/ip/Resize.h"
#include "ParticleEmitter.h"
#include "SimpleGUI.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mowa::sgui;

#define SGUI_CONFIG_FILE_EXT "cfg"


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class CinderApp : public AppNative 
{
public:
	void setup();
	
  // mouse events
  void mouseDown( ci::app::MouseEvent     _event );	
	void mouseUp(   ci::app::MouseEvent     _event );
  void mouseDrag( ci::app::MouseEvent     _event );
	void fileDrop ( ci::app::FileDropEvent  _event );
  void keyDown(   ci::app::KeyEvent       _event );

  // gui buttons
  bool openImageCallBack( ci::app::MouseEvent _event );
  bool nextImageCallBack( ci::app::MouseEvent _event );
  
	// misc routines
  void updateOutputArea( Vec2i& _imageSize );
  void setImage( fs::path& _path, double _currentTime = 0.0 );

  // main routines
  void update();
	void draw();
  void prepareSettings( Settings *settings );


  // properties
  Surface                     m_surface;
  gl::Texture                 m_texture;
  Area                        m_outputArea;
  ParticleEmitter             m_particleEmitter;
  gl::Fbo                     m_frameBufferObject;
  std::vector< ci::fs::path > m_files;
  double                      m_cycleImageEvery;
  int                         m_particleCount;
  int                         m_particleGroups;
  
  SimpleGUI*                  m_gui;
  ButtonControl*              m_openImageButton;
  ButtonControl*              m_nextImageButton;
  LabelControl*               m_currentImageLabel;
	PanelControl*               m_mainPanel;
	PanelControl*               m_helpPanel;

private:
  double                      m_lastTime;
  double                      m_currentTime;
  double                      m_cycleCounter;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CinderApp::setup()
{
  // config vars
  m_cycleImageEvery = 0.0;
  m_particleCount   = 0;
  m_particleGroups  = 0;

  // buffer for trails
  m_frameBufferObject = gl::Fbo( 800, 600, true );
  m_frameBufferObject.bindFramebuffer();
  gl::enableAlphaBlending();
  gl::clear( Color( 0.0f, 0.0f, 0.0f ) ); 
  m_frameBufferObject.unbindFramebuffer();

  // emitter
  m_particleEmitter.m_maxLifeTime        = 0.0;
  m_particleEmitter.m_minLifeTime        = 10.0;
  m_particleEmitter.m_fadeInTime         = 0.5f;
  m_particleEmitter.m_fadeOutTime        = 3.0f;
  m_particleEmitter.m_referenceSurface   = &m_surface;
  m_particleEmitter.m_screenTexture      = &m_frameBufferObject.getTexture();
  m_particleEmitter.m_particlesPerSecond = 0;

  // GUI
  m_gui             = new SimpleGUI( this );
	m_gui->lightColor = ColorA( 1, 1, 0, 1 );	
  m_gui->textFont   = Font( "Consolas", 12 );
  m_gui->addColumn();
  m_mainPanel = m_gui->addPanel();

  // general settings
  m_gui->addLabel( "General Settings" );
	m_gui->addParam( "Pic. Cycle Time", &m_cycleImageEvery,                       3.0f, 120.0f, 15.0f );
  m_gui->addParam( "Particle Size",   &m_particleEmitter.m_particleSizeRatio,   0.5f,   3.0f,  1.0f );
  m_gui->addParam( "Particle Speed",  &m_particleEmitter.m_particleSpeedRatio,  0.2f,   3.0f,  1.0f );
  m_gui->addParam( "Dampness",        &m_particleEmitter.m_dampness,           0.01f,  0.99f,  0.9f );

#ifdef _DEBUG
  m_gui->addParam( "#Particles", &m_particleCount,   50,   500,    150   );
  m_gui->addParam( "#Groups",    &m_particleGroups,  1,    10,     2   );
#else
  m_gui->addParam( "#Particles", &m_particleCount,   50,   500,    300   );
  m_gui->addParam( "#Groups",    &m_particleGroups,  1,    10,     5   );
#endif
  
	m_gui->addSeparator();
  m_gui->addLabel( "Flocking Settings" );
    
  m_gui->addParam( "Repel Str.",      &m_particleEmitter.m_repelStrength,       0.000f,     0.5f,   0.04f );
  m_gui->addParam( "Align Str.",      &m_particleEmitter.m_alignStrength,       0.000f,     0.5f,   0.04f );
  m_gui->addParam( "Att. Str.",       &m_particleEmitter.m_attractStrength,     0.000f,     0.5f,   0.02f );
  m_gui->addParam( "Grp. Repel Str.", &m_particleEmitter.m_groupRepelStrength,  0.000f,     0.5f,   0.01f );
  m_gui->addParam( "Area Size",       &m_particleEmitter.m_zoneRadiusSqrd,      625.0f, 10000.0f, 5625.0f ),
  m_gui->addParam( "Repel Area",      &m_particleEmitter.m_lowThresh,             0.0f,     1.0f,  0.125f );
  m_gui->addParam( "Align Area",      &m_particleEmitter.m_highThresh,            0.0f,     1.0f,   0.65f );

  m_gui->addSeparator();
  
  m_openImageButton = m_gui->addButton( "Open Image" );
  m_openImageButton->registerClick( this, &CinderApp::openImageCallBack );
  
  m_nextImageButton = m_gui->addButton( "Next Image" );
  m_nextImageButton->registerClick( this, &CinderApp::nextImageCallBack );

  // some info
  m_gui->addSeparator();
  m_gui->addLabel( "Current Image:" );
  m_currentImageLabel = m_gui->addLabel( "" );

  // deserved credits
  m_gui->addSeparator();
  m_gui->addLabel( "y3i12: Yuri Ivatchkovitch" );
  m_gui->addLabel( "http://y3i12.tumblr.com/"  );
  
  // Help!
  m_gui->addColumn();
  m_helpPanel = m_gui->addPanel();
  
  m_gui->addLabel( "Quick Help:"            );
  m_gui->addLabel( "F1  to show/hide help"  );
	m_gui->addLabel( "'h' to show/hide GUI"   );
  m_gui->addLabel( "'s' to save config"     );
  m_gui->addLabel( "'l' to load config"     );
  m_gui->addLabel( "'o' to open image"      );
  m_gui->addLabel( "SPACE to skip image"    );
  m_gui->addLabel( "ESC to quit"            );
  
  m_gui->addSeparator();
  m_gui->addSeparator();

  m_gui->addLabel( "Drag multiple files"    );
  m_gui->addLabel( "to slideshow!"          );
  
  // load images passed via args
  if ( getArgs().size() > 1 )
  {
    const std::vector< std::string >& args = getArgs();

    for ( size_t i = 1; i < args.size(); ++i )
    {
      m_files.push_back( fs::canonical( fs::path( args[ i ] ) ) );
    }

    setImage( m_files.front(), m_currentTime );
    if ( m_files.size() > 1 )
    {
      m_cycleCounter = 0;
    }
  }
  else
  {
    m_cycleCounter  = -1.0;
  }

  // mark the time to test counters  
  m_lastTime = ci::app::getElapsedSeconds();
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::mouseDown( ci::app::MouseEvent _event )
{
  m_gui->onMouseDown( _event );
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::mouseUp(  ci::app::MouseEvent _event )
{
    m_gui->onMouseUp( _event );
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::mouseDrag( ci::app::MouseEvent _event )
{
  m_gui->onMouseDrag( _event );
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::fileDrop (ci::app::FileDropEvent e)
{
  m_files = e.getFiles();
  
  setImage( m_files.front(), m_currentTime );
  
  if ( m_files.size() > 1 )
  {
    m_cycleCounter = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::keyDown( ci::app::KeyEvent _event )
{
  switch( _event.getChar() ) 
  {				
		case 'd': 
      {
        m_gui->dump();
      }
      break; //prints values of all the controls to the console			

		case 'l': 
      {
        std::vector< std::string > theExtensions;
        theExtensions.push_back( SGUI_CONFIG_FILE_EXT );
    
        fs::path aPath = getOpenFilePath( "", theExtensions );  
        if ( !aPath.empty() && fs::is_regular_file( aPath ) )
        {
          m_gui->load( aPath.string() );
        } 
      }
      break;

		case 's': 
      {
        std::vector< std::string > theExtensions;
        theExtensions.push_back( SGUI_CONFIG_FILE_EXT );
    
        fs::path aPath = getSaveFilePath( "", theExtensions );  
        if ( !aPath.empty() )
        {
          if ( aPath.extension() != SGUI_CONFIG_FILE_EXT )
          {
            aPath.replace_extension( SGUI_CONFIG_FILE_EXT );
          }

          m_gui->save( aPath.string() );
        } 
      }
      break;

    case 'h': 
      {
        m_mainPanel->enabled = !m_mainPanel->enabled;
      }

    case 'o':
      {
        openImageCallBack( ci::app::MouseEvent() );
      }
      break;
	}

	switch(_event.getCode()) 
  {
    case KeyEvent::KEY_ESCAPE: 
      {
        quit(); 
      }
      break;

    case KeyEvent::KEY_F1:     
      {
        m_helpPanel->enabled = !m_helpPanel->enabled; 
      }
      break;

    case KeyEvent::KEY_SPACE:  
      {
        nextImageCallBack( ci::app::MouseEvent() );
      }
      break;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool CinderApp::openImageCallBack( ci::app::MouseEvent _event )
{
  std::vector< std::string > theExtensions;
  theExtensions.push_back( "jpg" );
    
  m_cycleCounter  = -1.0;
  m_files.clear();

  fs::path aPath = getOpenFilePath( "", theExtensions );  
  if ( !aPath.empty() && fs::is_regular_file( aPath ) )
  {
    setImage( aPath, m_currentTime );
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool CinderApp::nextImageCallBack( ci::app::MouseEvent _event )
{
  m_cycleCounter = m_cycleImageEvery;
  return false;
}

////////////////////////////////////////////////////////////////////////////////

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

void CinderApp::setImage( fs::path& _path, double _currentTime )
{
  // load the image, resize and set the texture
  ci::Surface imageLoaded = loadImage( _path );
  Vec2i       aSize       = imageLoaded.getSize();
  float       theFactor   = min( static_cast< float >( getWindowSize().x ) / aSize.x, static_cast< float >( getWindowSize().y ) / aSize.y );

  m_surface = ci::Surface( static_cast< int >( aSize.x * theFactor ), static_cast< int >( aSize.y * theFactor ), false );
  ci::ip::resize( imageLoaded, imageLoaded.getBounds(), &m_surface, m_surface.getBounds() );
  m_texture = m_surface;
  
  // update  the image name
  m_currentImageLabel->setText( _path.filename().string() );

  // update the output area
  updateOutputArea( m_surface.getSize() );

  // kill old particles and add new ones
  m_particleEmitter.killAll( _currentTime );

  for ( int i = 0; i < m_particleGroups; ++i )
  {
    m_particleEmitter.addParticles( m_particleCount, i );
  }

  // resets the cycle counter;
  m_cycleCounter = -1.0;
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::update()
{
  m_currentTime = ci::app::getElapsedSeconds();
  double delta  = m_currentTime - m_lastTime;

  if ( m_cycleCounter != -1.0 )
  {
    m_cycleCounter += delta;

    if ( m_cycleCounter >= m_cycleImageEvery && m_files.size() > 1 )
    {
      m_cycleCounter -= m_cycleImageEvery;

      ci::fs::path aPath = m_files.front();
      m_files.erase( m_files.begin() );
      m_files.push_back( aPath );

      setImage( aPath, m_currentTime );
    }
  }

  m_particleEmitter.update( m_currentTime, delta );

  m_lastTime = m_currentTime;
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::draw()
{
	// clear out the window with black and set gl confs
	gl::clear( Color( 0.0f, 0.0f, 0.0f ) );
  gl::disableDepthRead();    
  gl::pushMatrices();
  gl::translate( Vec3f( 0.0f, 0.0f, 0.0f ) );

  // writes backed up frame buffer to the screen
  m_frameBufferObject.blitToScreen( getWindowBounds(), getWindowBounds() );
  
  // darkens the BG
  gl::enableAlphaBlending();
  gl::color( 0.0f, 0.0f, 0.0f, 0.01f ); 
  gl::drawSolidRect( getWindowBounds() );

  // do the drawing =D
  m_particleEmitter.draw();
    
  // save what happened to the framebuffer
  m_frameBufferObject.blitFromScreen( getWindowBounds(), getWindowBounds() );
   
  // reset gl confs
  gl::enableDepthRead();    
  gl::disableAlphaBlending();
  gl::popMatrices();	

  // draw the UI
  m_gui->draw();
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::prepareSettings( Settings *settings )
{
  settings->setWindowSize( 800, 600 );
  settings->setFrameRate( 60.0f );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CINDER_APP_NATIVE( CinderApp, RendererGl )
  
////////////////////////////////////////////////////////////////////////////////

