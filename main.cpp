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

// Pozicija ribe 1 i rotacija (WASD + QE)
glm::vec3 fishPos(0.0f, 0.0f, 0.0f);
float fishSpeed = 0.01f;
float fishRotationY = 0.0f;

// Pozicija ribe 2 i rotacija (strelice + KL)
glm::vec3 fish2Pos(1.0f, 0.0f, 0.0f);  // pocetna pozicija malo udesno
float fish2RotationY = 0.0f;

// Granice akvarijuma (sa marginom za stakla i ribu)
const float W = 6.0f, H = 4.0f, D = 3.0f;
const float margin = 0.3f;  // margina od zidova
const float minX = -W / 2 + margin;
const float maxX = W / 2 - margin;
const float minY = -H / 2 + 0.4f;  // malo iznad peska
const float maxY = H / 2 - margin/2;
const float minZ = -D / 2 + margin;
const float maxZ = D / 2 - margin;

// Struktura za mehurice
struct Bubble {
    glm::vec3 position;
    float speed;
    bool active;
};

std::vector<Bubble> bubbles;
float bubbleSpeed = 0.005f;  // sporije kretanje
bool zPressed = false;
bool oPressed = false;
const int CIRCLE_SEGMENTS = 32;

// Struktura za gusenice
struct Food {
    glm::vec3 position;
    glm::vec3 rotation;  // random rotacija po sve 3 ose
    float fallSpeed;
    bool active;
    bool onGround;  // da li je na pesku
};

std::vector<Food> foods;
float foodFallSpeed = 0.005f;  // sporije padanje
bool enterPressed = false;
float fish1Scale = 0.02f;  // pocetna velicina ribe 1
float fish2Scale = 0.05f;  // pocetna velicina ribe 2
const float collisionRadius = 0.3f;  // radijus za detekciju sudara

// Skrinja (chest)
bool chestOpen = false;  // da li je skrinja otvorena
bool yPressed = false;


