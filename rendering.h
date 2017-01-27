//
//  rendering.h
//  TextEditor-Refactor
//
//  Created by Xavier Slattery on 16/1/17.
//  Copyright © 2017 Xavier Slattery. All rights reserved.
//

#ifndef rendering_h
#define rendering_h

struct window_properties
{
    GLFWwindow* window;
    int window_width;
    int window_height;
    int viewport_width;
    int viewport_height;
    float vertical_scaling_factor;
    float horizontal_scaling_factor;
};

struct text_box
{
    //	char* text;
    unsigned int fontsize;
    
    glm::vec2 bitmap_size;
    
    struct tb_character
    {
        glm::vec2 position;
        glm::vec2 size;
        GLuint advance;
        glm::vec2 bearing;
    };
    std::map<GLchar, tb_character> character_info;
    
    glm::vec3 position;
    //	glm::vec2 size;
    
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint tex_id = 0;
    
    GLuint num_indices;
    
    glm::mat4 modelMtx;
    
    ~text_box()
    {
        glDeleteTextures( 1, &tex_id );
        glDeleteBuffers( 1, &ebo );
        glDeleteBuffers( 1, &vbo );
        glDeleteVertexArrays( 1, &vao );
    }
    
};

struct Line
{
    std::string characters;
};

struct Document
{
    char* documentName;
    std::vector<Line> lines;
    glm::ivec2 cursorPosition;
    text_box textBox;
};

void load_document( Document* doc, const char* docName )
{
    doc->lines.clear();
    std::string inputString;
    std::ifstream docInputStream( docName, std::ios::in );
    if( docInputStream.is_open() )
    {
        std::string inLine = "";
        while( getline( docInputStream, inLine ) )
        {
            inputString += inLine + "\n";
            Line line = {
                inLine
            };
            doc->lines.push_back( line );
        }
        docInputStream.close();
    }
    else
    {
        std::cout << "Impossible to open. Are you in the right directory ? : " << std::string( docName ) << "\n";
    }
}

struct rect_outline
{
    glm::vec2 size;
    
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    
    GLuint num_indices;
    
    glm::mat4 modelMtx;
};

