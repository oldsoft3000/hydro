#include "FluidParams.h"

#include <cmath>

const float KOEF_SPACING = 2.0;
const float KOEF_ORBIT = 2.0;
const int NUM_BOUNDARY_STEPS = 64;
const int NUM_ATOMS = 16;

const float FREQ_TANGENCIAL = 5;
const float FREQ_RADIAL = 5;

const float FREQ_RADIAL_0 = FREQ_RADIAL;
const float FREQ_RADIAL_1 = FREQ_RADIAL / 2;
const float FREQ_TANGENCIAL_0 = FREQ_TANGENCIAL;
const float FREQ_TANGENCIAL_1 = FREQ_TANGENCIAL / 2;

const float ANGLE_REVOLUTE = M_PI / 10;

const float DUMPING_TANGENCIAL = 0.5;
const float DUMPING_RADIAL = 0.5;

const float DUMPING_TANGENCIAL_0 = DUMPING_TANGENCIAL;
const float DUMPING_TANGENCIAL_1 = DUMPING_TANGENCIAL;
const float DUMPING_RADIAL_0 = DUMPING_RADIAL;
const float DUMPING_RADIAL_1 = DUMPING_RADIAL * 5;

const float DUMPING_LINEAR_KERNEL = 1.0;
const float DUMPING_LINEAR_ATOM_0 = 2;
const float DUMPING_LINEAR_ATOM_1 = 3000;

const float ATOM_RADIUS = 5.0 / 250.0;
const float KERNEL_RADIUS = 25.0 / 250.0;

const float KOEF_NUM_HEAD_ATOMS = 0.4;

const float PRISMATIC_UPPER_TRANSLATION = 45 * ATOM_RADIUS;
const float PRISMATIC_LOWER_TRANSLATION = 5 * ATOM_RADIUS;
