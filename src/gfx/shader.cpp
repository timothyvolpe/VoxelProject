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
	m_boundProgramIndex = 0;
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

bool CShaderManager::loadPrograms()
{
	boost::filesystem::path shaderDefPath;
	boost::property_tree::ptree shaderDefs;

	std::unordered_set<std::string> shaderStages;
	std::vector<ShaderProgramDefinition> programDefs;

	std::map<std::string, std::shared_ptr<CShaderStage>> compiledStages;

	m_pGameHandle->getLogger()->print( "Loading shader programs from %s...", SHADER_DEF_FILE );

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
		return false;
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

		// Create program definition
		ShaderProgramDefinition programDef;

		programDef.programName = it.first;

		// Determine which shaders are supported and can be loaded
		// Vertex shader, required support
		vertShaderStr = it.second.get<std::string>( "VertexShader", "" );
		if( !vertShaderStr.empty() ) {
			m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.vert...", vertShaderStr.c_str() );
			programDef.vertShader = (vertShaderStr + "/*.vert");
			shaderStages.insert( programDef.vertShader );
		}
		// Tessellation shader, non-required support
		if( glewIsSupported( "GL_ARB_tessellation_shader" ) )
		{
			tessCShaderStr = it.second.get<std::string>( "TessellationControlShader", "" );
			if( !tessCShaderStr.empty() ) {
				m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.tesc...", tessCShaderStr.c_str() );
				programDef.tessControlShader = (tessCShaderStr + "/*.tesc");
				shaderStages.insert( programDef.tessControlShader );
			}

			tessEShaderStr = it.second.get<std::string>( "TessellationEvalShader", "" );
			if( !tessEShaderStr.empty() ) {
				m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.tese...", tessEShaderStr.c_str() );
				programDef.tessEvalShader = (tessEShaderStr + "/*.tese");
				shaderStages.insert( programDef.tessEvalShader );
			}
		}
		else
			m_pGameHandle->getLogger()->print( "No tessellation shader support, ignoring!" );
		// Geometry shader, required support
		geomShaderStr = it.second.get<std::string>( "GeometryShader", "" );
		if( !geomShaderStr.empty() ) {
			m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.geom...", geomShaderStr.c_str() );
			programDef.geomShader = (geomShaderStr + "/*.geom");
			shaderStages.insert( (geomShaderStr + "/*.geom") );
		}
		// Fragment shader, required support
		fragShaderStr = it.second.get<std::string>( "FragmentShader", "" );
		if( !fragShaderStr.empty() ) {
			m_pGameHandle->getLogger()->print( "Loading shader stage %s/*.frag...", fragShaderStr.c_str() );
			programDef.fragShader = (fragShaderStr + "/*.frag");
			shaderStages.insert( programDef.fragShader );
		}

		// Get uniforms
		auto uniformIt = it.second.find( "Uniforms" );
		if( uniformIt != it.second.not_found() )
		{
			if( !(*uniformIt).second.empty() )
			{
				for( auto it2: (*uniformIt).second )
					programDef.uniformNames.push_back( it2.second.get_value<std::string>() );
			}
		}

		programDefs.push_back( programDef );
	}

	// Load and compile the shader stages
	if( !this->compileShaderStages( shaderStages, compiledStages ) )
		return false;
	// Link programs
	if( !this->linkPrograms( compiledStages, programDefs ) )
		return false;
	

	return true;
}

bool CShaderManager::compileShaderStages( const std::unordered_set<std::string> &shaderStages, std::map<std::string, std::shared_ptr<CShaderStage>> &compiledStages )
{
	compiledStages.clear();

	for( auto it: shaderStages )
	{
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

		// Create the shader stage object
		std::shared_ptr<CShaderStage> shaderStage = std::make_shared<CShaderStage>( m_pGameHandle, shaderType );

		if( !shaderStage->createFromFile( it ) )
			return false;
		// Add to list of compiled shaders
		compiledStages.insert( std::pair<std::string, std::shared_ptr<CShaderStage>>( it, shaderStage ) );
	}

	return true;
}

bool CShaderManager::linkPrograms( const std::map<std::string, std::shared_ptr<CShaderStage>> &compiledStages, const std::vector<ShaderProgramDefinition> &programDefs )
{
	// Iterate and link programs
	for( auto it: programDefs )
	{
		std::vector<std::shared_ptr<CShaderStage>> stages;

		m_pGameHandle->getLogger()->print( "Linking shader program %s...", it.programName.c_str() );

		// Load the stages associate with the shader
		// vert shader
		auto it2 = compiledStages.find( it.vertShader );
		if( it2 != compiledStages.end() )
			stages.push_back( (*it2).second );
		// tess control shader
		it2 = compiledStages.find( it.tessControlShader );
		if( it2 != compiledStages.end() )
			stages.push_back( (*it2).second );
		// tess eval shader
		it2 = compiledStages.find( it.tessEvalShader );
		if( it2 != compiledStages.end() )
			stages.push_back( (*it2).second );
		// geom shader
		it2 = compiledStages.find( it.geomShader );
		if( it2 != compiledStages.end() )
			stages.push_back( (*it2).second );
		// frag shader
		it2 = compiledStages.find( it.fragShader );
		if( it2 != compiledStages.end() )
			stages.push_back( (*it2).second );

		// make sure there are stages
		if( stages.empty() ) {
			m_pGameHandle->getLogger()->printError( "Failed to link shader program: program was empty" );
			return false;
		}

		// Create program
		std::shared_ptr<CShaderProgram> shaderProgram = std::make_shared<CShaderProgram>( m_pGameHandle, it.programName );
		if( !shaderProgram->initialize() )
			return false;

		// Attach shaders
		for( auto it: stages )
		{
			if( !it->attachShader( shaderProgram ) )
				return false;
		}
		if( !shaderProgram->link( (it).uniformNames ) )
			return false;

		m_shaderPrograms.push_back( shaderProgram );
		m_programIndexMap.insert( std::pair<std::string, unsigned int>( shaderProgram->getProgramName(), (unsigned int)m_shaderPrograms.size() ) );
	}

	return true;
}