void generateTextBox_texture( text_box* textbox, const char* font, unsigned int fontsize, int offset )
{
    textbox->fontsize = fontsize;
    
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) { std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl; return; }
    
    FT_Face face;
    if (FT_New_Face(ft, font, 0, &face)) { std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl; return; }
    
    FT_Set_Pixel_Sizes(face, 0, fontsize);
    
    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER)) { std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl; return; }
    
    
    GLubyte start = 32;
    GLubyte end = 126;
    
    struct CH
    {
        glm::ivec2 Size;		// Size of glyph
        glm::ivec2 Bearing;		// Offset from baseline to left/top of glyph
        GLuint Advance;			// Offset to advance to next glyph
        unsigned char* bitmap;
        
        void cleanUp() { delete [] bitmap; }
        
    };
    
    std::map<GLchar, CH> ch;
    
    for (GLubyte c = start; c <= end; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        
        CH tmpch = {
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (GLuint)face->glyph->advance.x,
            new unsigned char [ face->glyph->bitmap.width * face->glyph->bitmap.rows ]
        };
        
        memcpy( tmpch.bitmap, face->glyph->bitmap.buffer, face->glyph->bitmap.width * face->glyph->bitmap.rows );
        
        ch.insert( std::pair<GLchar, CH>( c, tmpch ) );
        
    }
    
    unsigned int minHeight = 0;
    unsigned int minWidth = 0;
    
    for (GLchar i = start; i <= end; i++)
    {
        if ( ch[i].Size.y > minHeight ) minHeight = ch[i].Size.y;
        
        text_box::tb_character tmpch = {
            glm::vec2( minWidth, 0 ),
            ch[i].Size,
            ch[i].Advance,
            glm::vec2( ch[i].Bearing.x, ch[i].Bearing.y )
        };
        
        textbox->character_info.insert( std::pair<GLchar, text_box::tb_character>( i, tmpch ) );
        
        minWidth += ( ch[i].Size.x + 3 );
    }
    
    textbox->bitmap_size = glm::vec2( minWidth, minHeight);
    
    unsigned char* combinedBitmap = new unsigned char [ minHeight * minWidth ]();
    
    int currentPosition = 0;
    int currentRow = 0;
    
    for ( GLchar i = start; i <= end; i++)
    {
        if( currentRow < ch[i].Size.y )
        {
            for (int k = 0; k < ch[i].Size.x; k++)
            {
                combinedBitmap[ currentPosition + k ] = ch[i].bitmap[ k + (int)currentRow*(int)ch[i].Size.x ];
            }
        }
        
        currentPosition += ( (int)ch[i].Size.x + 3 );
        
        if( i == end ) { i = start-1; currentRow++; currentPosition += offset;  }
        
        if( currentPosition >= minHeight * minWidth ) break;
    }
    
    for ( auto & element : ch )
    {
        element.second.cleanUp();
    }
    
    // FIXME(Xavier): Currently when the font size is changed, you also need to change the offset value for the
    //				  currentPosition variable every new row. I am currently unsure why this is and will look
    //				  for a fix in the future.
    //				  MAYBE: expose the offset so it can be set with the function call OR create a lookup table
    //				  that will automatically set it depending on the font size. This option will however take
    //				  a lot of grunt work testing lots of different font sizes. (It is possiable in this process)
    //				  a solution to the problem may arrise.)
    
    
    if(textbox->tex_id != 0) glDeleteTextures( 1, &textbox->tex_id );

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, minWidth, minHeight, 0, GL_RED, GL_UNSIGNED_BYTE, combinedBitmap );
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    textbox->tex_id = tex;
    
    if( minWidth > 16384 ) printf("WARNING: The texture exceeds 16,384 pixels in width, this is too large for OpenGL\n");
    else if( minWidth > 14000 ) printf("Woah! The texture exceeds 14,000 pixels in width!\n");
    else if( minWidth > 12000 ) printf("Woah! The texture exceeds 12,000 pixels in width!\n");
    else if( minWidth > 10000 ) printf("Woah! The texture exceeds 10,000 pixels in width!\n");
    else if( minWidth > 8192 ) printf("Woah! The texture exceeds 8192 pixels in width!\n");
    else if( minWidth > 4096 ) printf("Woah! The texture exceeds 4099 pixels in width!\n");
    
    delete [] combinedBitmap;
    
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
}

