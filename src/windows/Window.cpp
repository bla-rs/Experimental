#include "Window.h"
#include "../system/utility/Templates.h"

violet::Window::Window(float x, float y, int w, int h, float r, float g, float b,
		float a) {
	m_left = x;
	m_top = y;
	m_right = x + w;
	m_bottom = y + h;
	m_r = r;
	m_g = g;
	m_b = b;
	m_a = a;
	CloseFlag = false;
}

void violet::Window::addElement(std::string id, TextObject* element) 
{
	removeElement(id, true);
	m_elements[id] = element;
}

void violet::Window::addElement(std::string id, std::string text, 
	TextManager* manager, int x, int y, TextManager::TextHAlignFlag halign, 
	TextManager::TextVAlignFlag valign) {
	removeElement(id, true);
	TextObject* element = manager->getObject(text, x, y, halign, valign);
	m_elements[id] = element;
}

inline void violet::Window::addElement(Label label, TextManager* manager, int x, int y, 
	TextManager::TextHAlignFlag halign, 
	TextManager::TextVAlignFlag valign) {
	addElement(label.id, label.text, manager, x, y, halign, valign);
}


void violet::Window::addElements(const std::vector<Label>& labels, 
	TextManager* manager, int x, int y, int vstep, 
	TextManager::TextHAlignFlag halign, TextManager::TextVAlignFlag valign) {
	
	for (unsigned i = 0; i < labels.size(); ++i)
		addElement(labels[i], manager, x, y+i*vstep, halign, valign);
}

void violet::Window::removeElement(std::string name, bool remainHandler) {
	std::map<std::string, TextObject*>::iterator it = m_elements.find(name);
	if (it != m_elements.end()) {
		delete it->second;
		m_elements.erase(it);
	}
	if (!remainHandler)
		removeHandler(hdl_click, name);
}

void violet::Window::addHandler(HandlerType hdl, std::string elementName, boost::function<void (std::string)> handler) {
	removeHandler(hdl, elementName);
	if (hdl == hdl_click || hdl == hdl_lclick)
		m_lcHandlers[elementName] = handler;
	if (hdl == hdl_click || hdl == hdl_rclick)
		m_rcHandlers[elementName] = handler;
	if (hdl == hdl_move)
		m_mvHandlers[elementName] = handler;
}

void violet::Window::removeHandler(HandlerType hdl, std::string elementName) {
	if (hdl == hdl_all || hdl == hdl_click || hdl == hdl_lclick) {
		std::map<std::string, boost::function<void (std::string)> >::iterator it = m_lcHandlers.find(elementName);
		if (it != m_lcHandlers.end())
			m_lcHandlers.erase(elementName);
	}
	if (hdl == hdl_all || hdl == hdl_click || hdl == hdl_rclick) {
		std::map<std::string, boost::function<void (std::string)> >::iterator it = m_rcHandlers.find(elementName);
		if (it != m_rcHandlers.end())
			m_rcHandlers.erase(elementName);
	}
	if (hdl == hdl_all || hdl == hdl_move) {
		std::map<std::string, boost::function<void (std::string)> >::iterator it = m_mvHandlers.find(elementName);
		if (it != m_mvHandlers.end())
			m_mvHandlers.erase(elementName);
	}
}

void violet::Window::draw() {
	glDisable( GL_TEXTURE_2D);

	glPushMatrix();

	glLoadIdentity();

	glBegin( GL_QUADS);

	glNormal3f(0.0f, 0.0f, 1.0f);

	glColor4f(m_r, m_g, m_b, m_a);

	glVertex3f(m_left, m_top, 0.0f);
	glVertex3f(m_right, m_top, 0.0f);
	glVertex3f(m_right, m_bottom, 0.0f);
	glVertex3f(m_left, m_bottom, 0.0f);

	glEnd();

	glPopMatrix();

	glEnable(GL_TEXTURE_2D);

	std::map<std::string, TextObject*>::const_iterator iter;
	for (iter = m_elements.begin(); iter != m_elements.end(); ++iter) {
		TextObject* o = iter->second;
		o->draw(true, o->X, o->Y);
	}
}

void violet::Window::process(InputHandler* input) {
	int gmx = input->mouseX;
	int gmy = input->mouseY;

	// Mouse hover handlers
	std::map<std::string, boost::function<void (std::string)> >::const_iterator iter;
	
	for (iter = m_mvHandlers.begin(); iter != m_mvHandlers.end(); ++iter) {
		std::map<std::string, TextObject*>::iterator it = m_elements.find(iter->first);
		if (it != m_elements.end()) {
			TextObject* o = it->second;
			if (gmx > o->getLeft() && gmx < o->getRight() && gmy > o->getTop()
					&& gmy < o->getBottom()) {
				iter->second(iter->first);

				// Be careful, callback function (iter->second) can change set of handlers or even replace all handlers and iter will be not incrementable anymore!
				break;
			}
		}
	}

	// Left click handlers
	for (iter = m_lcHandlers.begin(); iter != m_lcHandlers.end(); ++iter) {
		std::map<std::string, TextObject*>::iterator it = m_elements.find(iter->first);
		if (it != m_elements.end()) {
			TextObject* o = it->second;
			if (gmx > o->getLeft() && gmx < o->getRight() && gmy > o->getTop()
					&& gmy < o->getBottom()) {
				o->GMask = 0.3f;
				if (input->getPressInput(InputHandler::MenuClickA)) {
					iter->second(iter->first);
					
					// Be careful, callback function (iter->second) can change set of handlers or even replace all handlers and iter will be not incrementable anymore!
					break;
				}
			} else {
				o->GMask = 1.0f;
			}
		}
	}

	// Right click handlers
	for (iter = m_rcHandlers.begin(); iter != m_rcHandlers.end(); ++iter) {
		std::map<std::string, TextObject*>::iterator it = m_elements.find(iter->first);
		if (it != m_elements.end()) {
			TextObject* o = it->second;
			if (gmx > o->getLeft() && gmx < o->getRight() && gmy > o->getTop()
					&& gmy < o->getBottom()) {
				o->RMask = 0.3f;
				if (input->getPressInput(InputHandler::MenuClickB)) {
					iter->second(iter->first);

					// Be careful, callback function (iter->second) can change set of handlers or even replace all handlers and iter will be not incrementable anymore!
					break;
				}
			} else {
				o->RMask = 1.0f;
			}
		}
	}
}

violet::Window::~Window() {
	clearMap(&m_elements);
	m_lcHandlers.clear();
	m_rcHandlers.clear();
	m_mvHandlers.clear();
}