bool CShaderManager::getProgramIndex( std::string programName, unsigned int *pIndex )
{
	auto it = m_programIndexMap.find( programName );
	if( it == m_programIndexMap.end() )
		return false;
	(*pIndex) = (*it).second;
	return true;
}

void CShaderManager::bindProgram( unsigned int programIndex )
{
	if( m_boundProgramIndex != programIndex ) {
		glUseProgram( m_shaderPrograms[programIndex-1]->getProgramId() );
		m_boundProgramIndex = programIndex;
	}
}

std::shared_ptr<CShaderProgram> CShaderManager::getProgramByIndex( unsigned int programIndex ) {
	assert( programIndex <= m_shaderPrograms.size() );
	return m_shaderPrograms[programIndex-1];
}

////////////////////
// CShaderProgram //
////////////////////

CShaderProgram::CShaderProgram( CGame* pGameHandle, std::string programName ) : m_pGameHandle( pGameHandle )
{
	m_programName = programName;
	m_shaderProgramId = 0;
}
CShaderProgram::~CShaderProgram() {
	this->deleteProgram();
}

bool CShaderProgram::initialize()
{
	assert( m_shaderProgramId == 0 );

	m_shaderProgramId = glCreateProgram();
	if( m_shaderProgramId == 0 ) {
		m_pGameHandle->getLogger()->printError( "Failed to create shader program object for shader program %s", m_programName.c_str() );
		return false;
	}
	return true;
}
void CShaderProgram::deleteProgram()
{
	if( m_shaderProgramId ) {
		glDeleteProgram( m_shaderProgramId );
		m_shaderProgramId = 0;
	}
}

bool CShaderProgram::link( std::vector<std::string> uniformNames )
{
	assert( m_shaderProgramId != 0 );

	GLint glStatus;
	GLenum glError;

	glLinkProgram( m_shaderProgramId );
	// Check if link was successful
	glGetProgramiv( m_shaderProgramId, GL_LINK_STATUS, &glStatus );
	if( glStatus == GL_FALSE )
	{
		GLint logLength = 0;
		std::string programLinkLog;

		// Get the shader log
		glGetProgramiv( m_shaderProgramId, GL_INFO_LOG_LENGTH, &logLength );
		if( logLength > 0 )
		{
			programLinkLog.resize( logLength );
			glGetProgramInfoLog( m_shaderProgramId, logLength, &logLength, &programLinkLog[0] );
			// Dump to a file
			if( !programLinkLog.empty() )
			{
				std::ofstream fileOut;
				boost::filesystem::path logFilePath;
				logFilePath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_SHADERS, SHADER_LINK_LOG );
				fileOut.open( logFilePath.c_str(), std::ios::out | std::ofstream::trunc );
				if( fileOut.is_open() )
				{
					auto time = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );
					fileOut << "Shader Program Link Info Log: ";
					fileOut << std::ctime( &time );
					fileOut << "Program " << m_programName.c_str() << " failed to link\n";
					fileOut << programLinkLog;

					m_pGameHandle->getLogger()->print( "Failed to link program %s, see %s", m_programName.c_str(), logFilePath.string().c_str() );
				}
				else
					m_pGameHandle->getLogger()->print( "Failed to write shader program link log to %s", logFilePath.c_str() );
			}
		}
		return false;
	}

	// Find and store uniform locations
	GLint uniformLoc = 0;
	for( auto it = uniformNames.begin(); it != uniformNames.end(); it++ )
	{
		uniformLoc = glGetUniformLocation( m_shaderProgramId, (*it).c_str() );
		if( uniformLoc == -1 ) {
			m_pGameHandle->getLogger()->print( "Failed to find uniform %s in shader program %s", (*it).c_str(), m_programName.c_str() );
			return false;
		}
		if( (glError = glGetError()) != GL_NO_ERROR ) {
			m_pGameHandle->getLogger()->print( "Failed to find uniform %s in shader program %s, GL error code %u", (*it).c_str(), m_programName.c_str(), glError );
			return false;
		}
		m_uniformNameToIndex.insert( std::pair<std::string, size_t>( (*it), m_uniformLocations.size() ) );
		m_uniformLocations.push_back( uniformLoc );
	}

	return true;
}

