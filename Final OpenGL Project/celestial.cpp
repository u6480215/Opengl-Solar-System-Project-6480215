#include "utils.h"
#include "celestial.h"
#include <random>
#include <ctime>

extern glm::vec3 cameraPos;
extern int sphereRes;
int ringRes = 6;
int ringSegments = 20;


void updateCelestialPosition(PlanetParams& satellite, float deltaTime, glm::vec3 mainPosition)
{
    satellite.orbitAngle -= deltaTime * satellite.orbitalSpeed;
    satellite.spinAngle += deltaTime * satellite.spinSpeed;

    glm::vec3 satelliteOffset = orbitMaker(satellite.semiMajorAxis, satellite.orbitAngle, satellite.eccentricity, satellite.inclination);
    satellite.position = mainPosition + satelliteOffset; // Position relative
}

glm::vec3 orbitMaker(float semiMajorAxis, float angle, float eccentricity, float inclination)
{
    float theta = glm::radians(angle);
    float x = semiMajorAxis * (cos(theta) - eccentricity);
    float z = semiMajorAxis * sqrt(1.0f - eccentricity * eccentricity) * sin(theta);

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(inclination), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec4 tiltedPosition = rotationMatrix * glm::vec4(x, 0.0f, z, 1.0f);

    return glm::vec3(tiltedPosition);
}

// Function to generate orbit path for the Moon relative to Earth
std::vector<glm::vec3> computeMoonOrbitPath(glm::vec3 center, float semiMajorAxis, float eccentricity, float inclination, int segments)
{
    std::vector<glm::vec3> orbitPoints;
    for (int i = 0; i < segments; ++i)
    {
        float angle = i * (360.0f / segments);
        glm::vec3 point = center + orbitMaker(semiMajorAxis, angle, eccentricity, inclination);
        orbitPoints.push_back(point);
    }
    return orbitPoints;
}

std::vector<glm::vec3> generateOrbitPath(float semiMajorAxis, float eccentricity, float inclination, int segments)
{
    std::vector<glm::vec3> orbitVertices;
    for (int i = 0; i < segments; ++i)
    {
        float angle = i * (360.0f / segments);
        orbitVertices.push_back(orbitMaker(semiMajorAxis, angle, eccentricity, inclination));
    }
    return orbitVertices;
}

void renderOrbitPath(GLuint shaderProgram, const std::vector<glm::vec3>& orbitPoints)
{
    GLuint orbitVAO, orbitVBO;

    // Create and bind the VAO and VBO for each orbit
    glGenVertexArrays(1, &orbitVAO);
    glGenBuffers(1, &orbitVBO);

    glBindVertexArray(orbitVAO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitVBO);
    glBufferData(GL_ARRAY_BUFFER, orbitPoints.size() * sizeof(glm::vec3), orbitPoints.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Use the shader program
    glUseProgram(shaderProgram);
    glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(cameraPos));
    // Render the orbit
    glBindVertexArray(orbitVAO);
    glDrawArrays(GL_LINE_LOOP, 0, orbitPoints.size());
    glBindVertexArray(0);

    // Cleanup VAO and VBO
    glDeleteBuffers(1, &orbitVBO);
    glDeleteVertexArrays(1, &orbitVAO);
}

