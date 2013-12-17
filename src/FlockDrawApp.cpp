#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/Texture.h"
#include "cinder/Surface.h"
#include "cinder/gl/Fbo.h"
#include "cinder/Filesystem.h"
#include "cinder/app/FileDropEvent.h"
#include "cinder/ip/Resize.h"
#include "cinder/qtime/MovieWriter.h"
#include "cinder/Utilities.h"
#include "ParticleEmitter.h"
#include "SimpleGUI.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#define SGUI_CONFIG_FILE_EXT "cfg"
#define FRAMERATE 60.0f
#define VIDEO_FRAMERATE 30.0f
#define WINDOWED

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include "FPSCounter.h"

#define DEBUG_DRAW

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class CinderApp : public ci::app::AppNative 
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
  bool openImageCallBack();
  bool nextImageCallBack();
  
	// misc routines
  void updateOutputArea( ci::Vec2i& _imageSize );
  void setImage( ci::fs::path& _path, double _currentTime = 0.0 );

  // main routines
  void update();
	void draw();
  void prepareSettings( ci::app::AppBasic::Settings *settings );


  // properties
  ci::Surface                 m_surface;
  ci::gl::Texture             m_texture;
  ci::Area                    m_outputArea;
  ParticleEmitter             m_particleEmitter;
  ci::gl::Fbo                 m_frameBufferObject;
  std::vector< ci::fs::path > m_files;
  double                      m_cycleImageEvery;
  int                         m_particleCount;
  int                         m_particleGroups;
  
  sgui::SimpleGUI*            m_gui;
  sgui::ButtonControl*        m_openImageButton;
  sgui::ButtonControl*        m_nextImageButton;
  sgui::LabelControl*         m_currentImageLabel;
	sgui::PanelControl*         m_mainPanel;
	sgui::PanelControl*         m_helpPanel;
	sgui::PanelControl*         m_FPSPanel;
  
  ci::fs::path                m_vidPath;
  long                        m_currentFrame;

