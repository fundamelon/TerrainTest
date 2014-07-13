#include "main.h"
#include <stdio.h>
#include <boost/thread.hpp>

#include "util.h"
#include "renderer.h"

void updateTerrainBuffers(Renderer* r, TerrainMesh* t) {
	t->flag_updating = true;
	double start_seconds = glfwGetTime();

//	printf("Scheduling terrain update...\n");
	t->flag_bufready = false;

	t->updateChunks();
	t->triangulate();

	t->genBuffers();

	t->flag_updating = false;

	double end_seconds = glfwGetTime();
	printf("Terrain generated in %f sec\n", end_seconds - start_seconds);

	t->flag_bufready = true;
}

int main() {
	Renderer* mainRenderer;
	TerrainMesh* mainTerrain;

	mainRenderer = new Renderer();
	mainRenderer->init();

	mainTerrain = new TerrainMesh();
	mainTerrain->setSeed(9);
	//	 8 : plains
	//   9 : hills
	//1000 : big cliff

	mainRenderer->setTerrain(mainTerrain);
	mainRenderer->initTerrain();

	mainRenderer->initCamera();

	// initialize terrain
	printf("Initializing terrain...\n");
	mainTerrain->flag_force_update = true;
	mainTerrain->update(glm::vec2(mainRenderer->getCamPos()));
	boost::thread* t;
	t = new boost::thread(updateTerrainBuffers, mainRenderer, mainTerrain);

	while (!mainRenderer->closeRequested()) {
		mainTerrain->update(glm::vec2(mainRenderer->getCamPos()));

		//start terrain update thread
		if (mainTerrain->flag_updated && !mainTerrain->flag_updating)	{
			delete t;
			t = new boost::thread(updateTerrainBuffers, mainRenderer, mainTerrain);
		}

		//regenerate and assign terrain VAOs
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