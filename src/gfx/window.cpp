#include "gfx\window.h"

CWindow::CWindow() {
	m_pSDLWindow = 0;
}
CWindow::~CWindow() {
}

bool CWindow::initialize()
{
	return true;
}
void CWindow::destroy()
{
}

SDL_Window* CWindow::getSDLWindow() {
	return m_pSDLWindow;
}