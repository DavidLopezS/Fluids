﻿#include <imgui\imgui.h>
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
};
int numberOfWaves = 3;
Wave* waves = new Wave[numberOfWaves];

float* ampli = new float[numberOfWaves];
float* waveX = new float[numberOfWaves];
float* waveZ = new float[numberOfWaves];
float* freq = new float[numberOfWaves];

float springColumn = 0.76f;
float springRow = 0.58f;
float fluidHeight = 3;

void initializeCloth() {
	fluidSurface[0].initPos = fluidSurface[0].pos = { -(13 * springColumn / 2), 0,-(17 * springRow / 2) };
	for (int i = 0; i < row; ++i) {
		for (int j = 0; j < col; ++j) {
			fluidSurface[i*col + j].initPos = fluidSurface[i*col + j].pos = { fluidSurface[0].initPos.x + j*springColumn ,0 ,fluidSurface[0].initPos.z + i*springRow };
			
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
		fluidSurface[i].pos = - fluidSurface[i].initPos;
		fluidSurface[i].yPos = fluidHeight;
		for (int j = 0; j < numberOfWaves; ++j) {
			fluidSurface[i].pos += (waves[j].k / glm::length(waves[j].k))  * waves[j].A * glm::sin(glm::dot(waves[j].k, fluidSurface[i].initPos) - (waves[j].w * time));
			fluidSurface[i].yPos += waves[j].A * glm::cos(glm::dot(waves[j].k, fluidSurface[i].initPos) - (waves[j].w * time));
		}
	}
}

void manageWave(float a, float wX, float wZ, float f, int waveN) {

	waves[waveN].A = a;
	waves[waveN].k.x = wX;
	waves[waveN].k.z = wZ;
	waves[waveN].w = f;

}

void PhysicsInit() {
	initializeCloth();
	//Possar valors predeterminats de la onada
}

float acumulateTime;
void PhysicsUpdate(float dt) {
	//TODO
	for (int i = 0; i < numberOfWaves; ++i) {
		manageWave(ampli[i], waveX[i], waveZ[i], freq[i], i);
	}
	acumulateTime += dt;
	if (acumulateTime >= 3.14) {
		acumulateTime == 0;
	}
	updateWavePos(acumulateTime);
	particleToFloatConverter();
	ClothMesh::updateClothMesh(vertArray);
}
void PhysicsCleanup() {
	delete[] fluidSurface;
	delete[] vertArray;
	delete[] waves;
	delete[] ampli;
	delete[] waveX;
	delete[] waveZ;
	delete[] freq;
}

void GUI() {
	{	//FrameRate
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::Separator();
		ImGui::Text("Waves parameters: ");
		ImGui::Text("   Wave 1      Wave 2       Wave 3");


		ImGui::SliderFloat3("Amplitude", ampli, 0.f, 1.f, "%.1f");
		ImGui::SliderFloat3("Wave X", waveX, 0.f, 1.f, "%.2f");
		ImGui::SliderFloat3("Wave Z", waveZ, 0.f, 1.f, "%.2f");
		ImGui::SliderFloat3("Frequency", freq, 0.f, 100.f, "%.3f");

		
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}