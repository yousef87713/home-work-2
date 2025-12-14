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

using namespace std;


using namespace glm;

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

bool eclipseMode = false;
float eclipseStartTime = 0.0f;
float eclipseDuration = 3.0f;
bool isSolarEclipse = false;
bool isLunarEclipse = false;
float timeScale = 1.0f;
bool speedUpToSolar = false;
bool speedUpToLunar = false;

float earthSelfRotationSpeed = 0.05f;
float earthOrbitSpeed = 0.5f;
float moonOrbitSpeed = 2.0f;
float moonRotationSpeed = 0.05f;

vec3 sunColor = vec3(1.0f, 0.75f, 0.4f);
vec3 earthColor = vec3(0.2f, 0.4f, 0.8f);
vec3 moonColor = vec3(0.95f, 0.95f, 0.9f);

void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void createOrbit(float radiusX, float radiusZ, unsigned int& VAO, unsigned int& VBO);
float myClamp(float value, float minVal, float maxVal);



float myClamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

void createOrbit(float radiusX, float radiusZ, unsigned int& VAO, unsigned int& VBO) {
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
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(vec3), &points[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System Simulation", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader allShader("./shaders/vs/L5.vs", "./shaders/fs/L5-Model.fs");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Sphere sun(1.5f, 48, 48);
    Sphere earth(0.7f, 32, 32);
    Sphere moon(0.4f, 24, 24);

    createOrbit(12.0f, 8.0f, earthOrbitVAO, earthOrbitVBO);
    createOrbit(2.2f, 2.2f, moonOrbitVAO, moonOrbitVBO);

    float simulatedTime = 0.0f;
    float moonBrightness = 1.0f;
    float earthBrightness = 1.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        simulatedTime += deltaTime * timeScale;
        float time = simulatedTime;

        processInput(window);

        glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mat4 projection = perspective(radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();

        allShader.use();
        allShader.setMat4("projection", projection);
        allShader.setMat4("view", view);
        allShader.setVec3("viewPos", camera.Position);

        mat4 sunModel = mat4(1.0f);
        sunModel = scale(sunModel, vec3(1.5f));
        allShader.setMat4("model", sunModel);
        allShader.setVec3("objectColor", sunColor);
        sun.Draw();

        mat4 orbitModel = mat4(1.0f);
        allShader.setMat4("model", orbitModel);
        allShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));
        glLineWidth(1.5f);
        glBindVertexArray(earthOrbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, 100);
        glBindVertexArray(0);

        float earthX = 12.0f * cos(time * earthOrbitSpeed);
        float earthZ = 8.0f * sin(time * earthOrbitSpeed);
        vec3 earthPos = vec3(earthX, 0.0f, earthZ);

        mat4 earthModel = mat4(1.0f);
        earthModel = translate(earthModel, earthPos);
        earthModel = rotate(earthModel, time * earthSelfRotationSpeed, vec3(0.0f, 1.0f, 0.0f));
        earthModel = rotate(earthModel, radians(23.5f), vec3(0.0f, 0.0f, 1.0f));
        earthModel = scale(earthModel, vec3(0.5f));

        mat4 moonOrbitModel = mat4(1.0f);
        moonOrbitModel = translate(moonOrbitModel, earthPos);
        allShader.setMat4("model", moonOrbitModel);
        allShader.setVec3("objectColor", vec3(0.6f, 0.8f, 1.0f));
        glLineWidth(1.5f);
        glBindVertexArray(moonOrbitVAO);
        glDrawArrays(GL_LINE_LOOP, 0, 100);
        glBindVertexArray(0);
        glLineWidth(1.0f);

        float moonAngle = time * moonOrbitSpeed;
        float moonLocalX = 2.2f * cos(moonAngle);
        float moonLocalZ = 2.2f * sin(moonAngle);
        vec3 moonPos = earthPos + vec3(moonLocalX, 0.0f, moonLocalZ);

        mat4 moonModel = mat4(1.0f);
        moonModel = translate(moonModel, earthPos);
        moonModel = translate(moonModel, vec3(moonLocalX, 0.0f, moonLocalZ));
        moonModel = rotate(moonModel, time * moonRotationSpeed, vec3(0.0f, 1.0f, 0.0f));
        moonModel = scale(moonModel, vec3(0.3f));

        vec3 sunPos = vec3(0.0f, 0.0f, 0.0f);

        vec3 sunToEarth = normalize(earthPos - sunPos);
        vec3 earthToMoon = normalize(moonPos - earthPos);
        vec3 sunToMoon = normalize(moonPos - sunPos);
        vec3 moonToEarth = normalize(earthPos - moonPos);

        float lunarAlignment = dot(sunToEarth, earthToMoon);
        float solarAlignment = dot(sunToMoon, moonToEarth);

        lunarAlignment = myClamp(lunarAlignment, -1.0f, 1.0f);
        solarAlignment = myClamp(solarAlignment, -1.0f, 1.0f);

        float lunarAngle = degrees(acos(lunarAlignment));
        float solarAngle = degrees(acos(solarAlignment));

        if (speedUpToSolar && !eclipseMode) {
            moonBrightness = 0.3f;
            earthBrightness = 1.0f;
        }
        else if (speedUpToLunar && !eclipseMode) {
            earthBrightness = 0.3f;
            moonBrightness = 1.0f;
        }
        else if (eclipseMode) {
            moonBrightness = 1.0f;
            earthBrightness = 1.0f;
        }
        else {
            moonBrightness = 1.0f;
            earthBrightness = 1.0f;
        }

        if (!eclipseMode && speedUpToLunar) {
            if (lunarAngle > 175.0f && lunarAngle < 185.0f) {
                isLunarEclipse = true;
                isSolarEclipse = false;
                eclipseMode = true;
                eclipseStartTime = time;
                timeScale = 0.0f;
                speedUpToLunar = false;
                speedUpToSolar = false;
                moonBrightness = 1.0f;
                earthBrightness = 1.0f;
            }
        }

        if (!eclipseMode && speedUpToSolar) {
            if (solarAngle > 175.0f && solarAngle < 185.0f) {
                isSolarEclipse = true;
                isLunarEclipse = false;
                eclipseMode = true;
                eclipseStartTime = time;
                timeScale = 0.0f;
                speedUpToSolar = false;
                speedUpToLunar = false;
                moonBrightness = 1.0f;
                earthBrightness = 1.0f;
            }
        }

        if (eclipseMode && (time - eclipseStartTime > eclipseDuration)) {
            eclipseMode = false;
            isSolarEclipse = false;
            isLunarEclipse = false;
            timeScale = 1.0f;
            moonBrightness = 1.0f;
            earthBrightness = 1.0f;
        }

        vec3 currentEarthColor = earthColor * earthBrightness;
        vec3 currentMoonColor = moonColor * moonBrightness;

        if (isSolarEclipse && eclipseMode) {
            float eclipseProgress = (time - eclipseStartTime) / eclipseDuration;
            eclipseProgress = myClamp(eclipseProgress, 0.0f, 1.0f);

            currentEarthColor = earthColor * (1.0f - 0.8f * eclipseProgress);

            currentMoonColor = moonColor * moonBrightness;
        }

        else if (isLunarEclipse && eclipseMode) {
            float eclipseProgress = (time - eclipseStartTime) / eclipseDuration;
            eclipseProgress = myClamp(eclipseProgress, 0.0f, 1.0f);
            vec3 bloodMoonColor = vec3(0.5f, 0.2f, 0.1f);
            currentMoonColor = mix(moonColor, bloodMoonColor, eclipseProgress);
            currentEarthColor = earthColor;
        }
        else if (!eclipseMode) {
            vec3 sunToMoonDir = normalize(moonPos - sunPos);
            vec3 moonNormal = normalize(moonPos - earthPos);
            float sunLight = dot(moonNormal, -sunToMoonDir);
            sunLight = myClamp(sunLight, 0.0f, 1.0f);
            vec3 moonDarkSide = vec3(0.3f, 0.3f, 0.35f);

            if (!isSolarEclipse && !isLunarEclipse) {
                currentMoonColor = mix(moonDarkSide, currentMoonColor, sunLight);
            }
        }


        allShader.setVec3("objectColor", currentEarthColor);
        allShader.setMat4("model", earthModel);
        earth.Draw();

        allShader.setVec3("objectColor", currentMoonColor);
        allShader.setMat4("model", moonModel);
        moon.Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &earthOrbitVAO);
    glDeleteBuffers(1, &earthOrbitVBO);
    glDeleteVertexArrays(1, &moonOrbitVAO);
    glDeleteBuffers(1, &moonOrbitVBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
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
        speedUpToSolar = false;
        speedUpToLunar = false;
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}