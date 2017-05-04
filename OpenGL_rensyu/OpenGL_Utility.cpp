#include "OpenGL_Utility.h"

bool GL::loadOBJ(
	const char * path,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;


	FILE * file;
    fopen_s( &file, path, "r");
	if (file == NULL) {
		printf("Impossible to open the file ! Are you in the right path ? See Tutorial 1 for details\n");
		getchar();
		return false;
	}

	while (1) {

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

				   // else : parse lineHeader

		if (strcmp(lineHeader, "v") == 0) {
			glm::vec3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		else if (strcmp(lineHeader, "vt") == 0) {
			glm::vec2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
			temp_uvs.push_back(uv);
		}
		else if (strcmp(lineHeader, "vn") == 0) {
			glm::vec3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		else if (strcmp(lineHeader, "f") == 0) {
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9) {
				printf("File can't be read by our simple parser :-( Try exporting with other options\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);
		}
		else {
			// Probably a comment, eat up the rest of the line
			char stupidBuffer[1000];
			fgets(stupidBuffer, 1000, file);
		}

	}

	// For each vertex of each triangle
	for (unsigned int i = 0; i<vertexIndices.size(); i++) {

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normalIndices[i];

		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex - 1];
		glm::vec2 uv = temp_uvs[uvIndex - 1];
		glm::vec3 normal = temp_normals[normalIndex - 1];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);

	}

	return true;
}

std::string GL::readallfile(std::string const &filename)
{
	std::ifstream ifs(filename);
	if (!ifs.is_open()) throw std::runtime_error("readallfile: cannot open file " + filename);
	std::string tmp;
	if (!std::getline(ifs, tmp, '\0')) return {};
	return tmp;
}

GLuint GL::compile_shader(char const *vertex_shader_src, char const *fragment_shader_src)
{
	GLint result, log_length;
	std::vector< char > log_buff;
	bool fail = false;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	char const *pvss = vertex_shader_src;
	glShaderSource(vertex_shader, 1, &pvss, nullptr);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		fail = true;
		glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0)
		{
			log_buff.resize(log_length);
			glGetShaderInfoLog(vertex_shader, log_length, nullptr, &log_buff[0]);
			std::clog << "Vertex Shader Compile Log\n";
			std::clog << &log_buff[0] << std::endl;
		}
	}

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	char const *pfss = fragment_shader_src;
	glShaderSource(fragment_shader, 1, &pfss, nullptr);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		fail = true;
		glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0)
		{
			log_buff.resize(log_length);
			glGetShaderInfoLog(fragment_shader, log_length, nullptr, &log_buff[0]);
			std::clog << "Fragment Shader Compile Log\n";
			std::clog << &log_buff[0] << std::endl;
		}
	}

	if (fail)
	{
		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		return 0;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result == GL_FALSE)
	{
		fail = true;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		if (log_length > 0)
		{
			log_buff.resize(log_length);
			glGetProgramInfoLog(program, log_length, nullptr, &log_buff[0]);
			std::clog << "Shader Link Log\n";
			std::clog << &log_buff[0] << std::endl;
		}
	}
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	if (fail)
	{
		glDeleteShader(program);
		return 0;
	}
	return program;
}

void GL::computeTangentBasis(
	// inputs
	std::vector<glm::vec3> const &vertices,
	std::vector<glm::vec2> const &uvs,
	std::vector<glm::vec3> const &normals,
	// outputs
	std::vector<glm::vec3> & tangents,
	std::vector<glm::vec3> & bitangents
) {

	for (unsigned int i = 0; i<vertices.size(); i += 3) {

		// Shortcuts for vertices
		glm::vec3 v0 = vertices[i + 0];
		glm::vec3 v1 = vertices[i + 1];
		glm::vec3 v2 = vertices[i + 2];

		// Shortcuts for UVs
		glm::vec2 uv0 = uvs[i + 0];
		glm::vec2 uv1 = uvs[i + 1];
		glm::vec2 uv2 = uvs[i + 2];

		// Edges of the triangle : postion delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		glm::vec3 bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);
		bitangents.push_back(bitangent);

	}

	// See "Going Further"
	for (unsigned int i = 0; i<vertices.size(); i += 1)
	{
		glm::vec3 n = normals[i];
		glm::vec3 &t = tangents[i];
		glm::vec3 &b = bitangents[i];

		// Gram-Schmidt orthogonalize
		t = glm::normalize(t - n * glm::dot(n, t));

		// Calculate handedness
		if (glm::dot(glm::cross(n, t), b) < 0.0f) {
			t = t * -1.0f;
		}

	}


}

