#include <GL2/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assimp/aabb.h> 
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>
#include "class/Shader.h"
#include "class/Camera.h"
#include "class/Model.h"
#include "class/ParticleGroup.h"

#include <iostream>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
bool intersection(glm::vec3 center, float radius, glm::vec3 ray);
int objectIntersection(glm::vec3 ray);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void highlightObject(glm::mat4 model, Model object, Shader shader, float scale, Shader borderShader);

const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;
const unsigned int MOON = 5;
const unsigned int CAMPFIRE = 6;
const unsigned int CABIN = 8;

bool cameraEnabled = true;
bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float scale = 1.1f;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
float timeBeforePause;
bool moonIntersection;
int interSectedObject =0;
vector<glm::vec3> reindeerPos;
glm::vec3 lightPos(12.0f, 20.0f, 0.0f);
glm::vec3 cottagePos(0.0f,-1.0f, 11.5f);
glm::vec3 moonlight(0.5f, 0.5f, 0.6f);
glm::vec3 campfirePos(0.0f, 1.0f, -4.5f);
glm::vec3 fireColor(1.0f, 1.0f, 0.0f);
glm::vec3 particlePos(-0.3f, -0.8f, -4.85f);

unsigned int getTexture(const char* path){
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    return texture;
}

int main() {
    if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
    } 

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Hello Triangle", NULL, NULL);
    if (!window) {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                                    
    glewExperimental = GL_TRUE;
    glewInit();

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    float groundVertices[] {
        0.5f,  0.5f, 0.0f, 1.0f, 1.0f,  // top right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f   // top left 
    };

    unsigned int groundIndices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    float pauseBGVertices[] = {
        1.0f,  1.0f, 0.0f,  // top right
        1.0f, -1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f,  // bottom left
        -1.0f,  1.0f, 0.0f   // top left 
    };

    GLuint PauseVBO, PauseVAO, PauseEBO;
    glGenVertexArrays(1, &PauseVAO);
    glGenBuffers(1, &PauseVBO);
    glGenBuffers(1, &PauseEBO);
    glBindVertexArray(PauseVAO);

    glBindBuffer(GL_ARRAY_BUFFER, PauseVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pauseBGVertices), pauseBGVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, PauseEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 


    GLuint GroundVBO, GroundVAO, GroundEBO;
    glGenVertexArrays(1, &GroundVAO);
    glGenBuffers(1, &GroundVBO);
    glGenBuffers(1, &GroundEBO);

    glBindVertexArray(GroundVAO);

    glBindBuffer(GL_ARRAY_BUFFER, GroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GroundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    
    Shader pauseShader("pausevs.txt", "pausefs.txt");
    Shader groundShader("groundvs.txt", "groundfs.txt");
    Shader modelShader("modelvs.txt", "modelfs.txt");
    Shader moonShader("moonvs.txt", "moonfs.txt");
    Shader particleShader("particlevs.txt", "particlefs.txt");
    Shader borderShader("modelvs.txt", "borderfs.txt");
    
    Model cottageModel(filesystem::path("assets/cottage/cottage.obj"));
    Model treeModel(filesystem::path("assets/tree/Tree.obj"));
    Model mountainModel(filesystem::path("assets/mountain/mountainpeak.obj"));
    Model moonModel(filesystem::path("assets/moon/moon.obj"));
    Model campfireModel(filesystem::path("assets/campfire/campfire.obj"));
    Model reindeerModel(filesystem::path("assets/reindeer/hierarchyReindeer.obj"));

    unsigned int snowTexture = getTexture("textures/snow2.jpeg");
    unsigned int moonTexture = getTexture("textures/moon.jpg");
    unsigned int snowyMountain = getTexture("textures/snowyMountain.jpeg");

    ParticleGroup particleGroup(fireColor, particlePos, 50);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    glm::vec3 identity = glm::vec3(1.0f);
    for(int i=0; i<4; i++) {
        reindeerPos.push_back(identity);
    }


    while(!glfwWindowShouldClose(window)) {

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glStencilMask(~0);
        glDisable(GL_SCISSOR_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glStencilMask(0x00);
        //camera Movement
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);
        glm::mat4 view = camera.GetViewMatrix();

        // 1. cursor position in Screen Space
        double mouse_x = SCR_WIDTH/2;
        double mouse_y = SCR_HEIGHT/2;

        // 2. 3D Normalised Device Coordinates
        float x = (2.0f * mouse_x) / SCR_WIDTH - 1.0f;
        float y = 1.0f - (2.0f * mouse_y) / SCR_HEIGHT;
        float z = 1.0f;
        glm::vec3 ray_nds = glm::vec3(x, y, z);

        // 3. 4D Homogenouse Clip Coordinates
        glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

        // 4. 4D Eye(Camera) Coordinates
        glm::vec4 ray_eye = inverse(projection) * ray_clip;
        ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

        // 5. 4D World Coordinates
        glm::vec4 ray_wor4 = (inverse(view) * ray_eye);
        glm::vec3 ray_wor = glm::vec3(ray_wor4.x, ray_wor4.y, ray_wor4.z);
        ray_wor = glm::normalize(ray_wor);

        interSectedObject = objectIntersection(ray_wor);

        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 diffuseColor = moonlight * glm::vec3(1.0f);
        glm::vec3 ambientColor = diffuseColor * glm::vec3(0.9f);
        modelShader.use();
        modelShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        modelShader.setVec3("dirLight.ambient", ambientColor);
        modelShader.setVec3("dirLight.diffuse", diffuseColor);
        modelShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
        modelShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);

        modelShader.setVec3("pointLight.position", campfirePos);
        modelShader.setVec3("pointLight.ambient", 0.05f, 0.027f, 0.027f);
        modelShader.setVec3("pointLight.diffuse", 1.0f, 0.54f, 0.0f);
        modelShader.setVec3("pointLight.specular", 0.5f, 0.27f, 0.27f);
        modelShader.setFloat("pointLight.constant", 1.0f);
        modelShader.setFloat("pointLight.linear", 0.09f);
        modelShader.setFloat("pointLight.quadratic", 0.032f);

        modelShader.setVec3("viewPos", camera.Position);
        modelShader.setMat4("projection", projection);    
        modelShader.setMat4("view", view);
        
        modelShader.setVec3("light.ambient", ambientColor);
        modelShader.setVec3("light.diffuse", diffuseColor);
        modelShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);

        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.1f, 0.1f, 0.1f);
        modelShader.setFloat("material.shininess", 4.0f);

        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.2f, 0.2f, 0.2f);
        modelShader.setFloat("material.shininess", 16.0f);

        for(float i=-10; i<=11; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-12.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=-8; i<=11; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-9.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=3; i<=12; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-6.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=-10; i<=11; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(10.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=-12; i<=15; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(7.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=3; i<=15; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(5.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=-11; i<=4; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i, -1.0f, -12.0f));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        for(float i=9; i<=12; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(13.0f, -1.0f, i));
            modelShader.setMat4("model", model);
            treeModel.Draw(modelShader);
        }

        glBindTexture(GL_TEXTURE_2D, snowyMountain);

        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f);
        modelShader.setFloat("material.shininess", 48.0f);

        for(float i=-22.0f; i<=22; i+=11) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i, -3.5f, -22.0f));
            model = glm::scale(model, glm::vec3(15.0f, 30.0f, 15.0f));
            model = glm::rotate(model, glm::radians(30.0f*i), glm::vec3(0.0f, 1.0f, 0.0f));
            modelShader.setMat4("model", model);
            mountainModel.Draw(modelShader);
        }

        for(float i=-11.0f; i<=22; i+=11) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-22.0f, -3.5f, i));
            model = glm::scale(model, glm::vec3(15.0f, 30.0f, 15.0f));
            model = glm::rotate(model, glm::radians(-40.0f*i), glm::vec3(0.0f, 1.0f, 0.0f));
            modelShader.setMat4("model", model);
            mountainModel.Draw(modelShader);
        }

        for(float i=-11.0f; i<=22; i+=11) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(22.0f, -3.5f, i));
            model = glm::scale(model, glm::vec3(15.0f, 30.0f, 15.0f));
            model = glm::rotate(model, glm::radians(50.0f*i), glm::vec3(0.0f, 1.0f, 0.0f));
            modelShader.setMat4("model", model);
            mountainModel.Draw(modelShader);
        }

        for(float i=-11.0f; i<=11; i+=11) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(i, -3.5f, 22.0f));
            model = glm::scale(model, glm::vec3(15.0f, 30.0f, 15.0f));
            model = glm::rotate(model, glm::radians(-60.0f*i), glm::vec3(0.0f, 1.0f, 0.0f));
            modelShader.setMat4("model", model);
            mountainModel.Draw(modelShader);
        }

        groundShader.use();

        glBindTexture(GL_TEXTURE_2D, snowTexture);
        glm::mat4 groundModel = glm::mat4(1.0f);
        groundModel = glm::translate(groundModel, glm::vec3(0.0f, -1.0f, 0.0f));
        groundModel = glm::rotate(groundModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        groundModel = glm::scale(groundModel, glm::vec3(30.0, 30.0, 80.0)); 

        groundShader.setMat4("model", groundModel);
        groundShader.setMat4("view", view);
        groundShader.setMat4("projection", projection);

        groundShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        groundShader.setVec3("dirLight.ambient", ambientColor);
        groundShader.setVec3("dirLight.diffuse", diffuseColor);
        groundShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        groundShader.setVec3("pointLight.position", campfirePos);
        groundShader.setVec3("pointLight.ambient", 0.05f, 0.027f, 0.027f);
        groundShader.setVec3("pointLight.diffuse", 1.0f, 0.54f, 0.0f);
        groundShader.setVec3("pointLight.specular", 0.5f, 0.27f, 0.27f);
        groundShader.setFloat("pointLight.constant", 1.0f);
        groundShader.setFloat("pointLight.linear", 0.09f);
        groundShader.setFloat("pointLight.quadratic", 0.032f);

        groundShader.setVec3("viewPos", camera.Position);

        groundShader.setVec3("material.ambient", 0.1f, 0.1f, 0.1f);
        groundShader.setVec3("material.diffuse", 0.4f, 0.4f, 0.4f);
        groundShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f);
        groundShader.setFloat("material.shininess", 12.0f);
        
        groundShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
        groundShader.setVec3("light.ambient", ambientColor);
        groundShader.setVec3("light.diffuse", diffuseColor);
        groundShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
       
        glBindVertexArray(GroundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        particleShader.use();
        
        particleShader.setMat4("view", view);
        particleShader.setMat4("projection", projection);

        particleGroup.Draw(particleShader, cameraEnabled, timeBeforePause);

        borderShader.use();
        borderShader.setMat4("view", view);
        borderShader.setMat4("projection", projection);

        modelShader.use();
        modelShader.setVec3("material.ambient", 0.3f, 0.3f, 0.3f);
        modelShader.setVec3("material.diffuse", 0.6f, 0.6f, 0.6f);
        modelShader.setVec3("material.specular", 0.1f, 0.1f, 0.1f); // specular lighting doesn't have full effect on this object's material
        modelShader.setFloat("material.shininess", 8.0f);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, 0.2f, -4.0f));
        modelShader.setMat4("model", model);
        if(objectIntersection(ray_wor) == CAMPFIRE) {
            scale = 1.01f;
            highlightObject(model, campfireModel, modelShader, scale, borderShader);
        }
        else {
            campfireModel.Draw(modelShader);
        }

        model = glm::mat4(1.0f);
        moonShader.use();
        model = glm::translate(model, lightPos);
        moonShader.setMat4("model", model);
        moonShader.setMat4("projection", projection);    
        moonShader.setMat4("view", view);   
        glBindTexture(GL_TEXTURE_2D, moonTexture);
        if(objectIntersection(ray_wor)==MOON ) {
            scale = 1.1f;
            highlightObject(model, moonModel, moonShader, scale, borderShader);
        }
        else {
            moonModel.Draw(moonShader);
        }
        
        modelShader.use();
        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.8f); // specular lighting doesn't have full effect on this object's material
        modelShader.setFloat("material.shininess", 48.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, cottagePos);
        model = glm::scale(model, glm::vec3(0.075f, 0.075f, 0.075f));
        modelShader.setMat4("model", model);

        if(objectIntersection(ray_wor) == CABIN && cameraEnabled == true) {
            scale = 1.05f;
            highlightObject(model, cottageModel, modelShader, scale, borderShader);
        }
        else {
            cottageModel.Draw(modelShader);
        }
 

        for(float i=0; i<=9; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.0f, -5.0f));
            if(cameraEnabled == true) {
                reindeerPos[i/3] = glm::vec3(cos(((float)glfwGetTime()+i)/2)*3.5, 0.0f, (sin(((float)glfwGetTime()+i)/2)*3.5)-5.0f);
                modelShader.use();
                model = glm::translate(model, glm::vec3(cos(((float)glfwGetTime()+i)/2)*3.5, 0.0f, sin(((float)glfwGetTime()+i)/2)*3.5));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, ((float)glfwGetTime()+i)/2, glm::vec3(0.0f, -1.0f, 0.0f));
                modelShader.setMat4("model", model);
                if(objectIntersection(ray_wor) == (i/3)+1) {
                    scale = 1.05f;
                    highlightObject(model, reindeerModel, modelShader, scale, borderShader);
                }
                else {
                    reindeerModel.Draw(modelShader);
                }
            }
            else {
                modelShader.use();
                model = glm::translate(model, glm::vec3(cos((timeBeforePause+i)/2)*3.5, 0.0f, sin((timeBeforePause+i)/2)*3.5));
                model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::rotate(model, (timeBeforePause+i)/2, glm::vec3(0.0f, -1.0f, 0.0f));
                modelShader.setMat4("model", model);
                reindeerModel.Draw(modelShader);
            }
            
        }

        if(cameraEnabled == false) {
            pauseShader.use();
            glBindVertexArray(PauseVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glm::mat4 nullView = glm::mat4(1.0f);
            glm::mat4 nullProj = glm::mat4(1.0f);
            modelShader.use();
            modelShader.setMat4("projection", nullProj);
            modelShader.setMat4("view", nullView);
            model = glm::mat4(1.0f);

            if(interSectedObject <=4) {
                model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
                model = glm::translate(model, glm::vec3(0.0f, -1.0f, -1.0f));
                model = glm::rotate(model, ((float)glfwGetTime())/2, glm::vec3(0.0f, 1.0f, 0.0f));
                modelShader.setMat4("model", model);
                reindeerModel.Draw(modelShader);
            }
            else if(interSectedObject == CABIN) {
                model = glm::scale(model, glm::vec3(0.014f, 0.014f, 0.014f));
                model = glm::translate(model, glm::vec3(0.0f, -22.0f, 0.0f));
                model = glm::rotate(model, ((float)glfwGetTime())/2, glm::vec3(0.0f, 1.0f, 0.0f));
                modelShader.setMat4("model", model);
                cottageModel.Draw(modelShader);
            }
            else if(interSectedObject == CAMPFIRE) {
                model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
                model = glm::translate(model, glm::vec3(-4.0f, 1.0f, 0.0f));
                modelShader.setMat4("model", model);
                campfireModel.Draw(modelShader);
            }
            else if(interSectedObject == MOON) {
                moonShader.use();
                glBindTexture(GL_TEXTURE_2D, moonTexture); 
                model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
                model = glm::rotate(model, ((float)glfwGetTime())/2, glm::vec3(0.0f, 1.0f, 0.0f));
                moonShader.setMat4("model", model);
                moonShader.setMat4("projection", nullProj);
                moonShader.setMat4("view", nullView);
                moonModel.Draw(moonShader);
            }
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
}

    glfwTerminate();
    return 0;
}

