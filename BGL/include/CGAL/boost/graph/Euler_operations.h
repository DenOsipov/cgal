// Copyright (c) 2012 GeometryFactory (France). All rights reserved.
// All rights reserved.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
//
// Author(s)     : Philipp Moeller

#ifndef CGAL_EULER_OPERATIONS_H
#define CGAL_EULER_OPERATIONS_H

#include <stdexcept>

#include <boost/graph/graph_traits.hpp>

#include <CGAL/assertions.h>
#include <CGAL/BGL/Helper.h>

namespace CGAL {

namespace EulerImpl {

template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
join_face(typename boost::graph_traits<Graph>::halfedge_descriptor h,
          Graph& g)
{
 typedef typename boost::graph_traits<Graph> Traits;
  typedef typename Traits::halfedge_descriptor           halfedge_descriptor;

  typedef typename Traits::face_descriptor               face_descriptor;


  halfedge_descriptor hop = opposite(h,g);
  halfedge_descriptor hprev = prev(h, g), gprev = prev(hop, g);
  face_descriptor f = face(h, g), f2 = face(hop, g);

  internal::remove_tip(hprev, g);

  internal::remove_tip(gprev, g);

  if(! internal::is_border(hop,g)){
    remove_face(f2, g);
  }
  bool fnull = internal::is_border(h,g);

  
  halfedge_descriptor hprev2 = hprev;
  while(hprev2 != gprev) {
    hprev2 = next(hprev2, g);
    set_face(hprev2, f, g);
  }
  
  if (! fnull)
    set_halfedge(f, hprev, g);
  set_halfedge(target(hprev,g), hprev, g);
  set_halfedge(target(gprev,g), gprev, g);
  //  internal::set_constant_vertex_is_border(g, target(h, g));
  //  internal::set_constant_vertex_is_border(g, target(opposite(h, g), g));

  remove_edge(edge(h, g), g);
  return hprev;

}
} // namespace EulerImpl


