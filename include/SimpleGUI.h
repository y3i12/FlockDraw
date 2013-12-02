// MowaLibs
//
// Copyright (c) 2011, Marcin Ignac / marcinignac.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//
// This code is intended to be used with the Cinder C++ library, http://libcinder.org
//
// Temptesta Seven font by Yusuke Kamiyamane http://p.yusukekamiyamane.com/fonts/
// "The fonts can be used free for any personal or commercial projects."

#pragma once

#include <vector>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include "cinder/app/App.h"
#include "cinder/Text.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"

namespace mowa { namespace sgui {
  
//-----------------------------------------------------------------------------
  
class Control;
template < class T > class TemplatedVarControl;
typedef TemplatedVarControl< float  > FloatVarControl;
typedef TemplatedVarControl< double > DoubleVarControl;
typedef TemplatedVarControl< int    > IntVarControl;
class   BoolVarControl;
class   EnumVarControl;
class   ButtonControl;
class   LabelControl;
class   SeparatorControl;
class   ColumnControl;
class   PanelControl;
class   TextureVarControl;
class   ColorVarControl;

template < typename T, unsigned int _size > class VectorVarControl;
typedef VectorVarControl< float, 2 >   Vec2fVarControl;
typedef VectorVarControl< float, 3 >   Vec3fVarControl;
typedef VectorVarControl< float, 4 >   Vec4fVarControl;

  
//-----------------------------------------------------------------------------

class SimpleGUI 
{
private:
  bool enabled;
  ci::Vec2f  mousePos;
  std::vector<Control*> controls;
  Control* selectedControl;
  
  ci::signals::connection  cnMouseDown;
  ci::signals::connection  cnMouseUp;
  ci::signals::connection cnMouseDrag;
  ci::signals::connection cnMouseWheel;

  void  init(ci::app::App* app);
public:
  static ci::ColorA darkColor;
  static ci::ColorA lightColor;
  static ci::ColorA bgColor;
  static ci::ColorA textColor;
  static float spacing;
  static ci::Vec2f padding;
  static ci::Vec2f sliderSize;
  static ci::Vec2f labelSize;
  static ci::Vec2f separatorSize;
  static ci::Font textFont;
  static ci::gl::TextureFontRef textureFont;
  
  enum 
  {
    RGB,
    HSV
  };

public:
  SimpleGUI( ci::app::App* app );
  ~SimpleGUI();

  bool                      isSelected()         { return selectedControl != 0; }
  Control*                  getSelectedControl() { return selectedControl;      }
  std::vector< Control* >&  getControls()        { return controls;             }  
  
  bool  onMouseDown(  ci::app::MouseEvent event );
  bool  onMouseUp(    ci::app::MouseEvent event );
  bool  onMouseDrag(  ci::app::MouseEvent event );
  bool  onMouseWheel( ci::app::MouseEvent event );
  
  void  draw();
  void  dump();
  void  save( std::string fileName = "" );
  void  load( std::string fileName = "" );
  
  bool  isEnabled();
  void  setEnabled( bool state );
  void  removeControl( Control* controlToRemove );


  FloatVarControl*    addParam( const std::string& paramName, float*  var, float  min = 0, float  max = 1, float  defaultValue = 0 );
  DoubleVarControl*   addParam( const std::string& paramName, double* var, double min = 0, double max = 1, double defaultValue = 0 );
  IntVarControl*      addParam( const std::string& paramName, int*    var, int    min = 0, int    max = 1, int    defaultValue = 0 );

  BoolVarControl*     addParam( const std::string& paramName, bool* var, bool   defaultValue = false, int groupId = -1);
  
  EnumVarControl*     addParam( const std::string& paramName, int* var, std::vector< std::string >& enumOptions );
  ColorVarControl*    addParam( const std::string& paramName, ci::ColorA* var, ci::ColorA const defaultValue = ci::ColorA( 0.0f, 1.0f, 1.0f, 1.0f ), int colorModel = RGB );

