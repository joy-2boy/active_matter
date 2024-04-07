#ifndef MATHE_H
#define MATHE_H

#include <math.h> //注意 mathe.h 包含了math.h
#define PI 3.14159265358979323846


void rng_init_mt19937(unsigned long seed);
double uniform_random();
double standard_gaussian_random();

/*
void rng_init(unsigned int seed);
double randnum_Lehmer(void);

double gauss_normal_rand(void);
*/

#endif