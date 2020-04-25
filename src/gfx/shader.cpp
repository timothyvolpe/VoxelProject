#include <boost\property_tree\json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <gl/glew.h>
#include <fstream>
#include <chrono>
#include "gfx\shader.h"
#include "game.h"
#include "filesystem.h"
#include "logger.h"

////////////////////
// CShaderManager //
////////////////////

CShaderManager::CShaderManager( CGame *pGameHandle ) : m_pGameHandle( pGameHandle )
{
}
CShaderManager::~CShaderManager()
{
}

bool CShaderManager::initialize()
{
	return true;
}
void CShaderManager::shutdown()
{
}

bool CShaderManager::loadShaders()
{
	boost::filesystem::path shaderDefPath;
	boost::property_tree::ptree shaderDefs;

	std::unordered_set<std::string> shaderStages;

	m_pGameHandle->getLogger()->print( "Loading shaders from %s...", SHADER_DEF_FILE );

	// Load and attempt to read the JSON file
	shaderDefPath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_SHADERS, SHADER_DEF_FILE );
	if( !boost::filesystem::is_regular_file( shaderDefPath ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to load %s file, or the file was invalid", SHADER_DEF_FILE );
		return false;
	}
	try {
		boost::property_tree::read_json( shaderDefPath.string(), shaderDefs );
	}
	catch( const boost::property_tree::json_parser_error &e ) {
		m_pGameHandle->getLogger()->printError( "Failed to load %s: %s", SHADER_DEF_FILE, e.what() );
	}

	// Iterate through each shader definition
	for( auto it: shaderDefs )
	{
		std::string vertShaderStr, tessCShaderStr, tessEShaderStr, geomShaderStr, fragShaderStr;

		// Make sure the name exists
		if( it.first.empty() ) {
			m_pGameHandle->getLogger()->printError( "Shader program in %s has a missing or invalid name", SHADER_DEF_FILE );
			return false;
		}
		// Add it to the list of shaders to compile

		// Determine which shaders are supported and can be loaded
		// Vertex shader, required support
		vertShaderStr = it.second.get<std::string>( "VertexShader", "" );
		if( !vertShaderStr.empty() ) {
			m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.vert...", vertShaderStr.c_str() );
			shaderStages.insert( (vertShaderStr + "/*.vert") );
		}
		// Tessellation shader, non-required support
		if( glewIsSupported( "GL_ARB_tessellation_shader" ) )
		{
			tessCShaderStr = it.second.get<std::string>( "TessellationControlShader", "" );
			if( !tessCShaderStr.empty() ) {
				m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.tesc...", tessCShaderStr.c_str() );
				shaderStages.insert( (tessCShaderStr + "/*.tesc") );
			}

			tessEShaderStr = it.second.get<std::string>( "TessellationEvalShader", "" );
			if( !tessEShaderStr.empty() ) {
				m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.tese...", tessEShaderStr.c_str() );
				shaderStages.insert( (tessEShaderStr + "/*.tese") );
			}
		}
		else
			m_pGameHandle->getLogger()->print( "No tessellation shader support, ignoring!" );
		// Geometry shader, required support
		geomShaderStr = it.second.get<std::string>( "GeometryShader", "" );
		if( !geomShaderStr.empty() ) {
			m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.geom...", geomShaderStr.c_str() );
			shaderStages.insert( (geomShaderStr + "/*.geom") );
		}
		// Fragment shader, required support
		fragShaderStr = it.second.get<std::string>( "FragmentShader", "" );
		if( !fragShaderStr.empty() ) {
			m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.frag...", fragShaderStr.c_str() );
			shaderStages.insert( (fragShaderStr + "/*.frag") );
		}
	}

	// Load and compile the shader stages
	if( !this->compileShaderStages( shaderStages ) )
		return false;

	return true;
}

bool CShaderManager::compileShaderStages( std::unordered_set<std::string> &shaderStages )
{
	for( auto it: shaderStages )
	{
		// Make sure this stage isnt already stored in memory
		if( m_shaderStageObjects.find( it ) != m_shaderStageObjects.end() ) {
			m_pGameHandle->getLogger()->printWarn( "Shader stage %s tried to compile twice, ignoring second time\n", it.c_str() );
			continue;
		}

		// Determine type
		std::string extensionType = it.substr( it.length() - 4 );
		GLenum shaderType;
		if( extensionType.compare( "vert" ) == 0 )
			shaderType = GL_VERTEX_SHADER;
		else if( extensionType.compare( "tesc" ) == 0 )
			shaderType = GL_TESS_CONTROL_SHADER;
		else if( extensionType.compare( "tese" ) == 0 )
			shaderType = GL_TESS_EVALUATION_SHADER;
		else if( extensionType.compare( "geom" ) == 0 )
			shaderType = GL_GEOMETRY_SHADER;
		else if( extensionType.compare( "frag" ) == 0 )
			shaderType = GL_FRAGMENT_SHADER;
		else {
			// shouldnt happen
			m_pGameHandle->getLogger()->printError( "Invalid shader stage type" );
			return false;
		}

		// Extract shader name
		size_t shaderNameEnd = it.find_last_of( '/' );
		if( shaderNameEnd == std::string::npos ) {
			// shouldnt happen
			m_pGameHandle->getLogger()->printError( "Invalid shader stage name" );
			return false;
		}
		std::string shaderName = it.substr( 0, shaderNameEnd );

		// Create the shader stage object
		std::shared_ptr<CShaderStage> shaderStage = std::make_shared<CShaderStage>( m_pGameHandle, shaderName, shaderType );

		if( !shaderStage->compileFromFile( it ) )
			return false;
	}

	return true;
}

////////////////////
// CShaderProgram //
////////////////////

