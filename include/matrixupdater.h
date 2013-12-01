#ifndef __MATRIXUPDATER_H__
#define __MATRIXUPDATER_H__

#include "cinder\Vector.h"
#include "cinder\Rect.h"

#include <vector>
#include <set>
#include <limits>

template < class T >
class MatrixUpdater
{
public:
  
  ////////////////////////////////////////////////////////////////////////////////

  class MUNode
  {

  public:
    MUNode( MatrixUpdater< T >* theOwner, size_t theId, ci::Vec2f& theTLPos, float theRadius ) :
      m_owner( theOwner ),
      m_id( theId ),
      m_area( theTLPos.x, theTLPos.y, theTLPos.x + 2 * theRadius, theTLPos.y + 2 * theRadius )
    {
    }
    
    //------------------------------------------------------------------------------

    virtual ~MUNode()
    {
    }
    
    //------------------------------------------------------------------------------

    void update()
    {
      typename std::set< T* >::iterator itr     = begin();
      typename std::set< T* >::iterator itr_end = end();

      for ( ; itr != itr_end; ++itr )
      {
        if ( !m_area.contains( ( *itr )->position() ) )
        {
          typename std::set< T* >::iterator tmp = itr;
          --itr;
          m_owner->insert( *tmp );
          m_objectList.erase( tmp );
        }
      }
    }
    
    //------------------------------------------------------------------------------

    void insert( T* theObject )
    {
      m_objectList.insert( theObject );
    }
    
    //------------------------------------------------------------------------------

    void erase( T* theObject )
    {
      m_objectList.erase( theObject );
    }
    
    //------------------------------------------------------------------------------

    MUNode* next()
    {
      if ( m_id < m_owner->m_nodeMatrix.size() )
      {
        m_owner->m_nodeMatrix[ m_id ];
      }
      return 0;
    }
    
    //------------------------------------------------------------------------------

    typename std::set< T* >::iterator begin() { return m_objectList.begin();  }
    typename std::set< T* >::iterator end()   { return m_objectList.end();    }
    size_t                            size()  { return m_objectList.size();   }

  protected:
    MatrixUpdater*    m_owner;
    size_t            m_id;    
    ci::Rectf         m_area;
    std::set< T* >    m_objectList;
  };
  
  friend class MatrixUpdater< T >::MUNode;

  ////////////////////////////////////////////////////////////////////////////////

  class iterator
  {
  public:
    iterator( ci::Vec2f& thePosition, 
              size_t theQuadrantX, 
              size_t theQuadrantY, 
              float theRadius, 
              typename MatrixUpdater< T >::MUNode* theNode, 
              typename std::set< T* >::iterator& theEndIterator ):
      m_position( thePosition ),
      m_quadrantX( theQuadrantX ),
      m_quadrantY( theQuadrantY ),
      m_squaredRadius( theRadius * theRadius ),
      m_squaredDist( std::numeric_limits< float >::infinity() ),
      m_node( theNode ),
      m_itr( theEndIterator )
    {
    }
    
    //------------------------------------------------------------------------------

    iterator( typename MatrixUpdater< T >::MUNode* theNode, typename std::set< T* >::iterator& theEndIterator ):
      m_position( std::numeric_limits< float >::infinity(), std::numeric_limits< float >::infinity() ),
      m_quadrantX( 0 ),
      m_quadrantY( 0 ),
      m_squaredRadius( std::numeric_limits< float >::infinity() ),
      m_squaredDist( std::numeric_limits< float >::infinity() ),
      m_node( theNode ),
      m_itr( theEndIterator )
    {
    }
    
    //------------------------------------------------------------------------------

    iterator():
      m_position( std::numeric_limits< float >::infinity(), std::numeric_limits< float >::infinity() ),
      m_quadrantX( 0 ),
      m_quadrantY( 0 ),
      m_squaredRadius( std::numeric_limits< float >::infinity() ),
      m_squaredDist( std::numeric_limits< float >::infinity() ),
      m_node( 0 ),
      m_itr( )
    {
    }
    
    //------------------------------------------------------------------------------