private:
  double                      m_lastTime;
  double                      m_currentTime;
  double                      m_cycleCounter;

  sgui::LabelControl*         m_fps;
  FPSCounter                  m_fpsCounter;
  FPSCounter                  m_upsCounter;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void CinderApp::setup()
{
  // config vars
  m_cycleImageEvery = 0.0;
  m_particleCount   = 0;
  m_particleGroups  = 0;
  m_currentFrame    = -1;

  // buffer for trails
  ci::Vec2i      displaySz = getWindowSize(); 

  m_frameBufferObject = ci::gl::Fbo( displaySz.x, displaySz.y, true );
  m_frameBufferObject.bindFramebuffer();
  ci::gl::enableAlphaBlending();
  ci::gl::clear( ci::Color( 0.0f, 0.0f, 0.0f ) ); 
  m_frameBufferObject.unbindFramebuffer();

  // emitter
  m_particleEmitter.m_maxLifeTime        = 0.0;
  m_particleEmitter.m_minLifeTime        = 10.0;
  m_particleEmitter.m_referenceSurface   = &m_surface;
  m_particleEmitter.m_screenTexture      = &m_frameBufferObject.getTexture();
  m_particleEmitter.m_particlesPerSecond = 0;

  // GUI
  m_gui             = new sgui::SimpleGUI( this );
	m_gui->lightColor = ci::ColorA( 1, 1, 0, 1 );	
  m_gui->textFont   = ci::Font( "Consolas", 12 );
  m_gui->addColumn();
  m_mainPanel = m_gui->addPanel();

  // general settings
  m_gui->addLabel( "General Settings" );
	m_gui->addParam( "Pic. Cycle Time", &m_cycleImageEvery,               3.0f, 120.0f, 15.0f );
  m_gui->addParam( "Particle Size",   &Particle::s_particleSizeRatio,   0.5f,   3.0f,  1.0f );
  m_gui->addParam( "Particle Speed",  &Particle::s_particleSpeedRatio,  0.2f,   3.0f,  1.0f );
  m_gui->addParam( "Dampness",        &Particle::s_dampness,           0.01f,  0.99f,  0.9f );
  m_gui->addParam( "Color Guidance",  &Particle::s_colorRedirection,    0.0f, 360.0f, 90.0f );

#ifdef WINDOWED
  m_gui->addParam( "#Particles", &m_particleCount,   50, 1000, 500 );
  m_gui->addParam( "#Groups",    &m_particleGroups,   1,   20,   5  );
#else
  int ptcs = std::min< int >( static_cast< int >( displaySz.x * 0.625f ), 1000 );
  m_gui->addParam( "#Particles", &m_particleCount,   50, 1000, ptcs );
  m_gui->addParam( "#Groups",    &m_particleGroups,   1,   20,    5 );
#endif
  
	m_gui->addSeparator();
  m_gui->addLabel( "Flocking Settings" );
    
  m_gui->addParam( "Repel Str.",      &m_particleEmitter.m_repelStrength,       0.000f,     10.0f,   2.0f );
  m_gui->addParam( "Align Str.",      &m_particleEmitter.m_alignStrength,       0.000f,     10.0f,   2.0f );
  m_gui->addParam( "Att. Str.",       &m_particleEmitter.m_attractStrength,     0.000f,     10.0f,   1.0f );
  m_gui->addParam( "Area Size",       &m_particleEmitter.m_zoneRadiusSqrd,      625.0f, 10000.0f, 5625.0f ),
  m_gui->addParam( "Repel Area",      &m_particleEmitter.m_lowThresh,             0.0f,     1.0f,  0.125f );
  m_gui->addParam( "Align Area",      &m_particleEmitter.m_highThresh,            0.0f,     1.0f,   0.65f );

  m_gui->addSeparator();
  
  m_openImageButton = m_gui->addButton( "Open Image" );
  m_openImageButton->registerCallback( this, &CinderApp::openImageCallBack );
  
  m_nextImageButton = m_gui->addButton( "Next Image" );
  m_nextImageButton->registerCallback( this, &CinderApp::nextImageCallBack );

  // some info
  m_gui->addSeparator();
  m_gui->addLabel( "Current Image:" );
  m_currentImageLabel = m_gui->addLabel( "" );

  // deserved credits
  m_gui->addSeparator();
  m_gui->addLabel( "y3i12: Yuri Ivatchkovitch" );
  m_gui->addLabel( "http://y3i12.com/"  );
  
  // Help!
  m_gui->addColumn();
  m_helpPanel = m_gui->addPanel();
  
  m_gui->addLabel( "Quick Help:"              );
  m_gui->addLabel( "F1  to show/hide help"    );
	m_gui->addLabel( "'h' to show/hide GUI"     );
  m_gui->addLabel( "'s' to save config"       );
  m_gui->addLabel( "'l' to load config"       );
  m_gui->addLabel( "'o' to open image"        );
  m_gui->addLabel( "'c' to start/end capture" );
  m_gui->addLabel( "'f' to hide/show fps"     );
  m_gui->addLabel( "SPACE to skip image"      );
  m_gui->addLabel( "ESC to quit"              );
  
  m_gui->addSeparator();
  m_gui->addSeparator();

  m_gui->addLabel( "Drag multiple files"    );
  m_gui->addLabel( "to slideshow!"          );
  
  m_FPSPanel = m_gui->addPanel();
  m_FPSPanel->enabled = false;
  m_gui->addColumn( 620, 5 );
  m_fps = m_gui->addLabel( "" );

  // load images passed via args
  if ( getArgs().size() > 1 )
  {
    const std::vector< std::string >& args = getArgs();

    for ( size_t i = 1; i < args.size(); ++i )
    {
      m_files.push_back( ci::fs::canonical( ci::fs::path( args[ i ] ) ) );
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
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::mouseUp( ci::app::MouseEvent _event )
{
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::mouseDrag( ci::app::MouseEvent _event )
{
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::fileDrop ( ci::app::FileDropEvent _event )
{
  m_files = _event.getFiles();
  
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
#if defined DEBUG_DRAW
		case 'd': 
      {
        ParticleEmitter::s_debugDraw = !ParticleEmitter::s_debugDraw;
        m_particleEmitter.killAll();
        m_particleEmitter.addParticles( 10, 1 );
      }
      break; //prints values of all the controls to the console			
#endif
		case 'l': 
      {
        std::vector< std::string > theExtensions;
        theExtensions.push_back( SGUI_CONFIG_FILE_EXT );
    
        ci::fs::path aPath = getOpenFilePath( "", theExtensions );  
        if ( !aPath.empty() && ci::fs::is_regular_file( aPath ) )
        {
          m_gui->load( aPath.string() );
        } 
      }
      break;

		case 'c': 
      {
        if ( m_currentFrame == -1 )
        {
          m_currentFrame   = 0;
          size_t vidNumber = 0;
          
          while ( true )
          {            
            m_vidPath = ci::getDocumentsDirectory() / ( "FlockDrawCapture_" + ci::toString( vidNumber ) );
           
            if ( !ci::fs::exists( m_vidPath ) )
            {
              ci::fs::create_directories( m_vidPath );
              break;
            }
            ++vidNumber;
          }
          setImage( m_files.front(), m_currentTime );
        }
        else // ends capture
        {
          m_currentFrame = -1;
          setImage( m_files.front(), m_currentTime );
        }
      }
      break;

		case 's': 
      {
        std::vector< std::string > theExtensions;
        theExtensions.push_back( SGUI_CONFIG_FILE_EXT );
    
        ci::fs::path aPath = getSaveFilePath( "", theExtensions );  
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
      break;

    case 'f':
      {
        m_FPSPanel->enabled = !m_FPSPanel->enabled;
      }
      break;
    case 'o':
      {
        openImageCallBack();
      }
      break;
	}

	switch(_event.getCode()) 
  {
  case ci::app::KeyEvent::KEY_ESCAPE: 
    {
      m_particleEmitter.killAll();
      quit(); 
    }
    break;

  case ci::app::KeyEvent::KEY_F1:     
    {
      m_helpPanel->enabled = !m_helpPanel->enabled; 
    }
    break;

  case ci::app::KeyEvent::KEY_SPACE:  
    {
      if ( m_files.size() == 1 )
      {
        setImage( m_files.front(), m_currentTime );
      }
      else
      {
        nextImageCallBack();
      }
    }
    break;
	}
}

////////////////////////////////////////////////////////////////////////////////

bool CinderApp::openImageCallBack()
{
  std::vector< std::string > theExtensions;
  theExtensions.push_back( "jpg" );
    
  m_files.clear();

  ci::fs::path aPath = getOpenFilePath( "", theExtensions );  
  if ( !aPath.empty() && ci::fs::is_regular_file( aPath ) )
  {
    setImage( aPath, m_currentTime );
  }

  m_cycleCounter  = -1.0;
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool CinderApp::nextImageCallBack()
{
  m_cycleCounter = m_cycleImageEvery;
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::updateOutputArea( ci::Vec2i& _imageSize )
{
    m_outputArea.x1 = m_outputArea.x2 = static_cast< int >( getWindowCenter().x );
    m_outputArea.y1 = m_outputArea.y2 = static_cast< int >( getWindowCenter().y );
    
    m_outputArea.x1 -= _imageSize.x / 2;
    m_outputArea.x2 += _imageSize.x / 2;
    m_outputArea.y1 -= _imageSize.y / 2;
    m_outputArea.y2 += _imageSize.y / 2;

    m_particleEmitter.m_position = ci::Vec2f( static_cast< float >( m_outputArea.x1 ), static_cast< float >( m_outputArea.y1 ) );
}

void CinderApp::setImage( ci::fs::path& _path, double _currentTime )
{
  // load the image, resize and set the texture
  ci::Surface imageLoaded = ci::loadImage( _path );
  ci::Vec2i   aSize       = imageLoaded.getSize();
  float       theFactor   = ci::math< float >::min( static_cast< float >( getWindowSize().x ) / aSize.x, static_cast< float >( getWindowSize().y ) / aSize.y );

  m_surface = ci::Surface( static_cast< int >( aSize.x * theFactor ), static_cast< int >( aSize.y * theFactor ), false );
  ci::ip::resize( imageLoaded, imageLoaded.getBounds(), &m_surface, m_surface.getBounds() );
  m_texture = m_surface;
  
  // update  the image name
  m_currentImageLabel->setText( _path.filename().string() );

  // update the output area
  updateOutputArea( m_surface.getSize() );

  // kill old particles and add new ones
  m_particleEmitter.killAll();

  for ( int i = 0; i < m_particleGroups; ++i )
  {
    m_particleEmitter.addParticles( m_particleCount, i );
  }

  // resets the cycle counter;
  m_cycleCounter = 0.0;
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::update()
{
  double delta = 0.0;
  if ( m_currentFrame != -1 ) // capturing video - renders constant framerate
  {
    delta = 1.0 / VIDEO_FRAMERATE;
    m_currentTime += delta;
  }
  else // not capturing, render whatever it goes
  {
    m_currentTime = ci::app::getElapsedSeconds();
    delta         = m_currentTime - m_lastTime;
  }

  if ( m_FPSPanel->enabled )
  {
    m_upsCounter.update();
  }

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
  if ( m_FPSPanel->enabled )
  {
    m_fpsCounter.update();
    if ( m_fpsCounter.m_updated )
    {
      m_fpsCounter.m_updated = false;
      std::ostringstream oss;
      oss << "fps: " << ( m_fpsCounter.get() ) << " / ups: " << ( m_upsCounter.get() );
      m_fps->setText( oss.str() );
    }
  }
  
	// clear out the window with black and set gl confs
	ci::gl::clear( ci::Color( 0.0f, 0.0f, 0.0f ) );
  ci::gl::disableDepthRead();    
  ci::gl::pushMatrices();
  ci::gl::translate( ci::Vec3f( 0.0f, 0.0f, 0.0f ) );

  // writes backed up frame buffer to the screen
  m_frameBufferObject.blitToScreen( getWindowBounds(), getWindowBounds() );
  
  // darkens the BG
  ci::gl::enableAlphaBlending();
  ci::gl::color( 0.0f, 0.0f, 0.0f, 0.01f ); 
  ci::gl::drawSolidRect( getWindowBounds() );

  // do the drawing =D
  m_particleEmitter.draw();

  // save what happened to the framebuffer
  m_frameBufferObject.blitFromScreen( getWindowBounds(), getWindowBounds() );  

  // captures the video
  if ( m_currentFrame != -1 ) 
  {
     ci::writeImage( m_vidPath / ( ci::toString( m_currentFrame ) + ".jpg" ), m_frameBufferObject.getTexture() );        
     m_currentFrame++;
  }

  if ( ParticleEmitter::s_debugDraw )
  {
    m_particleEmitter.debugDraw();
  }
   
  // reset gl confs
  ci::gl::enableDepthRead();    
  ci::gl::disableAlphaBlending();
  ci::gl::popMatrices();	
  
  // draw the UI
  m_gui->draw();
}

////////////////////////////////////////////////////////////////////////////////

void CinderApp::prepareSettings( Settings *settings )
{
#if defined WINDOWED
  settings->setWindowSize( 800, 600 );
#else
  // set the window size and etc.
  ci::DisplayRef display   = settings->getDisplay();
  ci::Vec2i      displaySz = display->getSize(); 

  settings->setWindowSize( displaySz.x, displaySz.y );
  settings->setBorderless( true );
  settings->setFullScreen( true );
#endif
  
  settings->setFrameRate( FRAMERATE );
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CINDER_APP_NATIVE( CinderApp, ci::app::RendererGl )
  
////////////////////////////////////////////////////////////////////////////////

