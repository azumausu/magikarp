#include <opencv2/opencv.hpp>
#include "OpenGL_Utility.h"

#ifdef _DEBUG
#pragma comment( lib, "opencv_world310d.lib" )
#else
#pragma comment( lib, "opencv_world310.lib" )
#endif

void window_framebuffer_size_callback( GLFWwindow *window, int width, int height )
{
    auto data = static_cast< GL::window_data * >( glfwGetWindowUserPointer( window ) );
    if( !data ) return;
    data->proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( width ) / height, 1.0f, 1000.0f );
    glfwMakeContextCurrent( window );
    glViewport( 0, 0, width, height );
}
void window_cursor_pos_callback( GLFWwindow *window, double _xpos, double _ypos )
{
    auto data = static_cast< GL::window_data * >( glfwGetWindowUserPointer( window ) );
    if( !data ) return;
    auto xpos = static_cast< float >( _xpos ), ypos = static_cast< float >( _ypos );
    if( data->draged )
    {
        auto const dx = (data->xpos - xpos) / 100.0f, dy = (data->ypos - ypos) / 100.0f;
        auto const h = std::hypot( dx, dy );
        data->model = glm::rotate( h, glm::vec3( -dy, dx, 0.0f ) ) * data->model;
    }
    data->xpos = xpos; data->ypos = ypos;
}
void window_mouse_button_callback( GLFWwindow *window, int button, int action, int mods )
{
    auto data = static_cast< GL::window_data * >( glfwGetWindowUserPointer( window ) );
    if( !data ) return;
    switch( button )
    {
    case GLFW_MOUSE_BUTTON_LEFT:
        switch( action ){
        case GLFW_PRESS: data->draged = true; break;
        case GLFW_RELEASE: data->draged = false; break;
        }
        break;
    }
}
void window_key_callback( GLFWwindow *window, int key, int scancode, int action, int mods )
{
    auto data = static_cast< GL::window_data * >( glfwGetWindowUserPointer( window ) );
    if( !data ) return;
    switch( key )
    {
    case GLFW_KEY_R:
        if( action == GLFW_PRESS )
        {
            glfwMakeContextCurrent( window );
            try
            {
                auto const vs = GL::readallfile( "NormalMapping.vertexshader" );
                auto const fs = GL::readallfile( "NormalMapping.fragmentshader" );
                if( auto p = GL::compile_shader( vs.c_str(), fs.c_str() ) ) data->program = p;
            }
            catch( ... )
            {}
        }
        break;
    }
}

