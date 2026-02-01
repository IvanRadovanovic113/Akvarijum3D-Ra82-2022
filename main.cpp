#include <iostream>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "shader.hpp"
#include "model.hpp"

bool transparent = false;

// Riba 1 (WASD + QE)
glm::vec3 fishPos(0.0f, 0.0f, 0.0f);
float fishSpeed = 0.0125f;
float fishRotationY = 0.0f;
float fish1Scale = 0.02f;

// Riba 2 (strelice + KL)
glm::vec3 fish2Pos(1.0f, 0.0f, 0.0f);
float fish2RotationY = 0.0f;
float fish2Scale = 0.05f;

// Granice akvarijuma
const float W = 6.0f, H = 4.0f, D = 3.0f;
const float margin = 0.3f;
const float minX = -W / 2 + margin;
const float maxX = W / 2 - margin;
const float minY = -H / 2 + 0.4f;
const float maxY = H / 2 - margin / 2;
const float minZ = -D / 2 + margin;
const float maxZ = D / 2 - margin;

// Mehurici
struct Bubble {
    glm::vec3 position;
    float speed;
    bool active;
};
std::vector<Bubble> bubbles;
float bubbleSpeed = 0.00625f;
bool zPressed = false;
bool oPressed = false;
const int CIRCLE_SEGMENTS = 32;

// Hrana (beans)
struct Food {
    glm::vec3 position;
    glm::vec3 rotation;
    float fallSpeed;
    bool active;
    bool onGround;
};
std::vector<Food> foods;
float foodFallSpeed = 0.00625f;
bool enterPressed = false;
const float collisionRadius = 0.3f;

// Skrinja
bool chestOpen = false;
bool yPressed = false;

// Kamera
float cameraAngle = 0.0f;
float cameraRadius = 6.0f;
float cameraHeight = 2.0f;
float cameraRotSpeed = 1.5f;

