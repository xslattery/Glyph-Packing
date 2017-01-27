
#include <OpenGL/gl3.h>
#define GLFW_INCLUDE_NONE // FIXES : gl.h and gl3.h conflict.
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <map>

#include "Shader.hpp"

#define DEFAULT_WINDOW_WIDTH 820/2 
#define DEFAULT_WINDOW_HEIGHT 520/2

#define GENERATE_FONT_TEXTURE 1

struct Sprite {
	
	GLuint vao = 0;
	GLuint vbo = 0;
	GLuint ebo = 0;
	GLuint tex_id = 0;

	GLuint num_indices;

	glm::vec2 size;
	glm::vec3 position;

	glm::mat4 model_mtx;

	void update_model_matrix();
	void render( GLuint shader );
};

void Sprite::update_model_matrix() {
	model_mtx = glm::translate( glm::mat4(1), position );
}

void Sprite::render ( GLuint shader ) {
	setUniformMat4( shader, "model", model_mtx );
	glBindTexture( GL_TEXTURE_2D, tex_id );
	glBindVertexArray( vao );
	glDrawElements( GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0 );
}

void create_packed_glyph_sprite ( GLuint shader, Sprite* sprite, FT_Library ft, unsigned int fontsize, const char* font ) {
	
	GLfloat vertices [20] = {
	
		0, 0, 0,								0, 0,
		0, sprite->size.y, 0,					0, 1,
		sprite->size.x, sprite->size.y, 0,		1, 1,
		sprite->size.x, 0, 0,					1, 0,
	};
	
	GLuint indices [6] = {
		0, 1, 2,
		0, 2, 3
	};
	
	sprite->num_indices = 6;
	
	if ( sprite->vao == 0 && sprite->vbo == 0 && sprite->ebo == 0 ) {
		glGenVertexArrays(1, &sprite->vao);
		glGenBuffers(1, &sprite->vbo);
		glGenBuffers(1, &sprite->ebo);
	}
	
	glBindVertexArray(sprite->vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, sprite->vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sprite->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	GLint posAttrib = glGetAttribLocation( shader, "position" );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0 );
	glEnableVertexAttribArray( posAttrib );
	
	GLint texAttrib = glGetAttribLocation( shader, "texcoord" );
	glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)) );
	glEnableVertexAttribArray( texAttrib );
	
	glBindVertexArray(0);

	#if GENERATE_FONT_TEXTURE

	// ----------------------------
	// Font Texture Generation:
	// ---------------------
	if( sprite->tex_id != 0 ) {
		glDeleteTextures(1, &sprite->tex_id );
		sprite->tex_id = 0;
	}

	if( sprite->tex_id == 0 ) {

		if ( fontsize > 200 ) fontsize = 200; // NOTE: The max size will be 200pixels aka 100pt

	    FT_Face face;
	    if ( FT_New_Face( ft, font, 0, &face ) ) { std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl; return; }
	    
	    FT_Set_Pixel_Sizes( face, 0, fontsize );
	    
	    if ( FT_Load_Char( face, 'X', FT_LOAD_RENDER ) ) { std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl; return; }
	    
	    struct Character {
	        glm::ivec2 size; 
	        glm::ivec2 bearing;	
	        GLuint advance;	
	        unsigned char* bitmap;
	        ~Character() { delete [] bitmap; }
	    };

	    std::map< GLchar, Character > characters;

	    GLubyte startCharacter = 0;
	    GLubyte endCharacter = 126;

	    for ( GLubyte c = startCharacter; c <= endCharacter; c++ ) {
	        if ( FT_Load_Char( face, c, FT_LOAD_RENDER ) ) {
	            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
	            continue;
	        }

	        characters.insert( std::pair<GLchar, Character>( c, Character() ) );
	        characters[c].size = glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows );
	        characters[c].bearing = glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top );
	        characters[c].advance = (GLuint)face->glyph->advance.x;
	        unsigned int dimensions = (unsigned int)face->glyph->bitmap.width * (unsigned int)face->glyph->bitmap.rows;
	        characters[c].bitmap = new unsigned char [ dimensions ];
			
			memcpy( characters[c].bitmap, face->glyph->bitmap.buffer, dimensions );
	    }

	    unsigned int combined_character_area = 0;

	    for (GLchar i = startCharacter; i <= endCharacter; i++) {
	        combined_character_area += characters[i].size.x * characters[i].size.y;
	    }

	    unsigned int recm_dim = sqrt( combined_character_area );
	    recm_dim *= 1.5;
	    
	    //
	    // NOTE(Xavier): The dimensions of the texture needs to be 8-byte aligned. This is so that OpenGL will interperate it correctly.
	    // 				 This was the cause of the 'offset issue' you were having earlier.
	    //

	    recm_dim = recm_dim + (8 - (recm_dim%8));

	    unsigned char* combinedBitmap = new unsigned char [ recm_dim * recm_dim ]();

	    int xx = 0;
	    int yy = 0;
	    for ( GLchar ch = startCharacter; ch <= endCharacter; ch++ ) {
	    	if ( characters[ch].size.x > 0 ) { 
	    		if ( xx + characters[ch].size.x + 1 > recm_dim ) { yy += fontsize; xx = 0; }
		    	for ( int y = yy; y < yy+characters[ch].size.y; y++ ) {
		    		for ( int x = xx; x < xx+characters[ch].size.x; x++ ) {
		    			combinedBitmap[ recm_dim*y + x ] = characters[ch].bitmap[ characters[ch].size.x*(y-yy) + x-xx ];
		    		}
		    	}
		    	xx += characters[ch].size.x + 1;
	    	}
	    }

		if( sprite->tex_id == 0 ) {
			glGenTextures(1, &sprite->tex_id);
		}

		glBindTexture(GL_TEXTURE_2D, sprite->tex_id);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, recm_dim, recm_dim, 0, GL_RED, GL_UNSIGNED_BYTE, combinedBitmap );
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		delete [] combinedBitmap;
	}

	#endif
}

