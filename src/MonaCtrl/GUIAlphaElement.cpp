#include "EasyGL.h"

GUIAlphaElement::GUIAlphaElement(const String& callback) :  GUIRectangle(callback) {
	setColor(225.0f, 225.0f, 225.0f);
	alphaFadeScale =     1.0;
	minAlpha       =     0.6f;
	color.w        = minAlpha;
}

bool GUIAlphaElement::LoadXMLSettings(const TiXmlElement* element) {
	if (!element)
		return false;

	setAlphaFadeScale(XMLArbiter::fillComponents1f(element, "alphaFadeScale", alphaFadeScale));
	setMinAlpha(XMLArbiter::fillComponents1f(element, "minAlpha", minAlpha));

	for (const TiXmlElement* outer = element->FirstChildElement();
		 outer;
		 outer = outer->NextSiblingElement() ) {
		const char* value = outer->Value();

		if (value) {
			if (!strcmp(value, "Color"))
				XMLArbiter::fillComponents4f(outer, color);

			if (!strcmp(value, "Text"))
				label.LoadXMLSettings(outer);
		}
	}

	setColor(color.x, color.y, color.z);
	return   GUIRectangle::LoadXMLSettings(element);
}

void  GUIAlphaElement::setColor(const Tuple3f& color) {
	setColor(color.x, color.y, color.z);
}

void GUIAlphaElement::setColor(double r, double g, double b) {
	color.Set(clamp(r, 0.0f, 255.0f),
			  clamp(g, 0.0f, 255.0f),
			  clamp(b, 0.0f, 255.0f),
			  clamp(color.w, 0.0f, 1.0f));
	color.x /= (color.x > 1.0) ? 255.0f : 1.0;
	color.y /= (color.y > 1.0) ? 255.0f : 1.0;
	color.z /= (color.z > 1.0) ? 255.0f : 1.0;
}

const Tuple4f& GUIAlphaElement::getColor() {
	return color;
}

void  GUIAlphaElement::setLabelString(const String& labelArg) {
	label = labelArg;
}

GUIText* GUIAlphaElement::getLabel() {
	return &label;
}

const String& GUIAlphaElement::getLabelString() {
	return label.getString();
}

void  GUIAlphaElement::setAlpha(double alphaArg) {
	color.w = clamp(alphaArg, minAlpha, 1.0f);
}

double GUIAlphaElement::getAlpha() {
	return color.w;
}

void  GUIAlphaElement::setAlphaFadeScale(double duration) {
	alphaFadeScale = clamp(duration, 0.0f, 1000.0f);
}

double GUIAlphaElement::getAlphaFadeScale() {
	return alphaFadeScale;
}

void  GUIAlphaElement::setMinAlpha(double minAlphaArg) {
	minAlpha = clamp(minAlphaArg, 0.2f, 1.0f);
}

double GUIAlphaElement::getMinAlpha() {
	return minAlpha;
}

void GUIAlphaElement::modifyCurrentAlpha(double clockTick) {
	if (!mouseOver && !pressed)
		setAlpha(color.w - clockTick * alphaFadeScale);
	else
		setAlpha(color.w + clockTick * alphaFadeScale);
}
