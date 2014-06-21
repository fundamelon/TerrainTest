#include "main.h"
#include <stdio.h>

#include "util.h"
#include "renderer.h"

int main() {
	Renderer* mainRenderer = new Renderer();
	mainRenderer->init();

	mainRenderer->loadTestScene();

	mainRenderer->initCamera();

	while (!mainRenderer->closeRequested()) {
		mainRenderer->render();

	}

	//close GL context and any other GLFW resources
	mainRenderer->terminate();
	return 0;
}