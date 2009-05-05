// Copyright (c) 2007-09  INRIA Sophia-Antipolis (France).
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
// $URL$
// $Id$
//
// Author(s) : Pierre Alliez and Laurent Saboret and Marc Pouget and Frederic Cazals

#ifndef CGAL_JET_ESTIMATE_NORMALS_H
#define CGAL_JET_ESTIMATE_NORMALS_H

#include <CGAL/value_type_traits.h>
#include <CGAL/Search_traits_3.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Monge_via_jet_fitting.h>
#include <CGAL/point_set_processing_assertions.h>
#include <CGAL/Memory_sizer.h>

#include <iterator>
#include <list>

CGAL_BEGIN_NAMESPACE


// ----------------------------------------------------------------------------
// Private section
// ----------------------------------------------------------------------------
namespace CGALi {


/// Estimate normal direction using jet fitting
/// on the k nearest neighbors.
///
/// @commentheading Precondition: k >= 2.
///
/// @commentheading Template Parameters:
/// @param Kernel Geometric traits class.
/// @param Tree KD-tree.
///
/// @return Computed normal. Orientation is random.
template < typename Kernel,
           typename Tree
>
typename Kernel::Vector_3
jet_estimate_normals(const typename Kernel::Point_3& query, ///< point to compute the normal at
                     Tree& tree, ///< KD-tree
                     unsigned int k, ///< number of neighbors
                     unsigned int degree_fitting)
{
  // basic geometric types
  typedef typename Kernel::Point_3  Point;
  typedef typename Kernel::Vector_3 Vector;

  // types for K nearest neighbors search
  typedef typename CGAL::Search_traits_3<Kernel> Tree_traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::iterator Search_iterator;

  // types for jet fitting
  typedef typename CGAL::Monge_via_jet_fitting<Kernel> Monge_jet_fitting;
  typedef typename Monge_jet_fitting::Monge_form Monge_form;

  // Gather set of (k+1) neighboring points.
  // Perform k+1 queries (as in point set, the query point is
  // output first). Search may be aborted if k is greater
  // than number of input points.
  std::vector<Point> points; points.reserve(k+1);
  Neighbor_search search(tree,query,k+1);
  Search_iterator search_iterator = search.begin();
  unsigned int i;
  for(i=0;i<(k+1);i++)
  {
    if(search_iterator == search.end())
      break; // premature ending
    points.push_back(search_iterator->first);
    search_iterator++;
  }
  CGAL_point_set_processing_precondition(points.size() >= 1);

  // performs jet fitting
  Monge_jet_fitting monge_fit;
  const unsigned int degree_monge = 1; // we seek for normal and not more.
  Monge_form monge_form = monge_fit(points.begin(), points.end(),
                                    degree_fitting, degree_monge);

  // output normal vector (already normalized in monge form)
  return monge_form.normal_direction();
}


} /* namespace CGALi */


// ----------------------------------------------------------------------------
// Public section
// ----------------------------------------------------------------------------


/// Estimate normal directions using jet fitting on the k nearest neighbors.
/// The output normals are randomly oriented.
///
/// @commentheading Precondition: k >= 2.
///
/// @commentheading Template Parameters:
/// @param InputIterator value_type must be convertible to Point_3<Kernel>.
/// @param OutputIterator value_type must be convertible from Vector_3<Kernel>.
/// @param Kernel Geometric traits class. It can be omitted and deduced automatically from the iterator type.
///
/// @return past-the-end output iterator.

// This variant requires the kernel.
template <typename InputIterator,
          typename OutputIterator,
          typename Kernel
>
OutputIterator
jet_estimate_normals(InputIterator first, ///< iterator over the first input point.
                     InputIterator beyond, ///< past-the-end iterator over input points.
                     OutputIterator normals, ///< output normals.
                     unsigned int k, ///< number of neighbors.
                     const Kernel& kernel, ///< geometric traits.
                     unsigned int degree_fitting)
{
  CGAL_TRACE("Call jet_estimate_normals()\n");

  // types for K nearest neighbors search structure
  typedef typename CGAL::Search_traits_3<Kernel> Tree_traits;
  typedef typename CGAL::Orthogonal_k_neighbor_search<Tree_traits> Neighbor_search;
  typedef typename Neighbor_search::Tree Tree;

  // precondition: at least one element in the container.
  // to fix: should have at least three distinct points
  // but this is costly to check
  CGAL_point_set_processing_precondition(first != beyond);

  // precondition: at least 2 nearest neighbors
  CGAL_point_set_processing_precondition(k >= 2);

  long memory = CGAL::Memory_sizer().virtual_size(); CGAL_TRACE("  %ld Mb allocated\n", memory>>20);
  CGAL_TRACE("  Create KD-tree\n");

  // instanciate a KD-tree search
  Tree tree(first,beyond);

  /*long*/ memory = CGAL::Memory_sizer().virtual_size(); CGAL_TRACE("  %ld Mb allocated\n", memory>>20);
  CGAL_TRACE("  Compute normals\n");

  // iterate over input points, compute and output normal
  // vectors (already normalized)
  InputIterator it;
  for(it = first; it != beyond; it++)
  {
    *normals = CGALi::jet_estimate_normals<Kernel,Tree>(*it,tree,k,degree_fitting);
    normals++;
  }

  /*long*/ memory = CGAL::Memory_sizer().virtual_size(); CGAL_TRACE("  %ld Mb allocated\n", memory>>20);
  CGAL_TRACE("End of jet_estimate_normals()\n");

  return normals;
}

/// @cond SKIP_IN_MANUAL
// This variant deduces the kernel from the iterator type.
template <typename InputIterator,
          typename OutputIterator
>
OutputIterator
jet_estimate_normals(InputIterator first, ///< iterator over the first input point
                     InputIterator beyond, ///< past-the-end iterator over input points
                     OutputIterator normals, ///< output normals
                     unsigned int k, ///< number of neighbors
                     unsigned int degree_fitting = 2)
{
  typedef typename std::iterator_traits<InputIterator>::value_type Value_type;
  typedef typename Kernel_traits<Value_type>::Kernel Kernel;
  return jet_estimate_normals(first,beyond,normals,k,Kernel(),degree_fitting);
}
/// @endcond


CGAL_END_NAMESPACE

#endif // CGAL_JET_ESTIMATE_NORMALS_H

