#include <sstream>
#include <iostream>
#include "Spatial2DGrid.h"
#include "cinder/Vector.h"

struct Point
{
  ci::Vec2f m_position;
  ci::Vec2f m_stablePosition;
  int       m_number;
};

struct PointsFunctor
{
  static inline ci::Vec2f& position( Point* p )
  {
    return p->m_position;
  }

  static inline ci::Vec2f& stable_position( Point* p )
  {
    return p->m_stablePosition;
  }
};

void outPoint( Point& _p )
{
  std::cout << "(" << _p.m_position.x << ", " << _p.m_position.y << ")";
}

int main ( int c, char** v )
{
  Spatial2DGrid< Point, PointsFunctor > grid( 100.0f, 100.0f, 10.0f, 10.0f, true );

  Point p[ 9 ];

  p[ 0 ].m_position = ci::Vec2f(  0.0f,  0.0f );
  p[ 1 ].m_position = ci::Vec2f( 10.0f,  0.0f );
  p[ 2 ].m_position = ci::Vec2f( 20.0f,  0.0f );
  p[ 3 ].m_position = ci::Vec2f(  0.0f, 10.0f );
  p[ 4 ].m_position = ci::Vec2f( 10.0f, 10.0f );
  p[ 5 ].m_position = ci::Vec2f( 20.0f, 10.0f );
  p[ 6 ].m_position = ci::Vec2f(  0.0f, 20.0f );
  p[ 7 ].m_position = ci::Vec2f( 10.0f, 20.0f );
  p[ 8 ].m_position = ci::Vec2f( 20.0f, 20.0f );

  p[ 0 ].m_number = 0;
  p[ 1 ].m_number = 1;
  p[ 2 ].m_number = 2;
  p[ 3 ].m_number = 3;
  p[ 4 ].m_number = 4;
  p[ 5 ].m_number = 5;
  p[ 6 ].m_number = 6;
  p[ 7 ].m_number = 7;
  p[ 8 ].m_number = 8;
  
  std::unordered_set< Point* > s;
  s.insert( p );

  for ( int i = 0; i < 9; ++i )
  {
    grid.insert( &p[ i ] );
  }

  for ( int i = 0; i < 9; ++i )
  {
    Spatial2DGrid< Point, PointsFunctor >::iterator itr = grid.lower_bound( p[ i ].m_position );
  
    std::cout <<  "point [ " << i <<" ]: ";
    outPoint( p[ i ] );
    std::cout <<  " have access to: " << std::endl;

    do
    {
      std::cout <<  "\tPoint [ " << itr->m_number << " ]: ";
      outPoint( *itr );
      std::cout <<  std::endl;
    }
    while ( ++itr );

    std::cout <<  "--------------------" << std::endl;
  }

  return 0;
}