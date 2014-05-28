// Copyright (c) 2007  GeometryFactory (France).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 3 of the License,
// or (at your option) any later version.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL$
// $Id$
//
//
// Author(s)     : Andreas Fabri, Fernando Cacciola

#ifndef CGAL_BOOST_GRAPH_PROPERTIES_POLYHEDRON_3_H
#define CGAL_BOOST_GRAPH_PROPERTIES_POLYHEDRON_3_H

#include <CGAL/boost/graph/properties.h>
#include <CGAL/boost/graph/graph_traits_Polyhedron_3.h>
#include <CGAL/Unique_hash_map.h>
#include <CGAL/squared_distance_2_1.h>

#include <functional>
#include <CGAL/boost/graph/function_property_map.h>

#define CGAL_HDS_PARAM_ template < class Traits, class Items, class Alloc> class HDS

namespace CGAL {

namespace internal {

template<class Handle>
class Polyhedron_index_map_external
  : public boost::put_get_helper<std::size_t, Polyhedron_index_map_external<Handle> >
{
public:
  typedef boost::readable_property_map_tag category;
  typedef std::size_t                      value_type;
  typedef std::size_t                      reference;
  typedef Handle                           key_type;

  template <typename InputIterator>
  Polyhedron_index_map_external(InputIterator begin, InputIterator end, std::size_t max)
    : map_(begin, end, 0, std::size_t(-1), max) {}

  reference operator[](const key_type& k) const { return map_[k]; }
private:
  CGAL::Unique_hash_map<key_type,std::size_t> map_;
};

// Special case for edges. Hashes each half_edge.
template<class Polyhedron>
class Polyhedron_edge_index_map_external
  : public boost::put_get_helper<std::size_t, Polyhedron_edge_index_map_external<Polyhedron> >
{
public:
  typedef boost::readable_property_map_tag                              category;
  typedef std::size_t                                                   value_type;
  typedef std::size_t                                                   reference;
  typedef typename boost::graph_traits<Polyhedron>::halfedge_descriptor key_type;

  Polyhedron_edge_index_map_external(Polyhedron& p)
    : map_(std::size_t(-1), num_halfedges(p))
  {
    unsigned int data = 0;
    typename boost::graph_traits<Polyhedron>::edge_iterator it, end;
    for(boost::tie(it, end) = edges(p); it != end; ++it, ++data) {
      map_[halfedge(*it, p)] = data;
      map_[opposite(halfedge(*it, p), p)] = data;
    }
  }

  reference operator[](const key_type& k) const { return map_[k]; }
private:
  CGAL::Unique_hash_map<typename boost::graph_traits<Polyhedron>::halfedge_descriptor,
                        std::size_t> map_;
};

template<typename Handle>
struct Wrap_squared
  : boost::put_get_helper< double, Wrap_squared<Handle> >
{
  typedef double value_type;
  typedef double reference;
  typedef Handle key_type;
  typedef boost::readable_property_map_tag category;

  template<typename E>
  double
  // TODO Kernel::FT, this is as bad as the old code, we need to
  // forward the halfedge type in HDS_edge to extract the kernel here.
  // Technically, there is also a general implementation for
  // HalfedgeGraph to prevent duplication in Surface_mesh.
  operator[](const E& e) const {
    return CGAL::squared_distance(e.halfedge()->vertex()->point(), e.halfedge()->opposite()->vertex()->point());
  }
};

  template<typename Polyhedron, typename Handle>
struct Index_accessor
    : boost::put_get_helper< std::size_t&, Index_accessor<Polyhedron,Handle> >
{
  typedef boost::lvalue_property_map_tag category;
  typedef std::size_t&                   reference;
  typedef std::size_t                    value_type;
  typedef Handle                         key_type;

  reference operator[](Handle h) const { return h->id(); }
};

template<typename Handle>
struct Edge_index_accessor
  : boost::put_get_helper< std::size_t, Edge_index_accessor<Handle> >
{
  typedef boost::readable_property_map_tag category;
  typedef std::size_t                      reference;
  typedef std::size_t                      value_type;
  typedef Handle                           key_type;

  reference operator[](Handle h) const { return h.id(); }
};

template<typename Polyhedron, typename Handle>
struct Is_border_accessor
{

  typedef boost::read_write_property_map_tag category;
  typedef bool                             value_type;
  typedef bool                             reference;
  typedef Handle                           key_type;

