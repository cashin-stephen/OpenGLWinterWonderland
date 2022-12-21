#ifndef PARTICLE_H
#define PARTICLE_H

#include <glm/glm.hpp>
#include <vector>
#include <cmath>

using namespace std;

class Particle {
public:
    //glm::vec3 position;
    glm::vec3 color;
    glm::vec3 pos;
    glm::vec3 offset;
    bool alive;
    int life;
    float birthTime;
    unsigned int VAO;
    float lightStrength;

    Particle(glm::vec3 color, glm::vec3 pos)//glm::vec2 position, glm::vec4 color)
    {
        this->pos = pos;
        this->color = color;
        
        // modulo value givees full range of possible values
        // divisor scales down size
        int spawnRange = 1000;
        float sizeDivisor = 2000.0f;
        float size = ((float)spawnRange)/sizeDivisor;
        float randxOffset = ((rand()%spawnRange));
        float randyOffset = ((rand()%spawnRange));
        float xoffset = randxOffset/sizeDivisor;
        float zoffset = randyOffset/sizeDivisor;
        offset = glm::vec3(xoffset, 0.0f, zoffset);
        // float randomc = rand()%50;
        float distanceToCenter = (sqrt(pow(abs((size/2)-xoffset),2) + pow(abs((size/2)-zoffset),2)));
        float maxDistanceToCenter = (sqrt(pow(abs((size/2)-size),2) + pow(abs((size/2)-size),2)));
        int startingLife = 1000;
        life = startingLife - distanceToCenter*(startingLife/maxDistanceToCenter);// + randomc;

        // life = ((startingLife*1.1) - distanceToCenter*(startingLife/distanceToCenter)) + randomc;
        birthTime = (float)glfwGetTime();

        setUp();
    }

    void Draw(Shader &shader) 
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, pos);
        model = glm::translate(model, offset);
        model = glm::translate(model, glm::vec3(0.0f, ((float)glfwGetTime()-birthTime)/20, 0.0f));
        model = glm::rotate(model, (float)glfwGetTime()*3, glm::vec3(1.0f,0.5f, 0.3f));
        
        lightStrength = (float)(life+100)/300;

        shader.setFloat("strength", lightStrength);
        shader.setMat4("model", model);
        shader.setVec3("color", color);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        life-=4;
    }

private:
    unsigned int VBO;

    void setUp()
    {
        float cubeVertices[] = {
            -0.03f, -0.03f, -0.03f,
            0.03f, -0.03f, -0.03f,
            0.03f,  0.03f, -0.03f,
            0.03f,  0.03f, -0.03f,
            -0.03f,  0.03f, -0.03f,  
            -0.03f, -0.03f, -0.03f,  

            -0.03f, -0.03f,  0.03f,  
            0.03f, -0.03f,  0.03f,  
            0.03f,  0.03f,  0.03f,
            0.03f,  0.03f,  0.03f,
            -0.03f,  0.03f,  0.03f,  
            -0.03f, -0.03f,  0.03f,  

            -0.03f,  0.03f,  0.03f,  
            -0.03f,  0.03f, -0.03f,
            -0.03f, -0.03f, -0.03f,  
            -0.03f, -0.03f, -0.03f,  
            -0.03f, -0.03f,  0.03f,  
            -0.03f,  0.03f,  0.03f,  

            0.03f,  0.03f,  0.03f,  
            0.03f,  0.03f, -0.03f,
            0.03f, -0.03f, -0.03f,  
            0.03f, -0.03f, -0.03f,  
            0.03f, -0.03f,  0.03f,  
            0.03f,  0.03f,  0.03f,  

            -0.03f, -0.03f, -0.03f,  
            0.03f, -0.03f, -0.03f,
            0.03f, -0.03f,  0.03f,  
            0.03f, -0.03f,  0.03f,  
            -0.03f, -0.03f,  0.03f,  
            -0.03f, -0.03f, -0.03f,  

            -0.03f,  0.03f, -0.03f,  
            0.03f,  0.03f, -0.03f,
            0.03f,  0.03f,  0.03f,  
            0.03f,  0.03f,  0.03f,  
            -0.03f,  0.03f,  0.03f,  
            -0.03f,  0.03f, -0.03f
        };
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    }
};
#endif