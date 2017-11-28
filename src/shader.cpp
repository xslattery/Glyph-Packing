//
//  Shader.cpp
//  OpenGL-Example
//
//  Created by Xavier Slattery on 11/12/2015.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#include <GLUT/GLUT.h>
#include <glm/glm.hpp>

#include "shader.hpp"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){
    
    // Create the shaders
    GLuint VertexShaderID = glCreateShader( GL_VERTEX_SHADER );
    GLuint FragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );
    
    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream( vertex_file_path, std::ios::in );
    if( VertexShaderStream.is_open() )
    {
        std::string Line = "";
        while( getline( VertexShaderStream, Line ) )
            VertexShaderCode += "\n" + Line;
        VertexShaderStream.close();
    }
    else
    {
        std::cout << "Impossible to open. Are you in the right directory ? : " << std::string( vertex_file_path ) << "\n";
        return 0;
    }
    
    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream( fragment_file_path, std::ios::in );
    if( FragmentShaderStream.is_open() ){
        std::string Line = "";
        while( getline(FragmentShaderStream, Line) )
            FragmentShaderCode += "\n" + Line;
        FragmentShaderStream.close();
    }
    
    GLint Result = GL_FALSE;
    int InfoLogLength;
    
    
    // Compile Vertex Shader
//    std::cout << "Compiling shader : " + std::string( vertex_file_path ) << "\n";
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource( VertexShaderID, 1, &VertexSourcePointer , NULL );
    glCompileShader( VertexShaderID );
    
    // Check Vertex Shader
    glGetShaderiv( VertexShaderID, GL_COMPILE_STATUS, &Result );
    glGetShaderiv( VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength );
    if ( InfoLogLength > 0 )
    {
        std::vector<char> VertexShaderErrorMessage( InfoLogLength+1 );
        glGetShaderInfoLog( VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0] );
        std::cout << &VertexShaderErrorMessage[0] << "\n";
    }
    
    // Compile Fragment Shader
//    std::cout << "Compiling shader : "+ std::string(fragment_file_path) << "\n";
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    
    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if ( InfoLogLength > 0 ){
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        std::cout << &FragmentShaderErrorMessage[0] << "\n";
    }
    
    // Link the program
//    std::cout << "Linking program\n";
    GLuint ProgramID = glCreateProgram();
    
    glAttachShader( ProgramID, VertexShaderID );
    glAttachShader( ProgramID, FragmentShaderID );
    glLinkProgram( ProgramID );
    
    // Check the program
    glGetProgramiv( ProgramID, GL_LINK_STATUS, &Result );
    glGetProgramiv( ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength );
    if ( InfoLogLength > 0 )
    {
        std::vector<char> ProgramErrorMessage( InfoLogLength+1 );
        glGetProgramInfoLog( ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0] );
        std::cout << &ProgramErrorMessage[0] << "\n";
        std::cout << "Shader failed to be created.\n";
    }
    
    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);
    
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    
    return ProgramID;
}

/* Getting and setting Uniforms. */
GLint getUniformLocation(const GLint programID, const GLchar* name)
{
    return glGetUniformLocation(programID, name);
}

void setUniform1f(const GLint programID, const GLchar* name, float value)
{
    glUniform1f(getUniformLocation(programID, name), value);
}

void setUniform1fv(const GLint programID, const GLchar* name, float* value, int count)
{
    glUniform1fv(getUniformLocation(programID, name), count, value);
}

void setUniform1i(const GLint programID, const GLchar* name, int value)
{
    glUniform1i(getUniformLocation(programID, name), value);
}

void setUniform1iv(const GLint programID, const GLchar* name, int* value, int count)
{
    glUniform1iv(getUniformLocation(programID, name), count, value);
}

void setUniform2f(const GLint programID, const GLchar* name, const glm::vec2& vector)
{
    glUniform2f(getUniformLocation(programID, name), vector.x, vector.y);
}

void setUniform3f(const GLint programID, const GLchar* name, const glm::vec3& vector)
{
    glUniform3f(getUniformLocation(programID, name), vector.x, vector.y, vector.z);
}

void setUniform4f(const GLint programID, const GLchar* name, const glm::vec4& vector)
{
    glUniform4f(getUniformLocation(programID, name), vector.x, vector.y, vector.z, vector.w);
}

void setUniformMat4(const GLint programID, const GLchar* name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(getUniformLocation(programID, name), 1, GL_FALSE, &(matrix[0][0]) );
}