  Vec2fVarControl*    addParam( const std::string& paramName, float* var, const ci::Vec2f& min = ci::Vec2f::zero(), const ci::Vec2f& max = ci::Vec2f( 1.0f, 1.0f )              );
  Vec3fVarControl*    addParam( const std::string& paramName, float* var, const ci::Vec3f& min = ci::Vec3f::zero(), const ci::Vec3f& max = ci::Vec3f( 1.0f, 1.0f, 1.0f )        );
  Vec4fVarControl*    addParam( const std::string& paramName, float* var, const ci::Vec4f& min = ci::Vec4f::zero(), const ci::Vec4f& max = ci::Vec4f( 1.0f, 1.0f, 1.0f, 1.0f )  );

  TextureVarControl*  addParam( const std::string& paramName, ci::gl::Texture* var, int scale = 1, bool flipVert = false );
  
  ButtonControl*     addButton( const std::string& buttonName );
  LabelControl*      addLabel(  const std::string& labelName  );  
  SeparatorControl*  addSeparator();  
  ColumnControl*     addColumn( int x = 0, int y = 0 );  
  PanelControl*      addPanel();
  
  Control*           getControlByName( const std::string& name );
  
  static ci::Vec2f   getStringSize( const std::string& str );
  static ci::Rectf   getScaledWidthRectf( ci::Rectf rect, float scale );
};
  
//-----------------------------------------------------------------------------
  
  
class Control 
{
public:
  enum Type 
  {
    FLOAT_VAR,
    DOUBLE_VAR,
    INT_VAR,
    BOOL_VAR,
    VECTOR_VAR,
    COLOR_VAR,
    TEXTURE_VAR,
    BUTTON,
    LABEL,
    SEPARATOR,
    COLUMN,
    PANEL
  };
  
  ci::Vec2f       position;
  ci::Rectf       activeArea;
  ci::ColorA      bgColor;
  Type            type;
  std::string     name;
  ci::gl::Texture labelTexture;
  SimpleGUI*      parentGui;
  
  Control( Type name, const std::string& label );  
  virtual ~Control() {};
  void setBackgroundColor( ci::ColorA color );

  void notifyUpdateListeners();
  
  virtual ci::Vec2f draw( ci::Vec2f pos ) = 0;
  
  virtual std::string toString() { return ""; };
  virtual void fromString(std::string& strValue) {};
  
  virtual void onMouseDown(  ci::app::MouseEvent event ) {};
  virtual void onMouseUp(    ci::app::MouseEvent event ) {};
  virtual void onMouseDrag(  ci::app::MouseEvent event ) {};
  virtual void onMouseWheel( ci::app::MouseEvent event ) {};
  
  virtual void updateLabel( const std::string& label );
};


class CallbackControl : public Control 
{
public:
  CallbackControl( Type name, const std::string& label ) : 
    Control( name, label ) 
  {
  }
    
  ci::CallbackId registerCallback( std::function< bool( void ) > callback ) 
  { 
    return callbacks.registerCb( callback ); 
  }

  template < typename T >
  ci::CallbackId registerCallback( T *obj, bool ( T::*callback )( void ) ) 
  { 
    return callbacks.registerCb( std::bind( std::mem_fun( callback ), obj ) ); 
  }

  void unregisterCallback( ci::CallbackId id ) 
  { 
    callbacks.unregisterCb( id ); 
  }
    
  bool triggerCallback();
    
private:
    ci::CallbackMgr< bool( void ) > callbacks;
};
  
//-----------------------------------------------------------------------------

template < class T >
class TemplatedVarControl : public CallbackControl
{
public:  
  T* var;
  T  min;
  T  max;

public:
    TemplatedVarControl(const std::string& name, T* var, T min=0, T max=1, T defaultValue = 0) :
    CallbackControl( getType< T >(), name )
  {
    this->name = name;
    this->var  = var;
    this->min  = min;
    this->max  = max;

    if ( defaultValue < min ) 
    {
      *var = min;
    }
    else if ( defaultValue > max ) 
    {
      *var = max;
    }
    else 
    {
      *var = defaultValue;
    }
  }

