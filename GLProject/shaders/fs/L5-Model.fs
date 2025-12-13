#version 460 core
out vec4 FragColor;

in vec3 fragPos;
in vec3 fragNormal;

uniform vec3 objectColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec3 color = objectColor;

    // Ambient
    vec3 ambient = 0.2 * color;

    // Diffuse
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightPos - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * color;

    // Specular
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16.0);
    vec3 specular = spec * vec3(1.0);

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
