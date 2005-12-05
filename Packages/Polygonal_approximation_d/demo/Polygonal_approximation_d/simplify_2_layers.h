// Copyright (c) 2002  Max Planck Institut fuer Informatik (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source$
// $Revision$ $Date$
// $Name$
//
// Author(s)     : Radu Ursu

#include <CGAL/IO/Qt_widget_layer.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Partition_traits_2.h>
#include <CGAL/partition_2.h>
#include <CGAL/polyap_traits.h>
#include <CGAL/polyap_fct.h>
#include <CGAL/IO/Qt_widget_Polygon_2.h>


template <class T>
class Qt_layer_show_greene_approx : public CGAL::Qt_widget_layer
{
public:
  typedef typename T::FT              FT;
  typedef CGAL::Cartesian<FT>         K;
  typedef CGAL::Partition_traits_2<K> Traits;

  Qt_layer_show_greene_approx(T &p, int& perc) : polygon(p), percentage(perc)
  {}

  void draw()
  {
    if(polygon.size() <=2){
      return;
    }

    typedef CGAL::Squared_Euclidean_error<K,CGAL::Max_tag,CGAL::Segment_tag,2> Error;
    std::list<K::Point_2> result;

    std::size_t n = (percentage * polygon.size())/100;
    if(n < 2) n = 2;

    double eps;
    Error error;
    
    polygonal_approximation_RS_bnp_gea(polygon.vertices_begin(), polygon.vertices_end(),
				       n,  eps,
				       std::back_inserter(result),
				       error);
    
    if(result.size()>=2){
      *widget << CGAL::RED; 
      typename std::list<K::Point_2>::const_iterator p_it, q_it, back_it = result.end();
      back_it--;
      for(p_it = result.begin(); 
	  p_it != back_it; p_it++)
	{
	  q_it = p_it;
	  q_it++;
	  *widget << K::Segment_2(*p_it, *q_it);
	}
    }
    
  };
private:
  int &percentage;
  T		&polygon;
  std::list<T>	greene_approx_polys;
};//end class 

template <class T>
class Qt_layer_show_optimal_convex : public CGAL::Qt_widget_layer
{
public:
  typedef typename T::FT              FT;
  typedef CGAL::Cartesian<FT>         K;
  typedef CGAL::Partition_traits_2<K> Traits;

  Qt_layer_show_optimal_convex(T &p) : polygon(p)
  {};
  void draw()
  {
    optimal_convex.clear();
    Traits  partition_traits;
    
    CGAL::optimal_convex_partition_2(polygon.vertices_begin(), 
                                       polygon.vertices_end(),
                           std::back_inserter(optimal_convex),
                                            partition_traits);    
    
    typename std::list<T>::const_iterator p_it;
    for(p_it = optimal_convex.begin(); p_it != optimal_convex.end(); p_it++)
    {
      *widget << CGAL::YELLOW; 
      *widget << *p_it;
    }
    
  };
private:
  T		&polygon;
  std::list<T>	optimal_convex;
};//end class 

template <class T>
class Qt_layer_show_polygon : public CGAL::Qt_widget_layer
{
public:
  
  Qt_layer_show_polygon(T &p) : polygon(p){};
  void draw()
  {
    *widget << CGAL::LineWidth(3);
    *widget << CGAL::BLACK; 
    *widget << polygon;
    *widget << CGAL::LineWidth(1);
    //*widget << CGAL::LineWidth(1);
    //*widget << CGAL::WHITE; 
    //*widget << polygon;
  };
	
private:
  T &polygon;
};//end class 


template <class T>
class Qt_layer_show_polygon_points : public CGAL::Qt_widget_layer
{
  typedef typename T::Point_2	Point_2;
public:
  
  Qt_layer_show_polygon_points(T &p) : polygon(p){};
  void draw()
  {
    typename T::const_iterator vert_it;

    
    for (vert_it = polygon.vertices_begin(); 
		vert_it != polygon.vertices_end(); vert_it++)
    {
      
      *widget << CGAL::GREEN << CGAL::PointSize (5) 
			<< CGAL::PointStyle (CGAL::DISC);
      *widget << Point_2((*vert_it).x(), (*vert_it).y());
    }
  };
	
private:
  T &polygon;
};//end class 

template <class T>
class Qt_layer_show_ymonotone : public CGAL::Qt_widget_layer
{
public:
  typedef typename T::FT	      FT;
  typedef CGAL::Cartesian<FT>	      K;
  typedef CGAL::Partition_traits_2<K> Traits;


  Qt_layer_show_ymonotone(T &p) : polygon(p)
  {};
  void draw()
  {
    ymonotone.clear();
    
    CGAL::y_monotone_partition_2(polygon.vertices_begin(), 
                                polygon.vertices_end(),
                                std::back_inserter(ymonotone));

    typename std::list<T>::const_iterator p_it;
    for(p_it = ymonotone.begin(); p_it != ymonotone.end(); p_it++)
    {
      *widget << CGAL::RED; 
      *widget << *p_it;
    }
    
  };
private:
  T		&polygon;
  std::list<T>	ymonotone;
};//end class 
