/**
 * @file bp_sphere.cc
 * Implements the beam pattern of a spherical array.
 */

#include <usml/beampatterns/bp_sphere.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <complex>

using namespace usml::beampatterns;
using std::complex;

// Constructor
bp_sphere::bp_sphere(double radius, unsigned num_az, unsigned num_el,
                     double theta_min, double theta_max, bool back_baffle)
    : _radius(radius), 
      _num_az(num_az),
      _num_el(num_el),
      _theta_min(theta_min),
      _theta_max(theta_max),
      _back_baffle(back_baffle) {
    // Precompute elevation angles θ_k
    _theta_k.resize(_num_el);
    if (_num_el == 1) {
        _theta_k[0] = (_theta_min + _theta_max) / 2.0;
    } else {
        double dtheta = (_theta_max - _theta_min) / (_num_el - 1);
        for (unsigned k = 0; k < _num_el; ++k) {
            _theta_k[k] = _theta_min + k * dtheta;
        }
    }
}

void bp_sphere::beam_level(const bvector& arrival,
                           const seq_vector::csptr& frequencies,
                           vector<double>* level, const bvector& steering,
                           double sound_speed) const {
    if (_back_baffle && arrival.front() <= 0.0) {
        *level = vector<double>(frequencies->size(), 0.0);
        return;
    }

    // Extract direction cosines
    double n_a = arrival.front(), e_a = arrival.right(), u_a = arrival.up();
    double n_s = steering.front(), e_s = steering.right(), u_s = steering.up();

    size_t total_sensors = static_cast<size_t>(_num_az) * _num_el;

    for (size_t f = 0; f < frequencies->size(); ++f) {
        double k0 = 2.0 * M_PI * (*frequencies)[f] / sound_speed;
        std::complex<double> sum(0.0, 0.0);

        for (unsigned k = 0; k < _num_el; ++k) {
            double theta_k = _theta_k[k];
            double sin_theta_k = std::sin(theta_k);
            double cos_theta_k = std::cos(theta_k);

            for (unsigned m = 0; m < _num_az; ++m) {
                double phi_m = 2.0 * M_PI * m / _num_az;

                // Arrival phase: k0 * r · u_a
                double phase_arr =
                    k0 * _radius *
                    (n_a * cos_theta_k * std::cos(phi_m) +
                     e_a * cos_theta_k * std::sin(phi_m) + u_a * sin_theta_k);

                // Steering phase: k0 * r · u_s
                double phase_steer =
                    k0 * _radius *
                    (n_s * cos_theta_k * std::cos(phi_m) +
                     e_s * cos_theta_k * std::sin(phi_m) + u_s * sin_theta_k);

                sum += std::exp(
                    std::complex<double>(0.0, phase_arr - phase_steer));
            }
        }

        // Return power pattern: |AF|^2
        (*level)[f] = std::norm(sum) / (total_sensors * total_sensors);
    }
}

void bp_sphere::directivity(const seq_vector::csptr& frequencies,
                            vector<double>* level, const bvector& steering,
                            double sound_speed) const {
    // Fallback to numerical integration (same as bp_model base class)
    bp_model::directivity(frequencies, level, steering, sound_speed);
}