    typename MatrixUpdater< T >::iterator& operator=( typename MatrixUpdater< T >::iterator& _other )
    {
      m_position      = _other.m_position;
      m_squaredRadius = _other.m_squaredRadius;
      m_node          = _other.m_node;
      m_itr           = _other.m_itr;
      return *this;
    }
    
    //------------------------------------------------------------------------------

    typename MatrixUpdater< T >::iterator& operator++( int )
    {
      return ++(*this);
    }
    
    //------------------------------------------------------------------------------

    typename MatrixUpdater< T >::iterator& operator++( )
    {
      if ( m_squaredRadius == std::numeric_limits< float >::infinity() )
      {
        setIteratorToNext();
        return *this;
      }
      else
      {
        while ( true )
        {
          if ( !setIteratorToNext() )
          {
            return *this;            
          }

          m_squaredDist = m_position.distanceSquared( ( *m_itr )->position() );

          if ( m_squaredDist <= m_squaredRadius )
          {
            return *this;
          }
        }
        return *this;
      }
    }
    
    //------------------------------------------------------------------------------

    bool operator != ( typename MatrixUpdater< T >::iterator& theIterator )
    {
      // special case to the end iterator
      if ( m_node == theIterator.m_node && m_node == 0 )
      {
        return false;
      }

      if ( m_node != theIterator.m_node || m_itr != theIterator.m_itr )
      {
        return true;
      }
      return false;
    }
    
    //------------------------------------------------------------------------------

    typename T* operator *()
    {
      return *m_itr;
    }

    //------------------------------------------------------------------------------

    float squaredDist() { return m_squaredDist; }
    
    //------------------------------------------------------------------------------

  protected:
    bool setIteratorToNext()
    {
      if ( !m_node ) 
      {
        return false;
      }
      
      if ( m_itr != m_node->end() )
      {
        ++m_itr;
      }

      if ( m_itr == m_node->end() )
      {
        do 
        {
          m_node = m_node->next();
          if ( m_node )
          {
            m_itr = m_node->begin();
          }
          else
          {
            return false;
          }
        } 
        while ( m_itr == m_node->end() );
      }

      return true;
    }

  protected:
    ci::Vec2f                             m_position;
    size_t                                m_quadrantX;
    size_t                                m_quadrantY;
    size_t                                m_i;
    size_t                                m_j;
    float                                 m_squaredRadius;
    float                                 m_squaredDist;
    typename MatrixUpdater< T >::MUNode*  m_node;
    typename std::set< T* >::iterator     m_itr;
  };
  
  ////////////////////////////////////////////////////////////////////////////////

public:
  MatrixUpdater( float theViewRadius, ci::Vec2f& theSize ) :  
    m_viewRadius( theViewRadius ),
    m_size( theSize )
  {
    m_nodesWidth    = static_cast< size_t >( ceil( m_size.x / ( m_viewRadius * 2 ) ) );
    m_nodesHeight   = static_cast< size_t >( ceil( m_size.x / ( m_viewRadius * 2 ) ) );
  
    size_t numNodes = static_cast< size_t >( ceil( m_nodesWidth * m_nodesHeight ) );

    m_nodeMatrix.reserve( numNodes );
  
    for ( size_t y = 0; y < m_nodesWidth; ++y )
    {
      for ( size_t x = 0; x < m_nodesHeight; ++x )
      {
        size_t id = y * m_nodesWidth + x;
        m_nodeMatrix.push_back( new MUNode( this, id, ci::Vec2f( x * ( m_viewRadius * 2 ), y * ( m_viewRadius * 2 ) ), theViewRadius ) );
      }
    }
  }
  
  //------------------------------------------------------------------------------

  virtual ~MatrixUpdater()
  {
    for ( size_t i = 0; i < m_nodeMatrix.size(); ++i )
    {
      delete m_nodeMatrix[ i ];
    }
  }

  //------------------------------------------------------------------------------

