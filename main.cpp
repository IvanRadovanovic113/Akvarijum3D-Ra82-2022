#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "shader.hpp"
#include "model.hpp"


bool transparent = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_T && action == GLFW_PRESS)
        transparent = !transparent;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

    // ==================== UCITAJ 3D MODEL =====================
    Model fishModel("res/12265_Fish_v1_L2.obj");

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

    // ===================== MODEL SHADER SETUP =====================
    modelShader.use();
    modelShader.setVec3("uLightPos", 0.0f, 3.0f, 3.0f);
    modelShader.setVec3("uViewPos", 0.0f, 2.0f, 6.0f);
    modelShader.setVec3("uLightColor", 1.0f, 1.0f, 1.0f);
    modelShader.setMat4("uP", proj);
    modelShader.setMat4("uV", view);

    // Rotacija modela ribe (posto je model okrenut na stranu)
    float fishRotation = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
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

        // ===================== 3D MODEL (RIBA) =====================
        modelShader.use();

        fishRotation += 0.5f;

        glm::mat4 fishMatrix = glm::mat4(1.0f);
        fishMatrix = glm::translate(fishMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        fishMatrix = glm::rotate(fishMatrix, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // ispravi orijentaciju
        fishMatrix = glm::rotate(fishMatrix, glm::radians(fishRotation), glm::vec3(0.0f, 0.0f, 1.0f)); // rotacija
        fishMatrix = glm::scale(fishMatrix, glm::vec3(0.02f)); // smanji model

        modelShader.setMat4("uM", fishMatrix);
        fishModel.Draw(modelShader);

        // ===================== STAKLA (CRTAJU SE POSLEDNJA ZBOG TRANSPARENTNOSTI) =====================
        glUseProgram(aquariumShader);
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
