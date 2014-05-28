#include <cassert>
#include "test_meshing_utilities.h"
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Mesh_triangulation_3.h>
#include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#include <CGAL/Mesh_criteria_3.h>
#include <CGAL/Polyhedral_mesh_domain_with_features_3.h>
#include <CGAL/make_mesh_3.h>

#include <sstream>

template <typename K>
struct Polyhedron_tester : public Tester<K>
{
  void polyhedron() const
  {
    // Domain
    typedef CGAL::Polyhedral_mesh_domain_with_features_3<K> Mesh_domain;
    typedef typename CGAL::Mesh_polyhedron_3<K>::type MeshPolyhedron_3;

    // Triangulation
    typedef typename CGAL::Mesh_triangulation_3<Mesh_domain>::type Tr;
    typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr,
      typename Mesh_domain::Corner_index,
      typename Mesh_domain::Curve_segment_index> C3t3;

    // Criteria
    typedef CGAL::Mesh_criteria_3<Tr> Mesh_criteria;

    MeshPolyhedron_3 P;

    std::stringstream poly("OFF\n"
                           "4 4 0\n"
                           "0 0 0\n"
                           "0 0 1\n"
                           "0 1 0\n"
                           "1 0 0\n"
                           "3  3 1 2\n"
                           "3  0 1 3\n"
                           "3  0 3 2\n"
                           "3  0 2 1\n");
    poly >> P;
    assert(P.size_of_vertices() == 4);
    assert(P.size_of_facets() == 4);
    Mesh_domain domain(P);

    // Get sharp features
    domain.detect_features();

    // Mesh criteria
    using namespace CGAL::parameters;
    const double cs = 0.408248;
    Mesh_criteria criteria(edge_size = cs/2.0,
                           facet_angle = 25,
                           facet_size = cs,
                           facet_distance = cs/10.0,
                           cell_radius_edge_ratio = 3,
                           cell_size = cs);
    // Mesh generation
    C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria);

    double vol = 1/6.;
    this->verify_c3t3_volume(c3t3, vol*0.95, vol*1.05);

    this->verify_c3t3(c3t3,domain,Polyhedral_tag(),
                      55, 65, 110, 125, 85, 120);
  }
};

int main() {
  typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
  Polyhedron_tester<K> test_epic;
  std::cerr << "Mesh generation from a polyhedron:\n";
  test_epic.polyhedron();

  return EXIT_SUCCESS;
}
