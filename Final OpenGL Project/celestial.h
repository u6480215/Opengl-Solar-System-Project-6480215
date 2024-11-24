#pragma once
#ifndef CELESTIAL_H
#define CELESTIAL_H
#include "texture_utils.h"
#include "utils.h"

extern int NUM_ASTEROIDS;

struct PlanetParams
{
    float semiMajorAxis;
    float orbitalSpeed;
    float spinSpeed;
    float tilt;
    float eccentricity;
    float inclination;
    float scale;
    GLuint texture;
    float ambientStrength;
    float specularStrength;
    float shininess;
    GLuint specularMap;
    bool useSpecularMap;
    glm::vec3 position;
    float orbitAngle;
    float spinAngle;
    glm::vec3 rimColor;
    float rimIntensity;
    glm::vec3 terminatorColor;
    float terminatorBlendFactor;
    glm::vec3 edgeColor;
    float edgeIntensity;
    GLuint nightMap;
    bool useNightMap;

    PlanetParams(float semiMajorAxis, float orbitalSpeed, float spinSpeed, float tilt, float eccentricity, float inclination, float scale,
        GLuint texture, float ambientStrength, float specularStrength, float shininess = false
        , glm::vec3 rimColor = glm::vec3(0.2f), float rimIntensity = 0.3f, glm::vec3 terminatorColor = glm::vec3(0.005f), 
        float terminatorBlendFactor = 0.7f, glm::vec3 edgeColor = glm::vec3(0.0f),
        float edgeIntensity = 0.0f, GLuint specularMap = 0, bool useSpecularMap = false, GLuint nightMap = 0, bool useNightMap = false)
        : semiMajorAxis(semiMajorAxis), orbitalSpeed(orbitalSpeed), spinSpeed(spinSpeed), tilt(tilt), eccentricity(eccentricity),
        inclination(inclination), scale(scale), texture(texture), ambientStrength(ambientStrength),
        specularStrength(specularStrength), shininess(shininess), rimColor(rimColor), rimIntensity(rimIntensity),
        terminatorColor(terminatorColor), terminatorBlendFactor(terminatorBlendFactor), 
        edgeColor(edgeColor), edgeIntensity(edgeIntensity),
        specularMap(specularMap),useSpecularMap(useSpecularMap), nightMap(nightMap), useNightMap(useNightMap),
        position(glm::vec3(0.0f)), orbitAngle(0.0f), spinAngle(0.0f) {}
};
struct Orbit
{
    std::vector<glm::vec3> points;
    Orbit(std::vector<glm::vec3> points)
        : points(points) {}
};

glm::vec3 orbitMaker(float semiMajorAxis, float angle, float eccentricity = 0.0f, float inclination = 0.0f);

std::vector<glm::vec3> computeMoonOrbitPath(glm::vec3 center, float semiMajorAxis, float eccentricity, float inclination, int segments = 360);

std::vector<glm::vec3> generateOrbitPath(float semiMajorAxis, float eccentricity, float inclination, int segments = 360);

void updateCelestialPosition(PlanetParams& satellite, float deltaTime, glm::vec3 mainPosition = glm::vec3(0.0f));

void renderOrbitPath(GLuint shaderProgram, const std::vector<glm::vec3>& orbitPoints);

void renderCloudLayer(GLuint shaderProgram, GLuint VAO, const PlanetParams& planet, GLuint cloudTexture,
    glm::vec3 cloudColor, float scale, float time = 0.0f, float alphaFactor = 1.0f,
    glm::vec3 rimColor = glm::vec3(0.85, 0.86, 0.99), float rimIntensity = 1.2f,
    glm::vec3 terminatorColor = glm::vec3(0.0010, 0.0072, 0.016), float terminatorBlendFactor = 5.0f);
void renderRing(GLuint shaderProgram, GLuint ringVAO, const PlanetParams& planet, GLuint  ringTexture, bool flipped = false);

void renderPlanet(GLuint shaderProgram, GLuint VAO, const PlanetParams& planet);
std::vector<glm::mat4> asteroids(float beltRadius, float beltWidth, float outlierProbability = 0.1f, float closerMultiplier = 1.5f , float furtherMultiplier = 1.5f, float yOutlierMultiplier=1.5f, float yInlierMultiplier = 1.5f);


void renderAsteroidBelt(GLuint shaderProgram, GLuint VAO, int vertexCount,
    float ambientStrength, float specularStrength,
    glm::vec3 rimColor, float rimIntensity,
    float asteroidRotationAngle);
#endif 