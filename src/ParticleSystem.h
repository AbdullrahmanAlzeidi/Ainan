#pragma once

#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderProgram.h"
#include "Window.h"
#include "Particle.h"

class ParticleSystem 
{
public:
	ParticleSystem();

	void Update(const float& deltaTime);
	void Draw();
	void SpawnParticle(const Particle& particle);
	void ClearParticles();

private:
	ShaderProgram m_Shader;
	std::vector<Particle> m_Particles;
};