  template < class T > Type getType()           { return Control::FLOAT_VAR;  }
  template <         > Type getType< float >()  { return Control::FLOAT_VAR;  }
  template <         > Type getType< double >() { return Control::DOUBLE_VAR; }
  template <         > Type getType< int >()    { return Control::INT_VAR;    }

  float getNormalizedValue()
  {
    return static_cast< float >( *var - min ) / static_cast< float >( max - min );
  }

  void setNormalizedValue(float value)
  {
    *var = static_cast< T >( min + value * ( max - min ) );
  }

  ci::Vec2f draw(ci::Vec2f pos)
  {
    activeArea = Rectf(
      pos.x, 
      pos.y + SimpleGUI::labelSize.y + SimpleGUI::padding.y, 
      pos.x + SimpleGUI::sliderSize.x, 
      pos.y + SimpleGUI::labelSize.y + SimpleGUI::padding.y + SimpleGUI::sliderSize.y
    );    
  
    gl::color( SimpleGUI::bgColor );
    gl::drawSolidRect( Rectf(
      ( pos - SimpleGUI::padding ).x, 
      ( pos - SimpleGUI::padding ).y, 
      ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x, 
      ( pos + SimpleGUI::labelSize + SimpleGUI::sliderSize + SimpleGUI::padding * 2 ).y )
    );  
  
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    gl::draw( labelTexture, pos );
  
    gl::color( SimpleGUI::darkColor );
    gl::drawSolidRect( activeArea );
  
    gl::color( SimpleGUI::lightColor );
    gl::drawSolidRect( SimpleGUI::getScaledWidthRectf( activeArea, getNormalizedValue() ) );
  
    pos.y += SimpleGUI::labelSize.y + SimpleGUI::padding.y + SimpleGUI::sliderSize.y + SimpleGUI::spacing;  
    return pos;
  }

  std::string toString()
  {
    std::stringstream ss;
    ss << *var;
    return ss.str();
  }

  void fromString( std::string& strValue )
  {
    *var = boost::lexical_cast< T >( strValue );
  }

  void onMouseDown( ci::app::MouseEvent event )
  {
    onMouseDrag( event );  
  }

  void onMouseDrag( ci::app::MouseEvent event )
  {
    float value = static_cast< float >( event.getPos().x - activeArea.x1 ) / static_cast< float >( activeArea.x2 - activeArea.x1 );
    value = math< float >::max( 0.0f, math< float >::min( value, 1.0f ) );  
    setNormalizedValue( value );
  }
  
  void onMouseWheel(ci::app::MouseEvent event)
  {
    const float step      = static_cast< float >( ( max - min ) / 100.0f );
    const float delta     = event.getWheelIncrement() * step;
    T           newValue  = static_cast< T >( *var + delta );
    
    *var = math< T >::clamp( newValue, min, max );
    triggerCallback();
    std::stringstream ss;
    ss <<  name << " " << *this->var;
    updateLabel( ss.str() );
}

};
    
//-----------------------------------------------------------------------------

class BoolVarControl : public CallbackControl 
{
public:
  bool* var;
  int   groupId;

public:
  BoolVarControl( const std::string& name, bool* var, bool defaultValue, int groupId );
  ci::Vec2f draw( ci::Vec2f pos );

  std::string toString();  
  void fromString( std::string& strValue );

