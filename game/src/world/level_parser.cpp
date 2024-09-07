#include "world/level_parser.hpp"
#include <tinyxml2.h>
using namespace tinyxml2;

Level *LevelParser::parseLevel(const char *levelFile) {
    // create a TinyXML document and load the map XML
    XMLDocument levelDocument;
    levelDocument.LoadFile(levelFile);
    // create the level object
    Level *pLevel = new Level();
    // get the root node
    XMLElement *pRoot = levelDocument.RootElement();
    m_tileSize = pRoot->IntAttribute("tilewidth");
    m_width = pRoot->IntAttribute("width");
    m_height = pRoot->IntAttribute("height");
    // parse the tilesets
    for (XMLElement *e = pRoot->FirstChildElement(); e != NULL;
        e = e->NextSiblingElement()) {
        if (e->Value() == std::string("tileset")) {
            parseTilesets(e, pLevel->getTilesets());
        }
    }
    // parse any object layers
    for (XMLElement *e = pRoot->FirstChildElement(); e != NULL;
        e = e->NextSiblingElement()) {
        if (e->Value() == std::string("layer")) {
            parseTileLayer(e, pLevel->getLayers(), pLevel->getTilesets());
        }
    }
    return pLevel;
}