#include "core/config_loader.hpp"
#include "tinyxml2.h"
#include <iostream>
bool ConfigLoader::loadConfigFromXML(const char* path, Config& config) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(path) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load XML file: " << path << std::endl;
        return false;
    }
    tinyxml2::XMLElement* root = doc.FirstChildElement("Game")->FirstChildElement("Config")->FirstChildElement("Init");
    if (root == nullptr) {
        std::cerr << "Failed to load game data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement* title = root->FirstChildElement("Title");
    if (title == nullptr) {
        std::cerr << "Failed to load title data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement* fullScreen = root->FirstChildElement("FullScreen");
    if (fullScreen == nullptr) {
        std::cerr << "Failed to load fullScreen data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement* screen = root->FirstChildElement("Screen");
    if (screen == nullptr) {
        std::cerr << "Failed to load screen data from XML" << std::endl;
        return false;
    }
    tinyxml2::XMLElement* frameRate = root->FirstChildElement("FrameRate");
    if (frameRate == nullptr) {
        std::cerr << "Failed to load frameRate data from XML" << std::endl;
        return false;
    }
    config.title = title->GetText();
    config.fullScreen = fullScreen->BoolText();
    config.screenWidth = screen->IntAttribute("width");	
    config.screenHeight = screen->IntAttribute("height");
    config.frameRate = frameRate->IntText();
    return true;
}