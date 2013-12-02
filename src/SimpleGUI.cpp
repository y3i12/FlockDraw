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

#include <iostream>
#include <sstream>
#include <fstream>
#include <boost/algorithm/string.hpp>
#include "SimpleGUI.h"
#include "cinder/Utilities.h"
#include "cinder/Font.h"
#include "cinder/CinderMath.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace mowa { namespace sgui {
  
//-----------------------------------------------------------------------------

Font                SimpleGUI::textFont      = Font();
gl::TextureFontRef  SimpleGUI::textureFont;
ColorA              SimpleGUI::darkColor     = ColorA(0.3f, 0.3f, 0.3f, 1.0f);
ColorA              SimpleGUI::lightColor    = ColorA(1.0f, 1.0f, 1.0f, 1.0f);
ColorA              SimpleGUI::bgColor       = ColorA(0.0f, 0.0f, 0.0f, 0.5f);
ColorA              SimpleGUI::textColor     = ColorA(1.0f,1.0f,1.0f,1.0f);  
float               SimpleGUI::spacing       = 7.0f;
Vec2f               SimpleGUI::padding       = Vec2f(3.0f, 3.0f);
Vec2f               SimpleGUI::sliderSize    = Vec2f(125.0f, 10.0f);
Vec2f               SimpleGUI::labelSize     = Vec2f(125.0f, 10.0f);
Vec2f               SimpleGUI::separatorSize = Vec2f(125.0f, 1.0f);
  
SimpleGUI::SimpleGUI( App* app ) 
{
  init( app );
  enabled = true;
}

SimpleGUI::~SimpleGUI() 
{
  console() << "Removing gui " << std::endl;
  
  selectedControl = 0;
  
  cnMouseDown.disconnect();
  cnMouseUp.disconnect();
  cnMouseDrag.disconnect();
  cnMouseWheel.disconnect();
    
  for ( size_t i = 0; i < controls.size(); ++i )
  {
      delete controls[ i ];
  }
}
  
void SimpleGUI::init(App* app) 
{  
  textFont               = Font("PF Tempesta Seven",8);
  SimpleGUI::textureFont = ci::gl::TextureFont::create( textFont );
  selectedControl        = NULL;

  cnMouseDown   = app->getWindow()->getSignalMouseDown().connect(   std::bind( &SimpleGUI::onMouseDown,  this, std::_1 ) );
  cnMouseUp     = app->getWindow()->getSignalMouseUp().connect(     std::bind( &SimpleGUI::onMouseUp,    this, std::_1 ) );
  cnMouseDrag   = app->getWindow()->getSignalMouseDrag().connect(   std::bind( &SimpleGUI::onMouseDrag,  this, std::_1 ) );
  cnMouseWheel  = app->getWindow()->getSignalMouseWheel().connect(  std::bind( &SimpleGUI::onMouseWheel, this, std::_1 ) );
}

FloatVarControl* SimpleGUI::addParam( const std::string& paramName, float* var, float min, float max, float defaultValue ) 
{
  FloatVarControl* control = new FloatVarControl( paramName, var, min, max, defaultValue );
  control->parentGui = this;
  controls.push_back( control );

  return control;
}
    
DoubleVarControl* SimpleGUI::addParam(const std::string& paramName, double* var, double min, double max, double defaultValue)
{
  DoubleVarControl* control = new DoubleVarControl( paramName, var, min, max, defaultValue );
  control->parentGui = this;
  controls.push_back( control );

  return control;
}

