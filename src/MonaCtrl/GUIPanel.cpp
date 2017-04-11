#if 0

#include "EasyGL.h"

GUIPanel::GUIPanel(const String& cbs) : GUIRectangle(cbs), GUIClippedRectangle() {
	setInterval(4, 4);
	widgetType = WT_PANEL;
	layout     = PL_FREE_LAYOUT;
}

bool GUIPanel::LoadXMLSettings(const char* stackPath) {
	String xmlFile = MediaPathManager::lookUpMediaPath(stackPath);
	bool        result = false;

	if (!xmlFile.GetCount())
		return Logger::writeErrorLog("Failed to locate the XML GUI file");

	TiXmlDocument cfgStack(xmlFile);

	if (!cfgStack.LoadFile())
		return  Logger::writeErrorLog("Invalid XML GUI description file");

	result = LoadXMLSettings(cfgStack.FirstChildElement());
	forceUpdate(true);
	return result;
}

bool GUIPanel::LoadXMLSettings(const TiXmlElement* element) {
	if (!XMLArbiter::inspectElementInfo(element, "Panel"))
		return Logger::writeErrorLog("Need a Panel node in the xml file");

	const char*  description = element->Attribute("description");
	String  type        = element->Attribute("layout") ? element->Attribute("layout") : "UNKNOWN";

	if (description)
		return  LoadXMLSettings(description);

	layout = (type == "CEN_YAXIS") ? PL_YAXIS_CEN_LAYOUT :
			 (type == "YAXIS"    ) ? PL_YAXIS_LAYOUT     :
			 (type == "XAXIS"    ) ? PL_XAXIS_LAYOUT     :
			 (type == "GRID"     ) ? PL_GRID_LAYOUT      : PL_FREE_LAYOUT;

	for (const TiXmlElement* outer = element->FirstChildElement();
		 outer;
		 outer = outer->NextSiblingElement() ) {
		String outerName  = outer->Value();

		if (!outerName.GetCount())
			continue;

		if (outerName == "CheckBox") {
			GUICheckBox* newCheckBox = new GUICheckBox();

			if (!newCheckBox->LoadXMLSettings(outer) || !addWidget(newCheckBox))
				deleteObject(newCheckBox);

			continue;
		}

		if (outerName == "TabbedPanel") {
			GUITabbedPanel* newTabbedPanel = new GUITabbedPanel();

			if (!newTabbedPanel->LoadXMLSettings(outer) || !addWidget(newTabbedPanel))
				deleteObject(newTabbedPanel);

			continue;
		}

		if (outerName == "RadioButton") {
			GUIRadioButton* newRadioButton = new GUIRadioButton();

			if (!newRadioButton->LoadXMLSettings(outer) || !addWidget(newRadioButton))
				deleteObject(newRadioButton);
			else {
				if (newRadioButton->isChecked())
					notify(newRadioButton);
			}

			continue;
		}

		if (outerName == "ComboBox") {
			GUIComboBox* newComboBox = new GUIComboBox();

			if (!newComboBox->LoadXMLSettings(outer) || !addWidget(newComboBox))
				deleteObject(newComboBox);

			continue;
		}

		if (outerName == "TextBox") {
			GUITextBox* newTextBox = new GUITextBox();

			if (!newTextBox->LoadXMLSettings(outer) || !addWidget(newTextBox))
				deleteObject(newTextBox);

			continue;
		}

		if (outerName == "Slider") {
			GUISlider* newSlider = new GUISlider();

			if (!newSlider->LoadXMLSettings(outer) || !addWidget(newSlider))
				deleteObject(newSlider);

			continue;
		}

		if (outerName == "Separator") {
			GUISeparator* newSeparator = new GUISeparator();

			if ((layout != PL_YAXIS_LAYOUT &&
				 layout != PL_XAXIS_LAYOUT &&
				 layout != PL_YAXIS_CEN_LAYOUT) ||
				(!newSeparator->LoadXMLSettings(outer)))
				deleteObject(newSeparator);
			else {
				newSeparator->setParent(this);
				newSeparator->setOrientation((layout != PL_YAXIS_LAYOUT && layout != PL_YAXIS_CEN_LAYOUT));
				elements.Add(newSeparator);
			}

			continue;
		}

		if (outerName == "Button") {
			GUIButton* newButton = new GUIButton();

			if (!newButton->LoadXMLSettings(outer) || !addWidget(newButton))
				deleteObject(newButton);

			continue;
		}

		if (outerName == "Label") {
			GUILabel* newLabel = new GUILabel();

			if (!newLabel->LoadXMLSettings(outer) || !addWidget(newLabel))
				deleteObject(newLabel);

			continue;
		}

		if (outerName == "Interval")
			setInterval(XMLArbiter::fillComponents2i(outer, interval));

		if (outerName == "Panel") {
			GUIPanel* panel = new GUIPanel();

			if (!panel->LoadXMLSettings(outer) || !addWidget(panel))
				deleteObject(panel);

			continue;
		}
	}

	return  GUIRectangle::LoadXMLSettings(element)  &&
			GUIClippedRectangle::loadXMLClippedRectangleInfo(element);
}