  friend reference get(Is_border_accessor pm, Handle h)
  {
    return h->is_border(); 
  }
  friend void put(Is_border_accessor pm, Handle h, bool b)
  {
     typedef typename Polyhedron::Halfedge::Base Sneak;
     if(b){
       static_cast<Sneak&>(*h).set_face(boost::graph_traits<Polyhedron>::null_face());
     }
  }
private:
  //  const Polyhedron& p;
};


template<typename Polyhedron, typename Handle>
struct Is_border_edge_accessor
  : boost::put_get_helper< bool, Is_border_edge_accessor<Polyhedron,Handle> >
{
  typedef boost::readable_property_map_tag category;
  typedef bool                             value_type;
  typedef bool                             reference;
  typedef Handle                           key_type;

  reference operator[](const Handle& h) const 
  {
    return h.halfedge()->is_border() || h.halfedge()->opposite()->is_border();
  }
};




template<typename Handle, typename ValueType, typename Reference>
struct Point_accessor
  : boost::put_get_helper< Reference, Point_accessor<Handle, ValueType, Reference> >
{
  typedef boost::lvalue_property_map_tag category;
  typedef Reference                      reference;
  typedef ValueType                      value_type;
  typedef Handle                         key_type;

  reference operator[](Handle h) const { return h->point(); }
};

} // internal

// the tag we dispatch on from property_map<G, Property>
template <class Tag>
struct Polyhedron_property_map {};

} // namespace CGAL

namespace CGAL {

// generalized 2-ary get functions
template<class Gt, class I, CGAL_HDS_PARAM_, class A, class PropertyTag>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag >::const_type
get(PropertyTag, CGAL::Polyhedron_3<Gt,I,HDS,A> const&)
{ return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag >::const_type(); }

template<class Gt, class I, CGAL_HDS_PARAM_, class A, class PropertyTag>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag >::type
get(PropertyTag, CGAL::Polyhedron_3<Gt,I,HDS,A>&)
{ return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag >::type(); }

// generalized 3-ary get functions
template<class Gt, class I, CGAL_HDS_PARAM_, class A, class PropertyTag, class Key>
typename boost::property_traits< typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag >::type >::reference
get(PropertyTag p, CGAL::Polyhedron_3<Gt,I,HDS,A>& g, const Key& key)
{ return get(get(p, g), key); }

template<class Gt, class I, CGAL_HDS_PARAM_, class A, class PropertyTag, class Key>
typename boost::property_traits< typename boost::property_map<CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag >::const_type >::reference
get(PropertyTag p, CGAL::Polyhedron_3<Gt,I,HDS,A> const& g, const Key& key)
{ return get(get(p, g), key); }

// generalized put
template<class Gt, class I, CGAL_HDS_PARAM_, class A, class PropertyTag, class Key,class Value>
void put(PropertyTag p, CGAL::Polyhedron_3<Gt,I,HDS,A>& g, const Key& key, const Value& value)
{
  typedef typename boost::property_map<CGAL::Polyhedron_3<Gt,I,HDS,A>, PropertyTag>::type Map;
  Map pmap = get(p, g);
  put(pmap, key, value);
}

} // boost

// specialization needs to be repeated for halfedge, vertex, face
#define CGAL_POLYHEDRON_INDEX_PM(ENTITY, TAG, ACCESSOR)                 \
  namespace CGAL {                                                      \
  template<> struct Polyhedron_property_map<boost::ENTITY##TAG> {  \
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>                 \
  struct bind_ {                                                        \
    typedef internal::ACCESSOR##_accessor<                              \
    CGAL::Polyhedron_3<Gt, I, HDS, A>, \
    typename boost::graph_traits< CGAL::Polyhedron_3<Gt, I, HDS, A>     \
                                  >::ENTITY##_descriptor > type;        \
    typedef type const_type;                                            \
  };                                                                    \
  };                                                                    \
  }

CGAL_POLYHEDRON_INDEX_PM(halfedge, _index_t, Index)
CGAL_POLYHEDRON_INDEX_PM(vertex, _index_t, Index)
CGAL_POLYHEDRON_INDEX_PM(face, _index_t, Index)
CGAL_POLYHEDRON_INDEX_PM(halfedge, _is_border_t, Is_border)

CGAL_POLYHEDRON_INDEX_PM(edge, _is_border_t, Is_border_edge)

#undef CGAL_POLYHEDRON_INDEX_PM

namespace CGAL {
// not done with macros, because HDS_edge::id does not return a
// reference
template <>
struct Polyhedron_property_map<boost::edge_index_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Edge_index_accessor<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::edge_descriptor > type;
    typedef type const_type;
  };
};

template <>
struct Polyhedron_property_map<boost::edge_weight_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Wrap_squared<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::edge_descriptor > type;
    typedef type const_type;
  };
};

template <>
struct Polyhedron_property_map<vertex_point_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Point_accessor<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::vertex_descriptor,
      typename Gt::Point_3, typename Gt::Point_3&> type;

    typedef internal::Point_accessor<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::vertex_descriptor,
      typename Gt::Point_3, const typename Gt::Point_3&> const_type;
  };
};