  namespace Euler {
/// \ingroup PkgBGLEulerOperations
/// @{

/**  
 * joins the two vertices incident to `h`, (that is `source(h, g)` and
 * `target(h, g)`) and removes `source(h,g)`. Returns the predecessor
 * of `h` around the vertex, i.e., `prev(opposite(h,g))`.  The
 * invariant `join_vertex(g, split_vertex(g,h)))` returns `h`.  The
 * time complexity is linear in the degree of the vertex removed.
 *
 * \image html euler_join_vertex.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 *
 * \param g the graph
 * \param h the halfedge which incident vertices are joint
 *
 * \returns `prev(opposite(h,g))`
 *
 * \pre The size of the faces incident to `h` and `opposite(h,g)` is at least 4.
 *
 * \post `source(h, g)` is invalidated
 * \post `h` is invalidated 
 * 
 * \sa `split_vertex()`
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
join_vertex(typename boost::graph_traits<Graph>::halfedge_descriptor h,
            Graph& g)
{
  typedef typename boost::graph_traits<Graph>              Traits;
  typedef typename Traits::halfedge_descriptor             halfedge_descriptor;
  typedef typename Traits::vertex_descriptor               vertex_descriptor;
  typedef Halfedge_around_target_iterator<Graph>           halfedge_around_vertex_iterator;

  halfedge_descriptor hop = opposite(h, g)
    , hprev = prev(hop, g)
    , gprev = prev(h, g)
    , hnext = next(hop, g)
    , gnext = next(h, g);
  vertex_descriptor v_to_remove = target(hop, g)
    , v = target(h, g);

  // this assertion fires needlessly
  // CGAL_precondition(std::distance(
  //                     halfedges_around_face(e, g).first,
  //                     halfedges_around_face(e, g).second) >= 4);

  CGAL_assertion( halfedge(v_to_remove, v, g).first == h );

  halfedge_around_vertex_iterator ieb, iee;
  for(boost::tie(ieb, iee) = halfedges_around_target(hop, g); ieb != iee; ++ieb) {
    CGAL_assertion( target(*ieb,g) == v_to_remove);
    set_target(*ieb ,v , g);
  }

  set_next(hprev, hnext, g);
  set_next(gprev, gnext, g);
  set_halfedge(v, gprev, g);
  // internal::set_constant_vertex_is_border(g, v);

  remove_edge(edge(h, g), g);
  remove_vertex(v_to_remove, g);

  return hprev;
}



/** 
 * splits the vertex `v`, and connects the new vertex and `v` with a new
 * edge. Let `hnew` be `opposite(next(h1, g), g)` after the
 * split. The split regroups the halfedges around the two vertices. The
 * edge sequence `hnew`, `opposite(next(h2, g), g)`, ..., `h1`
 * remains around the old vertex, while the halfedge sequence
 * `opposite(hnew, g)`, `opposite(next(h1, g), g)` (before the
 * split), ..., `h2` is regrouped around the new vertex. The split
 * returns `hnew`, i.e., the new edge incident to vertex `v`. The
 * time is proportional to the distance from `h1` to `h2` around the
 * vertex.
 *
 * \image html euler_split_vertex.png
 *
 * \tparam Graph must be a model of  `MutableFaceGraph`
 *
 * \param g the graph
 * \param h1 halfedge descriptor
 * \param h2 halfedge descriptor
 *
 * \returns `hnew`
 *
 * \pre `target(h1, g) == target(h2, g)`, that is  `h1` and `h2` are incident to the same vertex
 * \pre `h1 != h2`, that is no antennas
 *
 * \sa `join_vertex()`
 *
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
split_vertex(typename boost::graph_traits<Graph>::halfedge_descriptor h1,
             typename boost::graph_traits<Graph>::halfedge_descriptor h2,
             Graph& g)
{
  CGAL_assertion(h1 != h2);
  CGAL_assertion(target(h1, g) == target(h2, g));

  typename boost::graph_traits<Graph>::halfedge_descriptor 
    hnew = halfedge(add_edge(g), g),
    hnewopp = opposite(hnew, g);
  typename boost::graph_traits<Graph>::vertex_descriptor
    vnew = add_vertex(g);
  internal::insert_halfedge(hnew, h2, g);
  internal::insert_halfedge(hnewopp, h1, g);
  set_target(hnew, target(h1, g), g);
  
  typename boost::graph_traits<Graph>::halfedge_descriptor 
    end = hnewopp;
  do
  {
    set_target(hnewopp, vnew, g);
    hnewopp = opposite(next(hnewopp, g), g);
  } while (hnewopp != end);

  internal::set_vertex_halfedge(hnew, g);
  // internal::set_constant_vertex_is_border(g, target(hnew, g));
  internal::set_vertex_halfedge(hnewopp, g);
  // internal::set_constant_vertex_is_border(g, target(hnewopp, g));
  return hnew;
}

/**
 * splits the halfedge `h` into two halfedges inserting a new vertex that is a copy of `vertex(opposite(h,g),g)`.
 * Is equivalent to `split_vertex( prev(h,g), opposite(h,g),g)`. 
 * \returns the new halfedge `hnew` pointing to the inserted vertex. The new halfedge is followed by the old halfedge, i.e., `next(hnew,g) == h`.
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
split_edge(typename boost::graph_traits<Graph>::halfedge_descriptor h, Graph& g)
{ return split_vertex(prev(h,g), opposite(h,g),g); }


/**
 * joins the two faces incident to `h` and `opposite(h,g)`. 
 * The faces may be holes.
 *
 * If `Graph` is a model of `MutableFaceGraph`
 * the face incident to `opposite(h,g)` is removed.
 *
 * `join_face()` and `split_face()` are inverse operations, that is
 * `join_face(g, split_face(g,h))` returns `h`.
 *
 * \image html euler_join_face.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`.
 * \param g the graph
 * \param h the halfedge incident to one of the faces to be joined.
 *
 * \returns `prev(h)`
 *
 * \pre `out_degree(source(h,g)), g)) >= 3`
 * \pre `out_degree(target(h,g)) >= 3`
 *
 * \sa `split_face()`
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
join_face(typename boost::graph_traits<Graph>::halfedge_descriptor h,
          Graph& g)
{
  return EulerImpl::join_face(h,g);
}



/**
 * splits the face incident to `h1` and `h2`.  Creates the opposite
 * halfedges `h3` and `h4`, such that `next(h1,g) == h3` and `next(h2,g) == h4`.
 * Performs the inverse operation to `join_face()`. 
 *
 * If `Graph` is a model of `MutableFaceGraph` and if the update of faces is not disabled
 * a new face incident to `h4` is added. 
 *
 * \image html euler_split_face.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 *
 * \param g the graph
 * \param h1
 * \param h2
 *
 * \returns `h3`
 *
 * \pre `h1` and `h2` are incident to the same face
 * \pre `h1 != h2`
 * \pre `next(h1,g) != h2` and `next(h2,g) != h1` (no loop)
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
split_face(typename boost::graph_traits<Graph>::halfedge_descriptor h1,
           typename boost::graph_traits<Graph>::halfedge_descriptor h2,
           Graph& g)
{ 
  typedef typename boost::graph_traits<Graph> Traits;
  typedef typename Traits::halfedge_descriptor halfedge_descriptor;
  typedef typename Traits::face_descriptor face_descriptor;
  halfedge_descriptor hnew = halfedge(add_edge(g), g);
  face_descriptor fnew = add_face(g);
  internal::insert_tip( hnew, h2, g);
  internal::insert_tip( opposite(hnew, g), h1, g);
  set_face( hnew, face(h1,g), g);
  internal::set_face_in_face_loop(opposite(hnew,g), fnew, g);
  set_halfedge(face(hnew,g), hnew, g);
  set_halfedge(face(opposite(hnew,g),g), opposite(hnew,g), g);
  return hnew; 
}

  
/**
 * glues the cycle of halfedges of `h1` and `h2` together.
 * The vertices in the cycle of `h2` get removed.
 * If `h1` or `h2` are not border halfedges their faces get removed.
 * The invariant `join_loop(g, h, split_loop(g, h, i, j))` returns `h` and keeps 
 * the graph unchanged.
 * 
 * \image html euler_loop.png
 *
 * \tparam Graph must be a `MutableFaceGraph`
 *
 * \returns `h1`. 
 *
 * \pre The faces incident to `h` and `g` are different and have equal number of edges.
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
join_loop(typename boost::graph_traits<Graph>::halfedge_descriptor h1,
          typename boost::graph_traits<Graph>::halfedge_descriptor h2,
          Graph& g)
{
  typedef typename boost::graph_traits<Graph>              Traits;
  typedef typename Traits::vertex_descriptor               vertex_descriptor;
  typedef typename Traits::halfedge_descriptor             halfedge_descriptor;
  typedef typename Traits::face_descriptor                 face_descriptor;

  CGAL_precondition( internal::is_border(h1,g) || face(h1, g) != face(h2, g));
  if (! internal::is_border(h1,g))
    remove_face(face(h1, g), g);
  if (! internal::is_border(h2,g))
    remove_face(face(h2,g), g);
  halfedge_descriptor hi = h1;
  halfedge_descriptor gi = h2;
  CGAL_assertion_code( std::size_t termination_count = 0;)
  do {
    CGAL_assertion( ++termination_count != 0);
    halfedge_descriptor hii = hi;
    halfedge_descriptor gii = gi;
    hi = next(hi, g);
    // gi = find_prev(gi); // Replaced by search around vertex.
    set_face( hii, face( opposite(gii, g), g), g);
    set_halfedge(face(hii, g), hii, g);
    remove_vertex(target(opposite(gii, g), g), g);
    if ( next(opposite(next(opposite(gii,g), g), g), g) == gii) {
      gi = opposite(next(opposite(gii,g),g), g);
    } else {
      set_next(hii, next(opposite(gii,g), g), g);
      gii = opposite(next(opposite(gii, g), g), g);
      set_target( gii, target(hii, g), g);
      while ( next(opposite(next(gii, g), g), g) != gi) {
        CGAL_assertion( ++termination_count != 0);
        gii = opposite(next(gii,g), g);
        set_target( gii, target(hii, g), g);
      }
      gi = opposite(next(gii,g), g);
      set_next(gii, hi, g);
    }
  } while ( hi != h1);
  CGAL_assertion( gi == h2);
  do {
    halfedge_descriptor gii = gi;
    gi = next(gi, g);
    remove_edge(edge(gii,g), g);
  } while ( gi != h2);
  return h1;
}


/**
 * cuts the graph along the cycle `(h,i,j)` changing the genus 
 * (edge `j` runs on the backside of the three dimensional figure below).
 * Three new vertices and three new pairs of halfedges, and two new triangles are created.
 *
 * `h`, `i`, and `j` will be incident to the first new face. 
 * 
 * \image html euler_split_loop.png
 * 
 * \tparam Graph must be a `MutableFaceGraph`
 *
 * \returns the halfedge incident to the second new face.
 *
 * \pre `h`, `i`, and `j` denote distinct, consecutive vertices of the graph 
 * and form a cycle: i.e., `target(h) == target(opposite(i,g),g)`, … , 
 * `target(j,g) == target(opposite(h,g),g)`. 
 * \pre The six faces incident to `h`, `i`, and `j` are all distinct.
 */
  template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
split_loop(typename boost::graph_traits<Graph>::halfedge_descriptor h,
           typename boost::graph_traits<Graph>::halfedge_descriptor i,
           typename boost::graph_traits<Graph>::halfedge_descriptor j,
           Graph& g)
{
  typedef typename boost::graph_traits<Graph>              Traits;
  typedef typename Traits::vertex_descriptor               vertex_descriptor;
  typedef typename Traits::halfedge_descriptor             halfedge_descriptor;
  typedef typename Traits::face_descriptor                 face_descriptor;

   CGAL_precondition( h != i);
        CGAL_precondition( h != j);
        CGAL_precondition( i != j);
        CGAL_precondition( target(h,g) == target(opposite(i,g),g));
        CGAL_precondition( target(i,g) == target(opposite(j,g),g));
        CGAL_precondition( target(j,g) == target(opposite(h,g),g));
        // Create a copy of the triangle.
        halfedge_descriptor hnew = internal::copy(h,g);
        halfedge_descriptor inew = internal::copy(i,g);
        halfedge_descriptor jnew = internal::copy(j,g);
        internal::close_tip( hnew, add_vertex(g), g);
        internal::close_tip( inew, add_vertex(g), g);
        internal::close_tip( jnew, add_vertex(g), g);
        internal::insert_tip( opposite(inew, g), hnew, g);
        internal::insert_tip( opposite(jnew, g), inew, g);
        internal::insert_tip( opposite(hnew, g), jnew, g);
        // Make the new incidences with the old stucture.
        CGAL_assertion_code( std::size_t termination_count = 0;)
          if ( next(h,g) != i) {
            halfedge_descriptor nh = next(h, g);
            set_next(h, i, g);
            set_next(hnew, nh, g);
            nh = opposite(nh, g);
            while ( next(nh, g) != i) {
                CGAL_assertion( ++termination_count != 0);
                set_target( nh, target(hnew,g), g);
                nh = opposite(next(nh, g), g);
            }
            set_target( nh, target(hnew,g), g);
            set_next(nh, inew, g);
        }
        if ( next(i, g) != j) {
          halfedge_descriptor nh = next(i, g);
          set_next(i, j, g);
          set_next(inew, nh, g);
          nh = opposite(nh,g);
          while ( next(nh,g) != j) {
                CGAL_assertion( ++termination_count != 0);
                set_target( nh, target(inew, g), g);
                nh = opposite(next(nh, g), g);
            }
          set_target( nh, target(inew, g), g);
          set_next(nh, jnew, g);

        }
        if ( next(j,g) != h) {
          halfedge_descriptor nh = next(j, g);
          set_next(j, h, g);
          set_next(jnew, nh, g);
          nh = opposite(nh, g);
          while ( next(nh,g) != h) {
                CGAL_assertion( ++termination_count != 0);
                set_target( nh, target(jnew, g), g);
                nh = opposite(next(nh, g), g);
            }
          set_target(nh, target(jnew, g), g);
          set_next(nh, hnew, g);
        }
        // Fill the holes with two new faces.
        face_descriptor f = add_face(g);
        set_face( h, f, g);
        set_face( i, f, g);
        set_face( j, f, g);
        set_halfedge(face(h,g), h, g);
        f = add_face(g);
        set_face( opposite(hnew, g), f, g);
        set_face( opposite(inew, g), f, g);
        set_face( opposite(jnew, g), f, g);
        set_halfedge(face(opposite(hnew,g),g), opposite(hnew,g), g);
        // Take care of maybe changed halfedge pointers.
        set_halfedge(face(hnew, g), hnew, g);
        set_halfedge(face(inew, g), inew, g);
        set_halfedge(face(jnew, g), jnew, g);
        set_halfedge(target(hnew, g), hnew, g);
        set_halfedge(target(inew, g), inew, g);
        set_halfedge(target(jnew, g), jnew, g);
        return opposite(hnew, g);
}


/**
 * removes the incident face of `h` and changes all halfedges incident to the face into border halfedges 
 * or removes them from the graph if they were already border halfedges.
 *
 * If this creates isolated vertices they get removed as well. 
 *
 * \image html erase_facet1.png
 * \image html erase_facet2.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 *
 * \pre `h` is not a border halfedge
 *
 * \sa `make_hole()` for a more specialized variant.
 */
template< typename Graph >
void remove_face(typename boost::graph_traits<Graph>::halfedge_descriptor h,
                 Graph& g)
{
  typedef typename boost::graph_traits<Graph>            Traits;
  typedef typename Traits::halfedge_descriptor           halfedge_descriptor;
  typedef typename Traits::face_descriptor               face_descriptor;

  CGAL_precondition(! internal::is_border(h,g));
  face_descriptor f = face(h, g);

  halfedge_descriptor end = h;
  do {
    internal::set_border(h,g);
    halfedge_descriptor nh = next(h, g);
    bool h_border = internal::is_border(opposite(h, g),g);
    bool nh_bborder = internal::is_border(opposite(nh, g),g);

    if(h_border && nh_bborder && next(opposite(nh, g), g) == opposite(h, g)) {
      remove_vertex(target(h, g), g);
      if(h != end)
        remove_edge(edge(h, g), g);
    } else {
      if(nh_bborder) {
        internal::set_vertex_halfedge(opposite(next(opposite(nh, g), g), g), g);
        internal::remove_tip(h, g);
        //internal::set_constant_vertex_is_border(g, target(h, g));
      }
      if(h_border) {
        internal::set_vertex_halfedge(opposite(next(h, g), g), g);
        internal::remove_tip(prev(opposite(h, g), g), g);
        //internal::set_constant_vertex_is_border(g, target(prev(opposite(h, g), g), g));
        if(h != end)
          remove_edge(edge(h, g), g);
      }
    }
    h = nh;
  } while(h != end);
  remove_face(f, g);
  if(internal::is_border(opposite(h, g),g))
    remove_edge(edge(h, g), g);
}