bool CShaderProgram::getUniformIndex( std::string name, size_t* pIndex )
{
	auto it = m_uniformNameToIndex.find( name );
	if( it == m_uniformNameToIndex.end() )
		return false;
	(*pIndex) = (*it).second;
	return true;
}

GLint CShaderProgram::getUniformLocation( size_t uniformIndex ) {
	assert( uniformIndex < m_uniformLocations.size() );
	return m_uniformLocations[uniformIndex];
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

CShaderStage::CShaderStage( CGame *pGameHandle, GLenum shaderType ) : m_pGameHandle( pGameHandle )
{
	m_shaderType = shaderType;

	m_shaderObjectId = 0;
}
CShaderStage::~CShaderStage()
{
	this->deleteShader();
}

void CShaderStage::deleteShader()
{
	if( m_shaderObjectId )
	{
		this->detachFromAll();
		glDeleteShader( m_shaderObjectId );
		m_shaderObjectId = 0;
	}
}

bool CShaderStage::createFromFile( std::string relPath )
{
	boost::filesystem::path absFilePath;
	boost::filesystem::path shaderDir;
	std::string extension;
	std::ifstream fileIn;
	std::string glslCodeStr;
	GLint glStatus;

	assert( !relPath.empty() );

	m_shaderName = relPath;

	m_pGameHandle->getLogger()->print( "Compiling shader from file %s...", relPath.c_str() );

	// Construct file path
	absFilePath = m_pGameHandle->getFilesystem()->getGamePath( FilesystemLocations::LOCATION_SHADERS, relPath );

	// Get the shader stage directory
	shaderDir = absFilePath.parent_path();
	if( !boost::filesystem::is_directory( shaderDir ) ) {
		m_pGameHandle->getLogger()->printError( "Failed to compile shader stage: shader directory %s was not found", shaderDir.c_str() );
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
	// Check if compilation was successful
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
					fileOut << "Shader " << m_shaderName.c_str() << " (" << CShaderStage::getShaderTypeStr( m_shaderType ) << ") failed to compile\n";
					fileOut << shaderCompileLog;

					m_pGameHandle->getLogger()->print( "Failed to compile shader %s, see %s", m_shaderName.c_str(), logFilePath.string().c_str() );
				}
				else
					m_pGameHandle->getLogger()->print( "Failed to write shader compile log to %s", logFilePath.string().c_str() );
			}
		}
		return false;
	}
	return true;
}

bool CShaderStage::attachShader( std::shared_ptr<CShaderProgram> shaderProgram )
{
	assert( shaderProgram->getProgramId() != 0 );
	assert( m_shaderObjectId != 0 );

	GLenum glError;

	// Make sure it is not already attached
	if( std::find( m_programsAttachedTo.begin(), m_programsAttachedTo.end(), shaderProgram->getProgramId() ) != m_programsAttachedTo.end() ) {
		m_pGameHandle->getLogger()->printWarn( "Attempted to attach shader stage %s (%s) to program %s more than once!", m_shaderName, CShaderStage::getShaderTypeStr( m_shaderType ), shaderProgram->getProgramName().c_str() );
		return true;
	}

	glAttachShader( shaderProgram->getProgramId(), m_shaderObjectId );
	if( (glError = glGetError()) != GL_NO_ERROR ) {
		m_pGameHandle->getLogger()->printError( "Failed to attach shader stage to shader program, GL error code: %u", glError );
		return false;
	}
	m_programsAttachedTo.push_back( shaderProgram->getProgramId() );

	return true;
}
void CShaderStage::detachShader( std::shared_ptr<CShaderProgram> shaderProgram )
{
	assert( shaderProgram->getProgramId() != 0 );
	assert( m_shaderObjectId != 0 );

	GLenum glError;

	// make sure it is attached
	auto it = std::find( m_programsAttachedTo.begin(), m_programsAttachedTo.end(), shaderProgram->getProgramId() );
	if( it == m_programsAttachedTo.end() ) {
		m_pGameHandle->getLogger()->printWarn( "Attempted to deattach shader stage %s (%s) from program %s when it was not attached!", m_shaderName, CShaderStage::getShaderTypeStr( m_shaderType ), shaderProgram->getProgramName().c_str() );
		return;
	}

	glDetachShader( shaderProgram->getProgramId(), m_shaderObjectId );
	if( (glError = glGetError()) != GL_NO_ERROR ) {
		m_pGameHandle->getLogger()->printError( "Failed to deattach shader stage from shader program, GL error code: %u", glError );
		return;
	}
	m_programsAttachedTo.erase( it );
}
void CShaderStage::detachFromAll()
{
	assert( m_shaderObjectId != 0 );

	GLenum glError;

	for( auto it: m_programsAttachedTo ) {
		glDetachShader( (it), m_shaderObjectId );
		if( (glError = glGetError()) != GL_NO_ERROR )
			m_pGameHandle->getLogger()->printError( "Failed to deattach shader stage from shader program, GL error code: %u", glError );
	}
	m_programsAttachedTo.clear();
}