int main ( int argc, char** argv ) {
	
	glfwInit();
	
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
	glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );

	GLFWwindow *window = glfwCreateWindow( DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, "GLFW Window", nullptr, nullptr );
	
	if ( window == nullptr ) {
		printf("Failed to create window\n");
		glfwTerminate();
		return 0;
	}

	glfwMakeContextCurrent( window );

	int viewport_width, viewport_height;
	glfwGetFramebufferSize( window, &viewport_width, &viewport_height );
	
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	unsigned int window_width = mode->width;
	unsigned int window_height = mode->height;
	glfwSetWindowPos( window, window_width/2 - DEFAULT_WINDOW_WIDTH/2, window_height/2 - DEFAULT_WINDOW_HEIGHT/2 );
	
	glViewport( 0, 0, viewport_width, viewport_height );
	
	glCullFace( GL_BACK );
	glEnable( GL_CULL_FACE );
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor( 1, 1, 1, 1.0f);	
	
	// Camera Matrices:
	glm::mat4 viewMtx = glm::translate(glm::mat4(1), glm::vec3(0, 0, -10));
	glm::mat4 projectionMtx = glm::ortho(0.0f, (float)viewport_width, (float)viewport_height, 0.0f, 0.1f, 100.0f);

	FT_Library ft;
	if ( FT_Init_FreeType( &ft ) ) { std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl; return 0; }
	
	// Packed Glyth Texture Sprite: 
	GLuint shader = LoadShaders("Resources/Vertex.glsl", "Resources/Fragement.glsl");
	Sprite sprite;
	sprite.size = glm::vec2( 800, 500 );
	sprite.position = glm::vec3( 10, 10, 0 );
	sprite.update_model_matrix();
	unsigned int fontsize = 0;
	create_packed_glyph_sprite( shader, &sprite, ft, fontsize, "Resources/SFMono-Regular.ttf" );

	while ( !glfwWindowShouldClose( window ) ) {

		glfwWaitEventsTimeout( 0.2 );

		static bool viewResizeneeded = false;
		{
			int w, h; glfwGetWindowSize( window, &w, &h );
			if( w != window_width || h != window_height || viewResizeneeded )
			{
				window_width = w; window_height = h;
				glfwGetFramebufferSize( window, &viewport_width, &viewport_height );
				projectionMtx = glm::ortho(0.0f, (float)viewport_width, (float)viewport_height, 0.0f, 0.1f, 100.0f);
				glViewport( 0, 0, viewport_width, viewport_height );
				viewResizeneeded = false;

				sprite.size = glm::vec2( viewport_width-20, viewport_height-20 );
				sprite.update_model_matrix();
				create_packed_glyph_sprite( shader, &sprite, ft, fontsize, "Resources/SFMono-Regular.ttf" );
			}
		}

		glfwPollEvents();

		if( glfwGetKey( window, GLFW_KEY_SPACE ) ) {
			fontsize++;
			printf("Fontsize: %d\n", fontsize);
			create_packed_glyph_sprite( shader, &sprite, ft, fontsize, "Resources/SFMono-Regular.ttf" );
		}

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram( shader );
		setUniformMat4( shader, "view", viewMtx );
		setUniformMat4( shader, "projection", projectionMtx );
		sprite.render( shader );
		glUseProgram( 0 );
		
		glfwSwapBuffers( window );

	}
	
	FT_Done_FreeType( ft );

	glfwTerminate();

	return 0;
}