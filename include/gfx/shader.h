#pragma once

#include <string>
#include <unordered_set>
#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <glm/glm.hpp>

#define SHADER_DEF_FILE "shaders.json"
#define SHADER_COMPILE_LOG "compile.log"
#define SHADER_LINK_LOG "link.log"

/** If this is defined, missing uniforms will be a fatal error */
//#define STRICT_UNIFORMS

class CGame;
class CShaderStage;
class CShaderProgram;

struct ShaderProgramDefinition
{
	std::string programName;
	std::string vertShader, tessControlShader, tessEvalShader, geomShader, fragShader;
	std::vector<std::string> uniformNames;
};

/** Our uniform blocks are all hardcoded, the way to identify them can be found here. Must be continuous. */
enum UniformBlockIDs
{
	UNIFORM_BLOCK_MATRIX = 0,
	UNIFORM_BLOCK_COUNT
};

/** Defines a hard-coded uniform block */
struct UniformBlockData
{
	const char*		blockName;
	size_t			blockSize;

	GLuint			uboId;
	GLuint			uboBindingPoint;
};

/** Uniform block data, index corresponds to UniformBlockIDs value */
static const UniformBlockData UniformBlocknames[] ={
	{ "MatrixBlock", sizeof( glm::mat4 ) * 3 },
};

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

	std::vector<std::shared_ptr<CShaderProgram>> m_shaderPrograms;
	std::map<std::string, unsigned int> m_programIndexMap;

	std::vector<UniformBlockData> m_uniformBlocks;
	GLuint m_uboIndexCounter;

	unsigned int m_boundProgramIndex;

	/** Compiles the shader stages passed and stores them in temporary map m_shaderStageObjects */
	bool compileShaderStages( const std::unordered_set<std::string> &shaderStages, std::map<std::string, std::shared_ptr<CShaderStage>> &compiledStages );

	/** Takes the compiled stages and program definitions and links the programs, then stores them in m_shaderPrograms */
	bool linkPrograms( const std::map<std::string, std::shared_ptr<CShaderStage>> &compiledStages, const std::vector<ShaderProgramDefinition> &programDefs );

	/** Setup the global uniform blocks */
	bool createUniformBlocks();
	/** Cleanup the global uniform blocks */
	void destroyUniformBlocks();
public:
	CShaderManager( CGame *pGameHandle );
	~CShaderManager();

	bool initialize();
	void shutdown();

	/**
	* @brief Load the shader programs defined in the shaders.json file
	* @details Attempts to load each defined shader program. If a shader stage or program fails to load, it will be treated as a fatal error.
	*	The shader stages are aggregated and compiled, then each program is linked with its respective stages. Afterwards, the loaded shader stages
	*	are cleared from memory.
	* @returns True if all shader programs were loaded successfully, or false if there was a failure.
	*/
	bool loadPrograms();

	/**
	* @brief Gets the program index, used to retrieve the program object quickly, by name.
	* @details This function takes the program name, which is case-sensitive, as defined in shaders.json, and 
	*	attempts to retrieve its index from the index map. The index is used to quickly reference the program,
	*	and should be found and stored when an object needed the shader program is created. This function should not be called
	*	every time the program is used.
	* @param[in]	programName		The name of the shader program.
	* @param[out]	pIndex			The index of the shader program is stored here if found, otherwise it is untouched.
	* @returns Returns true if the index was found and stored in pIndex, false if it was not.
	*/
	bool getProgramIndex( std::string programName, unsigned int *pIndex );

	/**
	* @brief Bind a shader program.
	* @details If the program is already bound, this will have no effect.
	* param[in]		programIndex	The index of the shader program to bind.
	*/
	void bindProgram( unsigned int programIndex );

	/**
	* @brief Retrieves a pointer to the shader program class, by index.
	* @returns Pointer to shader program class.
	*/
	std::shared_ptr<CShaderProgram> getProgramByIndex( unsigned int programIndex );

	/**
	* @brief Retrieves a reference to the global uniform block data.
	* @returns A reference to the global uniform block data.
	*/
	inline std::vector<UniformBlockData>& getUniformBlockData() { return m_uniformBlocks; }

	/**
	* @brief Get a reference to a global uniform block's identifying information.
	* @param[in]	blockIdentifier		The hardcoded block identifier
	* @returns Reference to struct containing information on the uniform block
	*/
	UniformBlockData& getUniformBlock( UniformBlockIDs blockIdentifier );
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
private:
	CGame *m_pGameHandle;

	std::string m_programName;

	GLuint m_shaderProgramId;

	std::vector<GLint> m_uniformLocations;
	std::map<std::string, size_t> m_uniformNameToIndex;

	bool m_updateUniforms;
	std::vector<std::function<void()>> m_updateCallbacks;
