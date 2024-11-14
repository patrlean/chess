#ifndef LIGHT_HPP
#define LIGHT_HPP

struct Light {
    glm::vec3 position; // light position
    glm::vec3 color;    // light color
    float power;        // light intensity
    bool isEnabled;     // whether light is enabled
};

#endif