void highlightObject(glm::mat4 model, Model object, Shader shader, float scale, Shader borderShader) {
    glStencilFunc(GL_ALWAYS, 1, 0xFF); 
    glStencilMask(0xFF);
    object.Draw(shader);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);
    borderShader.use();
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    borderShader.setMat4("model", model);
    object.Draw(borderShader);
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);   
    glEnable(GL_DEPTH_TEST);  
}

bool intersection(glm::vec3 center, float radius, glm::vec3 ray) {
    glm::vec3 origin = camera.Position;
    float B = 2*(ray.x)*(origin.x-center.x)+2*(ray.y)*(origin.y-center.y)+2*(ray.z)*(origin.z-center.z);
    float C = (origin.x-center.x)*(origin.x-center.x)+(origin.y-center.y)*(origin.y-center.y)+(origin.z-center.z)*(origin.z-center.z)-(radius*radius);
    float d = (B*B)-(4*C);
    if(d >0) {
        return true;
    }
    return false;
}

float intersectionPoint(glm::vec3 center, float radius, glm::vec3 ray) {
    glm::vec3 origin = camera.Position;
    float B = 2*(ray.x)*(origin.x-center.x)+2*(ray.y)*(origin.y-center.y)+2*(ray.z)*(origin.z-center.z);
    float C = (origin.x-center.x)*(origin.x-center.x)+(origin.y-center.y)*(origin.y-center.y)+(origin.z-center.z)*(origin.z-center.z)-(radius*radius);
    float d = (B*B)-(4*C);
    if(d > 0) {
        float t1 = (-B+sqrt(d))/2;
        float t2 = (-B-sqrt(d))/2;
        return min(t1,t2);
    }
    return -1;

}

