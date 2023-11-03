#include <glad/glad.h>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "stb_image.h"

#define FOV 45.0f
#define WIDTH 1000.0f
#define HEIGHT 800.0f
#define NEAR 0.1f
#define FAR 100.0f

float fov = FOV;

void setupCursor(sf::RenderWindow &window);
void getCursorOffset(sf::RenderWindow &window, float &offsetX, float &offsetY, float &lastX, float &lastY, float deltaTime);

bool loadGlad();
void handleInput(sf::RenderWindow &window, float deltaTime, Camera &camera, float &lastX, float &lastY);

void setupBuffer(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO, float *vertices, unsigned int vSize, unsigned int *indices, unsigned int iSize);
void activateTexture(int index, unsigned int tex);
void loadTexture(int index, unsigned int &tex, const char *path, GLint internalFormat, GLenum format);

void updateModel(unsigned int index, sf::Clock &clock, Shader &ourShader);
glm::mat4 getPerspectiveProjection();
void updateCamera(Camera &camera, Shader &ourShader);

float vertices[] = {
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
    0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
    -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};
unsigned int indices[] = {
    // note that we start from 0!
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

glm::vec3 cubePositions[] = {
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(2.0f, 5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f, 3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f, 2.0f, -2.5f),
    glm::vec3(1.5f, 0.2f, -1.5f),
    glm::vec3(-1.3f, 1.0f, -1.5f)};

int main()
{

    sf::ContextSettings settings;
    settings.depthBits = 24;

    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "OpenGL", sf::Style::Close, settings);
    window.setActive(true);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!loadGlad())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    Shader ourShader("../shaders/shader.vert", "../shaders/shader.frag");

    unsigned int VAO, VBO, EBO;
    setupBuffer(VAO, VBO, EBO, vertices, sizeof(vertices), indices, sizeof(indices));

    unsigned int texture1, texture2;
    loadTexture(0, texture1, "../textures/container.jpg", GL_RGB, GL_RGB);
    loadTexture(1, texture2, "../textures/awesomeface.png", GL_RGBA, GL_RGBA);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("texture2", 1);

    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.f);

    // Setup cursor
    setupCursor(window);
    float lastX = WIDTH / 2, lastY = HEIGHT / 2;
    float offsetX = 0.0f, offsetY = 0.0f;
    float yaw = -90.0f, pitch = 0.0f;

    Camera camera;

    sf::Clock clock;
    float lastFrame = 0.0, deltaTime = 0.0;
    while (window.isOpen())
    {

        float currentFrame = clock.getElapsedTime().asSeconds();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // printf("%f\n", 1 / deltaTime);

        getCursorOffset(window, offsetX, offsetY, lastX, lastY, deltaTime);
        camera.ProcessMouseMovement(offsetX, offsetY);

        handleInput(window, deltaTime, camera, lastX, lastY);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        ourShader.use();
        activateTexture(0, texture1);
        activateTexture(1, texture2);

        // Bind vertex array object
        glBindVertexArray(VAO);
        updateCamera(camera, ourShader);
        // Draw rectangle with  vertices
        for (unsigned int i = 0; i < 10; i++)
        {
            updateModel(i, clock, ourShader);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // Draw rectangle with  indexes
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        window.display();
    }

    return 0;
}

void setupCursor(sf::RenderWindow &window)
{
    sf::Mouse::setPosition(sf::Vector2i(WIDTH / 2, HEIGHT / 2), window);
    window.setMouseCursorGrabbed(true);
    window.setMouseCursorVisible(false);
}

void getCursorOffset(sf::RenderWindow &window, float &offsetX, float &offsetY, float &lastX, float &lastY, float deltaTime)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    offsetX = mousePos.x - lastX;
    offsetY = lastY - mousePos.y;
    lastX = mousePos.x;
    lastY = mousePos.y;

    offsetX *= SENSITIVITY;
    offsetY *= SENSITIVITY;
}

bool loadGlad()
{
    return gladLoadGLLoader(reinterpret_cast<GLADloadproc>(sf::Context::getFunction));
}

void handleInput(sf::RenderWindow &window, float deltaTime, Camera &camera, float &lastX, float &lastY)
{
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    sf::Event event;
    while (window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            window.close();
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                window.close();
            }
        }
        if (event.type == sf::Event::MouseWheelScrolled)
        {
            if (event.mouseWheelScroll.delta > 0)
            {
                fov -= 1.0f;
            }
            else
            {
                fov += 1.0f;
            }
            if (fov < 1.0f)
                fov = 1.0f;
            if (fov > 45.0f)
                fov = 45.0f;
        }
        if (event.type == sf::Event::MouseMoved)
        {
            sf::Vector2i mousePos = sf::Mouse::getPosition(window);
            float margin = 10;
            if (mousePos.x >= WIDTH - margin)
            {
                sf::Mouse::setPosition(sf::Vector2i(mousePos.x - WIDTH / 2, mousePos.y), window);
                lastX -= WIDTH / 2;
            }
            if (mousePos.x <= margin)
            {
                sf::Mouse::setPosition(sf::Vector2i(mousePos.x + WIDTH / 2, mousePos.y), window);
                lastX += WIDTH / 2;
            }
            if (mousePos.y >= HEIGHT - margin)
            {
                sf::Mouse::setPosition(sf::Vector2i(mousePos.x, mousePos.y - HEIGHT / 2), window);
                lastY -= HEIGHT / 2;
            }
            if (mousePos.y <= margin)
            {
                sf::Mouse::setPosition(sf::Vector2i(mousePos.x, mousePos.y + HEIGHT / 2), window);
                lastY += HEIGHT / 2;
            }
        }
    }
}

void setupBuffer(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO, float *vertices, unsigned int vSize, unsigned int *indices, unsigned int iSize)
{

    // Vertex Array Object
    glGenVertexArrays(1, &VAO);
    // Vertex Buffer Object
    glGenBuffers(1, &VBO);
    // Element Buffer Object
    glGenBuffers(1, &EBO);

    // 1. bind Vertex Array Object
    glBindVertexArray(VAO);
    // 2. copy our vertices array in a vertex buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vSize, vertices, GL_STATIC_DRAW);
    // 3. copy our index array in a element buffer for OpenGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize, indices, GL_STATIC_DRAW);

    // x y z    r g b    s t
    // 4. then set the vertex attributes pointers for position x y z
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // // 5. then set the vertex attributes pointers for color r g b
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    // 6. then set the vertex attributes pointers for texture coords s t
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
}

void activateTexture(int index, unsigned int tex)
{
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, tex);
}

void loadTexture(int index, unsigned int &ref, const char *path, GLint internalFormat, GLenum format)
{
    glGenTextures(1, &ref);
    activateTexture(index, ref);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nColorChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(path, &width, &height, &nColorChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
}

void updateModel(unsigned int index, sf::Clock &clock, Shader &ourShader)
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cubePositions[index]);

    // float x = sin(clock.getElapsedTime().asSeconds()) * 2;
    // model = glm::translate(model, glm::vec3(x, 0.0f, 0.0f));

    float angle = index + clock.getElapsedTime().asSeconds() * 20;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

    ourShader.setMat4("model", model);
}

glm::mat4 getPerspectiveProjection()
{
    return glm::perspective(glm::radians(fov), WIDTH / HEIGHT, NEAR, FAR);
}

void updateCamera(Camera &camera, Shader &ourShader)
{

    glm::mat4 view = camera.GetViewMatrix();

    glm::mat4 projection = getPerspectiveProjection();

    ourShader.setMat4("view", view);
    ourShader.setMat4("projection", projection);
}