#include "EasyGL.h"

GUICheckBox::GUICheckBox(const String &callback) :  GUIAlphaElement(callback)
{
  setMinAlphaMark(0.0f);
  setSizes(20, 20);
  setAlphaMark(1.0f);
  setBGColor(0.8f, 0.8f, 0.8f);
  setChecked(false);
  widgetType = WT_CHECK_BOX;

  secondaryTexDesc = WT_CHECK_BOX_MARK;
  primaryTexDesc   = WT_CHECK_BOX;
  markOffset       = 0;
  markRatio        = 0.85f;
  extra            = 0;
}

bool GUICheckBox::LoadXMLSettings(const TiXmlElement *element)
{
  if(widgetType == WT_CHECK_BOX)
  if(!element || !element->Value() || strcmp(element->Value(),  "CheckBox"))
    return Logger::writeErrorLog("Need a CheckBox node in the xml file");

  if(widgetType == WT_RADIO_BUTTON)
  if(!element || !element->Value() || strcmp(element->Value(),  "RadioButton"))
    return Logger::writeErrorLog("Need a RadioButton node in the xml file");

  setChecked(XMLArbiter::analyzeBooleanAttr(element,  "checked", false));

  for(const TiXmlElement *outer = element->FirstChildElement();	
      outer;
   	  outer = outer->NextSiblingElement() )
  {
    const char * value = outer->Value();

    if(value)
    if(!strcmp(value, "BGColor"))
    {
      setBGColor(XMLArbiter::fillComponents3f(outer, bgColor));
      break;
    }

  }

  return GUIAlphaElement::LoadXMLSettings(element);
}

void  GUICheckBox::setMinAlphaMark(double mark)
{
  minAlphaMark = clamp(mark, 0.0f, 10.0f);
}

double  GUICheckBox::getMinAlphaMark()
{
  return minAlphaMark;
}

void  GUICheckBox::setAlphaMark(double mark)
{
  alphaMark = clamp(mark, minAlphaMark, 10.0f);
}

double  GUICheckBox::getAlphaMark()
{
  return alphaMark;
}

void  GUICheckBox::setChecked(bool enabledArg)
{
  if(active)
    checked = enabledArg;
}

bool  GUICheckBox::isChecked()
{
  return checked;
}

void  GUICheckBox::setBGColor(const Tuple3f& color)
{
  setBGColor(color.x, color.y, color.z);
}

void  GUICheckBox::setBGColor(double x, double y, double z)
{
  bgColor.Set(clamp(x, 0.0f, 1.0f),
              clamp(y, 0.0f, 1.0f),
              clamp(z, 0.0f, 1.0f));
}

const Tuple3f &GUICheckBox::getBGColor()
{
  return bgColor;
}

void GUICheckBox::render(double clockTick)
{
  if(!parent || !visible)
    return;

  modifyCurrentAlpha(clockTick);
  getCenter();

  GUITexCoordDescriptor *descriptor     = parent->getTexCoordsInfo(primaryTexDesc),
                        *descriptorCB   = parent->getTexCoordsInfo(secondaryTexDesc);

  int yCenter = (windowBounds.y + windowBounds.w)/2;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4fv(color);

  if(descriptor)
  {
    const Tuple4f &texCoords = descriptor->getTexCoords();
    parent->enableGUITexture();

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(texCoords.x, texCoords.y);
    glVertex2i(windowBounds.x, windowBounds.y);

    glTexCoord2f(texCoords.x, texCoords.w);
    glVertex2i(windowBounds.x, windowBounds.w);

    glTexCoord2f(texCoords.z, texCoords.y);
    glVertex2i(windowBounds.x + extra, windowBounds.y);

    glTexCoord2f(texCoords.z, texCoords.w);
    glVertex2i(windowBounds.x + extra, windowBounds.w);
    glEnd();

    parent->disableGUITexture();
  }
  else
  {
    glBegin(GL_TRIANGLE_STRIP);
    glVertex2i(windowBounds.x, windowBounds.y);
    glVertex2i(windowBounds.x, windowBounds.w);
    glVertex2i(windowBounds.x + extra, windowBounds.y);
    glVertex2i(windowBounds.x + extra, windowBounds.w);
    glEnd();
  }

  if(!checked)
    setAlphaMark(alphaMark - clockTick*alphaFadeScale*10.0f);
  else
    setAlphaMark(alphaMark + clockTick*alphaFadeScale*10.0f);
 
  if(alphaMark != minAlphaMark)
  {
    glColor4f(1.0f, 1.0f, 1.0f, alphaMark);

    if(descriptorCB)
    { 
      const Tuple4f &texCoords = descriptorCB->getTexCoords();
      parent->enableGUITexture();

      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(texCoords.x, texCoords.y);
      glVertex2i(windowBounds.x + markOffset, windowBounds.y + markOffset); 

      glTexCoord2f(texCoords.x, texCoords.w);
      glVertex2i(windowBounds.x + markOffset, windowBounds.w - markOffset); 

      glTexCoord2f(texCoords.z, texCoords.y);
      glVertex2i(windowBounds.x - markOffset + extra, windowBounds.y + markOffset); 

      glTexCoord2f(texCoords.z, texCoords.w);
      glVertex2i(windowBounds.x - markOffset + extra, windowBounds.w - markOffset); 
      glEnd();

      parent->disableGUITexture();
    }
    else
    {
      glBegin(GL_TRIANGLE_STRIP);
      glVertex2i(windowBounds.x + markOffset, windowBounds.y + markOffset); 
      glVertex2i(windowBounds.x + markOffset, windowBounds.w - markOffset); 
      glVertex2i(windowBounds.x - markOffset + extra, windowBounds.y + markOffset); 
      glVertex2i(windowBounds.x - markOffset + extra, windowBounds.w - markOffset); 
      glEnd();
   }
  }

  glDisable(GL_BLEND);
  label.printCenteredY(windowBounds.x + extra + 2, yCenter);
}

const Tuple4i &GUICheckBox::getWindowBounds()
{
  if(parent && update)
  {
    label.computeSizes();
    extra = int(float(label.GetHeight())*1.25);
    dimensions.Set(float(label.GetWidth() + extra), float(extra));
    markOffset = extra - int(dimensions.y*markRatio);
    GUIRectangle::computeWindowBounds();
  }
  return windowBounds;
}

void GUICheckBox::checkMouseEvents(MouseEvent &newEvent, int extraInfo, bool bits)
{
  GUIRectangle::checkMouseEvents(newEvent, extraInfo, true);
  GUIEventListener *eventsListener = getEventsListener();

  if(clicked)
  {
    setChecked(!checked);
    if(eventsListener)
    {
      GUIEvent evt(this);
      eventsListener->actionPerformed(evt);
    }
  }
  released = false;
  clicked  = false;
}
