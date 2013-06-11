#ifndef MAIN_MENU_INCLUDED
#define MAIN_MENU_INCLUDED

#include "Level.h"
#include <iostream>
#include <sstream>
#include <deque>

class Level;
class MainMenu;

class Hud : public EventHandler {
private:
    Core *core;
    Screen *hud;
    ScreenImage *imgHealth, *imgCoin;
    ScreenShape *healthBar, *healthBarBorder;
    ScreenLabel *lblLivesLeft, *lblPoints;
    Timer *tmrBlink;
    Vector2 imgCoinSize, imgHealthSize, healthBarSize, margin;
    Number healthBarBorderW, blinkThresh, blinkPeriod;
    Color colLblLivesLeft, colLblPoints, colMaxHealth, colMinHealth, colBordMaxHealth, colBordMinHealth;
    static int points, livesLeft;
    static Number health;
    
    void updateTextWithNum(ScreenLabel*lbl, int num);
public:
    static Number maxHealth;
    
    Hud(Core *core, Vector2 imgCoinSize = Vector2(20, 20), Vector2 imgHealthSize = Vector2(20, 20), Vector2 healthBarSize = Vector2(150, 25), Vector2 margin = Vector2(7, 2), Number healthBarBorderW = 2, Number blinkThresh = 0.2, Number blinkPeriod = 250, Color colLblLivesLeft = Color(0.2, 0.8, 0.2, 1.0), Color colLblPoints = Color(0.9, 0.7, 0.2, 1.0), Color colMaxHealth = Color(0.0, 1.0, 0.0, 1.0), Color colMinHealth = Color(1.0, 0.0, 0.0, 1.0), Color colBordMaxHealth = Color(0.0, 0.4, 0.0, 1.0), Color colBordMinHealth = Color(0.4, 0.0, 0.0, 1.0));
    ~Hud();
    
    void loadHud();
    void unloadHud();
    static int getPoints();
    static int getLivesLeft();
    static Number getHealth();
    static bool isDead();
    static bool isGameOver();
    void setPoints(int pts);
    void setLivesLeft(int lives);
    void setHealth(Number healthPts);
    void incPoints(int inc);
    void decPoints(int dec);
    void incLivesLeft(int inc);
    void decLivesLeft(int dec);
    void restoreHealth();
    void decHealth(int dec);
    void handleEvent(Event *e);
};

class MainMenuItem {
private:
    Core *core;
    Hud *hud;
    MainMenu *menu;
    Level *level;
    string geometryFile;
    
    void iniItem(string geomFile, string imageFile);
public:
    ScenePrimitive *image, *imageBorder;
    int numLevel;
    bool locked;
    static const int ITEM_EXIT;
    
    MainMenuItem(Core *core, Hud *hud, MainMenu* menu, int numLevel, bool locked = false);
    MainMenuItem(Core *core, Hud *hud, MainMenu* menu, int numLevel, string geometryFile, string imageFile, bool locked = false);
    ~MainMenuItem();
    
    void handleEvent(Event *e);
    void loadLevel();
    void exitLevel(int code);
    bool Update();
};

class MainMenu : public EventHandler {
private:
    Core *core;
    Hud *hud;
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
    
    MainMenu(Core *core, Sound *sndRotate, Sound *sndSelect, Sound *sndSelectLocked, int numLevels = 5, int numOthers = 1, int selectedItem = 0, Number incYaw = 3.0, Number itemSize = 50, Number itemBordW = 3, Number itemGap = 7, Color colActive = Color(1.0, 0.8, 0.0, 1.0), Color colInactive = Color(0.0, 0.3, 0.5, 1.0), Color colSelected = Color(1.0, 0.0, 0.0, 1.0), Color colLocked = Color(0.3, 0.3, 0.3, 1.0), Color colSelectedLocked = Color(0.2, 0.2, 0.2, 1.0), Scene *scn = NULL, SceneEntity *mn = NULL);
    ~MainMenu();
    
    void loadMenu();
    void unloadMenu();
    void unlockNextLevel(int level);
    void lockAll();
    void recolorBorders();
    void rotateMenu(bool left);
    void rotateTo(int target);
    void handleEvent(Event *e);
    bool Update();
};

#endif
