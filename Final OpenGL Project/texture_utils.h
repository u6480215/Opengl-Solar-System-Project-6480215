#pragma once
#ifndef TEXTURE_UTILS_H
#define TEXTURE_UTILS_H

#include "utils.h"

// Function to load a texture from a file path
GLuint loadTexture(const char* path);

// Function to load a specular map texture
GLuint specularTextureLoad(const char* filePath);

// Function to load a cubemap from a list of faces
GLuint loadCubemap(std::vector<std::string> faces);

unsigned int loadRingTexture(const char* path);

#endif // TEXTURE_UTILS_H
