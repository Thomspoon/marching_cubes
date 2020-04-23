// #pragma once

// #include <iostream>

// #include "glad/glad.h"

// // Texture2D is a wrapper over the OpenGL Texture2D functionality
// class Texture2D
// {
// public:
//     static Texture2D create(const char* texture_name);
//     static Texture2D create(GLuint width, GLuint height, unsigned char* data);

//     void bind() const;

//     ~Texture2D() 
//     {
//         std::cout << "Texture destroyed!" << std::endl;
//     }

//     // Holds the ID of the texture object, used for all texture operations to reference to this particlar texture
//     GLuint id;

// private:
//     Texture2D(GLuint id);
// };
