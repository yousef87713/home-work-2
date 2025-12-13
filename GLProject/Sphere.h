#pragma once
#include <vector>
#include <glad/glad.h>

class Sphere {
public:
    Sphere(float radius, int sectors = 36, int stacks = 18);

    void Draw();

private:
    GLuint VAO, VBO, EBO;
    unsigned int indexCount;
};
