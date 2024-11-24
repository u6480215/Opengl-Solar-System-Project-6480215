#pragma once
#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "utils.h"
extern int sphereRes;
extern int ringRes;
extern int ringSegments;
// Declare the function
GLuint createSphereVAO(float radius = 1.0f, int sectorCount = 36 * sphereRes, int stackCount = 18 * sphereRes);

GLuint createRingVAO(float innerRadius = 5.0, float outerRadius = 6.0f, int segments = ringSegments * ringRes);
#endif // SPHERE_H