void renderCloudLayer(GLuint shaderProgram, GLuint VAO, const PlanetParams& planet, GLuint cloudTexture,
    glm::vec3 cloudColor, float scale, float time, float alphaFactor,
    glm::vec3 rimColor, float rimIntensity, glm::vec3 terminatorColor, float terminatorBlendFactor) {
    // Model transformation matrix
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, planet.position);
    model = glm::rotate(model, glm::radians(planet.orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Orbit rotation
    model = glm::rotate(model, glm::radians(planet.tilt), glm::vec3(1.0f, 0.0f, 0.0f));       // Tilt rotation
    model = glm::rotate(model, glm::radians(planet.spinAngle - time), glm::vec3(0.0f, 1.0f, 0.0f));  // Add spinning animation
    model = glm::scale(model, glm::vec3(planet.scale * scale)); // Scale for clouds

    // Use the shader program
    glUseProgram(shaderProgram);

    // Pass uniform values to the shader

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(glGetUniformLocation(shaderProgram, "transparency"), alphaFactor);
    glUniform3f(glGetUniformLocation(shaderProgram, "cloudBaseColor"), cloudColor.r, cloudColor.g, cloudColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "rimIntensity"), rimIntensity);
    glUniform3f(glGetUniformLocation(shaderProgram, "rimColor"), rimColor.r, rimColor.g, rimColor.b);
    glUniform3f(glGetUniformLocation(shaderProgram, "terminatorColor"), terminatorColor.r, terminatorColor.g, terminatorColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "terminatorBlendFactor"), terminatorBlendFactor);
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time); // Pass time for dynamic effects

    // Bind the cloud texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cloudTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "cloudTexture"), 0);

    // Draw the cloud layer using the VAO
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36 * sphereRes * 18 * sphereRes * 6 * sphereRes, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void renderRing(GLuint shaderProgram, GLuint ringVAO, const PlanetParams& planet, GLuint  ringTexture, bool flipped) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, planet.position);
    model = glm::rotate(model, glm::radians(planet.orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Orbit rotation
    model = glm::rotate(model, glm::radians(planet.tilt), glm::vec3(1.0f, 0.0f, 0.0f));       // Tilt rotation
    model = glm::scale(model, glm::vec3(planet.scale)); // Adjust scale

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), planet.ambientStrength + 0.02);
    glUniform3f(glGetUniformLocation(shaderProgram, "rimColor"), planet.rimColor.r, planet.rimColor.g, planet.rimColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "rimIntensity"), planet.rimIntensity);
    glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), planet.specularStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), planet.shininess);
    glUniform1i(glGetUniformLocation(shaderProgram, "flipped"), flipped);
    // Bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ringTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    // Render ring
    glBindVertexArray(ringVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, (ringSegments + 1) * 2 * ringRes);
    glBindVertexArray(0);
}

void renderPlanet(GLuint shaderProgram, GLuint VAO, const PlanetParams& planet)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, planet.position);
    model = glm::rotate(model, glm::radians(planet.orbitAngle), glm::vec3(0.0f, 1.0f, 0.0f)); // Orbit rotation
    model = glm::rotate(model, glm::radians(planet.tilt), glm::vec3(1.0f, 0.0f, 0.0f));       // Tilt rotation
    model = glm::rotate(model, glm::radians(planet.spinAngle), glm::vec3(0.0f, 1.0f, 0.0f));  // Self rotation
    model = glm::scale(model, glm::vec3(planet.scale));

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), planet.ambientStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), planet.specularStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "shininess"), planet.shininess);
    glUniform3f(glGetUniformLocation(shaderProgram, "rimColor"), planet.rimColor.r, planet.rimColor.g, planet.rimColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "rimIntensity"), planet.rimIntensity);
    glUniform3f(glGetUniformLocation(shaderProgram, "edgeColor"), planet.edgeColor.r, planet.edgeColor.g, planet.edgeColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "edgeIntensity"), planet.edgeIntensity);
    glUniform3f(glGetUniformLocation(shaderProgram, "terminatorColor"), planet.terminatorColor.r, planet.terminatorColor.g, planet.terminatorColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "terminatorBlendFactor"), planet.terminatorBlendFactor);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, planet.texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    if (planet.useSpecularMap)
    {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, planet.specularMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "specularMap"), 1);
    }
    glUniform1i(glGetUniformLocation(shaderProgram, "useSpecularMap"), planet.useSpecularMap);

    if (planet.useNightMap)
    {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, planet.nightMap);
        glUniform1i(glGetUniformLocation(shaderProgram, "nightMap"), 2);
    }
    glUniform1i(glGetUniformLocation(shaderProgram, "useNightMap"), planet.useNightMap);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36 * sphereRes * 18 * sphereRes * 6 * sphereRes, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

