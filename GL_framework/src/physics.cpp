#include <imgui\imgui.h>
#include <imgui\imgui_impl_glfw_gl3.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <iostream>
#include <math.h>
#include <time.h>

bool show_test_window = false;

float sphereRadius = rand() % 3 + 0.5f;
glm::vec3 spherePos(rand() % 7 - 3, rand() % 7 + 1 - sphereRadius, rand() % 7 - 3);

namespace Sphere {
	extern void updateSphere(glm::vec3 pos = spherePos, float radius = sphereRadius);
}

namespace ClothMesh {
	extern void updateClothMesh(float *array_data);
}

glm::vec3 gravity = { 0, -9.8f, 0 };

int col = 14, row = 18;
int clothLength = col*row; // 14*18=252

class Particle {
public:
	glm::vec3 initPos; //x0
	glm::vec3 pos; //x
	float yPos;
	glm::vec3 direction;//Maybe in another class
};
Particle* fluidSurface = new Particle[clothLength];
float* vertArray = new float[clothLength * 3];

class Wave{
public:
	float A;//Amplitud
	float w;//Omega
	glm::vec3 k;//Wave vector
	glm::vec3 Φ;//Phi, desplaça el punt d'inici de la wave
};
int numberOfWaves = 1; //de moment no ho fem servir
Wave w1;

float springColumn = 0.76f;
float springRow = 0.58f;
float fluidHeight = 3;

void initializeCloth() {
	fluidSurface[0].initPos = fluidSurface[0].pos = { -(13 * springColumn / 2),fluidHeight,-(17 * springRow / 2) };
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			fluidSurface[i*col + j].initPos = fluidSurface[i*col + j].pos = { fluidSurface[0].initPos.x + j*springColumn ,fluidHeight ,fluidSurface[0].initPos.z + i*springRow };
			
		}
	}
}

void particleToFloatConverter() {
	for (int i = 0; i < 252; ++i) {
		vertArray[i * 3 + 0] = fluidSurface[i].pos.x;
		vertArray[i * 3 + 1] = fluidSurface[i].yPos;
		vertArray[i * 3 + 2] = fluidSurface[i].pos.z;
	}
}

void updateWavePos(float time) {
	for(int i = 0; i < 252; ++i){
		fluidSurface[i].pos = fluidSurface[i].initPos - (w1.k / glm::length(w1.k))  * w1.A * glm::sin(glm::dot(w1.k, fluidSurface[i].initPos) - (w1.w * time));
		fluidSurface[i].yPos = w1.A * glm::cos(glm::dot(w1.k, fluidSurface[i].initPos) - (w1.w * time));
	}
	
}

void PhysicsInit() {
	initializeCloth();
	w1.A = 1.f;
	w1.k = {0.3, 0, 0};
	w1.w = 0.1f;
}
void PhysicsUpdate(float dt) {
	//TODO
	updateWavePos(dt);
	particleToFloatConverter();
	ClothMesh::updateClothMesh(vertArray);
}
void PhysicsCleanup() {
	delete[] fluidSurface;
	delete[] vertArray;
}

void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		//TODO
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}