#include "EasyGL.h"

bool GUIFont::Build()
{
  if(!GetName().GetCount())
    return Logger::writeErrorLog("Null font path");
  
  return font.Load(GetName().c_str());
}

bool GUIFont::LoadXMLSettings(const TiXmlElement *element)
{
  if(!XMLArbiter::inspectElementInfo(element, "Font"))
    return Logger::writeErrorLog("Need a Font node in the xml file");

  SetName(element->Attribute("path"));

  return true;
}

CFont *GUIFont::getFontObject()
{
  return &font;
}

bool  GUIFont::operator ==(const GUIFont &compare)
{
  return (name   == compare.GetName());  
}

bool  GUIFont::operator !=(const GUIFont &compare)
{
  return !operator ==(compare);  
}

GUIFont::~GUIFont()
{
  font.~CFont();
}
