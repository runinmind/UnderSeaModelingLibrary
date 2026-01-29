/**
 * @file bp_cylinder.h
 * Models the beam pattern of a cylindrical array using direct summation
 * of the steering vector inner product (no separable approximation).
 */

#pragma once

#include <usml/beampatterns/bp_model.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>

#include <boost/numeric/ublas/vector.hpp>

namespace usml {
namespace beampatterns {

/// @ingroup beampatterns
/// @{

/**
 * Models a uniform cylindrical array (UCA + VLA) using exact numerical
 * evaluation of the beam response for arbitrary steering directions.
 *
 * The array consists of:
 * - M elements uniformly distributed around a circle of radius R (azimuth)
 * - K elements uniformly spaced along the vertical (up) axis with spacing d_z
 *
 * The beam pattern is computed as:
 * \f[
 * b(\mathbf{u}_a, \mathbf{u}_s, f) =
 * \left| \frac{1}{MK} \sum_{m=0}^{M-1} \sum_{k=0}^{K-1}
 * e^{j k_0 \left[ \mathbf{u}_a - \mathbf{u}_s \right]^\top \mathbf{r}_{m,k}}
 * \right| \f]
 *
 * where:
 * - \f$\mathbf{u}_a\f$ = arrival direction (unit vector in body frame),
 * - \f$\mathbf{u}_s\f$ = steering direction (unit vector),
 * - \f$\mathbf{r}_{m,k}\f$ = position of sensor (m,k),
 * - \f$k_0 = 2\pi f / c\f$.
 *
 * This implementation separates the double sum into:
 * - Azimuthal sum: computed numerically over M elements,
 * - Elevation sum: evaluated in closed form as a Dirichlet kernel (VLA).
 *
 * Note: The elevation factor assumes uniform linear spacing along "up",
 * and uses the standard line array formula based on sin(¦È) difference.
 */
class USML_DECLSPEC bp_cylinder : public bp_model {
   public:
    /**
     * Constructs a cylinder beam pattern model.
     *
     * @param radius      Radius of the cylinder (m).                        -R
     * @param num_elem_az Number of elements in azimuth (around the circle). -M
     * @param num_elem_el Number of elements in elevation (along up axis).   -K
     * @param spacing_el  Element spacing in elevation (m).
     * @param back_baffle Set gain to zero for arrivals with front <= 0 when true.
     */
    bp_cylinder(double radius, unsigned num_elem_az, unsigned num_elem_el,
                   double spacing_el, bool back_baffle = false);

     /**
     * Computes beam level (linear magnitude) for given arrival directions,
     * frequencies, and steering direction.
     *
     * Output is the magnitude of the beamformed output (not squared).
     * Matches the interface of bp_planar and bp_line.
     */
    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const;

    /**
     * Computes directivity (numerical integration fallback).
     */
    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const;

   private:
    /// Compute azimuthal beam factor using Bessel series.
    vector<double> azimuth_factor(const seq_vector::csptr& frequencies,
                                  const bvector& arrival,
                                  const bvector& steering = bvector(1.0, 0.0, 0.0),
                                  double sound_speed = 1500.0) const;

    /// Cylinder radius (m).
    const double _radius;
    /// Number of elements in azimuth (M).
    const unsigned _num_elem_az;
    /// Number of elements in elevation (K).
    const unsigned _num_elem_el;
    /// Element spacing in elevation (m).
    const double _spacing_el;
    /// Use back baffle (zero gain for rear hemisphere).
    const bool _back_baffle;
};

/// @}
}  // namespace beampatterns
}  // namespace usml