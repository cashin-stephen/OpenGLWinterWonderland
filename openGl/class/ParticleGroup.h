#ifndef PARTICLEGROUP_H
#define PARTICLEGROUP_H

#include <glm/glm.hpp>
#include <vector>

#include "Particle.h"
using namespace std;

class ParticleGroup {
public:
    vector<Particle> particles;
    glm::vec3 color;
    glm::vec3 pos;
    bool maxParticles;
    int numParticles;

    ParticleGroup(glm::vec3 color, glm::vec3 pos, unsigned int numParticles)
    {
        this->color = color;
        this->pos = pos;
        this->numParticles = numParticles;
        maxParticles = false;
    }

    void Draw(Shader &shader, bool cameraEnabled, float timeBeforePause) {
        if(maxParticles == false) {
            for (unsigned int i = 0; i < numParticles; i++) {
                Particle particle(color, pos);
                particles.push_back(particle);
            }
        }

        for(unsigned int i = 0; i < particles.size(); i++) {
            if(particles[i].life < -800) {
                particles[i] = Particle(color, pos);
                maxParticles = true;
            }
            else {
                particles[i].Draw(shader, cameraEnabled, timeBeforePause);
            }
            
        }
    }
};
#endif