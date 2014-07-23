#include "main.h"

#include "util.h"
#include "renderer.h"
#include "Terrain.h"

void updateTerrainBuffers(Terrain* t) {
	printf("Scheduling terrain update...\n");
	double start_seconds = glfwGetTime();

	t->regen();

	double end_seconds = glfwGetTime();
	printf("Terrain generated in %f sec\n", end_seconds - start_seconds);
}

int main() {
	Renderer* mainRenderer;
	Terrain* mainTerrain;

	mainRenderer = new Renderer();
	mainRenderer->init();

	mainTerrain = new Terrain();

	//TERRAIN SEED
	mainTerrain->setSeed(233);

	mainRenderer->setTerrain(mainTerrain);
	mainRenderer->initTerrain();

	mainRenderer->initCamera();

	// initialize terrain
	printf("Initializing terrain...\n");

	mainTerrain->forceUpdate();// flag_force_update = true;
	mainTerrain->update(glm::vec2(mainRenderer->getCamPos()));

	boost::thread* t;
	t = new boost::thread(updateTerrainBuffers, mainTerrain);

	while (!mainRenderer->closeRequested()) {
		mainTerrain->update(glm::vec2(mainRenderer->getCamPos()));

		//start terrain update thread
		if (mainTerrain->updateRequested())	{
			delete t;
			t = new boost::thread(updateTerrainBuffers, mainTerrain);
		}

		// if able, regenerate and assign terrain VAOs
		if (mainTerrain->buffersReady()) {
			mainRenderer->buildTerrainBuffers();
			mainTerrain->buffersCreated();
		}

		mainRenderer->render();
	}

	//close GL context and any other GLFW resources
	mainRenderer->terminate();
	return 0;
}