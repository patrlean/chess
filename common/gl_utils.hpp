#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <GL/glew.h>  
void checkOpenGLError(const char* stmt, const char* fname, int line);

#define GL_CHECK(stmt) do { \
            stmt; \
            checkOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)

#endif // GL_UTILS_H