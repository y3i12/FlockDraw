#if !defined __SPATIAL2DGRID_H__
#define      __SPATIAL2DGRID_H__

#include <unordered_set>
#include <vector>

#include <cinder/Vector.h>

// TODO: 
// 1. code the iterator
// 2. code the lower bound
// 3. code the upper bound

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template < class T >
class Cell 
{
public:
  Cell()
  {
  }

  virtual ~Cell(){}
  
  void insert( T* _object )
  {
    m_contents.insert( _object );
  }
  
  void erase( T* _object )
  {
    m_contents.erase( _object );
  }
  
  int m_column;
  int m_row;
  
  std::unordered_set< T* > m_contents;
  typename Cell< T >*      m_neighbors[ 9 ]; 
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template < class T, class PosAccess >
class Spatial2DGrid
{
public:

  class iterator
  {
  public:
    // begin itr
    iterator( ci::Vec2f& _position, typename Cell< T >* _cell ) :
      m_position( _position ),      
      m_column( 0 ),
      m_row( 0 ),
      m_cell( _cell )
    {
      setBegin();
    }

    // ctor for the end itr
    iterator( typename Cell< T >* _cell ) :
      m_position( ),
      m_column( 0 ),
      m_row( 0 ),
      m_cell( 0 )
    {
    }

    iterator( typename Spatial2DGrid< T, PosAccess >::iterator& _other ) :
      m_position( _other.m_position ),
      m_column( _other.m_column ),
      m_row( _other.m_row ),
      m_cell( _other.m_cell ),
      m_cellIterator( _other.m_cellIterator ),
      m_iteratingCell( _other.m_iteratingCell )
    {
    }

    ~iterator()
    {
    }

    bool operator != ( typename Spatial2DGrid< T, PosAccess >::iterator& _other )
    {
      // special case for the end itr
      if ( m_cell == 0 && m_cell == _other.m_cell )
      {
        return false;
      }

      if ( m_cell         != _other.m_cell        || 
           m_row          != _other.m_row         || 
           m_column       != _other.m_column      || 
           m_cellIterator != _other.m_cellIterator )
      {
        return true;
      }

      return false;
    }

    bool operator++( int )
    {
      return ++( *this );
    }

    bool operator++( )
    {
      if ( m_cell )
      { 
        next();
      }
      
      if ( !m_cell )
      {
        return false;
      }
      
      return true;
    }

    typename Spatial2DGrid< T, PosAccess >::iterator& operator = ( typename Spatial2DGrid< T, PosAccess >::iterator& _other )
    {
      m_position        = _other.m_position;
      m_column          = _other.m_column;
      m_row             = _other.m_row;
      m_cell            = _other.m_cell;
      m_cellIterator    = _other.m_cellIterator;
      m_iteratingCell   = _other.m_iteratingCell;
    }

    T& operator*()
    {
      return **m_cellIterator;
    }
    
    T* operator->()
    {
      return *m_cellIterator;
    }

  private:
    void setBegin()
    {
      if ( !getCurrentCell() )
      {
        setNextCell();
      }

      if ( m_cell )
      {
        m_cellIterator = m_iteratingCell->m_contents.begin();
        first();
      }
    }

    void setNextCell()
    {
      do
      {
        m_column++;

        if ( m_column > 2 )
        {
          m_column = 0; 
          ++m_row;
        
          if ( m_row > 2 )
          {
            m_row  = 0;
            m_cell = 0;
            return;
          }
        }
      }
      while ( !getCurrentCell() );

      m_cellIterator = m_iteratingCell->m_contents.begin();
    }

    inline typename Cell< T >* getCurrentCell()
    {
      return m_iteratingCell = m_cell->m_neighbors[ m_column + m_row * 3 ];
    }

    void first()
    {
      while ( m_cell && m_cellIterator == m_iteratingCell->m_contents.end() )
      {
        setNextCell();
      }
    }
    
    void next()
    {
      ++m_cellIterator;
      while ( m_cell && m_cellIterator == m_iteratingCell->m_contents.end() )
      {
        setNextCell();
      }
    }

