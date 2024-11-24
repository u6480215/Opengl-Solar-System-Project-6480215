#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"

// Defines possible options for camera movement
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 1.0f;
const float ZOOM = 45.0f;

class Camera
{
public:
    // Camera Attributes
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;

    // Euler Angles
    float Yaw;
    float Pitch;

    // Camera Options
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    // Orbit mode variables
    bool IsOrbiting = false;  // Indicates if the camera is in orbit mode
    float OrbitRadius;        // Distance from the origin

    // Constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = YAW,
        float pitch = PITCH)
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY),
        Zoom(ZOOM)
    {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        OrbitRadius = glm::length(Position);  // Initial orbit radius
        updateCameraVectors();
    }

    // Returns the view matrix calculated using LookAt matrix
    glm::mat4 GetViewMatrix()
    {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Process keyboard input
    void ProcessKeyboard(Camera_Movement direction, float deltaTime)
    {
        if (IsOrbiting) return;  // Disable keyboard movement in orbit mode
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD)
            Position += Front * velocity;
        if (direction == BACKWARD)
            Position -= Front * velocity;
        if (direction == LEFT)
            Position -= Right * velocity;
        if (direction == RIGHT)
            Position += Right * velocity;
    }

    // Process mouse movement
    void ProcessMouseMovement(float xoffset, float yoffset, bool leftMousePressed)
    {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        if (leftMousePressed)
        {
            if (!IsOrbiting)
            {
                IsOrbiting = true;
                OrbitRadius = glm::length(Position);  // Set orbit radius
            }

            // Update yaw and pitch for orbiting
            Yaw += xoffset;
            Pitch += yoffset;

            // Constrain pitch
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;

            // Update position based on spherical coordinates
            Position.x = OrbitRadius * cos(glm::radians(Pitch)) * cos(glm::radians(Yaw));
            Position.y = OrbitRadius * sin(glm::radians(Pitch));
            Position.z = OrbitRadius * cos(glm::radians(Pitch)) * sin(glm::radians(Yaw));

            // Always look at the origin
            Front = glm::normalize(-Position);
        }
        else
        {
            if (IsOrbiting)
            {
                // Transition back to free-camera mode
                IsOrbiting = false;

                // Align yaw and pitch with current direction
                Yaw = glm::degrees(atan2(Front.z, Front.x));
                Pitch = glm::degrees(asin(Front.y));
            }

            // Update yaw and pitch in free-camera mode
            Yaw += xoffset;
            Pitch += yoffset;

            // Constrain pitch
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;

            // Update camera vectors
            updateCameraVectors();
        }
    }

    // Process mouse scroll
    void ProcessMouseScroll(float yoffset)
    {
        Zoom -= (float)yoffset;
        if (Zoom < 1.0f)
            Zoom = 1.0f;
        if (Zoom > 45.0f)
            Zoom = 45.0f;
    }

private:
    // Update the camera vectors based on Euler angles
    void updateCameraVectors()
    {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);

        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

#endif