GUIRectangle* GUIPanel::getWidgetByCallbackString(const String& callbackString) {
	if (!callbackString.GetCount())
		return NULL;

	GUIRectangle* element = NULL,
				  *tPanel  = NULL;
	size_t t = 0;

	for (t = 0; t < elements.GetCount(); t++) {
		if (elements[t]->getCallbackString() == callbackString)
			return elements[t];
	}

	for (t = 0; t < elements.GetCount(); t++) {
		if (elements[t]->getWidgetType() == WT_PANEL)
			if (element = ((GUIPanel*)elements[t])->getWidgetByCallbackString(callbackString))
				break;

		if (elements[t]->getWidgetType() == WT_TABBED_PANEL)
			if (element = ((GUITabbedPanel*)elements[t])->getLowerPanel()->getWidgetByCallbackString(callbackString))
				break;
	}

	return element;
}

void GUIPanel::checkMouseEvents(MouseEvent& evt, int extraInfo, bool infoBits) {
	if (visible && active) {
		GUIRectangle::checkMouseEvents(evt, extraInfo, infoBits);

		for (size_t t = 0; t < elements.GetCount(); t++) {
			if (elements[t]->isActive())
				elements[t]->checkMouseEvents(evt, extraInfo, infoBits);
		}
	}
}
void GUIPanel::checkKeyboardEvents(KeyEvent evt, int extraInfo) {
	if (visible && active) {
		GUIRectangle::checkKeyboardEvents(evt, extraInfo);

		for (size_t t = 0; t < elements.GetCount(); t++)
			if (elements[t]->isActive())
				elements[t]->checkKeyboardEvents(evt, extraInfo);
	}
}

void  GUIPanel::render(double tick) {
	if (!isAttached() || !parent)
		return;

	Widgets comboBoxes;
	GUIComboBox* cbPTR;

	if (visible) {
		renderClippedBounds();

		for (int t = (int)elements.GetCount() - 1; t >= 0; t--)
			if (elements[t]->getWidgetType() != WT_COMBO_BOX)
				elements[t]->render(tick);
			else {
				cbPTR = (GUIComboBox*)elements[t];

				if (!cbPTR->isDeployed())
					cbPTR->render(tick);
				else
					comboBoxes.Add(elements[t]);
			}

		for (size_t t = 0; t < comboBoxes.GetCount(); t++)
			comboBoxes[t]->render(tick);
	}
}

void  GUIPanel::collectZWidgets(ZWidgets& container) {
	if (visible) {
		for (size_t t = 0; t < elements.GetCount(); t++)
			if (elements[t]->getWidgetType() != WT_PANEL) {
				ZWidget zWidget(elements[t]);
				zWidget.setDistance(float(elements[t]->getZCoordinate()));
				container.Add(zWidget);
			}
			else
				((GUIPanel*)elements[t])->collectZWidgets(container);

		ZWidget zWidget(this);
		zWidget.setDistance(float(getZCoordinate()));
		container.Add(zWidget);
	}
}

