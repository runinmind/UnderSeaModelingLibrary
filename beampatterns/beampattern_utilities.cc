/**
 * @file beampattern_utilities.cc
 * A collection of utilities for helping to build and configure beampatterns.
 */
#include <usml/beampatterns/beampattern_utilities.h>

namespace usml::beampatterns {

using namespace usml::types;

/**
 * Provides the element locations of a circular planar array on the
 * front/right plane
 */
void bp_con_ring(vector<double> radii, vector<int> num_elements,
                 vector<double> offsets, matrix<double> *elem_locations) {
    (*elem_locations).resize(norm_1(num_elements), 3);

    int i = 0;
    for (int n = 0; n < radii.size(); n++) {
        seq_linear anglesv(0, 2 * M_PI / num_elements(n), num_elements(n));
        vector<double> angle = anglesv;
        angle = offsets(n) - angle;

        for (int j = 0; j < num_elements(n); j++) {
            (*elem_locations)(i, 0) = radii(n) * cos(angle(j));
            (*elem_locations)(i, 1) = radii(n) * sin(angle(j));
            (*elem_locations)(i, 2) = 0;
            i++;
        }
    }
}

/**
 * Provides the element locations of a uniformly spaced array in
 * 3 dimensions
 */
void bp_con_uniform(int num_e_front, double spacing_front, int num_e_right,
                    double spacing_right, int num_e_up, double spacing_up,
                    matrix<double> *elem_locations) {
    (*elem_locations).resize((size_t)num_e_front * num_e_right * num_e_up, 3);
    size_t i = 0;
    for (int u = 0; u < num_e_up; u++) {
        for (int e = 0; e < num_e_right; e++) {
            for (int n = 0; n < num_e_front; n++) {
                (*elem_locations)(i, 0) =
                    ((double)n - ((double)num_e_front - 1.0) / 2.0) *
                    spacing_front;
                (*elem_locations)(i, 1) =
                    ((double)e - ((double)num_e_right - 1.0) / 2.0) *
                    spacing_right;
                (*elem_locations)(i, 2) =
                    ((double)u - ((double)num_e_up - 1.0) / 2.0) * spacing_up;
                i++;
            }
        }
    }
}

void bp_con_cylinder(double radius, int num_elem_az, 
                     int num_e_up,
                     double spacing_up, 
                     matrix<double> *elem_locations,
                     double offset) {
    (*elem_locations).resize((size_t)num_elem_az * num_e_up, 3);

    seq_linear anglesv(0, 2 * M_PI / num_elem_az, num_elem_az);
    vector<double> angle = anglesv;
    angle = offset - angle;

    seq_linear upv(-(num_e_up - 1) * spacing_up / 2, spacing_up, num_e_up);
    vector<double> up = upv;

    size_t i = 0;
    for (int n = 0; n < num_elem_az; n++) {
         for (int u = 0; u < num_e_up; u++) {
            (*elem_locations)(i, 0) = radius * cos(angle(n));
             (*elem_locations)(i, 1) = radius * sin(angle(n));
            (*elem_locations)(i, 2) = up(u);
            i++;
         }
    }
}

void bp_con_sphere(double radius, int num_elem_az, int num_elem_el,
                   matrix<double> *elem_locations,
                   double min_el_angle, double max_el_angle) {
    (*elem_locations).resize((size_t)num_elem_az * num_elem_el, 3);

    size_t index = 0;
    double el_range = max_el_angle - min_el_angle;

    for (int el_idx = 0; el_idx < num_elem_el; ++el_idx) {
        double el = min_el_angle + (el_range * el_idx) / (num_elem_el - 1.0);
        double cos_el = cos(el);
        double sin_el = sin(el);

        for (int az_idx = 0; az_idx < num_elem_az; ++az_idx) {
            double az = (2.0 * M_PI * az_idx) / num_elem_az;

            (*elem_locations)(index, 0) = radius * cos_el * cos(az);  // f
            (*elem_locations)(index, 1) = radius * cos_el * sin(az);  // r
            (*elem_locations)(index, 2) = radius * sin_el;            // u
            index++;
        }
    }
}

}  // namespace usml::beampatterns
