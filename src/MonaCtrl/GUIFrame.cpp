#include "EasyGL.h"

GUIFrame::GUIFrame() : GUIPanel("GUI MAIN PANEL")
{
  listener = NULL;
  update   = true;
}

bool GUIFrame::loadXMLSettings(const TiXmlElement *element) 
{
  if(!XMLArbiter::inspectElementInfo(element, "Panel"))
    return Logger::writeErrorLog("Need a Panel node in the xml file");

  bool defaultFont = true;

  for(const TiXmlElement *outer = element->FirstChildElement();	
      outer;
   	  outer = outer->NextSiblingElement() )
  {
    String elementName  = outer->Value();

    if(!elementName.GetCount())
      continue;
    
    if(elementName == "Font")
    {
      int fontIndex = GUIFontManager::addFont(outer);
      if(defaultFont && fontIndex >= 0)
      {
        GUIFontManager::setDefaultFont(fontIndex);
        defaultFont = false;
      }
      continue;
    }
 
    if(elementName == "Texture")
    {
      if(elementsTexture.loadXMLSettings(outer))
      {
        GUITexCoordDescriptor::setTextureHeight(elementsTexture.GetHeight());
        GUITexCoordDescriptor::setTextureWidth(elementsTexture.GetWidth());
      }
      continue;
    }

    if(elementName == "TexCoordsDesc")
    {
      GUITexCoordDescriptor descriptor;
      descriptor.loadXMLSettings(outer);
      addOrReplaceTexCoordsInfo(descriptor);
      continue;
    }
  }
  return   GUIPanel::loadXMLSettings(element);
}

void  GUIFrame::render(float tick)
{
  if(!visible)
    return;

  size_t t = 0;

  while(update_count)
  {
    for(t = 0; t < elements.GetCount(); t++)
      elements[t]->forceUpdate(true);
    update_count--;
  }

  for(t = 0; t < elements.GetCount(); t++)
    elements[t]->render(tick);
}

void GUIFrame::addOrReplaceTexCoordsInfo(GUITexCoordDescriptor &info)
{
  for(size_t t = 0; t < texCoords.GetCount(); t++)
    if(texCoords[t].getType() == info.getType())
    {
      texCoords[t].setTexCoords(info.getTexCoords());
      return;
    }
  texCoords.Add(info);
}

GUITexCoordDescriptor *GUIFrame::getTexCoordsInfo(int type)
{
  for(size_t t = 0; t < texCoords.GetCount(); t++)
    if(texCoords[t].getType() == type)
      return &texCoords[t];
  return NULL;
}

void  GUIFrame::setGUIEventListener(GUIEventListener *listener_)
{
  listener = listener_;
}

GUIEventListener *GUIFrame::getEventsListener()
{
  return listener;
}

void  GUIFrame::setElementsTexture(const char* texturePath)
{
  elementsTexture.load2D(texturePath);
}

void  GUIFrame::setElementsTexture(const Texture &texture)
{
  elementsTexture = texture;
}

Texture *GUIFrame::getElementsTexture()
{
  return !elementsTexture.getID() ? NULL : &elementsTexture;
}

void  GUIFrame::enableGUITexture()
{
  if(elementsTexture.getID())
    elementsTexture.activate();
}

void  GUIFrame::disableGUITexture()
{
  elementsTexture.deactivate();
}

void GUIFrame::forceUpdate(bool update)
{
  update_count = update ? getTreeHeight() + 1 : 0;
}

void GUIFrame::Clear()
{
  elementsTexture.destroy();
  texCoords.Clear();
  GUIPanel::Clear();
}

const Tuple4i &GUIFrame::getWindowBounds()
{
  windowBounds.set(0, 0, int(dimensions.x), int(dimensions.y));
  return windowBounds;
}

GUIFrame::~GUIFrame()
{
  Clear();
}
