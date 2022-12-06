#include <GL2/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <assimp/aabb.h> 
#include <stdio.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "class/Shader.h"
#include "class/Camera.h"
#include "class/Model.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

const unsigned int SCR_WIDTH = 640;
const unsigned int SCR_HEIGHT = 480;

bool firstMouse = true;
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::vec3 lightPos(12.0f, 20.0f, 0.0f);
glm::vec3 moonlight(0.5f, 0.5f, 0.6f);
glm::vec3 campfirePos(0.0f, 1.0f, -4.5f);
//glm::vec3 moonlight(1.0f, 1.0f, 1.0f);

unsigned int getTexture(const char* path){

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
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

    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit()) {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return 1;
    } 

    // uncomment these lines if on Apple OS X
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

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                                    
    // start GLEW extension handler
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);

    // tell GL to only draw onto a pixel if the shape is closer to the viewer
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"

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

    Shader groundShader("groundvs.txt", "groundfs.txt");
    Shader modelShader("modelvs.txt", "modelfs.txt");
    Shader moonShader("moonvs.txt", "moonfs.txt");
    Shader fireShader("firevs.txt", "firefs.txt");

    Model cottageModel(filesystem::path("assets/cottage/cottage.obj"));
    Model treeModel(filesystem::path("assets/tree/Tree.obj"));
    Model mountainModel(filesystem::path("assets/mountain/mountainpeak.obj"));
    Model moonModel(filesystem::path("assets/moon/moon.obj"));
    Model campfireModel(filesystem::path("assets/campfire/campfire.obj"));
    Model reindeerModel(filesystem::path("assets/reindeer/reindeer.obj"));

    unsigned int snowTexture = getTexture("textures/snow2.jpeg");
    unsigned int moonTexture = getTexture("textures/moon.jpg");
    unsigned int snowyMountain = getTexture("textures/snowyMountain.jpeg");

    groundShader.setInt("texture", 0);

    while(!glfwWindowShouldClose(window)) {
        //background colours
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //camera Movement
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Model shaders & translations & drawing

        moonShader.use();
        glBindTexture(GL_TEXTURE_2D, moonTexture);

        glm::mat4 projection = glm::mat4(1.0f);;
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        moonShader.setMat4("projection", projection);    

        glm::mat4 view = camera.GetViewMatrix();
        moonShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        moonShader.setMat4("model", model);
        moonModel.Draw(modelShader);

        fireShader.use();
        fireShader.setMat4("projection", projection);    
        fireShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, campfirePos);
        model = glm::translate(model, glm::vec3(-0.1f, -2.0f, -0.1f));
        model = glm::scale(model, glm::vec3(0.22f, 0.2f, 0.22f));
        fireShader.setMat4("model", model);
        moonModel.Draw(fireShader);

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

        //campfire material properties
        modelShader.setVec3("material.ambient", 0.3f, 0.3f, 0.3f);
        modelShader.setVec3("material.diffuse", 0.6f, 0.6f, 0.6f);
        modelShader.setVec3("material.specular", 0.1f, 0.1f, 0.1f); // specular lighting doesn't have full effect on this object's material
        modelShader.setFloat("material.shininess", 8.0f);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-4.0f, 0.2f, -4.0f));
        modelShader.setMat4("model", model);
        campfireModel.Draw(modelShader);

        //reindeer material Properties
        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.1f, 0.1f, 0.1f); // specular lighting doesn't have full effect on this object's material
        modelShader.setFloat("material.shininess", 4.0f);

        for(float i=0; i<=9; i+=3) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -1.0f, -5.0f));
            model = glm::translate(model, glm::vec3(cos(((float)glfwGetTime()+i)/2)*3.5, 0.0f, sin(((float)glfwGetTime()+i)/2)*3.5));
            model = glm::scale(model, glm::vec3(0.02f, 0.02f, 0.02f));
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, ((float)glfwGetTime()+i)/2, glm::vec3(0.0f, 0.0f, -1.0f));
            modelShader.setMat4("model", model);
            reindeerModel.Draw(modelShader);
        }

        // tree material properties
        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.2f, 0.2f, 0.2f); // specular lighting doesn't have full effect on this object's material
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

        //come back to texturing mountains
        glBindTexture(GL_TEXTURE_2D, snowyMountain);

        //Mountain material properties
        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.8f, 0.8f, 0.8f); // specular lighting doesn't have full effect on this object's material
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


        // material properties for cottage and bird
        // same for no reason at all
        modelShader.setVec3("material.ambient", 0.2f, 0.2f, 0.2f);
        modelShader.setVec3("material.diffuse", 0.5f, 0.5f, 0.5f);
        modelShader.setVec3("material.specular", 0.5f, 0.5f, 0.8f); // specular lighting doesn't have full effect on this object's material
        modelShader.setFloat("material.shininess", 48.0f);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.075f, 0.075f, 0.075f));
        model = glm::translate(model, glm::vec3(0.0f, -12.0f, 140.0f));
        modelShader.setMat4("model", model);
        cottageModel.Draw(modelShader);

        // ground shaders & translations & drawing
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

        //snow material Properties
        groundShader.setVec3("material.ambient", 0.1f, 0.1f, 0.1f);
        groundShader.setVec3("material.diffuse", 0.4f, 0.4f, 0.4f);
        groundShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f); // specular lighting doesn't have full effect on this object's material
        groundShader.setFloat("material.shininess", 12.0f);
        
        groundShader.setVec3("light.direction", -0.2f, -1.0f, -0.3f);
        groundShader.setVec3("light.ambient", ambientColor);
        groundShader.setVec3("light.diffuse", diffuseColor);
        groundShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
       
        glBindVertexArray(GroundVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
}

    // close GL context and any other GLFW resources
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
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

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}







