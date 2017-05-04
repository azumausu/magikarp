#pragma once
#define GLEW_STATIC
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <tuple>
#include <stdexcept>
#include <iterator>
#include <sstream>
#include <map>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#if defined( _MSC_VER ) && !defined( _M_AMD64 )
#error "only for x64"
#endif

#ifdef _MSC_VER
#pragma comment( lib, "glew32s.lib" )
#pragma comment( lib, "glfw3.lib" )
#pragma comment( lib, "opengl32" )
#endif

namespace GL
{
	struct PackedVertex {
		glm::vec3 position;
		glm::vec2 uv;
		glm::vec3 normal;
		bool operator<(const PackedVertex that) const {
			return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
		};
	};

    struct window_data
    {
        glm::mat4 proj, view, model;
        bool draged{ false };
        float xpos, ypos;
        GLuint program;
    };

	bool loadOBJ(
		const char * path,
		std::vector<glm::vec3> & out_vertices,
		std::vector<glm::vec2> & out_uvs,
		std::vector<glm::vec3> & out_normals
	);

	// Load a .DDS file using GLFW's own loader
	GLuint loadDDS(const char * imagepath);

	// Load a .BMP file using our custom loader
	GLuint loadBMP_custom(const char * imagepath);

    bool getSimilarVertexIndex(
        glm::vec3 & in_vertex,
        glm::vec2 & in_uv,
        glm::vec3 & in_normal,
        std::vector<glm::vec3> & out_vertices,
        std::vector<glm::vec2> & out_uvs,
        std::vector<glm::vec3> & out_normals,
        unsigned short & result
    );

	void indexVBO(
		std::vector<glm::vec3> & in_vertices,
		std::vector<glm::vec2> & in_uvs,
		std::vector<glm::vec3> & in_normals,

		std::vector<unsigned short> & out_indices,
		std::vector<glm::vec3> & out_vertices,
		std::vector<glm::vec2> & out_uvs,
		std::vector<glm::vec3> & out_normals
	);


	void indexVBO_TBN(
		std::vector<glm::vec3> & in_vertices,
		std::vector<glm::vec2> & in_uvs,
		std::vector<glm::vec3> & in_normals,
		std::vector<glm::vec3> & in_tangents,
		std::vector<glm::vec3> & in_bitangents,

		std::vector<unsigned short> & out_indices,
		std::vector<glm::vec3> & out_vertices,
		std::vector<glm::vec2> & out_uvs,
		std::vector<glm::vec3> & out_normals,
		std::vector<glm::vec3> & out_tangents,
		std::vector<glm::vec3> & out_bitangents
	);

	void computeTangentBasis(
		// inputs
		std::vector<glm::vec3> const &vertices,
		std::vector<glm::vec2> const &uvs,
		std::vector<glm::vec3> const &normals,
		// outputs
		std::vector<glm::vec3> & tangents,
		std::vector<glm::vec3> & bitangents
	);

	std::string readallfile(std::string const &filename);

	GLuint compile_shader(char const *vertex_shader_src, char const *fragment_shader_src);

    template< typename T, typename Alloc >
    GLuint make_gl_buffer(GLenum const type, GLenum const usage, std::vector< T, Alloc > const &vec)
    {
        GLuint id;
        glGenBuffers(1, &id);
        glBindBuffer(type, id);
        auto const size = std::size(vec);
        glBufferData(type, sizeof(vec[0]) * size, size ? &vec[0] : nullptr, usage);
        return id;
    }

    // plyを適当にパースする奴。全然正しくない。
    void load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index );
    std::tuple< std::vector< float >, std::vector< unsigned int > > load_ply( std::string const &filename );

    void calc_normal( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > &normal );
    std::vector< float > calc_normal( std::vector< float > const &point, std::vector< unsigned int > const &index );

    void minmax_coord( std::vector< float > const &point, std::tuple< float, float > &x_minmax, std::tuple< float, float > &y_minmax, std::tuple< float, float > &z_minmax );
    std::tuple< std::tuple< float, float >, std::tuple< float, float >, std::tuple< float, float > > minmax_coord( std::vector< float > const &point );

    template< typename T >
    static
    auto defer( T pfunc ) noexcept
    {
        class Call
        {
        private:
            T pfunc;
        public:
            Call( T _pfunc ) noexcept : pfunc( _pfunc ){}
            Call( Call const & ) = delete;
            Call( Call &&r ) noexcept : pfunc( r.pfunc ){ r.pfunc = nullptr; }
            Call &operator=( Call const & ) = delete;
            Call &operator=( Call && ) = delete;
            ~Call() noexcept{ if( pfunc != nullptr ) pfunc(); }
        };
        return Call{ pfunc };
    }

    GLuint TextureRGBImageUpLoad( const void *ImageData, const unsigned ImageWidth, const unsigned ImageHeight );
}