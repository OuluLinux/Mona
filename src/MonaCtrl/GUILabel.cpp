#include "EasyGL.h"

GUILabel::GUILabel(const String &ls,
                   const String &callback) :  GUIAlphaElement(callback)
{
  setSizes(40, 5);
  setLabelString(ls);
  widgetType = WT_LABEL;
}

bool GUILabel::LoadXMLSettings(const TiXmlElement *element)
{
  if(!XMLArbiter::inspectElementInfo(element, "Label"))
    return Logger::writeErrorLog("Need a Label node in the xml file");

  return  GUIAlphaElement::LoadXMLSettings(element);
}

void GUILabel::render(double clockTick)
{
  if(!parent || !visible)
    return;

  Tuple2i center = getCenter();

  if(label.needUpdating())
    parent->forceUpdate(true);

  switch(anchor)
  {
    case AT_CORNERLU: label.Print(windowBounds.x, windowBounds.y); break;
    case AT_CORNERLD: label.Print(windowBounds.x, windowBounds.y - label.GetHeight()); break;
    case AT_CORNERRU: label.Print(windowBounds.z, windowBounds.y); break;
    case AT_CORNERRD: label.Print(windowBounds.z, windowBounds.y - label.GetHeight()); break;
    default: label.printCenteredXY(center.x, center.y);
  }
}

const Tuple4i &GUILabel::getWindowBounds()
{
  if(parent && update)
  {
    int height       = 0,
        width        = 0;
    label.computeSizes();

    if(dimensions.x > 1)
      width  = dimensions.x  < label.GetWidth() ? label.GetWidth() : (int)dimensions.x;
    else
      width  = GetWidth()    < label.GetWidth() ? label.GetWidth() : GetWidth();
 
    if(dimensions.x > 1)
      height  = dimensions.y  < label.GetHeight() ? label.GetHeight() : (int)dimensions.y;
    else
      height  = GetHeight()   < label.GetHeight() ? label.GetHeight() : GetHeight();

    dimensions.Set(float(width),  float(height));
    GUIRectangle::computeWindowBounds();
  }
  return windowBounds;
}
