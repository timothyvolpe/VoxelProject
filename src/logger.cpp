#include "logger.h"

CLogger::CLogger() {
}
CLogger::~CLogger() {
}

bool CLogger::flush()
{
	return true;
}

bool CLogger::start()
{
	return true;
}
void CLogger::stop()
{
	this->flush();
}

bool CLogger::update()
{
	return true;
}