std::vector<glm::mat4> asteroids(float beltRadius, float beltWidth, float outlierProbability,
    float closerMultiplier, float furtherMultiplier,
    float yOutlierMultiplier, float yInlierMultiplier) {
    std::vector<glm::mat4> asteroidTransforms(NUM_ASTEROIDS);

    for (unsigned int i = 0; i < NUM_ASTEROIDS; i++) {
        glm::mat4 transform = glm::mat4(1.0f);

        // Determine outlier type: closer, further, or normal in XZ plane
        float outlierChanceXZ = static_cast<float>(rand()) / RAND_MAX;
        float radius = beltRadius + static_cast<float>(rand()) / RAND_MAX * beltWidth; // Normal belt radius

        if (outlierChanceXZ < outlierProbability / 2.0f) {
            // Spawn closer in
            radius *= closerMultiplier;
            radius -= outlierChanceXZ; // Shrink radius
        }
        else if (outlierChanceXZ < outlierProbability) {
            // Spawn further out
            radius *= furtherMultiplier; // Increase radius
            radius += outlierChanceXZ;
        }

        // Random Y offset with outliers
        float outlierChanceY = static_cast<float>(rand()) / RAND_MAX;
        float yOffset = static_cast<float>(rand()) / RAND_MAX * 1.1f - 1.2f; // Normal thickness range

        if (outlierChanceY < outlierProbability) {
            // Y-axis outliers
            yOffset *= yOutlierMultiplier + outlierChanceY; // Amplify the Y-offset
        }
        else if (outlierChanceY > outlierProbability) {
            // Spawn closer in Y
            yOffset *= yInlierMultiplier - outlierChanceY; // Reduce Y offset
        }

        // Random position in a toroidal region
        float angle = static_cast<float>(rand()) / RAND_MAX * 360.0f; // Random angle in degrees

        transform = glm::translate(transform, glm::vec3(
            radius * sin(glm::radians(angle)), // X position
            yOffset,                           // Y position
            radius * cos(glm::radians(angle))  // Z position
        ));

        // Random rotation around all three axes
        float rotX = static_cast<float>(rand()) / RAND_MAX * 360.0f; // Random rotation around X-axis
        float rotY = static_cast<float>(rand()) / RAND_MAX * 360.0f; // Random rotation around Y-axis
        float rotZ = static_cast<float>(rand()) / RAND_MAX * 360.0f; // Random rotation around Z-axis

        transform = glm::rotate(transform, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotate around X-axis
        transform = glm::rotate(transform, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate around Y-axis
        transform = glm::rotate(transform, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f)); // Rotate around Z-axis

        // Random scale with a chance for a size outlier
        float scaleChance = static_cast<float>(rand()) / RAND_MAX;
        float scale;
        if (scaleChance < 0.01f) { // 1% chance for a larger asteroid
            scale = 0.3f + static_cast<float>(rand()) / RAND_MAX * 1.2f; // Large scale between 1.0 and 2.5
        }
        else if (scaleChance < 0.017f) { // 1.7% chance for a large asteroid
            scale = 0.2f + static_cast<float>(rand()) / RAND_MAX * 0.7f; // Large scale between 1.0 and 2.5
        }
        else {
            scale = 0.1f + static_cast<float>(rand()) / RAND_MAX * 0.3f; // Normal scale between 0.1 and 0.4
        }
        transform = glm::scale(transform, glm::vec3(scale));

        asteroidTransforms[i] = transform;
    }

    return asteroidTransforms;
}

void renderAsteroidBelt(GLuint shaderProgram, GLuint VAO, int vertexCount,
    float ambientStrength, float specularStrength,
    glm::vec3 rimColor, float rimIntensity,
    float asteroidRotationAngle)
{
    // Set the asteroid rotation angle
    glUniform1f(glGetUniformLocation(shaderProgram, "asteroidRotationAngle"), asteroidRotationAngle);

    // Set lighting and material properties
    glUniform1f(glGetUniformLocation(shaderProgram, "ambientStrength"), ambientStrength);
    glUniform1f(glGetUniformLocation(shaderProgram, "specularStrength"), specularStrength);
    glUniform3f(glGetUniformLocation(shaderProgram, "rimColor"), rimColor.r, rimColor.g, rimColor.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "rimIntensity"), rimIntensity);

    // Bind and render
    glBindVertexArray(VAO);
    glDrawElementsInstanced(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0, NUM_ASTEROIDS);
    glBindVertexArray(0);
}
