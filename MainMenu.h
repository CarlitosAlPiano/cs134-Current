#ifndef MAIN_MENU_INCLUDED
#define MAIN_MENU_INCLUDED

#include "Level.h"
#include <iostream>
#include <sstream>
#include <deque>

class Level;
class MainMenu;

class MainMenuItem {
private:
    Core *core;
    MainMenu *menu;
    Level *level;
    string geometryFile;
    
    void iniItem(string geomFile, string imageFile);
public:
    ScenePrimitive *image, *imageBorder;
    int numLevel;
    bool locked;
    static const int ITEM_EXIT;
    
    MainMenuItem(Core *core, MainMenu* menu, int numLevel, bool locked = false);
    MainMenuItem(Core *core, MainMenu* menu, int numLevel, string geometryFile, string imageFile, bool locked = false);
    ~MainMenuItem();
    
    void handleEvent(Event *e);
    void loadLevel();
    void exitLevel(int code);
    bool Update();
};

class MainMenu : public EventHandler {
private:
    Core *core;
    Sound *sndRotate, *sndSelect, *sndSelectLocked;
    bool executingItem;
    int numLevels, selectedItem;
    Number angleRotLeft, incYaw, itemGap;
    Color colActive, colInactive, colSelected, colLocked, colSelectedLocked;
public:
    Scene *scene;
    SceneEntity *menu;
    int numMenuItems;
    Number itemSize, itemBordW, menuRad;
    
    deque<MainMenuItem*> items;
    
    MainMenu(Core *core, Sound *sndRotate, Sound *sndSelect, Sound *sndSelectLocked, int numLevels = 5, int numOthers = 1, int selectedItem = 0, Number incYaw = 3.0, Number itemSize = 50, Number itemBordW = 3, Number itemGap = 0, Color colActive = Color(1.0, 0.8, 0.0, 1.0), Color colInactive = Color(0.0, 0.3, 0.5, 1.0), Color colSelected = Color(1.0, 0.0, 0.0, 1.0), Color colLocked = Color(0.3, 0.3, 0.3, 1.0), Color colSelectedLocked = Color(0.2, 0.2, 0.2, 1.0), Scene *scn = NULL, SceneEntity *mn = NULL);
    ~MainMenu();
    
    void loadMenu();
    void unloadMenu();
    void unlockNextLevel(int level);
    void recolorBorders();
    void rotateMenu(bool left);
    void rotateTo(int target);
    void handleEvent(Event *e);
    bool Update();
};

#endif