  private:
    ci::Vec2f             m_position;
    int                   m_column;
    int                   m_row;
    typename Cell< T >*   m_cell;
    typename std::unordered_set< T* >::iterator m_cellIterator;
    typename Cell< T >*   m_iteratingCell;
  };

public:
  Spatial2DGrid( float _width, float _height, float _cellWidth, float _cellHeight, bool _wrapEdges = ture ) :
    m_width( _width ),
    m_height( _height ),
    m_cellWidth( _cellWidth ),
    m_cellHeight( _cellHeight ),
    m_wrapEdges( _wrapEdges ),
    m_columns( static_cast< int >( ceil( _width / _cellWidth ) ) ),
    m_rows( static_cast< int >( ceil( _height / _cellHeight ) ) ),
    m_cells( m_columns * m_rows )
  {
    m_matrix = new typename Cell< T >[ m_cells ]();
    
    for ( int column = 0; column < m_columns; ++column )
    {
      for ( int row = 0; row < m_columns; ++row )
      {
        typename Cell< T >* targetCell = getCell( column, row );

        // call the constructor and set the position
        targetCell->m_column = column;
        targetCell->m_row    = row;
        
        initCellNeighborhood( targetCell, _wrapEdges );
      }
    }
  }
  
  ~Spatial2DGrid()
  {
    delete [] m_matrix;
  }

  void insert( typename T* _object )
  {
    getCell( PosAccess::position( _object ) )->insert( _object );
    
    PosAccess::stable_position( _object ) = PosAccess::position( _object );
  }
  
  void erase( typename T* _object )
  {
    getCell( PosAccess::stable_x( _object ), PosAccess::stable_y( _object ) ).erase( _object );
  }
  
  typename Cell< T >* getCell( ci::Vec2f& _position )
  {
    return getCell( static_cast< int >( _position.x / m_cellWidth ), static_cast< int >( _position.y / m_cellHeight ) );
  }
  
  typename Cell< T >* getCell( int _column, int _row )
  {
    if ( _column < 0 || _row < 0 || _column >= m_columns || _row >= m_rows )
    {
      return 0;
    }
    
    return &m_matrix[ calcCellNumber( _column, _row ) ];
  }
  
  void update( typename T* _object )
  {
    if ( calcCellNumber( PosAccess::stable_position( _object ) ) != 
         calcCellNumber( PosAccess::position(        _object ) )   )
    {
      erase( object );
      insert( object );
    }
    else
    {
      PosAccess::stable_position( _object ) = PosAccess::position( _object );
    }
  }
  
  typename Spatial2DGrid< T, PosAccess >::iterator lower_bound( ci::Vec2f& _position )
  {
    return Spatial2DGrid< T, PosAccess >::iterator( _position, &m_matrix[ calcCellNumber( _position ) ] );
  }
  
  float     width()      { return m_width;      }
  float     height()     { return m_height;     }
  float     cellWidth()  { return m_cellWidth;  }
  float     cellHeight() { return m_cellHeight; }
    
  
private:
  inline int calcCellNumber( ci::Vec2f& _position )
  {
    return calcCellNumber( static_cast< int >( _position.x / m_cellWidth ), static_cast< int >( _position.y / m_cellHeight ) );
  }
  
  inline int calcCellNumber( int _column, int _row )
  {
    return _row * m_columns + _column;
  }
  
  void initCellNeighborhood( typename Cell< T >* _targetCell, bool _wrapEdges )
  {
    for ( int j = -1; j < 2; ++j )
    {
      for ( int i = -1; i < 2; ++i )
      {
        _targetCell->m_neighbors[ ( j + 1 ) * 3 + i + 1 ] = getCell( _targetCell->m_column + i, _targetCell->m_row + j );
      }
    }
  }
  
private:
  float     m_width;
  float     m_height;
  float     m_cellWidth;
  float     m_cellHeight;
  
  bool      m_wrapEdges;
  
  int       m_columns;
  int       m_rows;
  int       m_cells;
  
  typename Cell< T >* m_matrix;
  
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#endif // __SPATIAL2DGRID_H__
