#if !defined __FPS_COUNTER_H__
#define      __FPS_COUNTER_H__

#include "cinder/app/App.h"

class FPSCounter
{
public:
  // Constructor
  FPSCounter() : 
    m_fps( 0.0 ), 
    m_fpscount( 0.0 ),
    m_fpsinterval( 0.0 ),
    m_lastTime( -1.0 ),
    m_updated( false )
  {
  }

  // Update
  void update()
  {
    // increase the counter by one
    m_fpscount++;

    if ( m_lastTime == -1.0 )
    {
      m_lastTime = ci::app::getElapsedSeconds();
    }

    double currentTime = ci::app::getElapsedSeconds();
    m_fpsinterval     += currentTime - m_lastTime;
    m_lastTime        = currentTime;

    // one second elapsed? (equals 1000 milliseconds)
    if ( m_fpsinterval > 1.0 )
    {
      // save the current counter value to m_fps
      m_fps         = m_fpscount / m_fpsinterval;

      // reset the counter and the interval
      m_fpscount    = 0.0;
      m_fpsinterval = 0.0;
      m_updated     = true;
    }
  }

  // Get fps
  double get() const
  {
    return m_fps;
  }
  
  bool   m_updated;

protected:
  double m_fps;
  double m_fpscount;
  double m_fpsinterval;
  double m_lastTime;
};

#endif // __FPS_COUNTER_H__
