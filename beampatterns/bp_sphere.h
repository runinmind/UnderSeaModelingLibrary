/**
 * @file bp_sphere.h
 * Models the beam pattern of a spherical array using discrete summation
 * or closed-form solution (when applicable).
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
 * Beam pattern model for a spherical array.
 *
 * Sensors are distributed on a sphere of radius R.
 * - M: number of sensors in azimuth (around equator)
 * - K: number of sensors in elevation (from θ_min to θ_max)
 * - Sensor (m,k) position:
 *   x = R * cos(θ_k) * cos(ϕ_m)
 *   y = R * cos(θ_k) * sin(ϕ_m)
 *   z = R * sin(θ_k)
 * where:
 *   ϕ_m = 2π m / M,    m = 0,...,M-1
 *   θ_k = θ_min + k * Δθ, k = 0,...,K-1
 *   Δθ = (θ_max - θ_min) / (K-1)
 *
 * Coordinate system:
 *   ϕ = 0, θ = 0 → +x axis
 *   θ: elevation angle from xy-plane (θ > 0 → +z)
 *   ϕ: azimuth angle in xy-plane from +x (counterclockwise)
 *
 * Beam response is computed by direct summation over all sensors:
 * \f[
 * AF = \frac{1}{MK} \sum_{m=0}^{M-1} \sum_{k=0}^{K-1}
 * \exp\left\{ j k_0 R \left[
 * \cos\theta_a \cos(\phi_a - \phi_m) \cos\theta_k +
 * \sin\theta_a \sin\theta_k
 * - (\text{same for steering})
 * \right] \right\}
 * \f]
 *
 * Closed-form solution is used only when steering direction is axial
 * (e.g., along +x, +y, +z) and array is symmetric.
 */
class USML_DECLSPEC bp_sphere : public bp_model {
   public:
    /**
     * Constructs a spherical array beam pattern model.
     *
     * @param radius      Sphere radius (m).
     * @param num_az      Number of elements in azimuth (M).
     * @param num_el      Number of elements in elevation (K).
     * @param theta_min   Minimum elevation angle (rad), e.g., -π/2.
     * @param theta_max   Maximum elevation angle (rad), e.g., +π/2.
     * @param back_baffle Set gain to zero for arrivals with front <= 0 when
     * true.
     */
    bp_sphere(double radius, unsigned num_az, unsigned num_el, 
              double theta_min = -M_PI_2 + 1e-6,
              double theta_max = M_PI_2 - 1e-6,
              bool back_baffle = false);

    void beam_level(const bvector& arrival,
                    const seq_vector::csptr& frequencies, vector<double>* level,
                    const bvector& steering = bvector(1.0, 0.0, 0.0),
                    double sound_speed = 1500.0) const override;

    void directivity(const seq_vector::csptr& frequencies,
                     vector<double>* level,
                     const bvector& steering = bvector(1.0, 0.0, 0.0),
                     double sound_speed = 1500.0) const override;

   private:
    /// Sphere radius (m).
    const double _radius;
    /// Number of elements in azimuth (M).
    const unsigned _num_az;
    /// Number of elements in elevation (K).
    const unsigned _num_el;
    /// Minimum elevation angle (rad).
    const double _theta_min;
    /// Maximum elevation angle (rad).
    const double _theta_max;
    /// Use back baffle.
    const bool _back_baffle;

    /// Precomputed elevation angles θ_k.
    std::vector<double> _theta_k;
};

/// @}
}  // namespace beampatterns
}  // namespace usml
