#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <vector>
#include "Sphere.h"
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "stb_image.h"

using namespace std;
using namespace glm;
float earthSelfRotationSpeed = 0.5f;


int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;

unsigned int earthOrbitVAO = 0, earthOrbitVBO = 0;
unsigned int moonOrbitVAO = 0, moonOrbitVBO = 0;


Camera camera(vec3(0.0f, 0.0f, 4.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

vec3 lightPos(0.0f, -2.5f, 0.0f);

bool eclipseMode = false;
float eclipseStartTime = 0.0f;
float eclipseDuration = 3.0f;
bool isSolarEclipse = false;
bool isLunarEclipse = false;
float timeScale = 1.0f;
bool speedUpToSolar = false;
bool speedUpToLunar = false;


float myClamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void createOrbit(float radiusX, float radiusZ,
    unsigned int& VAO, unsigned int& VBO)
{
    vector<vec3> points;
    const int segments = 100;

    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * glm::pi<float>() * i / segments;
        float x = radiusX * cos(angle);
        float z = radiusZ * sin(angle);
        points.push_back(vec3(x, 0.0f, z));
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        points.size() * sizeof(vec3),
        &points[0],
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}


int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Home Work 2", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    Shader allShader("./shaders/vs/L5.vs", "./shaders/fs/L5-Model.fs");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Sphere sun(1.5f, 48, 48);
    Sphere earth(0.7f, 32, 32);
    Sphere moon(0.4f, 24, 24);

    createOrbit(8.0f, 4.0f, earthOrbitVAO, earthOrbitVBO);

    createOrbit(2.2f, 2.2f, moonOrbitVAO, moonOrbitVBO);

    while (!glfwWindowShouldClose(window))
    {
        allShader.use();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        static float simulatedTime = 0.0f;
        simulatedTime += deltaTime * timeScale;
        float time = simulatedTime;

        processInput(window);

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection = perspective(radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);

        allShader.setMat4("projection", projection);
        allShader.setMat4("view", camera.GetViewMatrix());
        allShader.setVec3("viewPos", camera.Position);

        mat4 sunModel = mat4(1.0f);
        sunModel = scale(sunModel, vec3(1.5f));
        allShader.setMat4("model", sunModel);
        allShader.setVec3("lightPos", vec3(0.0f));

        vec3 sunColor = vec3(1.0f, 0.9f, 0.5f);
        allShader.setVec3("objectColor", sunColor);

        sun.Draw();



        mat4 orbitModel = mat4(1.0f);
        allShader.setMat4("model", orbitModel);
        allShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));
        glLineWidth(2.0f);

        glBindVertexArray(earthOrbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, 100);
        glBindVertexArray(0);
        glLineWidth(1.0f);


        float earthOrbitSpeed = 0.5f;
        float earthRotationSpeed = 0.05f;

        float orbitTime = eclipseMode ? eclipseStartTime : time;
        float x = 8.0f * cos(orbitTime * earthOrbitSpeed);
        float z = 4.0f * sin(orbitTime * earthOrbitSpeed);

        mat4 earthModel = mat4(1.0f);
        earthModel = translate(earthModel, vec3(x, 0, z));
        earthModel = rotate(earthModel, time * earthRotationSpeed, vec3(0, 1, 0));
        earthModel = scale(earthModel, vec3(0.5f));
        earthModel = rotate(earthModel, radians(23.5f), vec3(0, 0, 1));

        vec3 earthColor = vec3(0.0f, 0.5f, 0.5f);


        mat4 moonOrbitModel = mat4(1.0f);
        moonOrbitModel = translate(moonOrbitModel, vec3(x, 0, z));
        allShader.setMat4("model", moonOrbitModel);
        allShader.setVec3("objectColor", vec3(0.6f, 0.8f, 1.0f));
        glLineWidth(2.0f);
        glBindVertexArray(moonOrbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, 100);
        glBindVertexArray(0);
        glLineWidth(1.0f);


        float moonOrbitSpeed = 2.0f;
        float moonRotationSpeed = 0.05f;

        float moonAngle = orbitTime * moonOrbitSpeed;
        float moonX = 2.2f * cos(moonAngle);
        float moonZ = 2.2f * sin(moonAngle);

        mat4 moonModel = mat4(1.0f);
        allShader.setBool("moonInShadow", (isSolarEclipse && eclipseMode));
        moonModel = translate(moonModel, vec3(x, 0, z));
        moonModel = translate(moonModel, vec3(moonX, 0, moonZ));
        moonModel = rotate(moonModel, time * moonRotationSpeed, vec3(0, 1, 0));
        moonModel = scale(moonModel, vec3(0.3f));

        vec3 sunPos = vec3(0.0f);
        vec3 earthPos = vec3(x, 0, z);
        vec3 moonPos = vec3(moonModel[3]);

        vec3 sunToEarthDir = normalize(earthPos - sunPos);
        vec3 earthToMoonDir = normalize(moonPos - earthPos);

        vec3 sunToMoonDir = normalize(moonPos - sunPos);
        vec3 moonToEarthDir = normalize(earthPos - moonPos);

        float lunarDotProduct = dot(sunToEarthDir, earthToMoonDir);
        float solarDotProduct = dot(sunToMoonDir, moonToEarthDir);

        if (lunarDotProduct > 1.0f) lunarDotProduct = 1.0f;
        if (lunarDotProduct < -1.0f) lunarDotProduct = -1.0f;
        if (solarDotProduct > 1.0f) solarDotProduct = 1.0f;
        if (solarDotProduct < -1.0f) solarDotProduct = -1.0f;

        float lunarAlignmentAngle = degrees(acos(lunarDotProduct));
        float solarAlignmentAngle = degrees(acos(solarDotProduct));

        if (lunarAlignmentAngle > 175.0f && lunarAlignmentAngle < 185.0f
            && speedUpToLunar && !eclipseMode)
        {
            isLunarEclipse = true;
            isSolarEclipse = false;

            eclipseMode = true;
            eclipseStartTime = time;
            timeScale = 0.0f; 
        }

        else if (solarAlignmentAngle > 175.0f && solarAlignmentAngle < 185.0f
            && speedUpToSolar && !eclipseMode)
        {
            isSolarEclipse = true;
            isLunarEclipse = false;

            eclipseMode = true;
            eclipseStartTime = time;
            timeScale = 0.0f; 
        }

        else {
            if (eclipseMode && (time - eclipseStartTime > eclipseDuration)) {
                eclipseMode = false;
                isSolarEclipse = false;
                isLunarEclipse = false;
            }
        }

        vec3 moonColor;

        if (isSolarEclipse && eclipseMode) {
            moonColor = vec3(0.05f, 0.05f, 0.05f);
        }
        else if (isLunarEclipse && eclipseMode) {
            float eclipseProgress = (time - eclipseStartTime) / eclipseDuration;
            float eclipseIntensity = myClamp(eclipseProgress, 0.0f, 1.0f);

            vec3 bloodMoonColor = vec3(0.4f, 0.1f, 0.05f);
            moonColor = mix(vec3(0.95f, 0.95f, 0.9f), bloodMoonColor, eclipseIntensity);
        }
        else {
            vec3 sunToMoonDir = normalize(moonPos - sunPos);
            vec3 moonNormal = normalize(moonPos - earthPos);
            float sunLightDot = dot(moonNormal, -sunToMoonDir);
            sunLightDot = myClamp(sunLightDot, 0.0f, 1.0f);

            vec3 moonDarkSide = vec3(0.3f, 0.3f, 0.35f);
            vec3 moonLightSide = vec3(0.95f, 0.95f, 0.9f);
            moonColor = mix(moonDarkSide, moonLightSide, sunLightDot);
        }

        allShader.setVec3("objectColor", earthColor);
        allShader.setMat4("model", earthModel);
        earth.Draw();

        allShader.setVec3("objectColor", moonColor);
        allShader.setMat4("model", moonModel);
        moon.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        eclipseMode = false;
        isSolarEclipse = false;
        isLunarEclipse = false;

        timeScale = 1.0f;
        speedUpToSolar = false;
        speedUpToLunar = false;
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        timeScale = 3.0f;
        speedUpToSolar = true;
        speedUpToLunar = false;
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        timeScale = 3.0f;
        speedUpToLunar = true;
        speedUpToSolar = false;
    }

    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        timeScale = 2.0f;
        eclipseMode = false;
        speedUpToSolar = false;
        speedUpToLunar = false;
    }


}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}