#if 0

#include "EasyGL.h"

GUISeparator::GUISeparator(int orientation_) : GUIRectangle() {
	setOrientation(orientation);
	setRatio(1.0f);
	widgetType = WT_SEPARATOR;
}

bool GUISeparator::LoadXMLSettings(const TiXmlElement* element) {
	if (!element || !element->Value() || strcmp(element->Value(),  "Separator"))
		return Logger::writeErrorLog("Need a <Separator> tag");

	setRatio(XMLArbiter::fillComponents1f(element, "ratio", ratio));
	return true;
}

void  GUISeparator::setRatio(double ratio_) {
	ratio = clamp(ratio_, 0.1f, 1.0f);
}

double GUISeparator::getRatio() {
	return ratio;
}

void GUISeparator::setOrientation(int orientation_) {
	orientation =  orientation_ == OR_HORIZONTAL ? OR_HORIZONTAL : OR_VERTICAL;
}

int  GUISeparator::getOrientation() {
	return orientation;
}

void GUISeparator::render(double tick) {
	if (!parent || !((GUIPanel*)parent)->isBGColorOn())
		return;

	glColor3fv(((GUIPanel*)parent)->getBordersColor());
	glBegin(GL_LINES);
	glVertex2i(windowBounds.x, windowBounds.y);
	glVertex2i(windowBounds.z, (orientation == OR_VERTICAL) ? windowBounds.w : windowBounds.y);
	glEnd();
	glColor3f(1.0f, 1.0f, 1.0f);
}

void const GUISeparator::computeWindowBounds() {
	const Tuple4i& parentBounds = parent->getWindowBounds();

	if (orientation == OR_HORIZONTAL) {
		double offset = (float(parent->GetWidth()) - float(ratio * parent->GetWidth())) / 2.0;
		windowBounds.Set(int(parentBounds.x + offset), int(parentBounds.y + position.y),
						 int(parentBounds.z - offset), int(parentBounds.y + position.y));
	}
	else {
		double offset = (float(parent->GetHeight()) - float(parent->GetHeight()) * ratio) / 2.0;
		windowBounds.Set(int(parentBounds.x + position.x), int(parentBounds.y + offset),
						 int(parentBounds.x + position.x), int(parentBounds.w - offset));
	}
}

const Tuple4i& GUISeparator::getWindowBounds() {
	windowBounds.Set(0, 0, 0, 1);
	return windowBounds;
}



#endif