void GUIPanel::pack() {
	if (!update)
		return;

	size_t t = 0;

	for (t = 0; t < elements.GetCount(); t++)
		elements[t]->forceUpdate(update);

	if (!t) {
		correctPosition();
		return;
	}

	switch (layout) {
	case PL_YAXIS_LAYOUT:
	case PL_YAXIS_CEN_LAYOUT:
		packYAxisLayout();
		break;

	case PL_XAXIS_LAYOUT:
		packXAxisLayout();
		break;

	default:
		packFreeLayout();
	}

	for (t = 0; t < elements.GetCount(); t++)
		elements[t]->forceUpdate(update);
}

void GUIPanel::forceUpdate(bool updateArg) {
	update = updateArg;
	pack();
}

void GUIPanel::packYAxisLayout() {
	computeWindowBounds();
	Vector<int> outerHeights,
		   outerWidths;
	size_t      t          = 0;
	int         height     = 0,
				xOffset    = 0,
				panelWidth = GetWidth();

	for (t = 0; t < elements.GetCount(); t++) {
		const Tuple4i& outerBounds = elements[t]->getWindowBounds();
		outerHeights.Add(outerBounds.w - outerBounds.y + interval.y);
		outerWidths.Add(outerBounds.z - outerBounds.x);
		height       += outerHeights[t];
		panelWidth    = outerWidths[t] > panelWidth ? outerWidths[t] : panelWidth;
	}

	dimensions.Set(float(panelWidth), float(height));
	GUIRectangle::computeWindowBounds();
	windowBounds.z += interval.x * 2;
	windowBounds.w += interval.y;
	update = false;
	height = interval.y;
	correctPosition();
	computeClippedBounds(windowBounds);

	for (t = 0; t < elements.GetCount(); t++) {
		xOffset = (layout == PL_YAXIS_CEN_LAYOUT) * (panelWidth - outerWidths[t]) / 2;
		elements[t]->setAnchorPoint(AT_CORNERLU);
		elements[t]->setPosition(float(interval.x + xOffset), float(height));
		elements[t]->computeWindowBounds();
		height += outerHeights[t];
	}
}

void GUIPanel::packXAxisLayout() {
	computeWindowBounds();
	Vector<int> outerWidths,
		   outerHeights;
	double       offset      = 0;
	size_t      t           = 0;
	int         height      = 0,
				width       = 0,
				panelHeight = windowBounds.w - windowBounds.y;

	for (t = 0; t < elements.GetCount(); t++) {
		const Tuple4i& outerBounds = elements[t]->getWindowBounds();
		outerHeights.Add(outerBounds.w - outerBounds.y);
		outerWidths.Add (outerBounds.z - outerBounds.x + interval.x);
		width       += outerWidths[t];
		height       = outerBounds.w - outerBounds.y;
		panelHeight  = height > panelHeight ? height : panelHeight;
	}

	dimensions.Set(float(width), float(panelHeight));
	GUIRectangle::computeWindowBounds();
	windowBounds.z += interval.x;
	windowBounds.w += interval.y * 2;
	update = false;
	width  = interval.x;
	correctPosition();
	computeClippedBounds(windowBounds);

	for (t = 0; t < elements.GetCount(); t++) {
		offset = clamp(float(panelHeight - outerHeights[t]) / 2.0f + interval.y, 0.0f, 1000.0f);
		elements[t]->setAnchorPoint(AT_CORNERLU);
		elements[t]->setPosition(float(width), offset);
		elements[t]->computeWindowBounds();
		width += outerWidths[t];
	}
}

void GUIPanel::packFreeLayout() {
	if (!update || !parent)
		return;

	Tuple4i newBounds = getWindowBounds();
	size_t  t         = 0;
	int     temp1     = 0,
			temp2     = 0;

	for (t = 0; t < elements.GetCount(); t++) {
		elements[t]->forceUpdate(update);
		const Tuple4i& outerBounds = elements[t]->getWindowBounds();
		newBounds.x = (outerBounds.x - clipSize) < newBounds.x ?
					  outerBounds.x - clipSize : newBounds.x;
		newBounds.y = (outerBounds.y - clipSize) < newBounds.y ?
					  outerBounds.y - clipSize : newBounds.y;
		newBounds.z = (outerBounds.z + clipSize) > newBounds.z ?
					  outerBounds.z + clipSize : newBounds.z;
		newBounds.w = (outerBounds.w + clipSize) > newBounds.w ?
					  outerBounds.w + clipSize : newBounds.w;
	}

	windowBounds = newBounds;
	update       = false;
	correctPosition();
	computeClippedBounds(windowBounds);

	for (t = 0; t < elements.GetCount(); t++)
		elements[t]->computeWindowBounds();
}