public:
	CShaderProgram( CGame* pGameHandle, std::string programName );
	~CShaderProgram();

	/**
	* @brief Initialize the shader and create the shader program.
	* @details Does not attach or link any shaders.
	* @returns Returns true if successfully created the program, or false otherwise.
	*/
	bool initialize();
	/**
	* @brief Deletes the shader program
	*/
	void deleteProgram();

	/**
	* @brief Links the shader program object.
	* @details Shader objects should be attached prior to calling this function. This also finds the uniform locations
	* @param[in]	uniformNames	The names of the uniforms to search for.
	* @returns True if successfully linked, or false if otherwise. Errors will be dumped to the #SHADER_LINK_LOG file.
	*/
	bool link( std::vector<std::string> uniformNames );

	/**
	* @brief Bind the shader object. Should only be called by shader manager.
	* @warning Should only be called by shader manager.
	*/
	void bind();

	/**
	* @brief Add to the list of functions to call when uniforms are updated.
	* @param[in]	callback	Should be a pointer to the callback function.
	*/
	void subscribeToUniformUpdate( std::function<void()> callback );

	/**
	* @brief Get uniform location from name.
	* @details Slow lookup to get the uniform index from the uniform name in the lookup table.
	*	Call this when initializing, then use the index to retrieve the uniform location.
	* @param[in]	name	The name of the uniform as it is in the shader.
	* @param[out]	pIndex	Uniform index used to retrieve the uniform location with getUniformLocation.
	* @returns Returns true if there was an entry for the given name, false if otherwise.
	*/
	bool getUniformIndex( std::string name, size_t* pIndex );

	/**
	* @brief Get the OpenGL uniform location.
	* @details This retrieves the uniform location used to update the uniforms value.
	* @param[in]	uniformIndex	The index of the uniform from getUniformIndex
	* @returns The location of the uniform used to update its location.
	*/
	GLint getUniformLocation( size_t uniformIndex );

	inline std::string getProgramName() { return m_programName;  }
	GLuint getProgramId() { return m_shaderProgramId; }

	/**
	* @brief Hint to the shader that the uniforms need to be updated.
	* @details This will tell the shader to call the subscribing uniform update functions
	*	after the next bind call.
	*/
	inline void requireUniformUpdate() { m_updateUniforms = true; }
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

	std::vector<GLuint> m_programsAttachedTo;
public:
	static const char* getShaderTypeStr( GLenum type );

	CShaderStage( CGame *pGameHandle, GLenum shaderType );
	~CShaderStage();

	/**
	* @brief Will detach shader from all programs and delete shader stage object.
	*/
	void deleteShader();

	/**
	* @brief Load the shader stage code from a file and compile.
	* @details This takes the shader stage code from a text file and compiles it to binary. This also creates the GL shader stage object
	*	if it did not exist prior.
	* @param[in]	relPath		The relative path to the shader in the shader directory.
	* @return Returns true if successfully loaded the code and compiled it, false if it failed. Errors will be dumped to the #SHADER_COMPILE_LOG file.
	*/
	bool createFromFile( std::string relPath );

	/**
	* @brief Attach the shader stage to a program with the given ID
	* @details The shader will be attached to a specific program. According to the docs, a shader can be attached to multiple programs
	*	at once. However, it cannot be deleted until it has been detached from all the programs. Therefore this function will store
	*	the programs it is attached to, and detech from all when deleted. Programs are stored by their ID, not a pointer to the class.
	* @param[in]	programId	The openGL object ID of the shader program to attach to.
	* @returns Returns true if successfully attached, or false otherwise. If the stage was already attached to the specific program, a warning will be displayed but no error will be returned.
	*/
	bool attachShader( std::shared_ptr<CShaderProgram> shaderProgram );
	/**
	* @brief Detach the shader from the program with the given ID
	* @details If the shader stage is not attached to the given program, a warning will be displayed.
	* @param[in]	programId	The openGL object ID of the shader program to detach from.
	*/
	void detachShader( std::shared_ptr<CShaderProgram> shaderProgram );
	/**
	* @brief Detach the shader stage from all programs, usually before deletion.
	*/
	void detachFromAll();

	/** Get the OpenGL enum value for the shader type */
	inline GLenum getShaderType() { return m_shaderType; }
	/** Get the OpenGL shader stage ID */
	inline GLuint getShaderId() { return m_shaderObjectId; }
};