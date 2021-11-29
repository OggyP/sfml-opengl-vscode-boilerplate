#include "Platform/Platform.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

#include "openGLsetup.hpp"
#include "verticies.hpp"

bool running = true;

const GLchar* screenVertexSource = R"glsl(
    #version 150 core
    in vec2 position;
    in vec2 texcoord;
    out vec2 Texcoord;
    void main()
    {
        Texcoord = texcoord;
        gl_Position = vec4(position, 0.0, 1.0);
    }
)glsl";
const GLchar* screenFragmentSource = R"glsl(
    #version 150 core
    in vec2 Texcoord;

    out vec4 outColor;

    uniform sampler2D texImage;

    void main()
	{
		outColor = texture(texImage, Texcoord);
    }
)glsl";

void renderingThread(sf::Window* window);
void renderingThread(sf::Window* window)
{
	// activate the window's context
	window->setActive(true);

	glewExperimental = GL_TRUE;
	glewInit();

	GLuint vaoQuad;
	glGenVertexArrays(1, &vaoQuad);

	glBindVertexArray(vaoQuad);

	GLuint vboQuad;
	glGenBuffers(1, &vboQuad);

	glBindBuffer(GL_ARRAY_BUFFER, vboQuad);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	GLuint sfmlIcon;
	glGenTextures(1, &sfmlIcon);

	sf::Image imageLoad;
	if (imageLoad.loadFromFile("./content/sfml.png"))
	{
		glBindTexture(GL_TEXTURE_2D, sfmlIcon);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageLoad.getSize().x, imageLoad.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageLoad.getPixelsPtr());

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Error loading SFML Image\n";
	}

	GLuint screenVertexShader, screenFragmentShader, screenShaderProgram;
	createShaderProgram(screenVertexSource, screenFragmentSource, screenVertexShader, screenFragmentShader, screenShaderProgram);

	specifyScreenVertexAttributes(screenShaderProgram);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sfmlIcon);

	glEnable(GL_DEPTH_TEST);

	while (running)
	{
		glClearColor(0.450, 0.937, 0.968, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(screenShaderProgram);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sfmlIcon);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		window->display();
	}
}

int main()
{
	util::Platform platform;

#if defined(_DEBUG)
	std::cout << "Hello World!" << std::endl;
#endif

	sf::RenderWindow window;
	// in Windows at least, this must be called before creating the window
	float screenScalingFactor = platform.getScreenScalingFactor(window.getSystemHandle());
	// Use the screenScalingFactor
	window.create(sf::VideoMode(200.0f * screenScalingFactor, 200.0f * screenScalingFactor), "SFML with OpenGL works!");
	platform.setIcon(window.getSystemHandle());

	window.setActive(false);

	// launch the rendering thread
	sf::Thread renderThread(&renderingThread, &window);
	renderThread.launch();

	sf::Event event;

	while (running)
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
				running = false;
			}
		}
	}

	return 0;
}