GLuint GL::loadBMP_custom(const char * imagepath) {

	printf("Reading image %s\n", imagepath);

	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	// Open the file
	FILE * file;
    fopen_s( &file, imagepath, "rb");
	if (!file) { printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); return 0; }

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if (fread(header, 1, 54, file) != 54) {
		printf("Not a correct BMP file\n");
		return 0;
	}
	// A BMP files always begins with "BM"
	if (header[0] != 'B' || header[1] != 'M') {
		printf("Not a correct BMP file\n");
		return 0;
	}
	// Make sure this is a 24bpp file
	if (*(int*)&(header[0x1E]) != 0) { printf("Not a correct BMP file\n");    return 0; }
	if (*(int*)&(header[0x1C]) != 24) { printf("Not a correct BMP file\n");    return 0; }

	// Read the information about the image
	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize == 0)    imageSize = width*height * 3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos == 0)      dataPos = 54; // The BMP header is done that way

										 // Create a buffer
	data = new unsigned char[imageSize];

	// Read the actual data from the file into the buffer
	fread(data, 1, imageSize, file);

	// Everything is in memory now, the file wan be closed
	fclose(file);

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete[] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}

// Since GLFW 3, glfwLoadTexture2D() has been removed. You have to use another texture loading library, 
// or do it yourself (just like loadBMP_custom and loadDDS)
//GLuint loadTGA_glfw(const char * imagepath){
//
//	// Create one OpenGL texture
//	GLuint textureID;
//	glGenTextures(1, &textureID);
//
//	// "Bind" the newly created texture : all future texture functions will modify this texture
//	glBindTexture(GL_TEXTURE_2D, textureID);
//
//	// Read the file, call glTexImage2D with the right parameters
//	glfwLoadTexture2D(imagepath, 0);
//
//	// Nice trilinear filtering.
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
//	glGenerateMipmap(GL_TEXTURE_2D);
//
//	// Return the ID of the texture we just created
//	return textureID;
//}



#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint GL::loadDDS(const char * imagepath) {

	unsigned char header[124];

	FILE *fp;

	/* try to open the file */
	fopen_s( &fp, imagepath, "rb");
	if (fp == NULL) {
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
		return 0;
	}

	/* verify the type of file */
	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fclose(fp);
		return 0;
	}

	/* get the surface desc */
	fread(&header, 124, 1, fp);

	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linearSize = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC = *(unsigned int*)&(header[80]);


	unsigned char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, fp);
	/* close the file pointer */
	fclose(fp);

	unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch (fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
			0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
		if (width < 1) width = 1;
		if (height < 1) height = 1;

	}

	free(buffer);

	return textureID;


}


// Returns true iif v1 can be considered equal to v2
bool is_near(float v1, float v2) {
	return fabs(v1 - v2) < 0.01f;
}

// Searches through all already-exported vertices
// for a similar one.
// Similar = same position + same UVs + same normal
bool GL::getSimilarVertexIndex(
	glm::vec3 & in_vertex,
	glm::vec2 & in_uv,
	glm::vec3 & in_normal,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals,
	unsigned short & result
) {
	// Lame linear search
	for (unsigned int i = 0; i<out_vertices.size(); i++) {
		if (
			is_near(in_vertex.x, out_vertices[i].x) &&
			is_near(in_vertex.y, out_vertices[i].y) &&
			is_near(in_vertex.z, out_vertices[i].z) &&
			is_near(in_uv.x, out_uvs[i].x) &&
			is_near(in_uv.y, out_uvs[i].y) &&
			is_near(in_normal.x, out_normals[i].x) &&
			is_near(in_normal.y, out_normals[i].y) &&
			is_near(in_normal.z, out_normals[i].z)
			) {
			result = i;
			return true;
		}
	}
	// No other vertex could be used instead.
	// Looks like we'll have to add it to the VBO.
	return false;
}

void indexVBO_slow(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	// For each input vertex
	for (unsigned int i = 0; i<in_vertices.size(); i++) {

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = GL::getSimilarVertexIndex(in_vertices[i], in_uvs[i], in_normals[i], out_vertices, out_uvs, out_normals, index);

		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else { // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);
			out_indices.push_back((unsigned short)out_vertices.size() - 1);
		}
	}
}