void generateTextBox_buffers( GLuint shader, text_box* textbox, const char* tb_text, glm::vec2 tb_size, int tb_properties = 0 )
{
    
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;// Anti-clockwise wind
    GLuint index_count = 0;
    GLuint vert_ofst = 0;
    
    GLfloat xoffset = 0;
    GLfloat yoffset = 0;
    
    for(GLubyte c = *tb_text++; c != '\0'; c = *tb_text++)
    {
        
        if( c == '\n' )
        {
            xoffset = 0;
            yoffset += textbox->fontsize;
            continue;
        }
        
        GLfloat vx = textbox->character_info[ c ].size.x;
        GLfloat vy = -textbox->character_info[ c ].bearing.y;
        GLfloat vys = textbox->character_info[ c ].size.y - textbox->character_info[ c ].bearing.y;
        
        GLfloat ux = (1.0f/textbox->bitmap_size.x)*textbox->character_info[ c ].position.x ;
        GLfloat uxs = (1.0f/textbox->bitmap_size.x)*(textbox->character_info[ c ].position.x+textbox->character_info[ c ].size.x);
        GLfloat uy = (1.0f/textbox->bitmap_size.y)*(textbox->character_info[ c ].position.y+textbox->character_info[ c ].size.y);
        
        GLfloat tmp_vrt_array [36] = {
            xoffset,		yoffset + vy,		0.0f,				1.0f, 0.0f, 0.0f, 1.0f,				 ux, 0.0f,
            xoffset,		yoffset + vys,		0.0f,				0.0f, 1.0f, 0.0f, 1.0f,				 ux,   uy,
            xoffset + vx,	yoffset + vys,		0.0f,				0.0f, 0.0f, 1.0f, 1.0f,				uxs,   uy,
            xoffset + vx,	yoffset + vy,		0.0f,				0.0f, 1.0f, 1.0f, 1.0f,				uxs, 0.0f
        };
        vertices.insert( vertices.end(), tmp_vrt_array, tmp_vrt_array+36 );
        
        xoffset += (textbox->character_info[ c ].advance >> 6);
        
        GLuint tmp_ind_array [6] = {
            vert_ofst, vert_ofst+1, vert_ofst+2, vert_ofst, vert_ofst+2, vert_ofst+3
        };
        indices.insert( indices.end(),  tmp_ind_array, tmp_ind_array+6 );
        index_count += 6;
        vert_ofst += 4;
        
    }
    
    textbox->num_indices = index_count;
    
    if( textbox->vao == 0 && textbox->vbo == 0 && textbox->ebo == 0 )
    {
        glGenVertexArrays(1, &textbox->vao);
        glGenBuffers(1, &textbox->vbo);
        glGenBuffers(1, &textbox->ebo);
    }
    
    glBindVertexArray(textbox->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, textbox->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textbox->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
    
    GLint posAttrib = glGetAttribLocation( shader, "position" );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)0 );
    glEnableVertexAttribArray( posAttrib );
    
    GLint colorAttrib = glGetAttribLocation( shader, "color" );
    glVertexAttribPointer( colorAttrib, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( colorAttrib );
    
    GLint texAttrib = glGetAttribLocation( shader, "texcoord" );
    glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( texAttrib );
    
    glBindVertexArray(0);
}

void renderTextBox( text_box* textbox )
{
    glBindTexture( GL_TEXTURE_2D, textbox->tex_id );
    glBindVertexArray( textbox->vao );
    glDrawElements( GL_TRIANGLES, textbox->num_indices, GL_UNSIGNED_INT, 0 );
}

