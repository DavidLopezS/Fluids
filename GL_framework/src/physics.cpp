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
glm::vec3 spherePos = { rand() % 7 - 3, rand() % 7 + 1 - sphereRadius, rand() % 7 - 3 };
float gravity = 9.8;

namespace Sphere {
	extern void updateSphere(glm::vec3 pos /*= spherePos*/, float radius /*= sphereRadius*/);
}

namespace ClothMesh {
	extern void updateClothMesh(float *array_data);
}

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
float fluidDensity = 10;

class Ball {
public:
	glm::vec3 pos = {0, 8, 0};//Posicio incial de l'esfera
	glm::vec3 velocity;
	glm::vec3 totalForce{ 0, -gravity, 0 };
	float mass = 1;
	float Vsub;//Volume of the sphere that is flooded

};

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

Ball sphere;
void moveBall(float time) {
	//Actualitzar la velocitat
	sphere.velocity = sphere.velocity + sphere.totalForce*time/sphere.mass;

	//Actualitzar posicio
	sphere.pos = sphere.pos + sphere.velocity * time;
}

glm::vec3 buoyanceF, dragF;
//http://www.1728.org/spher2.png +++++++++++ http://mathworld.wolfram.com/images/eps-gif/SphericalCap_1001.gif
void applyForces() {

	//if(sumergida 0,25,75,full%)... - apliquem les diferents formules que calguin

	//calcular bouyance
	float volume;
	glm::vec3 multiplyOnY = { 0,1,0 };
	buoyanceF = fluidDensity*gravity*multiplyOnY; //potser cal variar la densitat del fluid, potser la gravetat es negativa
	//calcular drag force
	dragF = { 0,0,0 };
	//sumar totes les forces
	sphere.totalForce = buoyanceF + dragF;

	//sphere.bForce = sphere.p * gravity * sphere.Vsub;
}

void PhysicsInit() {
	initializeCloth();
	//Possar valors predeterminats de la onada
	ampli[0] = 0.5;
	ampli[1] = 0.5;
	ampli[2] = 0.5;
	freq[0] = 1;
	freq[1] = 1;
	freq[2] = 1;
	waveX[0] = 0.1;
	waveX[1] = 0.1;
	waveX[2] = 0.1;
	waveZ[0] = 0.1;
	waveZ[1] = 0.1;
	waveZ[2] = 0.1;
}

float acumulateTime;
float seconds = 0;
float secondsUntilRestart = 20;
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

	sphereRadius = 1;
	applyForces();
	moveBall(dt);
	Sphere::updateSphere(sphere.pos, sphereRadius);

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
		ImGui::SliderFloat3("Frequency", freq, 0.f, 10.f, "%.3f");
		ImGui::Separator();
		ImGui::Text("Sphere parameters: ");

		
	}

	// ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}