struct Item {
    int index;
    float val;
};

int objectIntersection(glm::vec3 ray) {
    // moonInterSection
    vector<Item> list;
    struct Item item;
    int min =INT_MAX;
    float t =0;
    int minIndex =0;
    if(intersection(lightPos, 2, ray)) {
        t = intersectionPoint(lightPos, 2, ray);
        if(t >0) {
            item.index = MOON;
            item.val = t;
            list.push_back(item);
        }
    }
    for(int i=0; i<reindeerPos.size(); i++) {
        if(intersection(reindeerPos[i], 2, ray)) {
            t = intersectionPoint(reindeerPos[i], 2, ray);
            if(t >0) {
                item.index = i+1;
                item.val = t;
                list.push_back(item);
            }
        }
    }
    if(intersection(cottagePos, 4, ray)) {
        t = intersectionPoint(cottagePos, 4, ray);
        if(t >0) {
            item.index = CABIN;
            item.val = t;
            list.push_back(item);
        }
    }
    if(intersection(glm::vec3(campfirePos.x,campfirePos.y-1.5f, campfirePos.z), 0.2, ray)) {
        t = intersectionPoint(glm::vec3(campfirePos.x,campfirePos.y-1.5f, campfirePos.z), 0.2, ray);
        if(t >0) {
            item.index = CAMPFIRE;
            item.val = t;
            list.push_back(item);
        }
    }
    for (int i=0; i<list.size(); i++) {
        item = list[i];
        if(item.val <= min) {
            min = item.val;
            minIndex = item.index;
        }
    }
    return minIndex;
    
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if(cameraEnabled == false) {
            cameraEnabled = true;
            glfwSetTime((double)timeBeforePause);
        }
    }

    if(cameraEnabled == true) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // cout << interSectedObject << std::endl;
        if(cameraEnabled == true && interSectedObject !=0) {
            cameraEnabled = false;
            timeBeforePause = (float)glfwGetTime();
        }
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if(cameraEnabled == true) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.ProcessMouseMovement(xoffset, yoffset);
    }

}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}