void GUIPanel::correctPosition() {
	if (!parent)
		return;

	const Tuple4i&  parentBounds = parent->getWindowBounds();
	bool            update       = false;
	int             temp         = windowBounds.z - windowBounds.x;

	if (position.x < 0)
		if (parentBounds.z - windowBounds.z < abs(position.x)) {
			windowBounds.z  = parentBounds.z + int(position.x);
			windowBounds.x  = windowBounds.z - temp;
			update          = true;
		}

	if (position.x > 1)
		if (windowBounds.x <  position.x + parentBounds.x) {
			windowBounds.x  = parentBounds.x + int(position.x);
			windowBounds.z  = windowBounds.x + temp;
			update          = true;
		}

	if (position.y < 0)
		if (parentBounds.w - windowBounds.w < abs(position.y)) {
			temp            = windowBounds.w - windowBounds.y;
			windowBounds.w  = parentBounds.w + int(position.y);
			windowBounds.y  = windowBounds.w - temp;
			update          = true;
		}

	if (update)
		for (size_t t = 0; t < elements.GetCount(); t++) {
			elements[t]->forceUpdate(true);
			elements[t]->computeWindowBounds();
		}
}

bool  GUIPanel::addWidget(GUIRectangle* element) {
	if (element) {
		for (size_t t = 0; t < elements.GetCount(); t++)
			if (elements[t]->getCallbackString() == element->getCallbackString())
				return Logger::writeErrorLog(String("Panel already has a outer with CBS -> ")
											 + element->getCallbackString());

		elements.Add(element);
		element->setParent(this);
		return true;
	}

	return false;
}

void GUIPanel::Clear() {
	parent        = NULL;

	for (size_t t = 0; t < elements.GetCount(); t++)
		deleteObject(elements[t]);

	elements.Clear();
}

void GUIPanel::notify(GUIRectangle* element) {
	if (!element)
		return;

	switch (element->getWidgetType()) {
	case WT_RADIO_BUTTON:
		for (size_t t = 0; t < elements.GetCount(); t++)
			if (elements[t]->getWidgetType() == WT_RADIO_BUTTON &&
				elements[t] != element) {
				GUIRadioButton* rButton = (GUIRadioButton*)elements[t];
				rButton->setChecked(false);
			}

		break;
	}
}

int  GUIPanel::getWidgetCountByType(int type) {
	int  counter =  0;

	for (size_t t = 0; t < elements.GetCount(); t++)
		if (elements[t]->getWidgetType() == type)
			counter++;

	return counter;
}

int  GUIPanel::getTreeHeight() {
	int height = 1;

	for (size_t t = 0; t < elements.GetCount(); t++)
		if (elements[t]->getWidgetType() == WT_PANEL)
			height += ((GUIPanel*)elements[t])->getTreeHeight();

	return height;
}

void  GUIPanel::setInterval(const Tuple2i& dimensions) {
	setInterval(dimensions.x, dimensions.y);
}

void  GUIPanel::setInterval(int width, int height) {
	interval.Set(clamp(width, 0, 2500),
				 clamp(height, 0, 2500));
}

const Tuple2i& GUIPanel::getInterval() {
	return interval;
}

void  GUIPanel::setLayout(int layoutArg) {
	switch (layoutArg) {
	case PL_GRID_LAYOUT:
	case PL_YAXIS_CEN_LAYOUT:
	case PL_YAXIS_LAYOUT:
	case PL_XAXIS_LAYOUT:
		layout = layoutArg;
		break;

	default:
		layout = PL_FREE_LAYOUT;
	}
}

int   GUIPanel::getLayout() {
	return layout;
}

Widgets& GUIPanel::getWidgets() {
	return elements;
}

GUIPanel::~GUIPanel() {
	Clear();
}



#endif