//
// external indices
//

template <>
struct Polyhedron_property_map<edge_external_index_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Polyhedron_edge_index_map_external<
      CGAL::Polyhedron_3<Gt, I, HDS, A>
      > type;
    typedef type const_type;
  };
};

template <>
struct Polyhedron_property_map<halfedge_external_index_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Polyhedron_index_map_external<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::halfedge_descriptor > type;
    typedef type const_type;
  };
};


template <>
struct Polyhedron_property_map<vertex_external_index_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Polyhedron_index_map_external<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::vertex_descriptor > type;
    typedef type const_type;
  };
};

template <>
struct Polyhedron_property_map<face_external_index_t>
{
  template<class Gt, class I, CGAL_HDS_PARAM_, class A>
  struct bind_
  {
    typedef internal::Polyhedron_index_map_external<
      typename boost::graph_traits<
        CGAL::Polyhedron_3<Gt, I, HDS, A>
        >::face_descriptor > type;
    typedef type const_type;
  };
};

} // CGAL

namespace CGAL {

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::edge_external_index_t >::const_type
get(boost::edge_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> const& p)
{
  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::edge_external_index_t >::const_type(
    const_cast<CGAL::Polyhedron_3<Gt,I,HDS,A>& >(p));
}

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::halfedge_external_index_t >::const_type
get(boost::halfedge_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> const& p)
{
  CGAL::Polyhedron_3<Gt,I,HDS,A>& ncp = const_cast<CGAL::Polyhedron_3<Gt,I,HDS,A>&>(p);

  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::halfedge_external_index_t >::const_type(
    ncp.halfedges_begin(), ncp.halfedges_end(), ncp.size_of_halfedges());
}

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::vertex_external_index_t >::const_type
get(boost::vertex_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> const& p)
{
  CGAL::Polyhedron_3<Gt,I,HDS,A>& ncp = const_cast<CGAL::Polyhedron_3<Gt,I,HDS,A>&>(p);

  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::vertex_external_index_t >::const_type(
    ncp.vertices_begin(), ncp.vertices_end(), ncp.size_of_vertices());
}

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::face_external_index_t >::const_type
get(boost::face_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> const& p)
{
  CGAL::Polyhedron_3<Gt,I,HDS,A>& ncp = const_cast<CGAL::Polyhedron_3<Gt,I,HDS,A>&>(p);

  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::face_external_index_t >::const_type(
    ncp.facets_begin(), ncp.facets_end(), ncp.size_of_facets());
}

// the same blurb for non-const

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::edge_external_index_t >::type
get(boost::edge_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A>& p)
{
  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::edge_external_index_t >::type(
    p);
}

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::halfedge_external_index_t >::type
get(boost::halfedge_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> & ncp)
{
  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::halfedge_external_index_t >::type(
    ncp.halfedges_begin(), ncp.halfedges_end(), ncp.size_of_halfedges());
}

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::vertex_external_index_t >::type
get(boost::vertex_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> & ncp)
{
  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::vertex_external_index_t >::type(
    ncp.vertices_begin(), ncp.vertices_end(), ncp.size_of_vertices());
}


template<class Gt, class I, CGAL_HDS_PARAM_, class A>
typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::face_external_index_t >::type
get(boost::face_external_index_t, CGAL::Polyhedron_3<Gt,I,HDS,A> & ncp)
{
  return typename boost::property_map< CGAL::Polyhedron_3<Gt,I,HDS,A>, boost::face_external_index_t >::type(
    ncp.facets_begin(), ncp.facets_end(), ncp.size_of_facets());
}



} // namespace CGAL


namespace boost {

// property_map dispatcher into Polyhedron
template<class Gt, class I, CGAL_HDS_PARAM_, class A, class Tag>
struct property_map<CGAL::Polyhedron_3<Gt,I,HDS,A>, Tag>
{
  typedef typename CGAL::Polyhedron_property_map<Tag>::
      template bind_<Gt,I,HDS,A> map_gen;
  typedef typename map_gen::type       type;
  typedef typename map_gen::const_type const_type;
};

// What are those needed for ???
template<class Gt, class I, CGAL_HDS_PARAM_, class A>
struct edge_property_type<CGAL::Polyhedron_3<Gt,I,HDS,A> >
{
  typedef edge_weight_t type;
};

template<class Gt, class I, CGAL_HDS_PARAM_, class A>
struct vertex_property_type<CGAL::Polyhedron_3<Gt,I,HDS,A> >
{
  typedef CGAL::vertex_point_t type;
};

} // namespace boost


#undef CGAL_HDS_PARAM_

#endif // CGAL_BOOST_GRAPH_PROPERTIES_POLYHEDRON_3_H
