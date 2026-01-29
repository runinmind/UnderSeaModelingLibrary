/**
 * @file bp_cylinder.cc
 * Implements the beam pattern of a cylindrical array using direct summation.
 */

#include <usml/beampatterns/bp_cylinder.h>
#include <usml/types/bvector.h>
#include <usml/types/seq_vector.h>
#include <usml/ublas/vector_math.h>

#include <boost/numeric/ublas/vector.hpp>
#include <cmath>
#include <complex>

using namespace usml::beampatterns;
using std::complex;

// Constructor
bp_cylinder::bp_cylinder(double radius, unsigned num_elem_az, unsigned num_elem_el, 
                        double spacing_el, bool back_baffle)
    : _radius(radius),
      _num_elem_az(num_elem_az),
      _num_elem_el(num_elem_el),
      _spacing_el(spacing_el),
      _back_baffle(back_baffle) {
    // Validate inputs
    if (_num_elem_az == 0 || _radius <= 0.0) {
        throw std::invalid_argument(
            "Cylindrical array requires num_elem_az > 0 and radius > 0");
    }
    if (_num_elem_el == 0 || _spacing_el <= 0.0) {
        throw std::invalid_argument(
            "Elevation array requires num_elem_el > 0 and spacing_el > 0");
    }
}

vector<double> bp_cylinder::azimuth_factor(const seq_vector::csptr& frequencies, 
    const bvector& arrival, const bvector& steering, double sound_speed) const {
    // Compute cos(theta) and phi for arrival and steering
    double cos_theta_a = sqrt(arrival.front() * arrival.front() + arrival.right() * arrival.right());
    double cos_theta_s = sqrt(steering.front() * steering.front() + steering.right() * steering.right());
    double phi_a = std::atan2(arrival.right(), arrival.front());
    double phi_s = std::atan2(steering.right(), steering.front());

    // Precompute wave number scale: k = 2¦Ðf / c
    vector<double> kR = frequencies->data();
    kR *= 2.0 * M_PI / sound_speed * _radius;

    // for every freq
    vector<double> az_gain(frequencies->size(), 0.0);
    for (unsigned f_idx = 0; f_idx < frequencies->size(); ++f_idx) {
        complex<double> sum(0.0, 0.0);
        complex<double> A(0.0, -kR[f_idx]);

        for (unsigned m = 0; m < _num_elem_az; ++m) {
            double alpha = 2 * M_PI * static_cast<double>(m) / static_cast<double>(_num_elem_az);
            double phase_arr = cos_theta_a * cos(alpha - phi_a);
            double phase_steer = cos_theta_s * cos(alpha - phi_s);
            double phase_diff = phase_arr - phase_steer;
            sum += exp(A * phase_diff);  // -j * phase
        }
        az_gain[f_idx] = abs(sum) / _num_elem_az;
    }

    return az_gain;
}

void bp_cylinder::beam_level(const bvector& arrival,
                                const seq_vector::csptr& frequencies,
                                vector<double>* level, const bvector& steering,
                                double sound_speed) const {
    // Apply back baffle if enabled
    if (_back_baffle && arrival.front() <= 0.0) {
        *level = vector<double>(frequencies->size(), 0.0);
        return;
    }
    
    // --- Azimuth factor ---
    auto az_factor = azimuth_factor(frequencies, arrival, steering, sound_speed);
    
    // --- Elevation factor (VLA style) ---
    const double sin_theta_diff = arrival.up() - steering.up(); // sin¦È_a - sin¦È_s
    vector<double> kd = frequencies->data();
    kd *= M_PI * _spacing_el / sound_speed * sin_theta_diff;  // kd = ¦Ð d / ¦Ë * (sin¦È_a - sin¦È_s) 

    vector<double> el_factor = element_div(sin(_num_elem_el * kd) + 1e-200,
                                           _num_elem_el * sin(kd) + 1e-200);

    // --- result ---
    *level = abs2(element_prod(az_factor , el_factor));
}

void bp_cylinder::directivity(const seq_vector::csptr& frequencies,
                                 vector<double>* level, const bvector& steering,
                                 double sound_speed) const {
    // Fallback to numerical integration (same as bp_model base class)
    bp_model::directivity(frequencies, level, steering, sound_speed);
}