  /**
   * removes the incident face of `h` and changes all halfedges incident to the face into border halfedges.
   * \returns `h`. See `remove_face(g,h)` for a more generalized variant.
   *
   * \pre None of the incident halfedges of the face is a border halfedge.
   */
template< typename Graph>
void make_hole(typename boost::graph_traits<Graph>::halfedge_descriptor h,
               Graph& g)
{
  typedef typename boost::graph_traits<Graph>            Traits;
  typedef typename Traits::halfedge_descriptor           halfedge_descriptor;
  typedef typename Traits::face_descriptor               face_descriptor;
  typedef Halfedge_around_face_iterator<Graph>                   halfedge_around_face_iterator;

  CGAL_precondition(! internal::is_border(h,g));
  face_descriptor fd = face(h, g);
  halfedge_around_face_iterator hafib, hafie;
  for(boost::tie(hafib, hafie) = halfedges_around_face(h, g); 
      hafib != hafie; 
      ++hafib){
    CGAL_assertion(! internal::is_border(opposite(*hafib,g),g));
    internal::set_border(*hafib, g);
  }
  remove_face(fd,g);  
}



/** 
 * creates a barycentric triangulation of the face incident to `h`. Creates a new
 * vertex and connects it to each vertex incident to `h` and splits `face(h, g)` 
 * into triangular faces.
 * `h` remains incident to
 * the original face. The time complexity is linear in the size of the face.
 *
 * \image html euler_create_center.png
 *
 * \returns the halfedge `next(h, g)` after the
 * operation, i.e., a halfedge pointing to the new vertex. 
 *
 *  \pre `h` is not a border halfedge.
 *
 * \param g the graph
 * \param h halfedge descriptor
 * \tparam Graph must be a model of `MutableFaceGraph`
 * \sa `remove_center_vertex()`
 *
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
add_center_vertex(typename boost::graph_traits<Graph>::halfedge_descriptor h,
                  Graph& g) 
{
  typedef typename boost::graph_traits<Graph>              Traits;
  typedef typename Traits::vertex_descriptor               vertex_descriptor;
  typedef typename Traits::halfedge_descriptor             halfedge_descriptor;
  typedef typename Traits::face_descriptor                 face_descriptor;

  halfedge_descriptor hnew = halfedge(add_edge(g),g);
  vertex_descriptor vnew = add_vertex(g);
  internal::close_tip(hnew, vnew, g);
  internal::insert_tip(opposite(hnew, g), h, g);
  set_face(hnew, face(h, g), g);
  set_halfedge(face(h,g), h, g);
  halfedge_descriptor h2 = next(opposite(hnew, g), g);
  while ( next(h2, g) != hnew) {
    halfedge_descriptor gnew = halfedge(add_edge(g),g);
    internal::insert_tip( gnew, hnew, g);
    internal::insert_tip( opposite(gnew,g), h2, g);
    face_descriptor fnew = add_face(g);
    set_face( h2, fnew, g);
    set_face( gnew, fnew, g);
    set_face( next(gnew,g), fnew, g);
    set_halfedge(face(h2, g), h2, g);
    h2 = next(opposite(gnew, g), g);
  }
  set_face(next(hnew,g), face(hnew,g), g);
  internal::set_vertex_halfedge(hnew, g);
  return hnew;
}

/**
 * removes the vertex `target(h, g)` and all incident halfedges thereby merging all
 * incident faces.   The resulting face may not be triangulated. 
 * This function is the inverse operation of `add_center_vertex()`. 
 * The invariant `h == remove_center_vertex(g, add_center_vertex(g, h))` 
 * holds, if `h` is not a border halfedge. 
 *
 * \image html euler_erase_center.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 *
 * \param g the graph
 * \param h halfedge descriptor
 *
 * \returns `prev(h, g)`
 *
 * \pre None of the incident faces of `target(h,g)` is a
 * hole. There are at least two distinct faces incident to the faces
 * that are incident to `target(h,g)`. (This prevents the
 * operation from collapsing a volume into two faces glued together
 * with opposite orientations, such as would happen with any vertex of
 * a tetrahedron.)
 *
 * \sa `add_center_vertex()`
 *
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
remove_center_vertex(typename boost::graph_traits<Graph>::halfedge_descriptor h,
                     Graph& g) 
{
  typedef typename boost::graph_traits<Graph>              Traits;
  typedef typename Traits::vertex_descriptor               vertex_descriptor;
  typedef typename Traits::halfedge_descriptor             halfedge_descriptor;
  typedef typename Traits::face_descriptor                 face_descriptor;

  assert(internal::is_valid(g));
  // h points to the vertex that gets removed
  halfedge_descriptor h2    = opposite(next(h, g), g);
  halfedge_descriptor hret = prev(h, g);
  while (h2 != h) {
    halfedge_descriptor gprev = prev(h2, g);
    internal::set_vertex_halfedge(gprev, g);
    internal::remove_tip(gprev, g);

    remove_face(face(h2, g), g);

    halfedge_descriptor gnext = opposite(next(h2, g), g);
    remove_edge(edge(h2,g), g);
    h2 = gnext;
  }
  internal::set_vertex_halfedge(hret, g);
  internal::remove_tip(hret, g);
  remove_vertex(target(h, g), g);
  remove_edge(edge(h, g), g);
  internal::set_face_in_face_loop(hret, face(hret, g), g);
  set_halfedge(face(hret, g), hret, g);
  return hret;
}

/**
 * appends a new face to the border halfedge `h2` by connecting 
 * the tip of `h2` with the tip of `h1` with two new halfedges and a new vertex 
 * and creating a new face that is incident to `h2`. 
 *
 * \image html add_facet1.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 *
 * \returns the halfedge of the new edge that is incident to the new face
 * and the new vertex.
 *
 * \pre `h1` and `h2` are border halfedges
 * \pre `h1 != h2`,
 * \pre `h1` and `h2` are on the same border.
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
add_vertex_and_face_to_border(typename boost::graph_traits<Graph>::halfedge_descriptor h1,
                              typename boost::graph_traits<Graph>::halfedge_descriptor h2,
                              Graph& g){}

/**
 * appends a new face incident to the border halfedge `h1` and `h2` by connecting the vertex `target(h2,g)` 
 * and the vertex `target(h1,g)` with a new halfedge, and filling this separated part of the hole 
 * with a new face, such that the new face is incident to `h2`. 
 *
 * \image html add_facet2.png
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 *
 * \returns the halfedge of the new edge that is incident to the new face.
 *
 * \pre  `h1` and `h2` are border halfedges, 
 * \pre `h1 != h2`, 
 * \pre `next(h1,g) != h2`,
 * \pre `h1` and `h2` are on the same border.
 */
template<typename Graph>
typename boost::graph_traits<Graph>::halfedge_descriptor
add_face_to_border(typename boost::graph_traits<Graph>::halfedge_descriptor h1,
                   typename boost::graph_traits<Graph>::halfedge_descriptor h2,
                   Graph& g)
{
  CGAL_precondition(internal::is_border(h1,g) == true);
  CGAL_precondition(internal::is_border(h2,g) == true);
  CGAL_precondition(h1 != h2);
  CGAL_precondition(next(h1, g) != h2);

  typename boost::graph_traits<Graph>::face_descriptor f = add_face(g);
  typename boost::graph_traits<Graph>::edge_descriptor e = add_edge(g);
  typename boost::graph_traits<Graph>::halfedge_descriptor 
      newh= halfedge(e, g)
    , newhop = opposite(h1, g);

  set_next(newhop, next(h2, g), g);

  set_next(h2, newh, g);
  
  set_next(newh, next(h1, g), g);

  set_next(h1, newhop, g);
  
  set_target(newh, target(h1, g), g);
  set_target(newhop, target(h2, g), g);

  // make the vertices point to the border halfedge
  set_halfedge(target(h2,g), newhop, g);
  internal::set_border(newhop, g);

  CGAL::Halfedge_around_face_iterator<Graph> hafib,hafie;
  for(boost::tie(hafib, hafie) = halfedges_around_face(newh, g); 
      hafib != hafie; 
      ++hafib){
    set_face(*hafib, f, g);
  }

  set_halfedge(f, newh, g);

  return newh;
}


/**
 * collapses an edge in a graph.
 *
 * \tparam Graph must be a model of `MutableFaceGraph`
 * Let `v0` and `v1` be the source and target vertices, and let `e` and `e'` be the halfedges of edge `v0v1`.
 *
 * For `e`, let `en` and `ep` be the next and previous
 * halfedges, that is `en = next(e, g)`, `ep = prev(e, g)`, and let
 * `eno` and `epo` be their opposite halfedges, that is
 * `eno = opposite(en, g)` and `epo = opposite(ep, g)`.
 * Analoguously, for `e'` define  `en'`, `ep'`, `eno'`, and  `epo'`.
 *
 * Then, after the collapse of edge `v0v1` the following holds for `e` (and analoguously for `e'`)
 *
 * <UL>
 *   <LI>The edge `v0v1` is no longer in `g`.
 *   <LI>The faces incident to edge `v0v1` are no longer in `g`.
 *   <LI>Either `v0`, or `v1` is no longer in `g` while the other remains.
 *       Let `vgone` be the removed vertex and `vkept` be the remaining vertex.
 *   <LI>If `e` was a border halfedge, that is `is_border(e, g) == true`, then `next(ep,g) == en`, and `prev(en,g) == ep`.
 *   <LI>If `e` was not a border halfedge, that is `is_border(e, g) == false`, then `ep` and `epo` are no longer in `g` while `en` and `eno` are kept in `g`.
 *   <LI>For all halfedges `hv` in `halfedges_around_target(vgone, g)`, `target(*hv, g) == vkept` and `source(opposite(*hv, g), g) == vkept`.
 *   <LI>No other incidence information has changed in `g`.
 * </UL>
 * \returns vertex `vkept` (which can be either `v0` or `v1`).
 * \pre g must be a triangulated graph
 * \pre `safisfies_link_condition(g, v0v1) == true`.
 */
template<typename Graph>
typename boost::graph_traits<Graph>::vertex_descriptor
collapse_edge(typename boost::graph_traits<Graph>::edge_descriptor v0v1,
              Graph& g)
{
  typedef boost::graph_traits< Graph > Traits;
  typedef typename Traits::vertex_descriptor          vertex_descriptor;
  typedef typename Traits::edge_descriptor            edge_descriptor;
  typedef typename Traits::halfedge_descriptor            halfedge_descriptor;

  halfedge_descriptor pq = halfedge(v0v1,g);
  halfedge_descriptor qp = opposite(pq, g);
  halfedge_descriptor pt = opposite(prev(pq, g), g);
  halfedge_descriptor qb = opposite(prev(qp, g), g);
  
  bool lTopFaceExists         = ! internal::is_border(pq,g);
  bool lBottomFaceExists      = ! internal::is_border(qp,g);
  bool lTopLeftFaceExists     = lTopFaceExists    && ! internal::is_border(pt,g);
  bool lBottomRightFaceExists = lBottomFaceExists && ! internal::is_border(qb,g);

  CGAL_precondition( !lTopFaceExists    || (lTopFaceExists    && ( degree(target(pt, g), g) > 2 ) ) ) ;
  CGAL_precondition( !lBottomFaceExists || (lBottomFaceExists && ( degree(target(qb, g), g) > 2 ) ) ) ;

  vertex_descriptor q = target(pq, g);
  vertex_descriptor p = source(pq, g);
#if 0
  if(lTopLeftFaceExists && lBottomRightFaceExists){
    std::cerr <<    " // do it low level" << std::endl;
    halfedge_descriptor qt = next(pq,g);
    halfedge_descriptor pb = next(qp,g);
    halfedge_descriptor ppt = prev(pt,g);
    halfedge_descriptor pqb = prev(qb,g);
    if(halfedge(q,g) == pq){
      set_halfedge(q, pqb,g);
    }
    vertex_descriptor t = target(qt,g);
    if(halfedge(t,g) == pt){
      set_halfedge(t, qt,g);
    } 
    vertex_descriptor b = target(pb,g);
    if(halfedge(b,g) == qb){
      set_halfedge(t, pb,g);
    }
    set_face(qt, face(pt,g),g);
    set_halfedge(face(qt,g),qt,g);
    set_face(pb, face(qb,g),g);
    set_halfedge(face(pb,g),pb,g);
    set_next(qt, next(pt,g),g);
    set_next(pb, next(qb,g),g);
    set_next(ppt, qt,g);
    set_next(pqb,pb,g);
    remove_face(face(pq,g),g);
    remove_face(face(qp,g),g);
    remove_edge(v0v1,g);
    remove_edge(edge(pt,g),g);
    remove_edge(edge(qb,g),g);
    remove_vertex(p,g);
    Halfedge_around_target_circulator<Graph> beg(ppt,g), end(pqb,g);
    while(beg != end){
      assert(target(*beg,g) == p);
      set_target(*beg,q,g);
      --beg;
    }

    return q;
    // return the vertex kept
  }
#endif

  bool lP_Erased = false, lQ_Erased = false ;

  if ( lTopFaceExists )
  { 
    CGAL_precondition( ! internal::is_border(opposite(pt, g),g) ) ; // p-q-t is a face of the mesh
    if ( lTopLeftFaceExists )
    {
      //CGAL_ECMS_TRACE(3, "Removing p-t E" << pt.idx() << " (V" 
      //                << p.idx() << "->V" << target(pt, g).idx() 
      //                << ") by joining top-left face" ) ;

      join_face(pt,g);
    }
    else
    {
      //CGAL_ECMS_TRACE(3, "Removing p-t E" << pt.idx() << " (V" << p.idx() 
      //                << "->V" << target(pt, g).idx() << ") by erasing top face" ) ;

      remove_face(opposite(pt, g),g);

      if ( !lBottomFaceExists )
      {
        //CGAL_ECMS_TRACE(3, "Bottom face doesn't exist so vertex P already removed" ) ;

        lP_Erased = true ;
      }  
    } 
  }

  if ( lBottomFaceExists )
  {   
    CGAL_precondition( ! internal::is_border(opposite(qb, g),g) ) ; // p-q-b is a face of the mesh
    if ( lBottomRightFaceExists )
    {
      //CGAL_ECMS_TRACE(3, "Removing q-b E" << qb.idx() << " (V" 
      //                << q.idx() << "->V" << target(qb, g).idx() 
      //                << ") by joining bottom-right face" ) ;

      join_face(qb,g);
    }
    else
    {
      //CGAL_ECMS_TRACE(3, "Removing q-b E" << qb.idx() << " (V" 
      //                << q.idx() << "->V" << target(qb, g).idx() 
      //                << ") by erasing bottom face" ) ;

      remove_face(opposite(qb, g),g);

      if ( !lTopFaceExists )
      {
        //CGAL_ECMS_TRACE(3, "Top face doesn't exist so vertex Q already removed" ) ;
        lQ_Erased = true ;
      }  
    }
  }

  CGAL_assertion( !lP_Erased || !lQ_Erased ) ;

  if ( !lP_Erased && !lQ_Erased )
  {
    //CGAL_ECMS_TRACE(3, "Removing vertex P by joining pQ" ) ;

    join_vertex(pq,g);
    lP_Erased = true ;
  }    
  
  CGAL_assertion(g.is_valid());

  return lP_Erased ? q : p ;
}


template<typename Graph, typename EdgeIsConstrainedMap>
typename boost::graph_traits<Graph>::vertex_descriptor
collapse_edge(typename boost::graph_traits<Graph>::edge_descriptor v0v1,
              Graph& g
              , EdgeIsConstrainedMap Edge_is_constrained_map)
{
  CGAL_assertion( !get(Edge_is_constrained_map,pq) );

  typedef boost::graph_traits< Graph > Traits;
  typedef typename Traits::vertex_descriptor          vertex_descriptor;
  typedef typename Traits::edge_descriptor            edge_descriptor;
  typedef typename Traits::halfedge_descriptor            halfedge_descriptor;

  halfedge_descriptor pq = halfedge(v0v1,g);

  halfedge_descriptor qp = opposite(pq,g);
  halfedge_descriptor pt = opposite(prev(pq,g),g);
  halfedge_descriptor qb = opposite(prev(qp,g),g);
  halfedge_descriptor tq = opposite(next(pq,g),g);
  halfedge_descriptor bp = opposite(next(qp,g),g);

  bool lTopFaceExists         = ! internal::is_border(pq,g) ;
  bool lBottomFaceExists      = ! internal::is_border(qp,g) ;

  vertex_descriptor q = target(pq,g);
  vertex_descriptor p = source(pq,g);

  //used to collect edges to remove from the surface
  halfedge_descriptor edges_to_erase[2];
  halfedge_descriptor* edges_to_erase_ptr=edges_to_erase;

  // If the top facet exists, we need to choose one out of the two edges which one disappears:
  //   p-t if it is not constrained and t-q otherwise
  if ( lTopFaceExists )
  {
    if ( !get(Edge_is_constrained_map,pt) )
    {
      *edges_to_erase_ptr++=pt;
    }
    else
    {
      *edges_to_erase_ptr++=tq;
    }
  }

  // If the bottom facet exists, we need to choose one out of the two edges which one disappears:
  //   q-b if it is not constrained and b-p otherwise
  if ( lBottomFaceExists )
  {
    if ( !get(Edge_is_constrained_map,qb) )
    {
      *edges_to_erase_ptr++=qb;
    }
    else{
      *edges_to_erase_ptr++=bp;
    }
  }

  if (lTopFaceExists && lBottomFaceExists)
  {
    if ( face(edges_to_erase[0],g) == face(edges_to_erase[1],g)
         && (! internal::is_border(edges_to_erase[0],g)) )
    {
      // the vertex is of valence 3 and we simply need to remove the vertex
      // and its indicent edges
      bool lP_Erased = false;
      halfedge_descriptor edge =
        next(edges_to_erase[0],g) == edges_to_erase[1]?
          edges_to_erase[0]:edges_to_erase[1];
      if (target(edge,g) == p)
        lP_Erased = true;
      remove_center_vertex(edge,g);
      return lP_Erased? q : p;
    }
    else
    {
      if (!(internal::is_border(edges_to_erase[0],g)))
        join_face(edges_to_erase[0],g);
      else
        g.erase_facet(edges_to_erase[0]->opposite());
      if (!edges_to_erase[1]->is_border())
        join_face(edges_to_erase[1],g);
      else
        remove_face(opposite(edges_to_erase[1],g),g);
      join_vertex(pq,g);
      return q;
    }
  }
  else
  {
      if (lTopFaceExists)
      {
        if (!(internal::is_border(edges_to_erase[0],g))){
          join_face(edges_to_erase[0],g);
          join_vertex(pq,g);
          return q;
        }
        bool lQ_Erased = internal::is_border(opposite(next(pq,g),g),g);
        remove_face(opposite(edges_to_erase[0],g),g);
        return lQ_Erased?p:q;
      }

      if (! (internal::is_border(edges_to_erase[0],g))){
        join_face(edges_to_erase[0],g);
        join_vertex(qp,g);
        return p;
      }
      bool lP_Erased= internal::is_border(opposite(next(qp,g),g),g);
      remove_face(opposite(edges_to_erase[0],g),g);
      return lP_Erased?q:p;
  };
}


template<typename Graph>
void
flip_edge(typename boost::graph_traits<Graph>::halfedge_descriptor h,
          Graph& g)
{
  typedef boost::graph_traits<Graph> Traits;
  typedef typename Traits::vertex_descriptor   vertex_descriptor;
  typedef typename Traits::halfedge_descriptor halfedge_descriptor;
  typedef typename Traits::face_descriptor     face_descriptor;

  vertex_descriptor s = source(h,g); 
  vertex_descriptor t = target(h,g); 
  halfedge_descriptor nh = next(h,g), nnh = next(nh,g), oh = opposite(h,g), noh = next(oh,g), nnoh = next(noh,g);
  vertex_descriptor s2 = target(nh,g), t2 = target(noh,g);
  face_descriptor fh = face(h,g), foh = face(oh,g);

  assert(fh != Traits::null_face() && foh != Traits::null_face());

  if(halfedge(s,g) == oh){
    set_halfedge(s,nnh,g);
  }
  if(halfedge(t,g) == h){
    set_halfedge(t,nnoh,g);
  }
  set_next(h,nnoh,g);
  set_next(oh,nnh,g);
  set_target(h,t2,g);
  set_target(oh,s2,g);
  set_next(nh,h,g);
  set_next(noh,oh,g);
  set_next(nnoh,nh,g);
  set_next(nnh,noh,g);
  set_face(nnoh,fh,g);
  set_face(nnh,foh,g);
  set_halfedge(fh,h,g);
  set_halfedge(foh,oh,g);
}


  ///\cond DO_NOT_INCLUDE_IN_SUBMISSION

  /**  collapses `edge(coll,g)` and removes the edges `edge(rm0,g)` and `edge(rm1,g)`
   *   \todo  implement
   *   \todo finish doc
   */  

template<typename Graph>
typename boost::graph_traits<Graph>::vertex_descriptor
collapse_edge(typename boost::graph_traits<Graph>::halfedge_descriptor coll,
              typename boost::graph_traits<Graph>::halfedge_descriptor rm0,
              typename boost::graph_traits<Graph>::halfedge_descriptor rm1,
              Graph& g)
{}

  ///\endcond

	


/**
 *  \returns `true` if e satisfy the *link condition* \cgalCite{degn-tpec-98}, which guarantees that the surface is also 2-manifold after the edge collapse.
 * \todo implement the function
 */
  template<typename Graph>
bool
  safisfies_link_condition(typename boost::graph_traits<Graph>::edge_descriptor e,
                           Graph& g)
{
  return true;
}


/// @}

} // CGAL

} // CGAL


#endif /* CGAL_EULER_OPERATIONS_H */
