#include "main.h"
#include <stdio.h>
#include <boost/thread.hpp>

#include "util.h"
#include "renderer.h"

bool flag_loading_terrain = false;

void updateTerrainBuffers(Renderer* r, TerrainMesh* t) {
	while (flag_loading_terrain);
	flag_loading_terrain = true;
	printf("Scheduling terrain update...\n");
	t->flag_bufready = false;
	t->updateChunks();
	t->genBuffers();
	flag_loading_terrain = false;
	t->flag_bufready = true;
}

int main() {
	Renderer* mainRenderer;
	TerrainMesh* mainTerrain;

	mainRenderer = new Renderer();
	mainRenderer->init();
	mainRenderer->loadTestScene();
	mainTerrain = mainRenderer->initTerrain();

	mainRenderer->initCamera();

	// initialize terrain
	printf("Initializing terrain...\n");
	mainTerrain->flag_force_update = true;
	mainTerrain->update(glm::vec2(mainRenderer->getCamPos()));
	updateTerrainBuffers(mainRenderer, mainTerrain);

	while (!mainRenderer->closeRequested()) {
		mainTerrain->update(glm::vec2(mainRenderer->getCamPos()));

		if (mainTerrain->flag_updated)	{
			boost::thread t(updateTerrainBuffers, mainRenderer, mainTerrain);
		//	updateTerrainBuffers(mainRenderer, mainTerrain);
		//	t.join();
		}

		if (mainTerrain->flag_bufready) {
			mainRenderer->assignTerrainBuffer(mainRenderer->buildTerrainBuffers());
			mainTerrain->flag_bufready = false;
		}

		mainRenderer->render();

	}

	//close GL context and any other GLFW resources
	mainRenderer->terminate();
	return 0;
}