//
//  Shader.hpp
//  OpenGL-Example
//
//  Created by Xavier Slattery on 11/12/2015.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#ifndef Shader_hpp
#define Shader_hpp

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path);

/* Getting and setting Uniforms. */
GLint getUniformLocation(const GLint programID, const GLchar* name);
void setUniform1f(const GLint programID, const GLchar* name, float value);
void setUniform1fv(const GLint programID, const GLchar* name, float* value, int count);
void setUniform1i(const GLint programID, const GLchar* name, int value);
void setUniform1iv(const GLint programID, const GLchar* name, int* value, int count);
void setUniform2f(const GLint programID, const GLchar* name, const glm::vec2& vector);
void setUniform3f(const GLint programID, const GLchar* name, const glm::vec3& vector);
void setUniform4f(const GLint programID, const GLchar* name, const glm::vec4& vector);
void setUniformMat4(const GLint programID, const GLchar* name, const glm::mat4& matrix);

#endif /* Shader_hpp */
