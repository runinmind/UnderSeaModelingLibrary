/**
 * @file beampattern_utilities.h
 * A collection of utilities for helping to build and configure beampatterns.
 */
#pragma once

#include <usml/types/types.h>
#include <usml/usml_config.h>

namespace usml {
namespace beampatterns {

using namespace usml::types;

/// @ingroup beampatterns
/// @{

/**
 * @ingroup beampatterns
 * @{
 * @defgroup beampattern_utilities Beampattern Utilities
 * @{
 * A collection of utilities for helping to build and configure beampatterns
 * @}
 * @}
 *
 * @ingroup beampattern_utilities
 * @{
 */

/**
 * Provides the element locations of a circular planar array on the
 * front/right plane
 *
 * @param radii			    The radius of each ring
 * @param num_elements	    The number of elements in each ring
 * @param offsets		    The offset, in radians, for the offset of
 * the first element of each ring
 * @param elem_locations    The returned element locations in meters in an
 * 						    Nx3 matrix where N is the
 * sum of the elements in num_elements
 */
void USML_DECLSPEC bp_con_ring(vector<double> radii, vector<int> num_elements,
                 vector<double> offsets,
                 matrix<double> *elem_locations = nullptr);

/**
 * Provides the element locations of a uniformly spaced array in
 * 3 dimensions
 *
 * @param num_e_front	    Number of elements in the front/back direction
 * @param spacing_front	    Spacing, in meters, in the front/back direction
 * @param num_e_right        Number of elements in the right/left direction
 * @param spacing_right      Spacing, in meters, in the right/left direction
 * @param num_e_up          Number of elements in the up/down direction
 * @param spacing_up        Spacing, in meters, in the up/down direction
 * @param elem_locations    The returned element locations in meters in an
 * 						    Nx3 matrix where N is the
 * sum of the elements in num_elements. In front-right-up order
 */
void USML_DECLSPEC bp_con_uniform(int num_e_front, double spacing_front,
                                 int num_e_right,
                    double spacing_right, int num_e_up, double spacing_up,
                    matrix<double> *elem_locations = nullptr);

/**
 * Provides the element locations of a cylinder array.
 * 
 * @param radius            Radius of the cylinder (m).                        -R
 * @param num_elem_az       Number of elements in azimuth (around the circle). -M
 * @param num_e_up          Number of elements in the up/down direction        -K
 * @param spacing_up        Spacing, in meters, in the up/down direction
 * @param elem_locations    The returned element locations in meters in an
 * 						    Nx3 matrix where N is the sum of the elements in num_elements. 
 *                          In front-right-up order.
 * @param offset            The offset, in radians, for the offset of each ring
 */
void USML_DECLSPEC bp_con_cylinder(double radius, int num_elem_az, int num_e_up,
                                   double spacing_up,
                                   matrix<double> *elem_locations = nullptr,
                                   double offset = 0.0);

/**
 * Provides the element locations of a spherical array.
 *
 * @param radius            Radius of the sphere (m).
 * @param num_elem_az       Number of elements in azimuth (around each latitude ring).
 * @param num_elem_el       Number of elements in elevation (between min and max elevation).
 * @param elem_locations    The returned element locations in meters in an
 *                          Nx3 matrix where N = num_elem_az * num_elem_el.
 *                          In front-right-up order.
 * @param min_el_angle      Minimum elevation angle in radians [-¦Ð/2 + ¦Å, 0].
 * @param max_el_angle     Maximum elevation angle in radians [0, ¦Ð/2 - ¦Å].
 */
void USML_DECLSPEC bp_con_sphere(double radius, int num_elem_az,
                                 int num_elem_el,
                                 matrix<double> *elem_locations = nullptr,
                                 double min_el_angle = -M_PI_2 + 1e-6,
                                 double max_el_angle = M_PI_2 - 1e-6);
/// @}
}  // namespace beampatterns
}  // namespace usml