CShaderProgram::CShaderProgram()
{
}
CShaderProgram::~CShaderProgram()
{
}

//////////////////
// CShaderStage //
//////////////////

const char* CShaderStage::getShaderTypeStr( GLenum type )
{
	switch( type )
	{
	case GL_VERTEX_SHADER:
		return "VERT";
	case GL_TESS_CONTROL_SHADER:
		return "TESS CTRL";
	case GL_TESS_EVALUATION_SHADER:
		return "TESS EVAL";
	case GL_GEOMETRY_SHADER:
		return "GEOM";
	case GL_FRAGMENT_SHADER:
		return "FRAG";
	default:
		return "INVALID";
	}
}

CShaderStage::CShaderStage( CGame *pGameHandle, std::string shaderName, GLenum shaderType ) : m_pGameHandle( pGameHandle )
{
	m_shaderName = shaderName;
	m_shaderType = shaderType;

	m_shaderObjectId = 0;
}
CShaderStage::~CShaderStage()
{
	this->deleteShader();
}

void CShaderStage::deleteShader()
{
	if( m_shaderObjectId ) {
		glDeleteShader( m_shaderObjectId );
		m_shaderObjectId = 0;
	}
}

bool CShaderStage::compileFromFile( std::string relPath )
{
	boost::filesystem::path absFilePath;
	boost::filesystem::path shaderDir;
	std::string extension;
	std::ifstream fileIn;
	std::string glslCodeStr;
	GLint glStatus;

	assert( !relPath.empty() );

	m_pGameHandle->getLogger()->print( "Compiling shader from file %s...", relPath.c_str() );

	// Construct file path
	absFilePath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_SHADERS, relPath );

	// Get the shader stage directory
	shaderDir = absFilePath.parent_path();
	if( !boost::filesystem::is_directory( shaderDir ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to compile shader stage: shader directory %s was not found", m_shaderName.c_str() );
		return false;
	}

	// Get the extension
	size_t extensionStart;
	extensionStart = relPath.find_last_of( '.' );
	if( extensionStart == std::string::npos ) {
		// shouldnt happen
		m_pGameHandle->getLogger()->printError( "Failed to compile shader stage: invalid shader stage file extension" );
		return false;
	}
	extension = relPath.substr( extensionStart );
	boost::algorithm::to_lower( extension );
	// Find all files matching extension
	absFilePath = "";
	for( auto& entry : boost::make_iterator_range( boost::filesystem::directory_iterator( shaderDir ), {} ) )
	{
		if( !boost::filesystem::is_regular_file( entry ) )
			continue;
		// Check for extension match
		if( boost::algorithm::to_lower_copy( entry.path().extension().string() ).compare( extension ) == 0 ) {
			// use this file
			absFilePath = entry.path();
			break;
		}
	}
	if( absFilePath.empty() ) {
		m_pGameHandle->getLogger()->printError( "Failed to compile shader stage: file was missing or invalid" );
		return false;
	}

	// Read the entire contents of the file into a string
	fileIn.open( absFilePath.string(), std::ifstream::in );
	if( fileIn.is_open() )
	{
		fileIn.seekg( 0, std::ios::end );
		glslCodeStr.reserve( fileIn.tellg() );
		fileIn.seekg( 0, std::ios::beg );

		// Read into string object
		glslCodeStr.assign( (std::istreambuf_iterator<char>( fileIn )), std::istreambuf_iterator<char>() );
		if( glslCodeStr.empty() ) {
			m_pGameHandle->getLogger()->printError( "Failed to compile shader stage: file was empty." );
			return false;
		}
	}
	else {
		m_pGameHandle->getLogger()->printError( "Failed to compile shader stage: failed to read file." );
		return false;
	}

	// Create the opengl shader stage object if necessary
	if( !m_shaderObjectId ) {
		m_shaderObjectId = glCreateShader( m_shaderType );
		if( !m_shaderObjectId ) {
			m_pGameHandle->getLogger()->printError( "Failed to create GL shader object, GL error code: %u", glGetError() );
			return false;
		}
	}
	else
		m_pGameHandle->getLogger()->printWarn( "Shader object was already created, it will be overwritten." );

	// Compile the shader
	const GLchar* pShaderSrc = glslCodeStr.c_str();
	glShaderSource( m_shaderObjectId, 1, &pShaderSrc, 0 );
	glCompileShader( m_shaderObjectId );
	// Check if compiliation was successful
	glGetShaderiv( m_shaderObjectId, GL_COMPILE_STATUS, &glStatus );
	if( glStatus == GL_FALSE )
	{
		GLint logLength = 0;
		std::string shaderCompileLog;

		// Get the shader log
		glGetShaderiv( m_shaderObjectId, GL_INFO_LOG_LENGTH, &logLength );
		if( logLength > 0 )
		{
			shaderCompileLog.resize( logLength );
			glGetShaderInfoLog( m_shaderObjectId, logLength, &logLength, &shaderCompileLog[0] );
			// Dump to a file
			if( !shaderCompileLog.empty() )
			{
				std::ofstream fileOut;
				boost::filesystem::path logFilePath;
				logFilePath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_SHADERS, SHADER_COMPILE_LOG );
				fileOut.open( logFilePath.c_str(), std::ios::out | std::ofstream::trunc );
				if( fileOut.is_open() )
				{
					auto time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
					fileOut << "Shader Compile Info Log: ";
					fileOut << std::ctime( &time );
					fileOut << "Shader " << absFilePath.string() << " failed to compile\n";
					fileOut << shaderCompileLog;
				}
				else
					m_pGameHandle->getLogger()->print( "Failed to write shader compile log to %s", SHADER_COMPILE_LOG );
			}
		}
		return false;
	}
	return true;
}