void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
        transparent = !transparent;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void processInput(GLFWwindow* window)
{
    // WASD - kretanje levo/desno/napred/nazad
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        fishPos.z -= fishSpeed;
        fishRotationY = -90.0f;   // gleda napred (od kamere)
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        fishPos.z += fishSpeed;
        fishRotationY = 90.0f;  // gleda nazad (ka kameri)
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        fishPos.x -= fishSpeed;
        fishRotationY = 0.0f;    // gleda levo
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        fishPos.x += fishSpeed;
        fishRotationY = 180.0f;  // gleda desno
    }

    // QE - kretanje gore/dole (bez promene rotacije)
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        fishPos.y += fishSpeed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        fishPos.y -= fishSpeed;

    // Ogranici poziciju ribe 1 unutar akvarijuma
    if (fishPos.x < minX) fishPos.x = minX;
    if (fishPos.x > maxX) fishPos.x = maxX;
    if (fishPos.y < minY) fishPos.y = minY;
    if (fishPos.y > maxY) fishPos.y = maxY;
    if (fishPos.z < minZ) fishPos.z = minZ;
    if (fishPos.z > maxZ) fishPos.z = maxZ;

    // ===== RIBA 2 - strelice + KL =====
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

    // KL - kretanje gore/dole
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        fish2Pos.y += fishSpeed;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        fish2Pos.y -= fishSpeed;

    // Ogranici poziciju ribe 2 unutar akvarijuma
    if (fish2Pos.x < minX) fish2Pos.x = minX;
    if (fish2Pos.x > maxX) fish2Pos.x = maxX;
    if (fish2Pos.y < minY) fish2Pos.y = minY;
    if (fish2Pos.y > maxY) fish2Pos.y = maxY;
    if (fish2Pos.z < minZ) fish2Pos.z = minZ;
    if (fish2Pos.z > maxZ) fish2Pos.z = maxZ;

    // Z - riba 1 ispusta mehurice
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS && !zPressed) {
        zPressed = true;
        // Izracunaj poziciju usta na osnovu rotacije i velicine
        float mouthOffset = fish1Scale * 10.0f;  // udaljenost usta od centra
        glm::vec3 mouthDir(0.0f);
        if (fishRotationY == 0.0f) mouthDir = glm::vec3(-1, 0, 0);       // levo
        else if (fishRotationY == 180.0f) mouthDir = glm::vec3(1, 0, 0); // desno
        else if (fishRotationY == -90.0f) mouthDir = glm::vec3(0, 0, -1); // napred
        else if (fishRotationY == 90.0f) mouthDir = glm::vec3(0, 0, 1);  // nazad

        glm::vec3 mouthPos = fishPos + mouthDir * mouthOffset + glm::vec3(0, 0.05f, 0);

        for (int i = 0; i < 3; i++) {
            Bubble b;
            b.position = mouthPos + glm::vec3(
                (rand() % 100 - 50) / 500.0f,
                0.0f,
                (rand() % 100 - 50) / 500.0f
            );
            b.speed = bubbleSpeed + (rand() % 100) / 10000.0f;
            b.active = true;
            bubbles.push_back(b);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_RELEASE)
        zPressed = false;

    // O - riba 2 ispusta mehurice
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !oPressed) {
        oPressed = true;
        // Izracunaj poziciju usta na osnovu rotacije i velicine
        float mouthOffset2 = fish2Scale * 50.0f;  // udaljenost usta od centra
        glm::vec3 mouthDir2(0.0f);
        if (fish2RotationY == 0.0f) mouthDir2 = glm::vec3(-1, 0, 0);       // levo
        else if (fish2RotationY == 180.0f) mouthDir2 = glm::vec3(1, 0, 0); // desno
        else if (fish2RotationY == -90.0f) mouthDir2 = glm::vec3(0, 0, -1); // napred
        else if (fish2RotationY == 90.0f) mouthDir2 = glm::vec3(0, 0, 1);  // nazad

        glm::vec3 mouthPos2 = fish2Pos + mouthDir2 * mouthOffset2 + glm::vec3(0, 0.05f, 0);

        for (int i = 0; i < 3; i++) {
            Bubble b;
            b.position = mouthPos2 + glm::vec3(
                (rand() % 100 - 50) / 500.0f,
                0.0f,
                (rand() % 100 - 50) / 500.0f
            );
            b.speed = bubbleSpeed + (rand() % 100) / 10000.0f;
            b.active = true;
            bubbles.push_back(b);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE)
        oPressed = false;

    // Update pozicija mehurica
    for (auto& b : bubbles) {
        if (b.active) {
            b.position.y += b.speed;
            // Deaktiviraj mehur kada izadje iz vode
            if (b.position.y > maxY + 0.1f)
                b.active = false;
        }
    }

    // Ukloni neaktivne mehurice
    bubbles.erase(
        std::remove_if(bubbles.begin(), bubbles.end(),
            [](const Bubble& b) { return !b.active; }),
        bubbles.end()
    );

    // ENTER - ispusti hranu (beans)
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS && !enterPressed) {
        enterPressed = true;
        int numFood = 2 + rand() % 3;  // 2-4 komada hrane
        for (int i = 0; i < numFood; i++) {
            Food f;
            f.position = glm::vec3(
                minX + static_cast<float>(rand()) / RAND_MAX * (maxX - minX),
                maxY + 0.5f,  // pocni iznad vode
                minZ + static_cast<float>(rand()) / RAND_MAX * (maxZ - minZ)
            );
            f.rotation = glm::vec3(
                static_cast<float>(rand()) / RAND_MAX * 360.0f,  // random X rotacija
                static_cast<float>(rand()) / RAND_MAX * 360.0f,  // random Y rotacija
                static_cast<float>(rand()) / RAND_MAX * 360.0f   // random Z rotacija
            );
            f.fallSpeed = foodFallSpeed + (rand() % 50) / 5000.0f;
            f.active = true;
            f.onGround = false;
            foods.push_back(f);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE)
        enterPressed = false;

    // Update hrane - padanje i detekcija sudara
    for (auto& f : foods) {
        if (f.active) {
            // Padanje ako nije na zemlji
            if (!f.onGround) {
                f.position.y -= f.fallSpeed;
                // Proveri da li je dosla do peska
                if (f.position.y <= minY) {
                    f.position.y = minY;
                    f.onGround = true;
                }
            }

            // Detekcija sudara sa ribom 1
            float dist1 = glm::length(fishPos - f.position);
            if (dist1 < collisionRadius) {
                f.active = false;
                fish1Scale *= 1.1f;  // uvecaj ribu za 10%
            }

            // Detekcija sudara sa ribom 2
            float dist2 = glm::length(fish2Pos - f.position);
            if (dist2 < collisionRadius) {
                f.active = false;
                fish2Scale *= 1.1f;  // uvecaj ribu za 10%
            }
        }
    }

    // Y - otvori/zatvori skrinju
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS && !yPressed) {
        yPressed = true;
        chestOpen = !chestOpen;  // toggle
    }
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_RELEASE)
        yPressed = false;

    // Ukloni pojedenu hranu
    foods.erase(
        std::remove_if(foods.begin(), foods.end(),
            [](const Food& f) { return !f.active; }),
        foods.end()
    );
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    GLFWwindow* window = glfwCreateWindow(
        mode->width,
        mode->height,
        "Akvarijum",
        monitor,    // fullscreen
        nullptr
    );
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    glewInit();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // ==================== SHADERI =====================
    // Shader za akvarijum (dno, pesak, stakla, coskovi)
    unsigned int aquariumShader = createShader("basic.vert", "basic.frag");

    // Shader za 3D modele (ribe itd)
    Shader modelShader("model.vert", "model.frag");

    // ==================== UCITAJ 3D MODELE =====================
    Model fishModel("res/12265_Fish_v1_L2.obj");
    Model fish2Model("res/12999_Boesemani_Rainbow_v1_l2.obj");
    Model seaweedModel("res/morska_trava.obj");
    Model beanModel("res/260_Kidney Bean.obj");
    Model chestClosedModel("res/ChestClosed.obj");
    Model chestOpenModel("res/ChestOpen.obj");

    std::cout << "Chest closed meshes: " << chestClosedModel.meshes.size() << std::endl;
    std::cout << "Chest open meshes: " << chestOpenModel.meshes.size() << std::endl;

    // Pozicija skrinje - na centru da vidimo da li radi
    glm::vec3 chestPos(-1.7f, -1.55f, -0.5f);

    // Fiksne pozicije za dve morske trave na pesku
    float sandLevel = -H / 2 + 1.0f;  // podignut nivo iznad peska
    glm::vec3 seaweed1Pos(-1.5f, sandLevel, 1.2f);
    glm::vec3 seaweed2Pos(1.8f, sandLevel, -0.3f);

    // ==================== TEKSTURE =====================
    glUseProgram(aquariumShader);
    glUniform1i(glGetUniformLocation(aquariumShader, "uTex"), 0);

    unsigned int sandTexture = loadImageToTexture("res/sand3.jpg");

    if (sandTexture == 0)
        std::cout << "GRESKA: sand3.jpg nije ucitana!\n";
    else
        std::cout << "sand3.jpg ucitana, ID = " << sandTexture << std::endl;

    glBindTexture(GL_TEXTURE_2D, sandTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    // ===================== KOCKA =====================
    float vertices[] = {
        // pozicija         boja        uv      normala
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

    // ===================== MATRICE =====================
    glm::mat4 view = glm::lookAt(
        glm::vec3(0, 2, 6),
        glm::vec3(0, 0, 0),
        glm::vec3(0, 1, 0)
    );
    float aspect = (float)mode->width / (float)mode->height;

    glm::mat4 proj = glm::perspective(
        glm::radians(60.f),
        aspect,
        0.1f,
        100.f
    );

    // ===================== DIMENZIJE =====================
    float W = 6, H = 4, D = 3;
    float glass = 0.1f, corner = 0.15f, bottom = 0.2f;

    // ===================== PESAK - VERTEX GRID =====================
    const int GRID_X = 20;
    const int GRID_Z = 20;

    float amplitude = 0.15f;
    float sandY = -H / 2 + bottom + amplitude;

    std::vector<float> sandVertices;

    float dx = W / GRID_X;
    float dz = D / GRID_Z;

    for (int z = 0; z <= GRID_Z; z++)
    {
        for (int x = 0; x <= GRID_X; x++)
        {
            float xpos = -W / 2 + x * dx;
            float zpos = -D / 2 + z * dz;

            float y = sandY + ((rand() % 100) / 100.0f - 0.5f) * amplitude;

            float baseU = (float)x / GRID_X;
            float baseV = (float)z / GRID_Z;

            float offsetU = ((x * 7 + z * 3) % 8) * 0.015f;
            float offsetV = ((x * 5 + z * 9) % 8) * 0.015f;

            float u = baseU * 5.0f + offsetU;
            float v = baseV * 5.0f + offsetV;

            sandVertices.insert(sandVertices.end(), {
                xpos, y, zpos,     // position
                1,1,1,1,           // color
                u, v,              // UV
                0,1,0              // normal
                });
        }
    }

    std::vector<unsigned int> sandIndices;

    for (int z = 0; z < GRID_Z; z++)
    {
        for (int x = 0; x < GRID_X; x++)
        {
            int i0 = z * (GRID_X + 1) + x;
            int i1 = i0 + 1;
            int i2 = (z + 1) * (GRID_X + 1) + x;
            int i3 = i2 + 1;

            sandIndices.push_back(i0);
            sandIndices.push_back(i1);
            sandIndices.push_back(i3);

            sandIndices.push_back(i0);
            sandIndices.push_back(i3);
            sandIndices.push_back(i2);
        }
    }

    unsigned int sandVAO, sandVBO, sandEBO;

    glGenVertexArrays(1, &sandVAO);
    glGenBuffers(1, &sandVBO);
    glGenBuffers(1, &sandEBO);

    glBindVertexArray(sandVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sandVBO);
    glBufferData(GL_ARRAY_BUFFER,
        sandVertices.size() * sizeof(float),
        sandVertices.data(),
        GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sandEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sandIndices.size() * sizeof(unsigned int),
        sandIndices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    // ===================== KRUG ZA MEHURICE (BILLBOARD) =====================
    std::vector<float> circleVertices;

    // Centar kruga
    circleVertices.insert(circleVertices.end(), {
        0.0f, 0.0f, 0.0f,    // position
        1, 1, 1, 1,          // color
        0.5f, 0.5f,          // UV
        0, 0, 1              // normal (ka kameri)
    });

    // Tacke na obodu kruga
    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        float angle = 2.0f * 3.14159f * i / CIRCLE_SEGMENTS;
        float x = cos(angle) * 0.1f;
        float y = sin(angle) * 0.1f;
        circleVertices.insert(circleVertices.end(), {
            x, y, 0.0f,          // position
            1, 1, 1, 1,          // color
            0.5f + cos(angle) * 0.5f, 0.5f + sin(angle) * 0.5f,  // UV
            0, 0, 1              // normal
        });
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

    // ===================== MODEL SHADER SETUP =====================
    modelShader.use();
    modelShader.setVec3("uLightPos", 0.0f, 3.0f, 3.0f);
    modelShader.setVec3("uViewPos", 0.0f, 2.0f, 6.0f);
    modelShader.setVec3("uLightColor", 1.0f, 1.0f, 1.0f);
    modelShader.setMat4("uP", proj);
    modelShader.setMat4("uV", view);

    while (!glfwWindowShouldClose(window))
    {
        // Obradi input za kretanje ribe
        processInput(window);

        glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // ===================== AKVARIJUM SHADER =====================
        glUseProgram(aquariumShader);
        glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uV"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(aquariumShader, "uP"), 1, GL_FALSE, glm::value_ptr(proj));

        // ===================== KOCKA VAO (DNO) =====================
        glBindVertexArray(VAO);

        // ===================== DNO =====================
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 0);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 0, 0, 0, 1);

        glm::mat4 modelBottom = glm::translate(glm::mat4(1),
            glm::vec3(0, -H / 2 + bottom / 2, 0));
        modelBottom = glm::scale(modelBottom,
            glm::vec3(W / 0.2f, bottom / 0.2f, D / 0.2f));

        glUniformMatrix4fv(
            glGetUniformLocation(aquariumShader, "uM"),
            1, GL_FALSE, glm::value_ptr(modelBottom)
        );

        for (int i = 0; i < 6; i++)
            glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);

        // ===================== PESAK (SAND VAO) =====================
        glBindVertexArray(sandVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sandTexture);

        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 1);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 0);

        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"),
            1.0f, 1.0f, 1.0f, 1.0f);

        glm::mat4 sandModel = glm::mat4(1.0f);
        glUniformMatrix4fv(
            glGetUniformLocation(aquariumShader, "uM"),
            1, GL_FALSE, glm::value_ptr(sandModel)
        );

        glDrawElements(
            GL_TRIANGLES,
            (GLsizei)sandIndices.size(),
            GL_UNSIGNED_INT,
            0
        );

        // ===================== COSKOVI =====================
        glBindVertexArray(VAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 0);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"), 0, 0, 0, 1);

        glm::vec3 corners[4] = {
            { W / 2 - corner / 2, 0,  D / 2 - corner / 2},
            {-W / 2 + corner / 2, 0,  D / 2 - corner / 2},
            { W / 2 - corner / 2, 0, -D / 2 + corner / 2},
            {-W / 2 + corner / 2, 0, -D / 2 + corner / 2}
        };

        for (int i = 0; i < 4; i++)
        {
            glm::mat4 m = glm::translate(glm::mat4(1), corners[i]);
            m = glm::scale(m,
                glm::vec3(corner / 0.2f, H / 0.2f, corner / 0.2f));

            glUniformMatrix4fv(
                glGetUniformLocation(aquariumShader, "uM"),
                1, GL_FALSE, glm::value_ptr(m)
            );

            for (int f = 0; f < 6; f++)
                glDrawArrays(GL_TRIANGLE_FAN, f * 4, 4);
        }

        // ===================== 3D MODEL (RIBA 1 - WASD) =====================
        modelShader.use();
        modelShader.setVec3("uFallbackColor", 1.0f, 1.0f, 1.0f);  // bela ako nema teksture

        glm::mat4 fishMatrix = glm::mat4(1.0f);
        fishMatrix = glm::translate(fishMatrix, fishPos);
        fishMatrix = glm::rotate(fishMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // ispravi orijentaciju modela
        fishMatrix = glm::rotate(fishMatrix, glm::radians(fishRotationY), glm::vec3(0.0f, 0.0f, 1.0f)); // rotacija u pravcu kretanja
        fishMatrix = glm::scale(fishMatrix, glm::vec3(fish1Scale)); // velicina raste kad jede

        modelShader.setMat4("uM", fishMatrix);
        fishModel.Draw(modelShader);

        // ===================== 3D MODEL (RIBA 2 - strelice) =====================
        glm::mat4 fish2Matrix = glm::mat4(1.0f);
        fish2Matrix = glm::translate(fish2Matrix, fish2Pos);
        fish2Matrix = glm::rotate(fish2Matrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // ispravi orijentaciju modela
        fish2Matrix = glm::rotate(fish2Matrix, glm::radians(fish2RotationY), glm::vec3(0.0f, 0.0f, 1.0f)); // rotacija u pravcu kretanja
        fish2Matrix = glm::scale(fish2Matrix, glm::vec3(fish2Scale)); // velicina raste kad jede

        modelShader.setMat4("uM", fish2Matrix);
        fish2Model.Draw(modelShader);

        // ===================== MORSKE TRAVE =====================
        modelShader.setVec3("uFallbackColor", 0.2f, 0.6f, 0.3f);  // zelena boja za travu

        glm::mat4 seaweed1Matrix = glm::mat4(1.0f);
        seaweed1Matrix = glm::translate(seaweed1Matrix, seaweed1Pos);
        seaweed1Matrix = glm::scale(seaweed1Matrix, glm::vec3(1.0f));

        modelShader.setMat4("uM", seaweed1Matrix);
        seaweedModel.Draw(modelShader);

        glm::mat4 seaweed2Matrix = glm::mat4(1.0f);
        seaweed2Matrix = glm::translate(seaweed2Matrix, seaweed2Pos);
        seaweed2Matrix = glm::scale(seaweed2Matrix, glm::vec3(1.0f));

        modelShader.setMat4("uM", seaweed2Matrix);
        seaweedModel.Draw(modelShader);

        // ===================== HRANA (beans) =====================
        for (const auto& f : foods) {
            if (f.active) {
                glm::mat4 foodMatrix = glm::mat4(1.0f);
                foodMatrix = glm::translate(foodMatrix, f.position);
                // Random rotacija po sve 3 ose
                foodMatrix = glm::rotate(foodMatrix, glm::radians(f.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
                foodMatrix = glm::rotate(foodMatrix, glm::radians(f.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
                foodMatrix = glm::rotate(foodMatrix, glm::radians(f.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
                foodMatrix = glm::scale(foodMatrix, glm::vec3(0.0005f));  // velicina bean-a

                modelShader.setMat4("uM", foodMatrix);
                beanModel.Draw(modelShader);
            }
        }

        // ===================== SKRINJA (CHEST) =====================
        // Resetuj teksturu da ne koristi bean teksturu
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        modelShader.setVec3("uFallbackColor", 0.16f, 0.08f, 0.06f);  // tamno braon boja drveta iz MTL
        glm::mat4 chestMatrix = glm::mat4(1.0f);
        chestMatrix = glm::translate(chestMatrix, chestPos);
        chestMatrix = glm::rotate(chestMatrix, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // rotacija 45 stepeni udesno
        chestMatrix = glm::scale(chestMatrix, glm::vec3(1.1f));  // probaj razlicite velicine

        modelShader.setMat4("uM", chestMatrix);
        if (chestOpen) {
            chestOpenModel.Draw(modelShader);
        } else {
            chestClosedModel.Draw(modelShader);
        }

        // ===================== MEHURICI (BILLBOARD KRUGOVI) =====================
        glUseProgram(aquariumShader);
        glBindVertexArray(circleVAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 1);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"),
            0.7f, 0.9f, 1.0f, 0.5f);  // svetlo plava providna

        for (const auto& b : bubbles) {
            if (b.active) {
                glm::mat4 bubbleMatrix = glm::mat4(1.0f);
                bubbleMatrix = glm::translate(bubbleMatrix, b.position);
                // Billboard - uzmi samo rotaciju iz view matrice i invertiraj
                bubbleMatrix[0][0] = view[0][0]; bubbleMatrix[0][1] = view[1][0]; bubbleMatrix[0][2] = view[2][0];
                bubbleMatrix[1][0] = view[0][1]; bubbleMatrix[1][1] = view[1][1]; bubbleMatrix[1][2] = view[2][1];
                bubbleMatrix[2][0] = view[0][2]; bubbleMatrix[2][1] = view[1][2]; bubbleMatrix[2][2] = view[2][2];
                bubbleMatrix = glm::scale(bubbleMatrix, glm::vec3(0.8f));  // velicina mehura

                glUniformMatrix4fv(
                    glGetUniformLocation(aquariumShader, "uM"),
                    1, GL_FALSE, glm::value_ptr(bubbleMatrix)
                );

                glDrawArrays(GL_TRIANGLE_FAN, 0, CIRCLE_SEGMENTS + 2);
            }
        }

        // ===================== STAKLA (CRTAJU SE POSLEDNJA ZBOG TRANSPARENTNOSTI) =====================
        glBindVertexArray(VAO);
        glUniform1i(glGetUniformLocation(aquariumShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(aquariumShader, "transparent"), 1);
        glUniform4f(glGetUniformLocation(aquariumShader, "uColor"),
            0.6f, 0.8f, 1.0f, 0.3f);

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

        for (int i = 0; i < 4; i++)
        {
            glm::mat4 m = glm::scale(walls[i], wallScale[i]);

            glUniformMatrix4fv(
                glGetUniformLocation(aquariumShader, "uM"),
                1, GL_FALSE, glm::value_ptr(m)
            );

            for (int f = 0; f < 6; f++)
                glDrawArrays(GL_TRIANGLE_FAN, f * 4, 4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
