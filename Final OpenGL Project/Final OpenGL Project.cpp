#include "shader_m.h"
#include "camera.h"
#include "geometry.h"
#include "skybox.h"
#include "texture_utils.h"
#include "input_utils.h"
#include "celestial.h"
#include <cstdlib>

const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 900;

// Camera settings
Camera camera(glm::vec3(0.0f, 0.0f, 35.0f));
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 35.0f);
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
// Other utils
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
float speedFactor = 0.25f;
bool flashlightOn = false;
bool OrbitOn = true;
bool bloom = true;
bool skyBoxOn = true;
int sphereRes = 2;
float gamma = 2.2f;
float exposureVal = 1.5f;
int NUM_ASTEROIDS = 1050;

int main()
{
    // ------------- INITIALIZE DISPLAYS ------------
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
  
    // Set callbacks for mouse and scroll
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor
    printControls();
    // ------------------------- Bloom effect ----------------------------
    Shader framebufferProgram("framebuffer.vert", "framebuffer.frag");
    Shader blurProgram("framebuffer.vert", "blur.frag");

    framebufferProgram.use();
    glUniform1i(glGetUniformLocation(framebufferProgram.ID, "screenTexture"), 0);	
    glUniform1i(glGetUniformLocation(framebufferProgram.ID, "bloomTexture"), 1);
    blurProgram.use();
    glUniform1i(glGetUniformLocation(blurProgram.ID, "screenTexture"), 0);

	// Prepare framebuffer rectangle VBO and VAO
	unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    // Create Frame Buffer Object
    unsigned int postProcessingFBO;
    glGenFramebuffers(1, &postProcessingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);

    // Create Framebuffer Texture
    unsigned int postProcessingTexture;
    glGenTextures(1, &postProcessingTexture);
    glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTexture, 0);

    // Create Second Framebuffer Texture
    unsigned int bloomTexture;
    glGenTextures(1, &bloomTexture);
    glBindTexture(GL_TEXTURE_2D, bloomTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, bloomTexture, 0);

    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);

	// Create Render Buffer Object
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

	// Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << std::endl;

    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffer[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);

        fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Ping-Pong Framebuffer error: " << fboStatus << std::endl;
    }
    // ----------------------- Celestial bodies --------------------------
    Shader skyboxShader("skyBox.vs", "skyBox.fs");    
    Shader celestialShader("celestial.vs", "celestial.fs");
    Shader orbitShader("orbit_vertex_shader.vs", "orbit_fragment_shader.fs");
    Shader cloudShader("cloud.vs", "cloud.fs");
    Shader ringShader("ring.vs", "ring.fs");

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
 
    GLuint cubemapTexture = loadCubemap(skyboxFaces);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    GLuint sphereVAO = createSphereVAO();
    
   glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
   glm::vec4 lightColor = glm::vec4(1.0, 1.0, 1.0, 1.0);

   GLuint sunTexture = loadTexture("../textures/planets/sun.jpg");
   GLuint mercuryTexture = loadTexture("../textures/planets/mercury.jpg");
   GLuint venusTexture = loadTexture("../textures/planets/venus.jpg");
   // EARTH
   GLuint earthTexture = loadTexture("../textures/planets/earth_daymap.jpg");
   GLuint earthSpecularMap = specularTextureLoad("../textures/planets/earth_specular_map.jpg");
   GLuint cloudTexture = loadTexture("../textures/planets/earth_clouds.jpg");
   GLuint earthNightMap = loadTexture("../textures/planets/earth_nightmap.jpg");

   GLuint moonTexture = loadTexture("../textures/planets/moon.jpg");
   GLuint marsTexture = loadTexture("../textures/planets/mars.jpg");
   GLuint jupiterTexture = loadTexture("../textures/planets/jupiter.jpg");
   // SATURN
   GLuint saturnTexture = loadTexture("../textures/planets/saturn.jpg");
   GLuint saturnRingTexture = loadRingTexture("../textures/planets/saturn_ring_alpha.png");

   GLuint uranusTexture = loadTexture("../textures/planets/uranus.jpg");
   GLuint uranusRingTexture = loadRingTexture("../textures/planets/uranus_ring_alpha.png");


   GLuint neptuneTexture = loadTexture("../textures/planets/neptune.jpg");
   GLuint plutoTexture = loadTexture("../textures/planets/pluto.jpg");
   // planets
   PlanetParams sun(0.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0, 8.0f, sunTexture, 1.2f, 0.0f, 0.0f, glm::vec3(9.0,2.0,0.0), 20.0f, glm::vec3(0.0), 0.0f, glm::vec3(50.5), -50.0f);               //rim                        //term                                          // edge
   PlanetParams mercury(14.0f, 47.87f, 360.0f / 1406.4f, 0.034f, 0.206f, 7.0f, 0.5f, mercuryTexture, 0.0002f, 0.03f, 10.0f, glm::vec3(0.5f, 0.4f, 0.2f), 3.5f, glm::vec3(0.025f, 0.0015f, 0.0f) + 0.02f, 2.9f);
   PlanetParams venus(19.0f, 35.02f, 360.0f / (-243.0f * 24.0f), 177.4f, 0.007f, 3.4f, 0.8f, venusTexture, 0.001f, 0.04f, 10.0f, glm::vec3(0.8f, 0.4f, 0.3f), 4.45f, glm::vec3(0.02f, 0.0095f, 0.0f) + 0.02f, 2.9f);
   PlanetParams earth(24.0f, 29.78f, 360.0f, 23.5f, 0.017f, 0.0f, 1.0f, earthTexture, 0.00088f, 0.74f, 9.5f, glm::vec3(0.10, 0.26, 0.84), 10.32f, glm::vec3(0.0003, 0.005, 0.016), 2.6f, glm::vec3(-0.02f, -0.05f, 0.0f), 5.0f, earthSpecularMap, true, earthNightMap, true);
   PlanetParams mars(30.0f, 24.13f, 360.0f / 24.6f, 25.2f, 0.093f, 1.9f, 0.7f, marsTexture, 0.0008f, 0.1f, 8.0f, glm::vec3(0.5f, 0.45f, 0.45f), 2.02f, glm::vec3(0.075f, 0.035f, 0.02f), 5.9f);
   PlanetParams jupiter(55.0f, 13.07, 360.0f / 9.9f, 3.1f, 0.049f, 1.3f, 3.5f, jupiterTexture, 0.0007f, 0.06f, 5.0f, glm::vec3(0.55f, 0.54f, 0.5f), 1.03f, glm::vec3(0.04f, 0.0276f, 0.024f), 5.9f, glm::vec3(-0.07f, -0.01f, -0.00f), 4.6f);
   PlanetParams saturn(65.0f, 9.69f, 360.0f / 10.7f, 26.7f, 0.056f, 2.5f, 2.0f, saturnTexture, 0.0006f, 0.05f, 5.0f, glm::vec3(0.54f, 0.57f, 0.5f), 1.03f, glm::vec3(0.035f, 0.0276f, 0.026f), 5.9f, glm::vec3(-0.12f, -0.057f, -0.002f), 3.5f);
   PlanetParams uranus(75.0f, 6.81f, 360.0f / 17.2f, 97.8f, 0.046f, 0.8f, 1.5f, uranusTexture, 0.0005f, 0.04f, 5.0f, glm::vec3(0.7f, 0.7f, 0.8f), 0.73f, glm::vec3(0.041f, 0.045f, 0.046f), 5.5f, glm::vec3(-0.008f, -0.04f, -0.0f), 3.5f);
   PlanetParams neptune(85.0f, 5.43f, 360.0f / 16.1f, 28.3f, 0.046f, 0.8f, 1.5f, neptuneTexture, 0.0004f, 0.04f, 8.0f, glm::vec3(0.5f, 0.5f, 0.9f), 0.22f, glm::vec3(0.021f, 0.021f, 0.037f), 5.9f, glm::vec3(-0.0f, -0.027f, -0.0f), 3.5f);
   PlanetParams pluto(110.0f, 4.67f, 360.0f / 153.3f, 122.5f, 0.248f, 17.2f, 0.4f, plutoTexture, 0.0003f, 0.02f, 5.0f, glm::vec3(0.5f, 0.5f, 0.5f), 0.92f, glm::vec3(0.03f, 0.03f, 0.03f), 5.9f);
   PlanetParams moon(2.5f, 1.022f * 50, 360.0f / 27.3f, 6.68f, 0.0549f, 5.145f, 0.27f, moonTexture, 0.00087f, 0.1f, 32.0f, glm::vec3(0.4f, 0.4f, 0.4f), 2.95f, glm::vec3(0.04f, 0.0215f, 0.025f), 2.9f);
   std::vector<std::reference_wrapper<PlanetParams>> planets = { sun, mercury, venus, earth, moon, mars, jupiter, saturn, uranus, neptune, pluto };

   Orbit mercuryOrbit = generateOrbitPath(mercury.semiMajorAxis, mercury.eccentricity, mercury.inclination);
   Orbit venusOrbit = generateOrbitPath(venus.semiMajorAxis, venus.eccentricity, venus.inclination);
   Orbit earthOrbit = generateOrbitPath(earth.semiMajorAxis, earth.eccentricity, earth.inclination);
   Orbit marsOrbit = generateOrbitPath(mars.semiMajorAxis, mars.eccentricity, mars.inclination);
   Orbit jupiterOrbit = generateOrbitPath(jupiter.semiMajorAxis, jupiter.eccentricity, jupiter.inclination);
   Orbit saturnOrbit = generateOrbitPath(saturn.semiMajorAxis, saturn.eccentricity, saturn.inclination);
   Orbit uranusOrbit = generateOrbitPath(uranus.semiMajorAxis, uranus.eccentricity, uranus.inclination);
   Orbit neptuneOrbit = generateOrbitPath(neptune.semiMajorAxis, neptune.eccentricity, neptune.inclination);
   Orbit plutoOrbit = generateOrbitPath(pluto.semiMajorAxis, pluto.eccentricity, pluto.inclination);
   Orbit moonOrbit = generateOrbitPath(moon.semiMajorAxis, moon.eccentricity, moon.inclination);
   std::vector<std::reference_wrapper<Orbit>> orbits = { mercuryOrbit, venusOrbit, earthOrbit, moonOrbit, marsOrbit, jupiterOrbit, saturnOrbit, uranusOrbit, neptuneOrbit, plutoOrbit };
    GLuint saturnsRing = createRingVAO(saturn.scale + 0.5f, 1.4f);
    GLuint uranusRing = createRingVAO(uranus.scale, uranus.scale + 0.57f);
    std::vector<glm::vec3> moonOrbitPath;