int main()
{
    //windowサイズの設定
    static const unsigned int WIDTH = 1024u;
    static const unsigned int HEIGHT = 768u;
    //windowの生成
    if( !glfwInit() ) throw std::runtime_error( "glfwInit error" );
    auto ac1 = GL::defer( &glfwTerminate );
    glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR , 3 );
    glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR , 3 );
    glfwWindowHint( GLFW_SAMPLES, 16 );
    auto main_window = glfwCreateWindow( WIDTH, HEIGHT, "ply view", nullptr, nullptr );
    if( !main_window ) throw std::runtime_error( "glfwCreateWindow error" );
    glfwMakeContextCurrent( main_window );
    glewExperimental = GL_TRUE;
    if( glewInit() != GLEW_OK ) throw std::runtime_error( "glewInit error" );

    GL::window_data main_window_data;
    glfwSetWindowUserPointer( main_window, &main_window_data );
    
    glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    // デプステストとカリングを有効
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );
    //Linuxでこれいれないと動かなかったらしくほんとにおまじないとして使用している
    GLuint vao;
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    //シェーダコンパイル
    main_window_data.program = GL::compile_shader( GL::readallfile( "NormalMapping.vertexshader" ).c_str(), GL::readallfile( "NormalMapping.fragmentshader" ).c_str() );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(main_window_data.program, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(main_window_data.program, "V");
    GLuint ModelMatrixID = glGetUniformLocation(main_window_data.program, "M");
    GLuint ModelView3x3MatrixID = glGetUniformLocation(main_window_data.program, "MV3x3");

    // Load the texture
    GLuint DiffuseTexture = GL::loadDDS("diffuse.DDS");
    //GLuint NormalTexture = GL::loadBMP_custom("normal.bmp");
    cv::Mat img = cv::imread( "normal2.bmp" );
    GLuint NormalTexture = GL::TextureRGBImageUpLoad( img.data, img.cols, img.rows );
    GLuint SpecularTexture = GL::loadDDS("specular.DDS");

    // Get a handle for our "myTextureSampler" uniform
    GLuint DiffuseTextureID  = glGetUniformLocation(main_window_data.program, "DiffuseTextureSampler");
    GLuint NormalTextureID  = glGetUniformLocation(main_window_data.program, "NormalTextureSampler");
    GLuint SpecularTextureID  = glGetUniformLocation(main_window_data.program, "SpecularTextureSampler");


    //.objファイルのロード
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    bool res = GL::loadOBJ("Magikarp.obj", vertices, uvs, normals);
    //bool res = GL::loadOBJ("cylinder.obj", vertices, uvs, normals);
    //接線と従接線の計算
    std::vector<glm::vec3> tangents;
    std::vector<glm::vec3> bitangents;
    GL::computeTangentBasis(
        vertices, uvs, normals, // input
        tangents, bitangents    // output
    );

    //頂点と法線と接線と従接線のデータとインデックスの作成
    std::vector<unsigned short> indices;
    std::vector<glm::vec3> indexed_vertices;
    std::vector<glm::vec2> indexed_uvs;
    std::vector<glm::vec3> indexed_normals;
    std::vector<glm::vec3> indexed_tangents;
    std::vector<glm::vec3> indexed_bitangents;
    GL::indexVBO_TBN(
        vertices, uvs, normals, tangents, bitangents, 
        indices, indexed_vertices, indexed_uvs, indexed_normals, indexed_tangents, indexed_bitangents
    );

    //GPUへデータ転送
    auto const vertexbuffer = GL::make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, indexed_vertices );
    auto const uvbuffer = GL::make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, indexed_uvs );
    auto const normalbuffer = GL::make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, indexed_normals );
    auto const tangentbuffer = GL::make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, indexed_tangents );
    auto const bitangentbuffer = GL::make_gl_buffer( GL_ARRAY_BUFFER, GL_STATIC_DRAW, indexed_bitangents );
    auto const elementbuffer = GL::make_gl_buffer( GL_ELEMENT_ARRAY_BUFFER, GL_STATIC_DRAW, indices );


    auto const glmvec3tofloat = []( std::vector< glm::vec3 > const &glm_vertices, std::vector< GLfloat > &float_vertices )
    {
        float_vertices.clear();
        float_vertices.resize( glm_vertices.size() * 3 );
        for( int i = 0; i < glm_vertices.size(); i++ )
        {
            float_vertices[ i * 3 ] = glm_vertices[ i ][ 0 ];
            float_vertices[ i * 3 + 1 ] = glm_vertices[ i ][ 1 ];
            float_vertices[ i * 3 + 2 ] = glm_vertices[ i ][ 2 ];
        }
    };

    auto const floattoglmvec3 = [](  std::vector< GLfloat > const &float_vertices, std::vector< glm::vec3 > &glm_vertices )
    {
        glm_vertices.clear();
        glm_vertices.resize( float_vertices.size() / 3 );
        for( int i = 0; i < glm_vertices.size(); i++ )
        {
            glm_vertices[ i ][ 0 ] = float_vertices[ i * 3 ];
            glm_vertices[ i ][ 1 ] = float_vertices[ i * 3 + 1 ];
            glm_vertices[ i ][ 2 ] = float_vertices[ i * 3 + 2 ];
        }
    };

    std::vector< GLfloat > point;
    glmvec3tofloat( indexed_vertices, point );
    auto const minmax_coord = GL::minmax_coord( point );
    auto const &xminmax = std::get< 0 >( minmax_coord ), &yminmax = std::get< 1 >( minmax_coord ), &zminmax = std::get< 2 >( minmax_coord );
    auto const &xmin = std::get< 0 >( xminmax ), &xmax = std::get< 1 >( xminmax );
    auto const &ymin = std::get< 0 >( yminmax ), &ymax = std::get< 1 >( yminmax );
    auto const &zmin = std::get< 0 >( zminmax ), &zmax = std::get< 1 >( zminmax );
    auto const lx = (xmin + xmax) / 2.0f, ly = (ymin + ymax) / 2.0f, lz = (zmin + zmax) / 2.0f;
    auto const xl = xmax - xmin, yl = ymax - ymin, zl = zmax - zmin;

    main_window_data.proj = glm::perspective( glm::radians( 30.0f ), static_cast< float >( WIDTH ) / HEIGHT, 0.01f, 10000.0f );
    main_window_data.view = glm::lookAt( glm::vec3( 0.0f, 0.0f, -zl * 3 ), glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 0.0f, -1.0f, 0.0f ) );
    main_window_data.model = glm::mat4( 1.0f );
    glm::mat4 const c_model = glm::translate( -glm::vec3( lx, ly, lz ) );

    glViewport( 0, 0, WIDTH, HEIGHT );



    glfwSetFramebufferSizeCallback( main_window, window_framebuffer_size_callback );
    glfwSetCursorPosCallback( main_window, window_cursor_pos_callback );
    glfwSetMouseButtonCallback( main_window, window_mouse_button_callback );
    glfwSetKeyCallback( main_window, window_key_callback );

    glUseProgram(main_window_data.program);
    GLuint LightID = glGetUniformLocation(main_window_data.program, "LightPosition_worldspace");

    while( !glfwWindowShouldClose( main_window ) )
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glm::mat4 const model = main_window_data.model * c_model;
        glm::mat4 const mvp = main_window_data.proj * main_window_data.view * model;
        glm::mat3 const Rmat( model );

        glUseProgram( main_window_data.program );

        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &main_window_data.view[0][0]);
        glUniformMatrix3fv(ModelView3x3MatrixID, 1, GL_FALSE, &Rmat[0][0]);
        glm::vec3 lightPos = glm::vec3(0,0,4);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        // Bind our diffuse texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, DiffuseTexture);
        // Set our "DiffuseTextureSampler" sampler to user Texture Unit 0
        glUniform1i(DiffuseTextureID, 0);

        // Bind our normal texture in Texture Unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, NormalTexture);
        // Set our "Normal	TextureSampler" sampler to user Texture Unit 0
        glUniform1i(NormalTextureID, 1);

        // Bind our normal texture in Texture Unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, SpecularTexture);
        // Set our "Normal	TextureSampler" sampler to user Texture Unit 0
        glUniform1i(SpecularTextureID, 2);

        glEnableClientState( GL_VERTEX_ARRAY );

        glEnableVertexAttribArray( 0 );
        glBindBuffer( GL_ARRAY_BUFFER, vertexbuffer );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );

        glEnableVertexAttribArray( 1 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );

        glEnableVertexAttribArray( 2 );
        glBindBuffer( GL_ARRAY_BUFFER, normalbuffer );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );

        glEnableVertexAttribArray( 3 );
        glBindBuffer( GL_ARRAY_BUFFER, tangentbuffer );
        glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );

        glEnableVertexAttribArray( 4 );
        glBindBuffer( GL_ARRAY_BUFFER, uvbuffer );
        glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast< void * >( 0 ) );

        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, elementbuffer );

        glDrawElements( GL_TRIANGLES, static_cast< GLsizei >( indices.size() ), GL_UNSIGNED_SHORT, reinterpret_cast< void * >( 0 ) );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(3);
        glDisableVertexAttribArray(4);

        glDisableClientState( GL_VERTEX_ARRAY );
        glfwSwapBuffers( main_window );
        glfwPollEvents();
    }


    return 1;
}