// Frame limiter
const float targetFPS = 75.0f;
const float frameTime = 1.0f / targetFPS;
float lastFrameTime = 0.0f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
        transparent = !transparent;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void processInput(GLFWwindow* window)
{
    // Riba 1 - WASD
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        fishPos.z -= fishSpeed;
        fishRotationY = -90.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        fishPos.z += fishSpeed;
        fishRotationY = 90.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        fishPos.x -= fishSpeed;
        fishRotationY = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        fishPos.x += fishSpeed;
        fishRotationY = 180.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        fishPos.y += fishSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        fishPos.y -= fishSpeed;

    fishPos.x = glm::clamp(fishPos.x, minX, maxX);
    fishPos.y = glm::clamp(fishPos.y, minY, maxY);
    fishPos.z = glm::clamp(fishPos.z, minZ, maxZ);

    // Riba 2 - strelice
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        fish2Pos.z -= fishSpeed;
        fish2RotationY = -90.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        fish2Pos.z += fishSpeed;
        fish2RotationY = 90.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        fish2Pos.x -= fishSpeed;
        fish2RotationY = 0.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        fish2Pos.x += fishSpeed;
        fish2RotationY = 180.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        fish2Pos.y += fishSpeed;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        fish2Pos.y -= fishSpeed;

    fish2Pos.x = glm::clamp(fish2Pos.x, minX, maxX);
    fish2Pos.y = glm::clamp(fish2Pos.y, minY, maxY);
    fish2Pos.z = glm::clamp(fish2Pos.z, minZ, maxZ);

    // Z - mehurici ribe 1
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !zPressed) {
        zPressed = true;
        float mouthOffset = fish1Scale * 10.0f;
        glm::vec3 mouthDir(0.0f);
        if (fishRotationY == 0.0f) mouthDir = glm::vec3(-1, 0, 0);
        else if (fishRotationY == 180.0f) mouthDir = glm::vec3(1, 0, 0);
        else if (fishRotationY == -90.0f) mouthDir = glm::vec3(0, 0, -1);
        else if (fishRotationY == 90.0f) mouthDir = glm::vec3(0, 0, 1);

        glm::vec3 mouthPos = fishPos + mouthDir * mouthOffset + glm::vec3(0, 0.05f, 0);
        for (int i = 0; i < 3; i++) {
            Bubble b;
            b.position = mouthPos + glm::vec3((rand() % 100 - 50) / 500.0f, 0.0f, (rand() % 100 - 50) / 500.0f);
            b.speed = bubbleSpeed + (rand() % 100) / 10000.0f;
            b.active = true;
            bubbles.push_back(b);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE) zPressed = false;

    // O - mehurici ribe 2
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !oPressed) {
        oPressed = true;
        float mouthOffset2 = fish2Scale * 5.0f;
        glm::vec3 mouthDir2(0.0f);
        if (fish2RotationY == 0.0f) mouthDir2 = glm::vec3(-1, 0, 0);
        else if (fish2RotationY == 180.0f) mouthDir2 = glm::vec3(1, 0, 0);
        else if (fish2RotationY == -90.0f) mouthDir2 = glm::vec3(0, 0, -1);
        else if (fish2RotationY == 90.0f) mouthDir2 = glm::vec3(0, 0, 1);

        glm::vec3 mouthPos2 = fish2Pos + mouthDir2 * mouthOffset2 + glm::vec3(0, 0.05f, 0);
        for (int i = 0; i < 3; i++) {
            Bubble b;
            b.position = mouthPos2 + glm::vec3((rand() % 100 - 50) / 500.0f, 0.0f, (rand() % 100 - 50) / 500.0f);
            b.speed = bubbleSpeed + (rand() % 100) / 10000.0f;
            b.active = true;
            bubbles.push_back(b);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) oPressed = false;

    // Update mehurica
    for (auto& b : bubbles) {
        if (b.active) {
            b.position.y += b.speed;
            if (b.position.y > maxY + 0.1f) b.active = false;
        }
    }
    bubbles.erase(std::remove_if(bubbles.begin(), bubbles.end(), [](const Bubble& b) { return !b.active; }), bubbles.end());

    // ENTER - ispusti hranu
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
        enterPressed = true;
        int numFood = 2 + rand() % 3;
        for (int i = 0; i < numFood; i++) {
            Food f;
            f.position = glm::vec3(
                minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX),
                maxY + 0.5f,
                minZ + static_cast<float>(rand()) / RAND_MAX * (maxZ - minZ)
            );
            f.rotation = glm::vec3(
                static_cast<float>(rand()) / RAND_MAX * 360.0f,
                static_cast<float>(rand()) / RAND_MAX * 360.0f,
                static_cast<float>(rand()) / RAND_MAX * 360.0f
            );
            f.fallSpeed = foodFallSpeed + (rand() % 50) / 5000.0f;
            f.active = true;
            f.onGround = false;
            foods.push_back(f);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) enterPressed = false;

    // Update hrane - padanje i kolizija
    for (auto& f : foods) {
        if (f.active) {
            if (!f.onGround) {
                f.position.y -= f.fallSpeed;
                if (f.position.y <= minY) {
                    f.position.y = minY;
                    f.onGround = true;
                }
            }
            if (glm::length(fishPos - f.position) < collisionRadius) {
                f.active = false;
                fish1Scale *= 1.1f;
            }
            if (glm::length(fish2Pos - f.position) < collisionRadius) {
                f.active = false;
                fish2Scale *= 1.1f;
            }
        }
    }

    // Y - otvori/zatvori skrinju
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && !yPressed) {
        yPressed = true;
        chestOpen = !chestOpen;
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE) yPressed = false;

    // N/M - rotacija kamere
    if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) cameraAngle += cameraRotSpeed;
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) cameraAngle -= cameraRotSpeed;

    foods.erase(std::remove_if(foods.begin(), foods.end(), [](const Food& f) { return !f.active; }), foods.end());
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Akvarijum", monitor, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Shaderi
    unsigned int aquariumShader = createShader("basic.vert", "basic.frag");
    Shader modelShader("model.vert", "model.frag");

    // 3D modeli
    Model fishModel("res/12265_Fish_v1_L2.obj");
    Model fish2Model("res/12999_Boesemani_Rainbow_v1_l2.obj");
    Model seaweedModel("res/morska_trava.obj");
    Model beanModel("res/260_Kidney Bean.obj");
    Model chestClosedModel("res/ChestClosed.obj");
    Model chestOpenModel("res/ChestOpen.obj");

    glm::vec3 chestPos(-1.7f, -1.55f, -0.15f);
    float sandLevel = -H / 2 + 0.8f;
    glm::vec3 seaweed1Pos(-1.5f, sandLevel, 1.2f);
    glm::vec3 seaweed2Pos(1.8f, sandLevel, -0.3f);

    // Teksture
    glUseProgram(aquariumShader);
    glUniform1i(glGetUniformLocation(aquariumShader, "uTex"), 0);

    unsigned int sandTexture = loadImageToTexture("res/sand3.jpg");
    glBindTexture(GL_TEXTURE_2D, sandTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // Kocka VAO (za dno, coskove, stakla)
    float vertices[] = {
        0.1f,0.1f,0.1f,   0,0,0,1,   0,0,   0,0,1,
       -0.1f,0.1f,0.1f,   0,0,0,1,   1,0,   0,0,1,
       -0.1f,-0.1f,0.1f,  0,0,0,1,   1,1,   0,0,1,
        0.1f,-0.1f,0.1f,  0,0,0,1,   0,1,   0,0,1,

       -0.1f,0.1f,0.1f,   0,0,0,1,   0,0,  -1,0,0,
       -0.1f,0.1f,-0.1f,  0,0,0,1,   1,0,  -1,0,0,
       -0.1f,-0.1f,-0.1f, 0,0,0,1,   1,1,  -1,0,0,
       -0.1f,-0.1f,0.1f,  0,0,0,1,   0,1,  -1,0,0,

        0.1f,-0.1f,0.1f,  0,0,0,1,   0,0,   0,-1,0,
       -0.1f,-0.1f,0.1f,  0,0,0,1,   1,0,   0,-1,0,
       -0.1f,-0.1f,-0.1f, 0,0,0,1,   1,1,   0,-1,0,
        0.1f,-0.1f,-0.1f, 0,0,0,1,   0,1,   0,-1,0,

        0.1f,0.1f,0.1f,   0,0,0,1,   0,0,   0,1,0,
        0.1f,0.1f,-0.1f,  0,0,0,1,   1,0,   0,1,0,
       -0.1f,0.1f,-0.1f,  0,0,0,1,   1,1,   0,1,0,
       -0.1f,0.1f,0.1f,   0,0,0,1,   0,1,   0,1,0,

        0.1f,0.1f,0.1f,   0,0,0,1,   0,0,   1,0,0,
        0.1f,-0.1f,0.1f,  0,0,0,1,   1,0,   1,0,0,
        0.1f,-0.1f,-0.1f, 0,0,0,1,   1,1,   1,0,0,
        0.1f,0.1f,-0.1f,  0,0,0,1,   0,1,   1,0,0,

        0.1f,0.1f,-0.1f,  0,0,0,1,   0,0,   0,0,-1,
        0.1f,-0.1f,-0.1f, 0,0,0,1,   1,0,   0,0,-1,
       -0.1f,-0.1f,-0.1f, 0,0,0,1,   1,1,   0,0,-1,
       -0.1f,0.1f,-0.1f,  0,0,0,1,   0,1,   0,0,-1
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    int stride = (3 + 4 + 2 + 3) * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Matrice
    float aspect = (float)mode->width / (float)mode->height;
    glm::mat4 proj = glm::perspective(glm::radians(60.f), aspect, 0.1f, 100.f);

    // Dimenzije akvarijuma
    float W = 6, H = 4, D = 3;
    float glass = 0.1f, corner = 0.15f, bottom = 0.2f;

    // Pesak VAO
    const int GRID_X = 20, GRID_Z = 20;
    float amplitude = 0.15f;
    float sandY = -H / 2 + bottom + amplitude;

    std::vector<float> sandVertices;
    float dx = W / GRID_X, dz = D / GRID_Z;

    for (int z = 0; z <= GRID_Z; z++) {
        for (int x = 0; x <= GRID_X; x++) {
            float xpos = -W / 2 + x * dx;
            float zpos = -D / 2 + z * dz;
            float y = sandY + ((rand() % 100) / 100.0f - 0.5f) * amplitude;
            float u = (float)x / GRID_X * 5.0f + ((x * 7 + z * 3) % 8) * 0.015f;
            float v = (float)z / GRID_Z * 5.0f + ((x * 5 + z * 9) % 8) * 0.015f;
            sandVertices.insert(sandVertices.end(), { xpos, y, zpos, 1,1,1,1, u, v, 0,1,0 });
        }
    }

    std::vector<unsigned int> sandIndices;
    for (int z = 0; z < GRID_Z; z++) {
        for (int x = 0; x < GRID_X; x++) {
            int i0 = z * (GRID_X + 1) + x;
            int i1 = i0 + 1;
            int i2 = (z + 1) * (GRID_X + 1) + x;
            int i3 = i2 + 1;
            sandIndices.insert(sandIndices.end(), { (unsigned)i0, (unsigned)i1, (unsigned)i3, (unsigned)i0, (unsigned)i3, (unsigned)i2 });
        }
    }

    unsigned int sandVAO, sandVBO, sandEBO;
    glGenVertexArrays(1, &sandVAO);
    glGenBuffers(1, &sandVBO);
    glGenBuffers(1, &sandEBO);
    glBindVertexArray(sandVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sandVBO);
    glBufferData(GL_ARRAY_BUFFER, sandVertices.size() * sizeof(float), sandVertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sandEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sandIndices.size() * sizeof(unsigned int), sandIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    // Krug VAO (za mehurice)
    std::vector<float> circleVertices;
    circleVertices.insert(circleVertices.end(), { 0.0f, 0.0f, 0.0f, 1, 1, 1, 1, 0.5f, 0.5f, 0, 0, 1 });
    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = 2.0f * 3.14159f * i / CIRCLE_SEGMENTS;
        float x = cos(angle) * 0.1f, y = sin(angle) * 0.1f;
        circleVertices.insert(circleVertices.end(), { x, y, 0.0f, 1, 1, 1, 1, 0.5f + cos(angle) * 0.5f, 0.5f + sin(angle) * 0.5f, 0, 0, 1 });
    }

    unsigned int circleVAO, circleVBO;
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    // Model shader setup
    modelShader.use();
    modelShader.setVec3("uLightColor", glm::vec3(1.0f));
    modelShader.setFloat("uLightIntensity", 1.2f);
    modelShader.setVec3("uLight2Color", glm::vec3(1.0f, 0.85f, 0.3f));
    modelShader.setFloat("uLight2Intensity", 0.0f);
    modelShader.setMat4("uP", proj);

    while (!glfwWindowShouldClose(window))
    {
        // Frame limiter - cekaj do 75 FPS
        float currentTime = (float)glfwGetTime();
        float elapsed = currentTime - lastFrameTime;
        if (elapsed < frameTime) {
            glfwWaitEventsTimeout(frameTime - elapsed);
        }
        lastFrameTime = (float)glfwGetTime();

        processInput(window);

        // Kamera
        float camX = sin(glm::radians(cameraAngle)) * cameraRadius;
        float camZ = cos(glm::radians(cameraAngle)) * cameraRadius;
        glm::vec3 cameraPos(camX, cameraHeight, camZ);
        glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(aquariumShader);
        glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uV"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uP"), 1, GL_FALSE, glm::value_ptr(proj));

        // Dno
        glBindVertexArray(VAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 0);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 0, 0, 0, 1);

        glm::mat4 modelBottom = glm::translate(glm::mat4(1), glm::vec3(0, -H / 2 + bottom / 2, 0));
        modelBottom = glm::scale(modelBottom, glm::vec3(W / 0.2f, bottom / 0.2f, D / 0.2f));
        glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uM"), 1, GL_FALSE, glm::value_ptr(modelBottom));
        for (int i = 0; i < 6; i++) glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);

        // Pesak
        glBindVertexArray(sandVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sandTexture);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 1);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 1.0f, 1.0f, 1.0f, 1.0f);
        glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uM"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glDrawElements(GL_TRIANGLES, (GLsizei)sandIndices.size(), GL_UNSIGNED_INT, 0);

        // Coskovi
        glBindVertexArray(VAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 0, 0, 0, 1);

        glm::vec3 corners[4] = {
            { W / 2 - corner / 2, 0,  D / 2 - corner / 2},
            {-W / 2 + corner / 2, 0,  D / 2 - corner / 2},
            { W / 2 - corner / 2, 0, -D / 2 + corner / 2},
            {-W / 2 + corner / 2, 0, -D / 2 + corner / 2}
        };
        for (int i = 0; i < 4; i++) {
            glm::mat4 m = glm::translate(glm::mat4(1), corners[i]);
            m = glm::scale(m, glm::vec3(corner / 0.2f, H / 0.2f, corner / 0.2f));
            glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uM"), 1, GL_FALSE, glm::value_ptr(m));
            for (int f = 0; f < 6; f++) glDrawArrays(GL_TRIANGLE_FAN, f * 4, 4);
        }

        // 3D Modeli
        modelShader.use();
        modelShader.setMat4("uV", view);
        modelShader.setVec3("uViewPos", cameraPos);
        modelShader.setVec3("uLightPos", cameraPos);

        glm::vec3 treasurePos = chestPos + glm::vec3(0.0f, 0.8f, 0.0f);
        modelShader.setVec3("uLight2Pos", treasurePos);
        modelShader.setFloat("uLight2Intensity", chestOpen ? 2.5f : 0.0f);

        // Riba 1
        modelShader.setVec3("uFallbackColor", 1.0f, 1.0f, 1.0f);
        glm::mat4 fishMatrix = glm::translate(glm::mat4(1.0f), fishPos);
        fishMatrix = glm::rotate(fishMatrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        fishMatrix = glm::rotate(fishMatrix, glm::radians(fishRotationY), glm::vec3(0, 0, 1));
        fishMatrix = glm::scale(fishMatrix, glm::vec3(fish1Scale));
        modelShader.setMat4("uM", fishMatrix);
        fishModel.Draw(modelShader);

        // Riba 2
        glm::mat4 fish2Matrix = glm::translate(glm::mat4(1.0f), fish2Pos);
        fish2Matrix = glm::rotate(fish2Matrix, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        fish2Matrix = glm::rotate(fish2Matrix, glm::radians(fish2RotationY), glm::vec3(0, 0, 1));
        fish2Matrix = glm::scale(fish2Matrix, glm::vec3(fish2Scale));
        modelShader.setMat4("uM", fish2Matrix);
        fish2Model.Draw(modelShader);

        // Morske trave
        modelShader.setVec3("uFallbackColor", 0.2f, 0.6f, 0.3f);
        glm::mat4 seaweed1Matrix = glm::translate(glm::mat4(1.0f), seaweed1Pos);
        modelShader.setMat4("uM", seaweed1Matrix);
        seaweedModel.Draw(modelShader);

        glm::mat4 seaweed2Matrix = glm::translate(glm::mat4(1.0f), seaweed2Pos);
        modelShader.setMat4("uM", seaweed2Matrix);
        seaweedModel.Draw(modelShader);

        // Hrana
        for (const auto& f : foods) {
            if (f.active) {
                glm::mat4 foodMatrix = glm::translate(glm::mat4(1.0f), f.position);
                foodMatrix = glm::rotate(foodMatrix, glm::radians(f.rotation.x), glm::vec3(1, 0, 0));
                foodMatrix = glm::rotate(foodMatrix, glm::radians(f.rotation.y), glm::vec3(0, 1, 0));
                foodMatrix = glm::rotate(foodMatrix, glm::radians(f.rotation.z), glm::vec3(0, 0, 1));
                foodMatrix = glm::scale(foodMatrix, glm::vec3(0.0005f));
                modelShader.setMat4("uM", foodMatrix);
                beanModel.Draw(modelShader);
            }
        }

        // Skrinja
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        modelShader.setVec3("uFallbackColor", 0.16f, 0.08f, 0.06f);
        glm::mat4 chestMatrix = glm::translate(glm::mat4(1.0f), chestPos);
        chestMatrix = glm::rotate(chestMatrix, glm::radians(45.0f), glm::vec3(0, 1, 0));
        chestMatrix = glm::scale(chestMatrix, glm::vec3(1.1f));
        modelShader.setMat4("uM", chestMatrix);
        if (chestOpen) chestOpenModel.Draw(modelShader);
        else chestClosedModel.Draw(modelShader);

        // Mehurici (billboard)
        glUseProgram(aquariumShader);
        glBindVertexArray(circleVAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 1);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 0.7f, 0.9f, 1.0f, 0.5f);

        for (const auto& b : bubbles) {
            if (b.active) {
                glm::mat4 bubbleMatrix = glm::translate(glm::mat4(1.0f), b.position);
                bubbleMatrix[0][0] = view[0][0]; bubbleMatrix[0][1] = view[1][0]; bubbleMatrix[0][2] = view[2][0];
                bubbleMatrix[1][0] = view[0][1]; bubbleMatrix[1][1] = view[1][1]; bubbleMatrix[1][2] = view[2][1];
                bubbleMatrix[2][0] = view[0][2]; bubbleMatrix[2][1] = view[1][2]; bubbleMatrix[2][2] = view[2][2];
                bubbleMatrix = glm::scale(bubbleMatrix, glm::vec3(0.8f));
                glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uM"), 1, GL_FALSE, glm::value_ptr(bubbleMatrix));
                glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
            }
        }

        // Stakla (poslednja zbog transparentnosti)
        glBindVertexArray(VAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 1);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 0.6f, 0.8f, 1.0f, 0.3f);

        glm::mat4 walls[4] = {
            glm::translate(glm::mat4(1), glm::vec3(0, 0,  D / 2 - glass / 2)),
            glm::translate(glm::mat4(1), glm::vec3(0, 0, -D / 2 + glass / 2)),
            glm::translate(glm::mat4(1), glm::vec3(W / 2 - glass / 2, 0, 0)),
            glm::translate(glm::mat4(1), glm::vec3(-W / 2 + glass / 2, 0, 0))
        };
        glm::vec3 wallScale[4] = {
            {W / 0.2f, H / 0.2f, glass / 0.2f},
            {W / 0.2f, H / 0.2f, glass / 0.2f},
            {glass / 0.2f, H / 0.2f, D / 0.2f},
            {glass / 0.2f, H / 0.2f, D / 0.2f}
        };
        for (int i = 0; i < 4; i++) {
            glm::mat4 m = glm::scale(walls[i], wallScale[i]);
            glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uM"), 1, GL_FALSE, glm::value_ptr(m));
            for (int f = 0; f < 6; f++) glDrawArrays(GL_TRIANGLE_FAN, f * 4, 4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