//------------------------------------------ ASTEROIDS ----------------------------------------------
    Shader asteroidShader("asteroid.vs", "asteroid.fs");
    GLuint asteroidTexture = loadTexture("../textures/planets/asteroid.jpg");
    int asteroidHeight = 5; int asteroidWidth = 4;
    GLuint asteroid = createSphereVAO(0.7, asteroidHeight, asteroidWidth);
    std::vector<glm::mat4> asteroidTransforms = asteroids(38.5,6.5,0.2f, 0.9f,1.08f,1.1,-1.1);
    
    // Create a buffer for instance matrices
    unsigned int instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, NUM_ASTEROIDS * sizeof(glm::mat4), &asteroidTransforms[0], GL_STATIC_DRAW);

    // Link the buffer to a vertex array object (VAO)
    glBindVertexArray(asteroid); 

    // Set instance attributes
    std::size_t vec4Size = sizeof(glm::vec4);
    for (unsigned int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(3 + i); // Use attribute locations 3, 4, 5, 6
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * vec4Size));
        glVertexAttribDivisor(3 + i, 1); // One per instance
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //--------------------------------------------------------------------------------------------------------
    float time = 0.0f;
    float asteroidRotationAngle = 0.0f;
    glCullFace(GL_FRONT);

    while (!glfwWindowShouldClose(window))
    {   
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CW);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        time += deltaTime; // Increment time
        
        asteroidRotationAngle -= deltaTime * 0.1f * speedFactor; // Adjust speed of rotation as needed
        if (asteroidRotationAngle <= 360.0f)
            asteroidRotationAngle += 360.0f; // Keep angle within 0-360 degrees
        processInput(window);
        // Bind the custom framebuffer
        if (bloom) {
            glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);
        }
        else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0); // Default framebuffer
        }
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //----------------------------------- MAIN DRAWINGS ----------------------------------------
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        // SET UP SHADERS
            cloudShader.use();
            cloudShader.setMat4("view", view);
            cloudShader.setMat4("projection", projection);
            cloudShader.setVec3("lightPos", lightPos);
            cloudShader.setVec4("lightColor", lightColor);
            cloudShader.setVec3("viewPos", cameraPos);
            cloudShader.setBool("flashlightOn", flashlightOn);
            cloudShader.setVec3("flashlightDir", cameraFront);
            cloudShader.setFloat("ambientStrength", 0.001f);
            cloudShader.setBool("haveBloom", bloom);
            cloudShader.setFloat("exposure",exposureVal);
            celestialShader.use();
            celestialShader.setMat4("view", view);
            celestialShader.setMat4("projection", projection);
            celestialShader.setVec3("lightPos", lightPos);
            celestialShader.setVec4("lightColor", lightColor);
            celestialShader.setVec3("viewPos", cameraPos);
            celestialShader.setVec3("flashlightDir", cameraFront);
            celestialShader.setInt("flashlightOn", flashlightOn);
            celestialShader.setFloat("gamma", gamma);
            celestialShader.setBool("haveBloom", bloom);
            celestialShader.setFloat("exposure", exposureVal);
            orbitShader.use();
            orbitShader.setMat4("view", view);
            orbitShader.setMat4("projection", projection);
            orbitShader.setVec3("cameraPos", cameraPos);
            orbitShader.setBool("haveBloom", bloom);
            orbitShader.setFloat("gamma", gamma);    
            ringShader.use();
            ringShader.setMat4("view", view);
            ringShader.setMat4("projection", projection);
            ringShader.setVec3("lightPos", lightPos);
            ringShader.setVec4("lightColor", lightColor);
            ringShader.setVec3("viewPos", cameraPos);
            ringShader.setBool("flashlightOn", flashlightOn);
            ringShader.setVec3("flashlightDir", cameraFront);
            ringShader.setBool("haveBloom", bloom);
            ringShader.setFloat("exposure", exposureVal);
            glm::mat4 skyView = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation
            glm::mat4 skyProjection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
            skyboxShader.use();
            skyboxShader.setMat4("view", skyView);
            skyboxShader.setMat4("projection", skyProjection);
            skyboxShader.setBool("haveBloom", bloom);
            skyboxShader.setFloat("exposure", exposureVal);
            asteroidShader.use();
            asteroidShader.setMat4("view", view);
            asteroidShader.setMat4("projection", projection);
            asteroidShader.setVec3("lightPos", lightPos);
            asteroidShader.setVec4("lightColor", lightColor);
            asteroidShader.setVec3("viewPos", cameraPos);
            asteroidShader.setVec3("flashlightDir", cameraFront);
            asteroidShader.setInt("flashlightOn", flashlightOn);
            asteroidShader.setBool("haveBloom", bloom);
            asteroidShader.setFloat("exposure", exposureVal);

        celestialShader.use();
        for (auto& planet_wrapper : planets)
        {
            auto& planet = planet_wrapper.get();
            if (&planet == &moon)
            {
                continue;
            }
            updateCelestialPosition(planet, deltaTime * speedFactor); // Update only planets
            renderPlanet(celestialShader.ID, sphereVAO, planet);
            if (&planet == &earth)
            {
                updateCelestialPosition(moon, deltaTime * speedFactor, planet.position);
                renderPlanet(celestialShader.ID, sphereVAO, moon);
            }
        }
        asteroidShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, asteroidTexture);
        glUniform1i(glGetUniformLocation(asteroidShader.ID, "texture1"), 0);
        renderAsteroidBelt(asteroidShader.ID, asteroid, asteroidHeight * asteroidWidth * 6,
            0.00,0.01f, glm::vec3(0.001),0.02,asteroidRotationAngle);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        orbitShader.use();
        if(OrbitOn){
            for (auto& orbits_wrapper : orbits)
            {
                auto& orbit = orbits_wrapper.get();
                if (&orbit != &moonOrbit)
                    renderOrbitPath(orbitShader.ID, orbit.points);
            }
            moonOrbitPath = computeMoonOrbitPath(earth.position, moon.semiMajorAxis, moon.eccentricity, moon.inclination);
            renderOrbitPath(orbitShader.ID, moonOrbitPath);}
        if (skyBoxOn) {
            // skybox cube
            skyboxShader.use();
            glDepthFunc(GL_LEQUAL); // Change depth function so skybox is drawn behind everything
            glBindVertexArray(skyboxVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
            glDrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
            glDepthFunc(GL_LESS); // Reset to default depth function// set depth function back to default   
        }
        // Activate cloud shader
        cloudShader.use();
        renderCloudLayer(
            cloudShader.ID, sphereVAO, earth, cloudTexture, glm::vec3(0.02, 0.04, 0.12), 1.004f, time, 0.91f, glm::vec3(0.02, 0.11, 0.85), 4.5);
        renderCloudLayer(
            cloudShader.ID, sphereVAO, earth, cloudTexture, glm::vec3(0.92, 0.92, 0.96), 1.01f, time+0.1, 1.0f, glm::vec3(0.37, 0.48, 0.87),2.5);
        glDisable(GL_CULL_FACE);
        ringShader.use();
        renderRing(ringShader.ID, saturnsRing, saturn, saturnRingTexture);
        renderRing(ringShader.ID, uranusRing, uranus, uranusRingTexture, true);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        //-------------------------------------------------------------------------------------
        if (bloom) {
            // Bounce the image data around to blur multiple times
		    bool horizontal = true, first_iteration = true;
		    // Amount of time to bounce the blur
		    int amount = 4;
		    blurProgram.use();
		    for (unsigned int i = 0; i < amount; i++)
		    {
			    glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			    glUniform1i(glGetUniformLocation(blurProgram.ID, "horizontal"), horizontal);

			    // In the first bounc we want to get the data from the bloomTexture
			    if (first_iteration)
			    {
				    glBindTexture(GL_TEXTURE_2D, bloomTexture);
				    first_iteration = false;
			    }
			    // Move the data between the pingPong textures
			    else
			    {
				    glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);
			    }

			    // Render the image
                glFrontFace(GL_CCW);
			    glBindVertexArray(rectVAO);
			    glDisable(GL_DEPTH_TEST);
			    glDrawArrays(GL_TRIANGLES, 0, 6);

			    // Switch between vertical and horizontal blurring
			    horizontal = !horizontal;
		}

            // Uses counter clock-wise standard
            glFrontFace(GL_CCW);
            // Bind the default framebuffer
            // Combine bloom with the original scene
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            framebufferProgram.use();
            framebufferProgram.setFloat("exposure", exposureVal);
            framebufferProgram.setBool("haveBloom", bloom);

            glBindVertexArray(rectVAO);
            glDisable(GL_DEPTH_TEST);

            // Bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, postProcessingTexture);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongBuffer[!horizontal]);

            // Draw the fullscreen quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteVertexArrays(1, &saturnsRing);
    glDeleteVertexArrays(1, &asteroid);
    glDeleteVertexArrays(1, &uranusRing);
    glDeleteProgram(celestialShader.ID);
    glDeleteProgram(orbitShader.ID);
    glDeleteProgram(cloudShader.ID);
    glDeleteProgram(asteroidShader.ID);
    glDeleteProgram(ringShader.ID);
    glDeleteBuffers(1, &skyboxVBO);

    glDeleteFramebuffers(1, &postProcessingFBO);
    glfwDestroyWindow(window);

    glfwTerminate();
    return 0;
}
