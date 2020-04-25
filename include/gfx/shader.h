#pragma once

#include <string>
#include <unordered_set>
#include <map>
#include <memory>

#define SHADER_DEF_FILE "shaders.json"
#define SHADER_COMPILE_LOG "compile.log"
#define SHADER_LINK_LOG "link.log"

class CGame;
class CShaderStage;

/**
* @brief Shader service provider for all game objects.
* @details This class manages all the shader objects in the game, and allows game objects to
*	retrieve these shader objects via a reference ID. Only the support shader types will be available.
* @author Timothy Volpe
* @date 4/25/2020
*/
class CShaderManager
{
private:
	CGame *m_pGameHandle;

	std::map<std::string, std::shared_ptr<CShaderStage>> m_shaderStageObjects;

	/** Compiles the shader stages passed and stores them in temporary map m_shaderStageObjects */
	bool compileShaderStages( std::unordered_set<std::string> &shaderStages );
public:
	CShaderManager( CGame *pGameHandle );
	~CShaderManager();

	bool initialize();
	void shutdown();

	/**
	* @brief Load the shader defined in the shaders.json file
	* @details Attempts to load each defined shader. If a shader fails to load, it will be treated as a fatal error.
	* @returns True if all shaders were loaded successfully, or false if there was a failure.
	*/
	bool loadShaders();
};

/**
* @brief Handles an GLSL shader program object.
* @details A GLSL shader program object can be created by compiling and linking shader objects,
*	such as vertex shaders, fragment shaders, geometry shaders, tessalation shaders, and compute shaders.
* @author Timothy Volpe
* @date 4/25/2020
*/
class CShaderProgram
{
public:
	CShaderProgram();
	~CShaderProgram();
};

/**
* @brief Intermediate class for handling GLSL shader stage objects.
* @details These shader objects are created and sent to the CShaderProgram class and used to load and compile the shader code.
*	After a shader object has been linked into all the shader program objects that use it, it can be deleted to free up memory.
* @author Timothy Volpe
* @date 4/25/2020
*/
class CShaderStage
{
private:
	CGame *m_pGameHandle;

	std::string m_shaderName;
	GLenum m_shaderType;

	GLuint m_shaderObjectId;
public:
	static const char* getShaderTypeStr( GLenum type );

	CShaderStage( CGame *pGameHandle, std::string shaderName, GLenum shaderType );
	~CShaderStage();

	void deleteShader();

	/**
	* @brief Load the shader stage code from a file and compile.
	* @details This takes the shader stage code from a text file and compiles it to binary. This also creates the GL shader stage object
	*	if it did not exist prior.
	* @param[in]	relPath		The relative path to the shader in the shader directory.
	* @return Returns true if successfully loaded the code and compiled it.
	*/
	bool compileFromFile( std::string relPath );

	inline std::string getShaderName() { return m_shaderName; }
	inline GLenum getShaderType() { return m_shaderType; }
};