#include "utils.h"
#include "geometry.h"
// Sphere vertices and rendering setup
GLuint createSphereVAO(float radius, int sectorCount, int stackCount) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> normals;
    std::vector<float> texCoords;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = M_PI / 2 - i * M_PI / stackCount;
        float xy = radius * cos(stackAngle);
        float z = radius * sin(stackAngle);

        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * 2 * M_PI / sectorCount;
            float x = xy * cos(sectorAngle);
            float y = xy * sin(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(z);
            vertices.push_back(-y);

            glm::vec3 normal = glm::normalize(glm::vec3(x, z, -y));
            normals.push_back(normal.x);
            normals.push_back(normal.y);
            normals.push_back(normal.z);

            float s = (float)j / sectorCount;
            float t = (float)i / stackCount;
            texCoords.push_back(s);
            texCoords.push_back(t);
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        for (int j = 0; j < sectorCount; ++j) {
            int first = i * (sectorCount + 1) + j;
            int second = first + sectorCount + 1;
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    GLuint VAO, VBO, EBO, normalVBO, texVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &texVBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, texVBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(float), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
    return VAO;
}

GLuint createRingVAO(float innerRadius, float outerRadius, int segments) {
    std::vector<float> ringVertices;

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = cos(angle);
        float z = sin(angle);

        // Outer ring vertex
        ringVertices.push_back(x * outerRadius); // x
        ringVertices.push_back(0.0f);           // y
        ringVertices.push_back(z * outerRadius); // z
        ringVertices.push_back(1.0f);           // Alpha interpolation coordinate (1.0)
        ringVertices.push_back(0.0f);           // Normal x
        ringVertices.push_back(1.0f);           // Normal y
        ringVertices.push_back(0.0f);           // Normal z

        // Inner ring vertex
        ringVertices.push_back(x * innerRadius); // x
        ringVertices.push_back(0.0f);           // y
        ringVertices.push_back(z * innerRadius); // z
        ringVertices.push_back(0.0f);           // Alpha interpolation coordinate (0.0)
        ringVertices.push_back(0.0f);           // Normal x
        ringVertices.push_back(1.0f);           // Normal y
        ringVertices.push_back(0.0f);           // Normal z
    }

    GLuint ringVAO, ringVBO;
    glGenVertexArrays(1, &ringVAO);
    glGenBuffers(1, &ringVBO);

    glBindVertexArray(ringVAO);
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, ringVertices.size() * sizeof(float), ringVertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Alpha interpolation attribute
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Normal attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    return ringVAO;
}
