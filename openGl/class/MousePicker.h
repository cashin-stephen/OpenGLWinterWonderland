#ifndef MOUSEPICKER_H
#define MOUSEPICKER_H

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include "Camera.h"


using namespace std;

class MousePicker {
public:
    glm::vec3 currentRay;
    glm::mat4 projectionMatrix;
    glm::mat4 viewMatrix;
    Camera camera;
    int height;
    int width;
    GLFWwindow* window;

    MousePicker(Camera camera, glm::mat4 projectionMatrix, GLFWwindow* window, int height, int width) {
        this->camera = camera;
        this->projectionMatrix = projectionMatrix;
        this->window = window;
        this->height = height;
        this->width = width;
    }

    void update(){
        viewMatrix = camera.GetViewMatrix();
        currentRay = calculateMouseRay();
    }

    glm::vec3 calculateMouseRay() {
        double mouse_x, mouse_y;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        float x = (2.0f * mouse_x) / width - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / height;
        float z = 1.0f;
        glm::vec3 coords = glm::vec3(x, y, z);
        glm::vec4 clipCoords = glm::vec4(coords.x, coords.y, -1.0f, 1.0f);
        glm::vec4 eyeCoords = toEyeCoords(clipCoords);
        glm::vec3 worldRay = toWorldCoords(eyeCoords);
        return worldRay;
    }

    glm::vec3 toWorldCoords(glm::vec4 eyeCoords) {
        glm::vec4 rayWorld = inverse(viewMatrix) * eyeCoords;
        glm::vec3 mouseRay = glm::vec3(rayWorld.x, rayWorld.y, rayWorld.z);
        mouseRay = glm::normalize(mouseRay);
        return mouseRay;
    }

    glm::vec4 toEyeCoords(glm::vec4 clipCoords) {
        glm::vec4 eyeCoords = inverse(projectionMatrix) * clipCoords;
        eyeCoords = glm::vec4(eyeCoords.x, eyeCoords.y, -1.0, 0.0);
        return eyeCoords;
    }

};
#endif