  typename MatrixUpdater< T >::iterator lower_bound( ci::Vec2f& thePosition )
  {
    size_t x = static_cast< size_t >( m_size.x / thePosition.x );
    size_t y = static_cast< size_t >( m_size.y / thePosition.y );

    // test for overflow
    if ( x - 1 < m_nodesWidth )
    {
      x -= 1;
    }

    if ( y - 1 < m_nodesHeight )
    {
      y -= 1;
    }
    
    MUNode* theMUNode    = 0;

    for ( size_t j = 0; j < 3; ++j )
    {
      for ( size_t i = 0; i < 3; ++i )
      {
        theMUNode = m_nodeMatrix[ ( x + i ) + ( y + j ) * m_nodesWidth ];

        if ( theMUNode->size() )
        {
          j = 3;
          i = 3;
          break;
        }
      } 
    }

    return typename MatrixUpdater< T >::iterator( thePosition, m_viewRadius, theMUNode, theMUNode->begin() );
  }
  
  //------------------------------------------------------------------------------

  typename MatrixUpdater< T >::iterator upper_bound( ci::Vec2f& thePosition )
  {
    size_t x = static_cast< size_t >( m_size.x / thePosition.x );
    size_t y = static_cast< size_t >( m_size.y / thePosition.y );

    // test for overflow
    if ( x + 1 < m_nodesWidth )
    {
      x += 1;
    }

    if ( y + 1 < m_nodesHeight )
    {
      y += 1;
    }

    size_t theTargetNode = x + y * m_nodesWidth;
    return typename MatrixUpdater< T >::iterator( thePosition, x, y, m_viewRadius, m_nodeMatrix[ theTargetNode ], m_nodeMatrix[ theTargetNode ]->end() );
  }
  
  //------------------------------------------------------------------------------

  typename MatrixUpdater< T >::iterator lower_bound( typename T* theObject )
  {
    return lower_bound( theObject->position() );
  }

  typename MatrixUpdater< T >::iterator upper_bound( typename T* theObject )
  {
    return upper_bound( theObject->position() );
  }

  //------------------------------------------------------------------------------

  typename MatrixUpdater< T >::iterator begin() 
  { 
    size_t                           i = 0;

    for ( ; i < m_nodeMatrix.size(); ++i )
    {
      if ( m_nodeMatrix[ i ]->size() )
      {
        return typename MatrixUpdater< T >::iterator( m_nodeMatrix[ i ], m_nodeMatrix[ i ]->begin() );
      }      
    }

    return end(); 
  }

  typename MatrixUpdater< T >::iterator end()   { return typename MatrixUpdater< T >::iterator( 0, typename std::set< T* >::iterator() ); }

  //------------------------------------------------------------------------------

  void update()
  {
    for ( size_t i = 0; i < m_nodeMatrix.size(); ++i )
    {
      m_nodeMatrix[ i ]->update();
    }
  }
  
  //------------------------------------------------------------------------------

  void insert( T* theObject )
  {
    size_t x = static_cast< size_t >( m_size.x / theObject->position().x );
    size_t y = static_cast< size_t >( m_size.y / theObject->position().y );

    m_nodeMatrix[ x + y * m_nodesWidth ]->insert( theObject );
  }

  void erase( T* theObject )
  {
    size_t x = static_cast< size_t >( m_size.x / theObject->position().x );
    size_t y = static_cast< size_t >( m_size.y / theObject->position().y );

    m_nodeMatrix[ x + y * m_nodesWidth ]->erase( theObject );    
  }

  //------------------------------------------------------------------------------

  // dimensions related methods
  float         viewRadius()   { return m_viewRadius;    }
  ci::Vec2f&    size()         { return m_size;          }
  size_t        nodesWidth()   { return m_nodesWidth;    }
  size_t        nodesHeight()  { return m_nodesHeight;   }
  
  //------------------------------------------------------------------------------

protected:
  float         m_viewRadius;
  ci::Vec2f     m_size;
  size_t        m_nodesWidth;
  size_t        m_nodesHeight;

  std::vector< typename MatrixUpdater< T >::MUNode* > m_nodeMatrix;
};

#endif // __MATRIXUPDATER_H__