IntVarControl* SimpleGUI::addParam( const std::string& paramName, int* var, int min, int max, int defaultValue ) 
{
  IntVarControl* control = new IntVarControl( paramName, var, min, max, defaultValue );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

BoolVarControl* SimpleGUI::addParam( const std::string& paramName, bool* var, bool defaultValue, int groupId ) 
{
  BoolVarControl* control = new BoolVarControl( paramName, var, defaultValue, groupId );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}
    
EnumVarControl*  SimpleGUI::addParam( const std::string& paramName, int* var, std::vector<std::string>& enumOptions )
{
  EnumVarControl* control = new EnumVarControl( paramName, var, enumOptions );
  control->parentGui = this;
  controls.push_back( control );

  return control;
}
  
ColorVarControl* SimpleGUI::addParam( const std::string& paramName, ColorA* var, ColorA defaultValue, int colorModel ) 
{
  ColorVarControl* control = new ColorVarControl( paramName, var, defaultValue, colorModel );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

Vec2fVarControl* SimpleGUI::addParam( const std::string& paramName, float* var, const Vec2f& min, const Vec2f& max ) 
{
  float _min[ 2 ];
  _min[ 0 ] = min.x;
  _min[ 1 ] = min.y;

  float _max[ 2 ];
  _max[ 0 ] = max.x;
  _max[ 1 ] = max.y;

  Vec2fVarControl* control = new Vec2fVarControl( paramName, var, _min, _max );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

Vec3fVarControl* SimpleGUI::addParam( const std::string& paramName, float* var, const Vec3f& min, const Vec3f& max ) 
{
  float _min[ 3 ];
  _min[ 0 ] = min.x;
  _min[ 1 ] = min.y;
  _min[ 2 ] = min.z;

  float _max[ 3 ];
  _max[ 0 ] = max.x;
  _max[ 1 ] = max.y;
  _max[ 2 ] = max.z;

  Vec3fVarControl* control = new Vec3fVarControl( paramName, var, _min, _max );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

Vec4fVarControl* SimpleGUI::addParam( const std::string& paramName, float* var, const Vec4f& min, const Vec4f& max ) 
{
  float _min[ 4 ];
  _min[ 0 ] = min.x;
  _min[ 1 ] = min.y;
  _min[ 2 ] = min.z;
  _min[ 3 ] = min.w;

  float _max[ 4 ];
  _max[ 0 ] = max.x;
  _max[ 1 ] = max.y;
  _max[ 2 ] = max.z;
  _max[ 3 ] = max.w;

  Vec4fVarControl* control = new Vec4fVarControl( paramName, var, _min, _max );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

TextureVarControl* SimpleGUI::addParam( const std::string& paramName, gl::Texture* var, int scale, bool flipVert ) 
{
  TextureVarControl* control = new TextureVarControl( paramName, var, scale, flipVert );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

ButtonControl* SimpleGUI::addButton( const std::string& buttonName ) 
{
  ButtonControl* control = new ButtonControl( buttonName );
  control->parentGui = this;
  controls.push_back( control );

  return control;
}
  
LabelControl* SimpleGUI::addLabel( const std::string& labelName ) 
{
  LabelControl* control = new LabelControl( labelName );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

SeparatorControl* SimpleGUI::addSeparator() 
{
  SeparatorControl* control = new SeparatorControl();
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}
  
ColumnControl* SimpleGUI::addColumn( int x, int y ) 
{
  ColumnControl* control = new ColumnControl( x, y );
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

PanelControl* SimpleGUI::addPanel() 
{
  PanelControl* control = new PanelControl();
  control->parentGui = this;  
  controls.push_back( control );

  return control;
}

void SimpleGUI::removeControl( Control* controlToRemove ) 
{
  std::vector< Control* >::iterator it = controls.begin();
  while( it!= controls.end() ) 
  {
    Control* control = *it;
    if( control == controlToRemove ) 
    {
      controls.erase( it );
      return;
    }
    ++it;
  }
}
  
void SimpleGUI::draw() 
{  
  if (!enabled) return;

  gl::pushMatrices();
  gl::setMatricesWindow(getWindowSize());
  gl::disableDepthRead();
  gl::disableDepthWrite();
  gl::enableAlphaBlending();

  Vec2f position = Vec2f(spacing, spacing);
  
  ColumnControl* currColumn = 0;
  PanelControl*  currPanel  = 0;
  
  std::vector<Control*>::iterator it     = controls.begin();
  std::vector<Control*>::iterator it_end = controls.end();

  while ( it != it_end ) 
  {
    Control* control = *it++;

    if ( control->type == Control::PANEL ) 
    {
      currPanel = ( PanelControl* )control;      
    }    

    if ( control->type == Control::COLUMN ) 
    {
      if ( currColumn == NULL ) //first column        
      { 
        position.x = -SimpleGUI::labelSize.x;
        //each column moves everything to the right so we want compensate that
        position.y = 0;
      }

      currColumn = ( ColumnControl* )control;
      currPanel  = NULL;
    }

    if ( currPanel != NULL && !currPanel->enabled ) 
    {
      continue;
    }

    position = control->draw( position );
  }

  gl::disableAlphaBlending();
  gl::enableDepthRead();
  gl::enableDepthWrite();
  gl::color( ColorA( 1, 1, 1, 1 ) );
  gl::popMatrices();
}
  
bool SimpleGUI::isEnabled() 
{
  return enabled;
}

void SimpleGUI::setEnabled(bool state) 
{
  enabled = state;
}

void SimpleGUI::dump() 
{
  std::vector<Control*>::iterator it     = controls.begin();
  std::vector<Control*>::iterator it_end = controls.end();

  while ( it != it_end )  
  {
    Control* control = *it++;    
    std::string str = control->toString();
    console() << control->name << " : " + str << std::endl;
  }
}
  
void SimpleGUI::save( std::string fileName ) 
{
  console() << "SimpleGUI::save " << fileName << std::endl;

  std::fstream file_op(fileName.c_str(), std::ios::out);  
  
  std::vector<Control*>::iterator it     = controls.begin();
  std::vector<Control*>::iterator it_end = controls.end();

  while ( it != it_end ) 
  {
    Control* control = *it++;    
    std::string str = control->toString();
    file_op << control->name << " : " + str << std::endl;
  }
  file_op.close();
}

void SimpleGUI::load( std::string fileName ) 
{
  console() << "SimpleGUI::load " << fileName << std::endl;

  std::fstream file_op(fileName.c_str(), std::ios::in);

  char str[2000];

  if ( ( file_op.rdstate() & std::ifstream::failbit ) != 0 ) 
  {
    console() << "SimpleGUI::load " << "failed" << std::endl;
    return;
  }

  while ( !file_op.eof() ) 
  {
    file_op.getline( str, 2000 );

    std::string line     = str;
    size_t      colonPos = line.find( ":" );
    
    if ( colonPos == std::string::npos ) 
    {
      continue;
    }
    
    std::string name  = line.substr( 0, colonPos );
    std::string value = line.substr( colonPos + 1 );
    
    boost::trim( name  );
    boost::trim( value );
    
    Control* control = getControlByName( name );
    if ( control ) 
    {
      control->fromString( value );
    }    
  }    
  file_op.close();
}
  

bool SimpleGUI::onMouseDown( MouseEvent event ) 
{
  if ( !enabled ) return false;
  
  std::vector<Control*>::iterator it     = controls.begin();
  std::vector<Control*>::iterator it_end = controls.end();

  while ( it != it_end )  
  {
    Control* control = *it;

    if ( control ) 
    {
      if ( control->activeArea.contains( event.getPos() ) ) 
      {
        selectedControl = control;
        selectedControl->onMouseDown( event );
        return true;
      }
    }
    it++;
  }  

  return false;
}

bool SimpleGUI::onMouseUp( MouseEvent event ) 
{
  if ( !enabled ) return false;
  
  if ( selectedControl != NULL ) 
  {
    selectedControl->onMouseUp( event );
    selectedControl = NULL;
    return true;
  }  
  return false;
}
  
bool SimpleGUI::onMouseDrag( MouseEvent event ) 
{
  if ( !enabled ) return false;
  
  mousePos = event.getPos();
  
  if ( selectedControl ) 
  {
    selectedControl->onMouseDrag( event );
    return true;
  }
  return false;
}
    
bool SimpleGUI::onMouseWheel( MouseEvent event ) 
{
  if ( !enabled ) return false;
    
  std::vector<Control*>::iterator it     = controls.begin();
  std::vector<Control*>::iterator it_end = controls.end();

  while ( it != it_end )   
  {
    Control* control = *it;

    if( control ) 
    {
      if ( control->activeArea.contains( event.getPos() ) ) 
      {
        control->onMouseWheel( event );
        return true;
      }
    }
    it++;
  }

  return false;
}
  
Vec2f SimpleGUI::getStringSize( const std::string& str ) 
{
  TextLayout text;
  text.addLine( str );
  Surface s = text.render( true );
  return s.getSize();
}
  
Rectf SimpleGUI::getScaledWidthRectf( Rectf rect, float scale ) 
{
  return Rectf( rect.getX1(), rect.getY1(), rect.getX1() + rect.getWidth() * scale, rect.getY1() + rect.getHeight() );
}
  
Control* SimpleGUI::getControlByName( const std::string& name ) 
{
  std::vector<Control*>::iterator it     = controls.begin();
  std::vector<Control*>::iterator it_end = controls.end();

  while ( it != it_end )   
  {
    Control* control = *it++;

    if ( control->name == name ) 
    {
      return control;
    }    
  }

  return NULL;
}



//-----------------------------------------------------------------------------
  
Control::Control( Control::Type _type, const std::string& _name ) : 
  type( _type ),
  name( _name )
{
  bgColor     = ColorA( 0,0, 0, 0.5 );
  activeArea  = Rectf( 0.0f, 0.0f, 0.0f, 0.0f );
  updateLabel( name );
}

void Control::setBackgroundColor( ColorA color ) 
{
  bgColor = color;
}
    
    
bool CallbackControl::triggerCallback() 
{
  CallbackMgr< bool( void ) >::iterator itr     = callbacks.begin();
  CallbackMgr< bool( void ) >::iterator itr_end = callbacks.begin();

  for( ; itr != itr_end; ++itr ) 
  {
    if ( ( itr->second )( ) )
    {
      return true;
    }
  }

  return false;
}
    
void Control::updateLabel( const std::string& label )
{
    TextLayout layout;
    
    layout.clear( ColorA( 0.0f, 0.0f, 0.0f, 0.0f ) );
    layout.setFont(  SimpleGUI::textFont  );
    layout.setColor( SimpleGUI::textColor );
    layout.setLeadingOffset( -2.0f );
    layout.addLine( label );
    
    labelTexture = gl::Texture( layout.render( true, false ) );
}
  
//-----------------------------------------------------------------------------

  
BoolVarControl::BoolVarControl( const std::string& name, bool* var, bool defaultValue, int groupId ) :
  CallbackControl( Control::BOOL_VAR, name )
{
  this->var     = var;
  this->groupId = groupId;
  *var          = defaultValue;
}  

Vec2f BoolVarControl::draw(Vec2f pos) 
{
  activeArea = Rectf( pos.x, pos.y, pos.x + SimpleGUI::sliderSize.y, pos.y + SimpleGUI::sliderSize.y );
  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( Rectf(
    ( pos - SimpleGUI::padding ).x, 
    ( pos - SimpleGUI::padding ).y, 
    ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x, 
    ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).y )
  );
  
  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  gl::draw( labelTexture, Vec2f( pos.x + SimpleGUI::sliderSize.y + SimpleGUI::padding.x * 2, pos.y) );

  gl::color( ( *var ) ? SimpleGUI::lightColor : SimpleGUI::darkColor );

  gl::drawSolidRect( activeArea );

  pos.y += SimpleGUI::sliderSize.y + SimpleGUI::spacing;  

  return pos;
}  
  
std::string BoolVarControl::toString() 
{
  std::stringstream ss;
  ss << *var;
  return ss.str();  
}

void BoolVarControl::fromString( std::string& strValue ) 
{
  int value = boost::lexical_cast< int >( strValue );  
  *var = value ? true : false;
  triggerCallback();
}
  
void BoolVarControl::onMouseDown( MouseEvent event ) 
{
  if ( groupId > -1 ) 
  {
    std::vector< Control* >& parentControls = parentGui->getControls();
    
    for ( size_t i = 0; i < parentControls.size(); ++i ) 
    {
      Control* control = parentControls[ i ];

      if ( ( control->type == Control::BOOL_VAR ) && ( static_cast< BoolVarControl* >( control )->groupId == this->groupId ) ) 
      {
        *static_cast< BoolVarControl* >( control )->var = ( control == this );
      }            
    }
  }
  else 
  {
    *this->var = !( *this->var );
  }

  triggerCallback();
}
    
//-----------------------------------------------------------------------------

EnumVarControl::EnumVarControl( const std::string& name, int* var, std::vector< std::string >& options ) :
  IntVarControl( name, var, 0, options.size() -1, 0),
  enumOptions( options )
{
  for ( size_t i = 0; i < options.size(); ++i ) 
  {
    elementAreas.push_back( Rectf( 0, 0, 1, 1 ) );
  }
}
    
Vec2f EnumVarControl::draw( Vec2f pos ) 
{
  // setup
  float height  = ( enumOptions.size() + 1 ) * ( SimpleGUI::sliderSize.y + SimpleGUI::padding.y );
  Vec2f enumPos = pos;
  activeArea    = Rectf( pos.x, pos.y, pos.x + SimpleGUI::sliderSize.x, pos.y + height );
  enumPos.y    += SimpleGUI::labelSize.y + SimpleGUI::padding.y;

  for ( size_t i = 0; i < enumOptions.size(); ++i ) 
  {
    elementAreas[ i ] = Rectf( enumPos.x,
                                enumPos.y,
                                enumPos.x + SimpleGUI::sliderSize.y,
                                enumPos.y + SimpleGUI::sliderSize.y );

    enumPos.y += SimpleGUI::sliderSize.y + SimpleGUI::padding.y;
  }
    
  // draw label + bg
  Rectf bgRect = Rectf(
                        ( pos - SimpleGUI::padding ).x,
                        ( pos - SimpleGUI::padding ).y,
                        ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x,
                        ( pos + SimpleGUI::labelSize + SimpleGUI::sliderSize*enumOptions.size() + SimpleGUI::padding * ( enumOptions.size() + 1 ) ).y );

  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( bgRect );
    
  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  gl::draw( labelTexture, pos );
    
  // draw options
  for ( size_t i = 0; i < enumOptions.size(); ++i ) 
  {
    gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
    SimpleGUI::textureFont->drawString( enumOptions[ i ], Vec2f( elementAreas[ i ].x1 + SimpleGUI::sliderSize.y + SimpleGUI::padding.x * 2, elementAreas[ i ].y2 - SimpleGUI::padding.y ) );
        
    gl::color( ( *var == i ) ? SimpleGUI::lightColor : SimpleGUI::darkColor );
    gl::drawSolidRect( elementAreas[ i ] );
  }

  pos.y += height + SimpleGUI::spacing;

  return pos;
}
    
void EnumVarControl::onMouseDown( MouseEvent event ) 
{
  for ( size_t i = 0; i < enumOptions.size(); ++i ) 
  {
    if ( elementAreas[ i ].contains( event.getPos( ) ) ) 
    {
      *this->var = i;
      triggerCallback();
      std::stringstream ss;
      ss <<  name << " " << *this->var;
      updateLabel( ss.str() );
      break;
    }
  }
}
    
void EnumVarControl::onMouseDrag( MouseEvent event ) 
{
  return;
}
    
//-----------------------------------------------------------------------------

template < typename T, unsigned int _size >
VectorVarControl< T, _size >::VectorVarControl( const std::string& name, T* var, T min[ _size ], T max[ _size ] ) :
  CallbackControl( Control::VECTOR_VAR, name )
{
  this->var = var;
  for ( int i=0; i < _size; ++i )
  {
    this->min[ i ] = min[ i ];
    this->max[ i ] = max[ i ];
  }
  activeTrack = 0;
}
    
template < typename T, unsigned int _size >
float VectorVarControl< T, _size >::getNormalizedValue(const int element) 
{
  assert( element < _size );
  return static_cast< float >( var[ element ] - min[ element ] ) / static_cast< float >( max[ element ] - min[ element ] );
}

template < typename T, unsigned int _size >
void VectorVarControl< T, _size >::setNormalizedValue( const int element, const float value, const bool silent ) 
{
  T newValue = min[ element ] + value * ( max[ element ] - min[ element ] );

  if ( newValue != var[ element ] ) 
  {
    var[ element ] = newValue;

    if ( silent == false )
    {
        triggerCallback();
    }
  }
}

template < typename T, unsigned int _size >
Vec2f VectorVarControl< T, _size >::draw(Vec2f pos) 
{
  for ( int i = 0; i < _size; ++i )
  {
    int count = i + 1;
    elementArea[ i ] = Rectf(
                              pos.x, 
                              pos.y + SimpleGUI::labelSize.y + SimpleGUI::sliderSize.y*i + SimpleGUI::padding.y*count, 
                              pos.x + SimpleGUI::sliderSize.x, 
                              pos.y + SimpleGUI::sliderSize.y*count + SimpleGUI::padding.y*count + SimpleGUI::labelSize.y
                            );
  }
    
  activeArea = Rectf(
                      pos.x, 
                      pos.y + SimpleGUI::labelSize.y, 
                      pos.x + SimpleGUI::sliderSize.x, 
                      pos.y + SimpleGUI::sliderSize.y*_size + SimpleGUI::padding.y*_size + SimpleGUI::labelSize.y
                      );
    
  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( Rectf(
                          ( pos - SimpleGUI::padding ).x, 
                          ( pos - SimpleGUI::padding ).y, 
                          ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x, 
                          ( pos + SimpleGUI::labelSize + SimpleGUI::sliderSize * _size + SimpleGUI::padding * ( _size + 1 ) ).y )
                    );
    
  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  gl::draw( labelTexture, pos );
  gl::color( SimpleGUI::darkColor );

  for ( int i=0; i < _size; ++i )
  {
    gl::drawSolidRect( elementArea[ i ] );
  }

  gl::color( SimpleGUI::lightColor );
  for ( int i = 0; i < _size; ++i )
  {
    Rectf rect = SimpleGUI::getScaledWidthRectf( elementArea[ i ], getNormalizedValue( i ) );
    gl::drawLine( Vec2f( rect.x2, rect.y1 ), Vec2f( rect.x2, rect.y2 ) );  
  }    

  pos.y += SimpleGUI::labelSize.y + SimpleGUI::padding.y + SimpleGUI::sliderSize.y * _size + SimpleGUI::padding.y * ( _size - 1 ) + SimpleGUI::spacing;    

  return pos;
}

template < typename T, unsigned int _size >
std::string VectorVarControl< T, _size >::toString() 
{
  std::stringstream ss;
  for ( int i = 0; i < _size; ++i )
  {
    ss << var[ i ];
    if( i < (_size-1) ) ss << " ";
  }
  return ss.str();
}

template < typename T, unsigned int _size >
void VectorVarControl< T, _size >::fromString( std::string& strValue ) 
{
  std::vector< std::string > strs;
  boost::split( strs, strValue, boost::is_any_of ("\t " ) );

  for ( int i = 0; i < _size; ++i )
  {
    var[ i ] = static_cast< float >( boost::lexical_cast< double >( strs[ i ] ) );
  }
}

template < typename T, unsigned int _size >
void VectorVarControl<T,_size>::onMouseDown( MouseEvent event ) 
{  
  for ( int i = 0; i < _size; ++i )
  {
    if ( elementArea[ i ].contains( event.getPos() ) ) 
    {
      activeTrack = i;
      break;
    }
  }

  onMouseDrag( event );
}

template < typename T, unsigned int _size >
void VectorVarControl<T,_size>::onMouseDrag( MouseEvent event) 
{  
    float value = ( event.getPos().x - activeArea.x1 ) / ( activeArea.x2 - activeArea.x1 );
    value = math< float >::max( 0.0, math< float >::min(value, 1.0 ) );  
    setNormalizedValue( activeTrack, value );
}
    
template < typename T, unsigned int _size >
void VectorVarControl<T,_size>::onMouseWheel( MouseEvent event ) 
{
  for ( int i = 0; i < _size; ++i )
  {
    if ( elementArea[i].contains( event.getPos() ) )
    {
      activeTrack = i;
      break;
    }
  }

  const float step     = ( max[ activeTrack ] - min[ activeTrack ] ) / 100.0f;
  const float delta    = event.getWheelIncrement() * step;
  float       newValue = var[ activeTrack ] + delta;
  
  var[ activeTrack ] = math< float >::clamp( newValue, min[ activeTrack ],max[ activeTrack ] );

  if ( var[ activeTrack ] == newValue )
  {
    triggerCallback();
  }
}
    
template class VectorVarControl< float, 2 >;
template class VectorVarControl< float, 3 >;
template class VectorVarControl< float, 4 >;
  
//-----------------------------------------------------------------------------

ColorVarControl::ColorVarControl( const std::string& name, ColorA* var, ColorA defaultValue, int colorModel ) :
  CallbackControl( Control::COLOR_VAR, name )
{
  this->var = var;
  this->colorModel = colorModel;
  activeTrack = 0;
}
  
Vec2f ColorVarControl::draw( Vec2f pos ) 
{
  activeArea1 = Rectf(
    pos.x, 
    pos.y + SimpleGUI::labelSize.y + SimpleGUI::padding.y, 
    pos.x + SimpleGUI::sliderSize.x, 
    pos.y + SimpleGUI::sliderSize.y + SimpleGUI::labelSize.y + SimpleGUI::padding.y
  );  
  
  activeArea2 = Rectf(
    pos.x, 
    pos.y + SimpleGUI::labelSize.y + SimpleGUI::sliderSize.y + SimpleGUI::padding.y * 2, 
    pos.x + SimpleGUI::sliderSize.x, 
    pos.y + SimpleGUI::sliderSize.y * 2 + SimpleGUI::padding.y*2 + SimpleGUI::labelSize.y
  );  
  
  activeArea3 = Rectf(
    pos.x, 
    pos.y + SimpleGUI::labelSize.y + SimpleGUI::sliderSize.y * 2 + SimpleGUI::padding.y * 3, 
    pos.x + SimpleGUI::sliderSize.x, 
    pos.y + SimpleGUI::sliderSize.y * 3 + SimpleGUI::padding.y * 3 + SimpleGUI::labelSize.y
  );
  
  activeArea4 = Rectf(
    pos.x, 
    pos.y + SimpleGUI::labelSize.y + SimpleGUI::sliderSize.y * 3 + SimpleGUI::padding.y * 4, 
    pos.x + SimpleGUI::sliderSize.x, 
    pos.y + SimpleGUI::sliderSize.y * 4 + SimpleGUI::padding.y * 4 + SimpleGUI::labelSize.y
  );
  
  activeArea = Rectf(
    pos.x, 
    pos.y + SimpleGUI::labelSize.y, 
    pos.x + SimpleGUI::sliderSize.x, 
    pos.y + SimpleGUI::sliderSize.y * 4 + SimpleGUI::padding.y * 4 + SimpleGUI::labelSize.y
  );  
  
  Vec4f values;
  if ( colorModel == SimpleGUI::RGB ) 
  {
    values = Vec4f( var->r, var->g, var->b, var->a );
  }
  else 
  {
    Vec3f hsv = rgbToHSV( *var );
    values.x = hsv.x;
    values.y = hsv.y;
    values.z = hsv.z;
    values.w = var->a;    
  }  

  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( Rectf(
    ( pos - SimpleGUI::padding ).x, 
    ( pos - SimpleGUI::padding ).y, 
    ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x, 
    ( pos + SimpleGUI::labelSize + SimpleGUI::sliderSize * 4 + SimpleGUI::padding * 5 ).y )
  );
  
  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  gl::draw( labelTexture, pos );
  
  gl::color( SimpleGUI::darkColor );
  gl::drawSolidRect( activeArea1 );
  gl::drawSolidRect( activeArea2 );
  gl::drawSolidRect( activeArea3 );
  gl::drawSolidRect( activeArea4 );

  Rectf rect1 = SimpleGUI::getScaledWidthRectf( activeArea1, values.x );
  Rectf rect2 = SimpleGUI::getScaledWidthRectf( activeArea2, values.y );
  Rectf rect3 = SimpleGUI::getScaledWidthRectf( activeArea3, values.z );
  Rectf rect4 = SimpleGUI::getScaledWidthRectf( activeArea4, values.w );

  gl::color( 1.0f,0.0f,0.0f );
  gl::drawLine( Vec2f( rect1.x2, rect1.y1 ), Vec2f( rect1.x2, rect1.y2 ) );
  
  gl::color( 0.0f,1.0f,0.0f );
  gl::drawLine( Vec2f( rect2.x2, rect2.y1 ), Vec2f( rect2.x2, rect2.y2 ) );
  
  gl::color( 0.0f,0.0f,1.0f );
  gl::drawLine( Vec2f( rect3.x2, rect3.y1 ), Vec2f( rect3.x2, rect3.y2 ) );
  
  gl::color( SimpleGUI::lightColor );
  gl::drawLine( Vec2f( rect4.x2, rect4.y1 ), Vec2f( rect4.x2, rect4.y2 ) );        
  
  pos.y += SimpleGUI::labelSize.y + SimpleGUI::padding.y + SimpleGUI::sliderSize.y * 4 + SimpleGUI::padding.y * 3 + SimpleGUI::spacing;

  return pos;
}
    
std::string ColorVarControl::toString() 
{
  std::stringstream ss;
  ss << var->r << " " << var->g << " " << var->b << " " << var->a;  
  return ss.str();
}

void ColorVarControl::fromString(std::string& strValue) 
{
  std::vector< std::string > strs;
  boost::split( strs, strValue, boost::is_any_of( "\t " ) );

  var->r = boost::lexical_cast< float >( strs[ 0 ] );
  var->g = boost::lexical_cast< float >( strs[ 1 ] );  
  var->b = boost::lexical_cast< float >( strs[ 2 ] );  
  var->a = boost::lexical_cast< float >( strs[ 3 ] );  
}
  
  
void ColorVarControl::onMouseDown( MouseEvent event ) 
{  
  if ( activeArea1.contains( event.getPos() ) ) 
  {
    activeTrack = 1;
  }
  else if ( activeArea2.contains( event.getPos() ) ) 
  {
    activeTrack = 2;
  }
  else if ( activeArea3.contains( event.getPos() ) ) 
  {
    activeTrack = 3;
  }
  else if ( activeArea4.contains( event.getPos() ) ) 
  {
    activeTrack = 4;
  }

  onMouseDrag( event );
}
  
void ColorVarControl::onMouseDrag( MouseEvent event ) 
{  
  float value = ( event.getPos().x - activeArea.x1 ) / ( activeArea.x2 - activeArea.x1 );
  value = math< float >::max( 0.0, math< float >::min( value, 1.0 ) );  
  
  setValueForElement( activeTrack, value );
}

void ColorVarControl::onMouseWheel( MouseEvent event ) 
{
  if ( activeArea1.contains( event.getPos() ) ) 
  {
    activeTrack = 1;
  }
  else if ( activeArea2.contains( event.getPos() ) ) 
  {
    activeTrack = 2;
  }
  else if ( activeArea3.contains( event.getPos() ) ) 
  {
    activeTrack = 3;
  }
  else if ( activeArea4.contains( event.getPos() ) ) 
  {
    activeTrack = 4;
  }

  const float step     = 0.01f;
  const float delta    = event.getWheelIncrement() * step;
  float       value    = getValueForElement( activeTrack );
  float       newValue = math< float >::clamp( value + delta );

  if ( newValue != value ) 
  {
      setValueForElement( activeTrack, newValue, false );
  }
}

    
void ColorVarControl::setValueForElement( const int element, const float value, const bool silent ) 
{
  if ( colorModel == SimpleGUI::RGB ) 
  {
    switch ( element ) 
    {
      case 1: var->r = value; break;
      case 2: var->g = value; break;
      case 3: var->b = value; break;
      case 4: var->a = value; break;        
    }
  }
  else 
  {
    Vec3f hsv = rgbToHSV( *var );

    switch ( element ) 
    {
      case 1: hsv.x  = value; break;
      case 2: hsv.y  = value; break;
      case 3: hsv.z  = value; break;
      case 4: var->a = value; break;        
    }

    *var = ColorA( CM_HSV, hsv.x, hsv.y, hsv.z, var->a );
  }
    
  if( silent == false )
  {
      triggerCallback();
  }
}
    
float ColorVarControl::getValueForElement( const int element ) 
{
  if ( colorModel == SimpleGUI::RGB ) 
  {
    switch ( element ) 
    {
      case 1: return var->r;
      case 2: return var->g;
      case 3: return var->b;
      case 4: return var->a;        
    }
  }
  else 
  {
    Vec3f hsv = rgbToHSV( *var );

    switch ( element ) 
    {
      case 1: return hsv.x;
      case 2: return hsv.y;
      case 3: return hsv.z;
      case 4: return var->a;        
    }
  }
    
  return 0.0f;
}
  
//-----------------------------------------------------------------------------  
  
ButtonControl::ButtonControl( const std::string& name ) : 
  CallbackControl( Control::BUTTON, name )
{
  this->pressed = false;
}

Vec2f ButtonControl::draw( Vec2f pos ) 
{
  activeArea = Rectf(
             pos.x, 
             pos.y, 
             pos.x + SimpleGUI::sliderSize.x, 
             pos.y + SimpleGUI::labelSize.y + SimpleGUI::padding.y * 1
             );  
  
  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( Rectf(
              ( pos - SimpleGUI::padding ).x, 
              ( pos - SimpleGUI::padding ).y, 
              ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x, 
              ( pos + SimpleGUI::sliderSize + SimpleGUI::padding * 2 ).y )
            );
  
  
  gl::color( pressed ? SimpleGUI::lightColor : SimpleGUI::darkColor );
  gl::drawSolidRect( activeArea );        

  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  gl::draw( labelTexture, Vec2f( pos.x + SimpleGUI::padding.x * 2, pos.y + floor( SimpleGUI::padding.y * 0.5f ) ) );

  pos.y += SimpleGUI::sliderSize.y + SimpleGUI::spacing + SimpleGUI::padding.y;

  return pos;
}
  
void ButtonControl::onMouseDown( MouseEvent event ) 
{
  pressed = true;
  bool handled = triggerCallback();

  if ( handled ) 
  {
    pressed = false;
  }
}
  
void ButtonControl::onMouseUp( MouseEvent event ) 
{
  pressed = false;  
}

//-----------------------------------------------------------------------------  

LabelControl::LabelControl( const std::string& name ) :
  Control( Control::LABEL, name )
{
}
  
void LabelControl::setText( const std::string& text ) 
{
  name = text;
}  
  
Vec2f LabelControl::draw( Vec2f pos ) 
{
  if ( bgColor ) 
  {
    gl::color( bgColor );
  }
  else 
  {
    gl::color( SimpleGUI::bgColor );
  }
  gl::drawSolidRect( Rectf(
    ( pos - SimpleGUI::padding ).x, 
    ( pos - SimpleGUI::padding ).y, 
    ( pos + SimpleGUI::sliderSize + SimpleGUI::padding ).x, 
    ( pos + SimpleGUI::labelSize  + SimpleGUI::padding ).y
  ) );        
  
  gl::color( ColorA( 1.0f, 1.0f, 1.0f, 1.0f ) );
  gl::draw(labelTexture, pos);

  pos.y += SimpleGUI::labelSize.y + SimpleGUI::spacing;    
  
  return pos;
}
  
//-----------------------------------------------------------------------------    
  
SeparatorControl::SeparatorControl() :
  Control( Control::SEPARATOR, "Separator" )
{
}  
  
Vec2f SeparatorControl::draw( Vec2f pos ) 
{
  activeArea = Rectf( pos - SimpleGUI::padding, pos + SimpleGUI::separatorSize + SimpleGUI::padding );

  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( activeArea );        
  
  pos.y += SimpleGUI::separatorSize.y + SimpleGUI::spacing;
  
  return pos;
}
  
//-----------------------------------------------------------------------------    

ColumnControl::ColumnControl( int x, int y ) :
  Control( Control::COLUMN, "Column" )
{
  this->x          = x;
  this->y          = y;
  this->activeArea = Rectf( 0.0f, 0.0f, 0.0f, 0.0f );
}  
  
Vec2f ColumnControl::draw( Vec2f pos ) 
{
  if ( this->x == 0 && this->y == 0 ) 
  {
    pos.x += SimpleGUI::labelSize.x + SimpleGUI::spacing;
    pos.y = SimpleGUI::spacing;
  }
  else 
  {
    pos.x = static_cast< float >( x );
    pos.y = static_cast< float >( y );
  }
  return pos;
}
  
//-----------------------------------------------------------------------------    
  
PanelControl::PanelControl() :
  Control( Control::PANEL, "Panel" )
{
  this->enabled = true;
}
  
Vec2f PanelControl::draw( Vec2f pos ) 
{
  return pos;
}
  
//-----------------------------------------------------------------------------
  
TextureVarControl::TextureVarControl( const std::string& name, gl::Texture* var, int scale, bool flipVert ) :
  CallbackControl( Control::TEXTURE_VAR, name )
{
  this->var      = var;
  this->scale    = static_cast< float >( scale );
  this->flipVert = flipVert;
}  
  
Vec2f TextureVarControl::draw( Vec2f pos ) 
{
  if ( !*var ) 
  {
    std::cout << "Missing texture" << std::endl;
    return pos;        
  }
  
  Vec2f textureSize;
  textureSize.x = math< float >::floor( SimpleGUI::sliderSize.x * scale + SimpleGUI::spacing * ( scale - 1 ) );
  textureSize.y = math< float >::floor( textureSize.x / var->getAspectRatio() );

  activeArea = Rectf( pos.x, pos.y, pos.x + textureSize.x, pos.y + textureSize.y ); 

  gl::color( SimpleGUI::bgColor );
  gl::drawSolidRect( Rectf(
    activeArea.x1 - SimpleGUI::padding.x, 
    activeArea.y1 - SimpleGUI::padding.y, 
    activeArea.x2 + SimpleGUI::padding.x, 
    activeArea.y2 + SimpleGUI::padding.y
  ) );

  gl::color( ColorA( 1, 1, 1, 1 ) );
  gl::draw( *var, activeArea );  

  pos.y += activeArea.getHeight() + SimpleGUI::spacing;

  return pos;  
}
    
void TextureVarControl::resetTexture( gl::Texture* var )
{
  this->var = var;
}
    
void TextureVarControl::onMouseDown( MouseEvent event ) 
{
  triggerCallback();
}
  
//-----------------------------------------------------------------------------  

} //namespace sgui
} //namespace mowa