void generateTextBox_buffers_from_document( GLuint shader, Document* doc, glm::vec2 tb_size, int tb_properties = 0 )
{
    std::vector<GLfloat> vertices;
    std::vector<GLuint> indices;// Anti-clockwise wind
    GLuint index_count = 0;
    GLuint vert_ofst = 0;
    
    GLfloat xoffset = 0;
    GLfloat yoffset = doc->textBox.fontsize;
    
    for ( int i = 0; i < doc->lines.size(); i++ )
    {
        for( int j = 0; j < doc->lines[i].characters.length(); j++ )
        {
            GLuint c = doc->lines[i].characters[j];
            
            if( c == '\t' )
            {
                xoffset += (doc->textBox.character_info[ ' ' ].advance >> 6) * 4;
                continue;
            }
            
            if( c == ' ' )
            {
                xoffset += (doc->textBox.character_info[ ' ' ].advance >> 6);
                continue;
            }
            
            xoffset += doc->textBox.character_info[ c ].bearing.x;
            
            GLfloat vx = doc->textBox.character_info[ c ].size.x;
            GLfloat vy = -doc->textBox.character_info[ c ].bearing.y;
            GLfloat vys = doc->textBox.character_info[ c ].size.y - doc->textBox.character_info[ c ].bearing.y;
            
            //			if( xoffset + vx > tb_size.x )
            //			{
            //				xoffset = 0;
            //				yoffset += doc->textBox.fontsize;
            //			}
            
            GLfloat ux = (1.0f/doc->textBox.bitmap_size.x)*doc->textBox.character_info[ c ].position.x ;
            GLfloat uxs = (1.0f/doc->textBox.bitmap_size.x)*(doc->textBox.character_info[ c ].position.x+doc->textBox.character_info[ c ].size.x);
            GLfloat uy = (1.0f/doc->textBox.bitmap_size.y)*(doc->textBox.character_info[ c ].position.y+doc->textBox.character_info[ c ].size.y);
            
            GLfloat tmp_vrt_array [36] = {
                xoffset,		yoffset + vy,		0.0f,				1.0f, 0.0f, 0.0f, 1.0f,				 ux, 0.0f,
                xoffset,		yoffset + vys,		0.0f,				0.0f, 1.0f, 0.0f, 1.0f,				 ux,   uy,
                xoffset + vx,	yoffset + vys,		0.0f,				0.0f, 0.0f, 1.0f, 1.0f,				uxs,   uy,
                xoffset + vx,	yoffset + vy,		0.0f,				0.0f, 1.0f, 1.0f, 1.0f,				uxs, 0.0f
            };
            vertices.insert( vertices.end(), tmp_vrt_array, tmp_vrt_array+36 );
            
            xoffset -= doc->textBox.character_info[ c ].bearing.x;
            
            xoffset += (doc->textBox.character_info[ c ].advance >> 6);
            
            GLuint tmp_ind_array [6] = {
                vert_ofst, vert_ofst+1, vert_ofst+2, vert_ofst, vert_ofst+2, vert_ofst+3
            };
            indices.insert( indices.end(),  tmp_ind_array, tmp_ind_array+6 );
            index_count += 6;
            vert_ofst += 4;
            
        }
        xoffset = 0;
        yoffset += doc->textBox.fontsize;
        if( yoffset > tb_size.y ) break;
    }
    
    //	printf( "Num Verts: %lu\n", vertices.size()/9 );
    //	printf( "Num Incices: %u\n", index_count );
    
    doc->textBox.num_indices = index_count;
    
    if( doc->textBox.vao == 0 && doc->textBox.vbo == 0 && doc->textBox.ebo == 0 )
    {
        glGenVertexArrays(1, &doc->textBox.vao);
        glGenBuffers(1, &doc->textBox.vbo);
        glGenBuffers(1, &doc->textBox.ebo);
    }
    
    glBindVertexArray(doc->textBox.vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, doc->textBox.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, doc->textBox.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_DYNAMIC_DRAW);
    
    GLint posAttrib = glGetAttribLocation( shader, "position" );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)0 );
    glEnableVertexAttribArray( posAttrib );
    
    GLint colorAttrib = glGetAttribLocation( shader, "color" );
    glVertexAttribPointer( colorAttrib, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( colorAttrib );
    
    GLint texAttrib = glGetAttribLocation( shader, "texcoord" );
    glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), (void*)(7 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( texAttrib );
    
    glBindVertexArray(0);
}

void generateRectOutline_buffers( GLuint shader, rect_outline* ro )
{
    GLfloat vertices [28] =
    {
        0, 0, 0,			0.0, 1.0, 1.0, 1.0,
        0, 1, 0,			0.0, 1.0, 1.0, 1.0,
        1, 1, 0,			0.0, 1.0, 1.0, 1.0,
        1, 0, 0,			0.0, 1.0, 1.0, 1.0
    };
    
    GLuint indices [8] =
    {
        0, 1,
        1, 2,
        2, 3,
        0, 3,
    };
    
    ro->num_indices = 8;
    
    glGenVertexArrays(1, &ro->vao);
    glGenBuffers(1, &ro->vbo);
    glGenBuffers(1, &ro->ebo);
    
    glBindVertexArray(ro->vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, ro->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ro->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    GLint posAttrib = glGetAttribLocation( shader, "position" );
    glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0 );
    glEnableVertexAttribArray( posAttrib );
    
    GLint colorAttrib = glGetAttribLocation( shader, "color" );
    glVertexAttribPointer( colorAttrib, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)) );
    glEnableVertexAttribArray( colorAttrib );
    
    glBindVertexArray(0);
}

void render_rectOutline( rect_outline* ro )
{
    glLineWidth( 10 );
    glBindVertexArray( ro->vao );
    glDrawElements( GL_LINES, ro->num_indices, GL_UNSIGNED_INT, 0 );
}

#endif /* rendering_h */
