#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/IO/Polyhedron_iostream.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/boost/graph/properties_Polyhedron_3.h>
#include <CGAL/boost/graph/iterator.h>

#include <boost/foreach.hpp>

#include <iostream>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef CGAL::Polyhedron_3<Kernel>     Polyhedron;

typedef boost::graph_traits<Polyhedron> GraphTraits;
typedef GraphTraits::vertex_descriptor vertex_descriptor;
typedef GraphTraits::halfedge_descriptor halfedge_descriptor;
typedef GraphTraits::edge_descriptor edge_descriptor;
typedef GraphTraits::out_edge_iterator out_edge_iterator;

typedef CGAL::Halfedge_around_face_circulator<Polyhedron> halfedge_around_face_circulator;
typedef CGAL::Halfedge_around_target_circulator<Polyhedron> halfedge_around_target_circulator;
typedef CGAL::Vertex_around_target_circulator<Polyhedron> vertex_around_target_circulator;
typedef CGAL::Face_around_target_circulator<Polyhedron> face_around_target_circulator;

typedef CGAL::Halfedge_around_source_circulator<Polyhedron> halfedge_around_source_circulator;

typedef CGAL::Halfedge_around_target_iterator<Polyhedron> halfedge_around_target_iterator;
typedef CGAL::Halfedge_around_face_iterator<Polyhedron> halfedge_around_face_iterator;

int main()
{ 
  Polyhedron P;
  std::cin >> P;
  halfedge_descriptor hd = *halfedges(P).first;
  {
    halfedge_around_face_circulator hafc(hd,P), done(hafc);
    
    do {
      std::cout << get(CGAL::vertex_point, P, target(*hafc,P)) << std::endl;
      ++hafc;
    }while(hafc != done);
  }

  {
    halfedge_around_target_circulator havc(hd,P), done(havc);
    
    do {
      std::cout << get(CGAL::vertex_point, P, target(*havc,P)) << std::endl;
      ++havc;
    }while(havc != done);
  }
  {
    vertex_around_target_circulator havc(hd,P), done(havc);
    
    do {
      std::cout << get(CGAL::vertex_point, P, *havc) << std::endl;
      ++havc;
    }while(havc != done);
  }
  {
    face_around_target_circulator havc(hd,P), done(havc);
    
    do {
      //std::cout << get(CGAL::vertex_point, P, *havc) << std::endl;
      ++havc;
    }while(havc != done);
  }
  {
    halfedge_around_source_circulator havc(hd,P), done(havc);
    
    do {
      std::cout << get(CGAL::vertex_point, P, target(*havc,P)) << std::endl;
      ++havc;
    }while(havc != done);
  }

  {
    halfedge_around_target_iterator vit, end;
    boost::tie(vit,end) = halfedges_around_target(hd,P);
    
    while(vit!= end) {
      halfedge_descriptor hd = *vit;
      std::cout << get(CGAL::vertex_point, P, target(hd,P)) << std::endl;
      ++vit;
    }
  }

   {
    halfedge_around_face_iterator vit, end;
    boost::tie(vit,end) = halfedges_around_face(hd,P);
    
    while(vit!= end) {
      halfedge_descriptor hd = *vit;
      std::cout << get(CGAL::vertex_point, P, target(hd,P)) << std::endl;
      ++vit;
    }
  }
 

  {
    out_edge_iterator ohi, end;
    for(boost::tie(ohi,end) = out_edges(target(hd,P),P); ohi != end; ++ohi){
      edge_descriptor ed = *ohi;
      halfedge_descriptor hd2 = halfedge(ed,P);
      std::cout << get(CGAL::vertex_point, P, target(hd2,P)) << std::endl;
    }
  } 

  {
    BOOST_FOREACH(edge_descriptor ed,out_edges(target(hd,P),P)){
      halfedge_descriptor hd2 = halfedge(ed,P);
      std::cout << get(CGAL::vertex_point, P, target(hd2,P)) << std::endl;
    }
  }
  return 0;
}
