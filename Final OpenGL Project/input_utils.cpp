#include "input_utils.h"
#include "utils.h"
#include "camera.h"

// External variables

extern Camera camera;
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern float lastX;
extern float lastY;
extern float deltaTime;
extern float speedFactor;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
float cameraSpeed = 0.1f* deltaTime;
float tempSpeedFactor = speedFactor;
extern bool flashlightOn;
extern bool OrbitOn;
extern bool bloom;
extern bool skyBoxOn;
extern float exposureVal;
// Utility function to compile shaders
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}
GLuint createProgram(const char* vertexShaderSource, const char* fragmentShaderSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void printControls() {
    cout << "[W][A][S][D] [SPACE] [SHIFT] for Movement.\n";
    cout << "[Ctrl] Sprint.\n";
    cout << "[Q][E] to Control Simulation Speed.\n";
    cout << "[P] Pause.\n";
    cout << "[F] Flashlight.\n";
    cout << "[Left Click] Orbital Movement.\n";
    cout << "[T] Toggle Path.\n";
    cout << "[B] Toggle Skybox.\n";
    cout << "[Scroll] Zoom.\n";
    cout << "[X] Bloom\n";
    cout << "[UP] [DOWN] Exposure.\n"; 
}

void processInput(GLFWwindow* window) {
    static bool pKeyPressed = false; // To prevent multiple triggers while key is held
    static bool paused = false;     // This will track whether the simulation is paused
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp; // Move up
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp; // Move down
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);}
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        speedFactor += 0.02f; // Increase speed
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        speedFactor = std::max(0.01f, speedFactor - 0.02f); // Decrease speed, ensuring it stays positive

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        exposureVal += 0.005f; // Increase speed
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        exposureVal = std::max(0.005f, exposureVal - 0.005f);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!pKeyPressed) {
            paused = !paused;  // Toggle the paused state
            if (paused) {
                tempSpeedFactor = speedFactor;
                speedFactor = 0.0f; // Freeze the speed when paused
            }
            else {
                speedFactor = tempSpeedFactor; // Restore the speed when unpaused
            }
            pKeyPressed = true;
        }
    }
    else {
        pKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        cameraSpeed = 0.5f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE)
    {
        cameraSpeed = 0.1f;
    }

    static bool fKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!fKeyPressed) {
            flashlightOn = !flashlightOn;
            fKeyPressed = true;
        }
    }
    else {
        fKeyPressed = false;
    }

    static bool bKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        if (!bKeyPressed) {
            skyBoxOn = !skyBoxOn;
            bKeyPressed = true;
        }
    }
    else {
        bKeyPressed = false;
    }

    static bool tKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!tKeyPressed) {
            OrbitOn = !OrbitOn;
            tKeyPressed = true;
        }
    }
    else {
        tKeyPressed = false;
    }

    static bool xKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (!xKeyPressed) {
            bloom = !bloom; 
            xKeyPressed = true;
        }
    }
    else {
        xKeyPressed = false;
    }
}

bool firstMouse = true;
float sensitivity = 0.2f;
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float distanceFromCenter;  // Distance from the origin
    static bool leftClickHeld = false; // Track whether the left mouse button is held

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate mouse offsets
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update yaw and pitch
    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to avoid flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Check if the left mouse button is pressed
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        leftClickHeld = true;

        // Calculate distance from the origin (for spherical movement)
        distanceFromCenter = glm::length(cameraPos);

        // Update camera position in spherical coordinates
        glm::vec3 position;
        position.x = distanceFromCenter * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        position.y = distanceFromCenter * sin(glm::radians(pitch));
        position.z = distanceFromCenter * cos(glm::radians(pitch)) * sin(glm::radians(yaw));

        cameraPos = position;

        // Set cameraFront to point toward the origin
        cameraFront = glm::normalize(-cameraPos); // Point toward the origin
        camera.ProcessMouseMovement(xoffset, yoffset, leftClickHeld);
    }
    else if (leftClickHeld && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        // Handle left mouse button release
        leftClickHeld = false;

        // Update `yaw` and `pitch` to align with the current `cameraFront`
        yaw = glm::degrees(atan2(cameraFront.z, cameraFront.x));
        pitch = glm::degrees(asin(cameraFront.y)); // Constrain pitch
    }

    // If not holding left mouse button, allow free-camera mode
    if (!leftClickHeld) {
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        cameraFront = glm::normalize(front);
        camera.ProcessMouseMovement(xoffset, yoffset, leftClickHeld);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
    camera.ProcessMouseScroll( yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);  // Set the viewport to match the new window dimensions
}