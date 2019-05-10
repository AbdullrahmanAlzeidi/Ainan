#pragma once

#include "graphics/ShaderProgram.h"
#include "Window.h"
#include "Particle.h"
#include "Camera.h"
#include "ParticleCustomizer.h"
#include "PerlinNoise2D.h"
#include "InspectorInterface.h"
#include "graphics/Texture.h"

namespace ALZ {

	class ParticleSystem : public InspectorInterface
	{
	public:
		ParticleSystem();
		~ParticleSystem();

		void Update(const float& deltaTime, Camera& camera) override;
		void Render(Camera& camera) override;
		void SpawnAllParticlesOnQue(const float& deltaTime, Camera& camera);
		void SpawnParticle(const Particle& particle);
		void ClearParticles();
		void DisplayGUI(Camera& camera) override;

		ParticleSystem(const ParticleSystem& Psystem);
		ParticleSystem operator=(const ParticleSystem& Psystem);

	public:
		ParticleCustomizer Customizer;
		//only for spawning on mouse press
		bool ShouldSpawnParticles;
		float TimeTillNextParticleSpawn = 0.0f;
		unsigned int ActiveParticleCount = 0;

	private:
		void* m_ParticleInfoBuffer;

		std::vector<Particle> m_Particles;
		unsigned int m_ParticleCount;
		PerlinNoise2D m_Noise;
	};
}