void GL::indexVBO(
	std::vector<glm::vec3> & in_vertices,
	std::vector<glm::vec2> & in_uvs,
	std::vector<glm::vec3> & in_normals,

	std::vector<unsigned short> & out_indices,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
) {
	std::map<PackedVertex, unsigned short> VertexToOutIndex;

	auto getSimilarVertexIndex_fast = [](
		GL::PackedVertex & packed,
		std::map<GL::PackedVertex, unsigned short> & VertexToOutIndex,
		unsigned short & result
	)
	{
		std::map<GL::PackedVertex, unsigned short>::iterator it = VertexToOutIndex.find(packed);
		if (it == VertexToOutIndex.end()) {
			return false;
		}
		else {
			result = it->second;
			return true;
		}
	};

	// For each input vertex
	for (unsigned int i = 0; i<in_vertices.size(); i++) {

		PackedVertex packed = { in_vertices[i], in_uvs[i], in_normals[i] };


		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = getSimilarVertexIndex_fast(packed, VertexToOutIndex, index);

		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);
		}
		else { // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);
			unsigned short newindex = (unsigned short)out_vertices.size() - 1;
			out_indices.push_back(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
}







void GL::indexVBO_TBN(
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
) {
	// For each input vertex
	for (unsigned int i = 0; i<in_vertices.size(); i++) {

		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = GL::getSimilarVertexIndex(in_vertices[i], in_uvs[i], in_normals[i], out_vertices, out_uvs, out_normals, index);

		if (found) { // A similar vertex is already in the VBO, use it instead !
			out_indices.push_back(index);

			// Average the tangents and the bitangents
			out_tangents[index] += in_tangents[i];
			out_bitangents[index] += in_bitangents[i];
		}
		else { // If not, it needs to be added in the output data.
			out_vertices.push_back(in_vertices[i]);
			out_uvs.push_back(in_uvs[i]);
			out_normals.push_back(in_normals[i]);
			out_tangents.push_back(in_tangents[i]);
			out_bitangents.push_back(in_bitangents[i]);
			out_indices.push_back((unsigned short)out_vertices.size() - 1);
		}
	}
}

void GL::load_ply( std::string const &filename, std::vector< float > &point, std::vector< unsigned int > &index )
{
    point.clear();
    index.clear();
    std::ifstream ifs( filename, std::ios::binary );
    if( !ifs.is_open() ) throw std::runtime_error( "load_ply: cannnot open " + filename );
    int vertexline = 0, faceline = 0;
    enum class format{
        ASCII, BINARY_LE, UNKNOWN
    } read_format = format::UNKNOWN;
    std::string line;
    while( std::getline( ifs, line ) )
    {
        if( line.substr( 0, 7 ) == "format " )
        {
            if( line.substr( 7, 6 ) == "ascii " )
            {
                read_format = format::ASCII;
            }
            else if( line.substr( 7, 21 ) == "binary_little_endian " )
            {
                read_format = format::BINARY_LE;
            }
        }
        else if( line.substr( 0, 15 ) == "element vertex " )
        {
            vertexline = std::atoi( line.substr( 15 ).c_str() );
        }
        else if( line.substr( 0, 13 ) == "element face " )
        {
            faceline = std::atoi( line.substr( 13 ).c_str() );
        }
        else if( line.substr( 0, 10 ) == "end_header" )
        {
            break;
        }
    }
    if( read_format == format::UNKNOWN ) throw std::runtime_error( "load_ply: cannot recognize ply format" );
    if( vertexline == 0 || faceline == 0 ) throw std::runtime_error( "load_ply: oh no" );
    point.resize( vertexline * 3 );
    index.resize( faceline * 3 );
    switch( read_format )
    {
    case format::ASCII:
    {
        for( int i = 0; i < vertexline && std::getline( ifs, line ); ++i )
        {
            std::istringstream iss( line );
            auto const ind = i * 3;
            iss >> point[ ind + 0 ] >> point[ ind + 1 ] >> point[ ind + 2 ];
        }
        for( int i = 0; i < faceline && std::getline( ifs, line ); ++i )
        {
            std::istringstream iss( line );
            auto const ind = i * 3;
            int dummy;
            iss >> dummy >> index[ ind + 0 ] >> index[ ind + 1 ] >> index[ ind + 2 ];
            if( dummy != 3 )
            {
                point.clear();
                index.clear();
                return;
            }
        }
    }
    break;
    case format::BINARY_LE:
    {
        for( int i = 0; i < vertexline; ++i )
        {
            auto const ind = i * 3;
            for( int j = 0; j < 3; ++j ) ifs.read( reinterpret_cast< char * >( &point[ ind + j ] ), sizeof( float ) );
        }
        for( int i = 0; i < faceline; ++i )
        {
            auto const ind = i * 3;
            char dummy;
            ifs.read( &dummy, sizeof( char ) );
            if( dummy != 3 )
            {
                point.clear();
                index.clear();
                return;
            }
            for( int j = 0; j < 3; ++j ) ifs.read( reinterpret_cast< char * >( &index[ ind + j ] ), sizeof( float ) );
        }
    }
    break;
    case format::UNKNOWN:
        throw std::logic_error( "never comes here" );
    }
}


std::tuple< std::vector< float >, std::vector< unsigned int > > GL::load_ply( std::string const &filename )
{
    std::vector< float > vf;
    std::vector< unsigned int > uf;
    load_ply( filename, vf, uf );
    return std::make_tuple( std::move( vf ), std::move( uf ) );
}

// 各頂点での法線ベクトルの計算
void GL::calc_normal( std::vector< float > const &point, std::vector< unsigned int > const &index, std::vector< float > &normal )
{
    auto const num_of_point = std::size( point ) / 3;
    normal.clear();
    normal.reserve( num_of_point * 3 );
    std::vector< bool > flag( num_of_point, false );
    std::vector< glm::vec3 > tmp_normal( num_of_point, glm::vec3( 0.0f, 0.0f, 0.0f ) );
    auto const index_size = std::size( index );
    for( auto i = 0u; i + 2 < index_size; i += 3 )
    {
        auto const *pp1 = &point[ index[ i + 0 ] * 3 ], *pp2 = &point[ index[ i + 1 ] * 3 ], *pp3 = &point[ index[ i + 2 ] * 3 ];
        glm::vec3 const p1( pp1[ 0 ], pp1[ 1 ], pp1[ 2 ] ), p2( pp2[ 0 ], pp2[ 1 ], pp2[ 2 ]  ), p3( pp3[ 0 ], pp3[ 1 ], pp3[ 2 ] );
        auto const n = glm::normalize( glm::cross( p1 - p2, p1 - p3 ) );
        for( auto i : { index[ i + 0 ], index[ i + 1 ], index[ i + 2 ] } )
        {
            flag[ i ] = true;
            tmp_normal[ i ] += n;
        }
    }
    for( auto i = 0u; i < num_of_point; ++i )
    {
        auto const n = flag[ i ] ? glm::normalize( tmp_normal[ i ] ) : glm::vec3( 0.0f, 0.0f, 0.0f );
        normal.insert( normal.end(), { n.x, n.y, n.z });
    }
}

std::vector< float > GL::calc_normal( std::vector< float > const &point, std::vector< unsigned int > const &index )
{
    std::vector< float > normal;
    calc_normal( point, index, normal );
    return std::move( normal );
}

template< typename VecFloat, typename Func >
static
typename std::enable_if<
    std::is_same<
    typename std::remove_cv< VecFloat >::type,
    std::vector< float >
    >::value
>::type foreach_point( VecFloat &point, Func func )
{
    auto const size = std::size( point );
    for( auto i = 0u; i + 2u < size; i += 3 )
    {
        func( point[ i + 0u ], point[ i + 1u ], point[ i + 2u ] );
    }
}

void GL::minmax_coord( std::vector< float > const &point, std::tuple< float, float > &x_minmax, std::tuple< float, float > &y_minmax, std::tuple< float, float > &z_minmax )
{
    x_minmax = y_minmax = z_minmax = std::make_tuple( std::numeric_limits< float >::infinity(), -std::numeric_limits< float >::infinity() );
    float &xmin = std::get< 0 >( x_minmax ), &xmax = std::get< 1 >( x_minmax ), &ymin = std::get< 0 >( y_minmax ), &ymax = std::get< 1 >( y_minmax ), &zmin = std::get< 0 >( z_minmax ), &zmax = std::get< 1 >( z_minmax );
    foreach_point(
        point,
        [ & ]( float x, float y, float z ){
        if( x < xmin ) xmin = x;
        if( xmax < x ) xmax = x;
        if( y < ymin ) ymin = y;
        if( ymax < y ) ymax = y;
        if( z < zmin ) zmin = z;
        if( zmax < z ) zmax = z;
    }
    );
}

std::tuple< std::tuple< float, float >, std::tuple< float, float >, std::tuple< float, float > > GL::minmax_coord( std::vector< float > const &point )
{
    std::tuple< float, float > x, y, z;
    minmax_coord( point, x, y, z );
    return std::make_tuple( x, y, z );
}


GLuint GL::TextureRGBImageUpLoad( const void *ImageData, const unsigned ImageWidth, const unsigned ImageHeight )
{
    GLuint texture_id;
    glGenTextures( 1, &texture_id );
    assert( glGetError() == GL_NO_ERROR );
    glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
    assert( glGetError() == GL_NO_ERROR );
    glBindTexture( GL_TEXTURE_2D, texture_id );
    assert( glGetError() == GL_NO_ERROR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, ImageWidth, ImageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, ImageData );
    assert( glGetError() == GL_NO_ERROR );
    return texture_id;
}