  void onMouseDown( ci::app::MouseEvent event );
    
};

//-----------------------------------------------------------------------------
//
    
class EnumVarControl : public IntVarControl 
{
public:
  std::vector< std::string > enumOptions;
  std::vector< ci::Rectf   > elementAreas;

public:
  EnumVarControl( const std::string& name, int* var, std::vector< std::string >& options );
  ci::Vec2f draw( ci::Vec2f pos );
  void onMouseDown( ci::app::MouseEvent event );
  void onMouseDrag( ci::app::MouseEvent event );
};
    
//-----------------------------------------------------------------------------

template <typename T, unsigned int _size>
class VectorVarControl : public CallbackControl 
{
public:
    T* var;
    T  max[ _size ];
    T  min[ _size ];
    ci::Rectf  elementArea[ _size ];
    int activeTrack;

public:
  VectorVarControl( const std::string& name, T* var, T min[ _size ], T max[ _size ] );
  ci::Vec2f draw( ci::Vec2f pos );
  
  float getNormalizedValue( const int element );
  void  setNormalizedValue( const int element, const float value, const bool silent = false );
  
  virtual std::string toString();
  virtual void fromString( std::string& strValue );
  
  void onMouseDown(  ci::app::MouseEvent event );  
  void onMouseDrag(  ci::app::MouseEvent event );
  void onMouseWheel( ci::app::MouseEvent event );
};
      
//-----------------------------------------------------------------------------

class ColorVarControl : public CallbackControl 
{
public:
  ci::ColorA* var;
  ci::Rectf  activeArea1;
  ci::Rectf  activeArea2;
  ci::Rectf  activeArea3;
  ci::Rectf  activeArea4;  
  int    activeTrack;
  int    colorModel;

public:
  ColorVarControl( const std::string& name, ci::ColorA* var, ci::ColorA defaultValue, int colorModel );
  
  ci::Vec2f draw(ci::Vec2f pos);
  
  std::string toString();  //saved as "r g b a"
  void fromString( std::string& strValue ); //expecting "r g b a"
  
  void onMouseDown(  ci::app::MouseEvent event );  
  void onMouseDrag(  ci::app::MouseEvent event );
  void onMouseWheel( ci::app::MouseEvent event );
  
  void  setValueForElement( const int element, const float value, const bool silent = false );
  float getValueForElement( const int element );
};

  
//-----------------------------------------------------------------------------

class ButtonControl : public CallbackControl 
{
private:
  bool pressed;

public:
  ButtonControl( const std::string& name );

  ci::Vec2f draw( ci::Vec2f pos );

  void onMouseDown( ci::app::MouseEvent event );
  void onMouseUp(   ci::app::MouseEvent event );
};

//-----------------------------------------------------------------------------
  
class LabelControl : public Control 
{
public:
  LabelControl( const std::string& name );
  void setText( const std::string& text );
  ci::Vec2f draw( ci::Vec2f pos );  
};
  
//-----------------------------------------------------------------------------
  
class SeparatorControl : public Control 
{
public:
  SeparatorControl();
  ci::Vec2f draw( ci::Vec2f pos );  
};

//-----------------------------------------------------------------------------

class ColumnControl : public Control 
{
public:
  int x;
  int y;

public:
  ColumnControl( int x = 0, int y = 0 );
  ci::Vec2f draw( ci::Vec2f pos );  
}; 
  
//-----------------------------------------------------------------------------
 
class PanelControl : public Control 
{
public:
  bool enabled;

public:
  PanelControl();
  ci::Vec2f draw( ci::Vec2f pos );
};
  

//-----------------------------------------------------------------------------

class TextureVarControl : public CallbackControl 
{
public:
  ci::gl::Texture* var;
  float scale;
  bool flipVert;  

public:
  TextureVarControl( const std::string& name, ci::gl::Texture* var, int scale, bool flipVert = false );

  ci::Vec2f draw( ci::Vec2f pos);  
  void resetTexture( ci::gl::Texture* var );
  void onMouseDown( ci::app::MouseEvent event);
};
    
//-----------------------------------------------------------------------------

} //namespace sgui
} //namespace vrg