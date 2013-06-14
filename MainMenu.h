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
    ScreenImage *imgHealth, *imgCoin, *imgInstructions;
    ScreenShape *healthBar, *healthBarBorder, *levelEndedBkgnd, *levelEndedBkgndBorder;
    ScreenLabel *lblLivesLeft, *lblPoints, *lblLevelEndedL1, *lblLevelEndedL2;
    Sound *sndLifeUp;
    Timer *tmrBlink;
    Vector2 imgCoinSize, imgHealthSize, healthBarSize, levelEndedBkgndSize, margin;
    Number healthBarBorderW, levelEndedBkgndBorderW, blinkThresh, blinkPeriod;
    Color colLblLivesLeft, colLblPoints, colMaxHealth, colMinHealth, colBordMaxHealth, colBordMinHealth, colLevelEndedBkgndSurvived, colLevelEndedBkgndDied;
    int instructionsPage;
    static int points, livesLeft;
    static Number health;
    bool loaded;
    
    void updateTextWithNum(ScreenLabel*lbl, int num);
public:
    static int pointsPerLife, numPagesInstructions;
    static Number maxHealth;
    
    Hud(Core *core, Vector2 imgCoinSize = Vector2(20, 20), Vector2 imgHealthSize = Vector2(20, 20), Vector2 healthBarSize = Vector2(150, 25), Vector2 levelEndedBkgndSize = Vector2(550,350), Vector2 margin = Vector2(7, 2), Number healthBarBorderW = 2, Number levelEndedBkgndBorderW = 2, Number blinkThresh = 0.5, Number blinkPeriod = 250, Color colLblLivesLeft = Color(0.2, 0.8, 0.2, 1.0), Color colLblPoints = Color(0.9, 0.7, 0.2, 1.0), Color colMaxHealth = Color(0.0, 1.0, 0.0, 1.0), Color colMinHealth = Color(1.0, 0.0, 0.0, 1.0), Color colBordMaxHealth = Color(0.0, 0.4, 0.0, 1.0), Color colBordMinHealth = Color(0.4, 0.0, 0.0, 1.0), Color colLevelEndedBkgndSurvived = Color(0.2, 0.8, 0.2, 0.4), Color colLevelEndedBkgndDied = Color(0.5, 0.2, 0.2, 0.4));
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
    void showLevelEndedText(int code, int numLevel, int numTotalLevels = -1, bool nextLevelLocked = true);
    void hideLevelEndedText();
    void showInstructions(int page);
    void hideInstructions();
    string getInstructionsFileName(int page);
    void showPrevInstructPage();
    void showNextInstructPage();
    void handleEvent(Event *e);
};

class MainMenuItem : public EventHandler {
private:
    Core *core;
    Hud *hud;
    MainMenu *menu;
    Level *level;
    Timer *tmrLevelEnded;
    string geometryFile;
    bool levelEnded;
    int levelEndedCode, levelEndedTextPeriod;
    
    void iniItem(string geomFile, string imageFile);
    Vector2 getStarImageSize();
public:
    ScenePrimitive *image, *imageBorder, *imgStars;
    int numLevel, numStars;
    bool isLevel, locked;
    static const int ITEM_EXIT, ITEM_INSTRUCTIONS;
    
    MainMenuItem(Core *core, Hud *hud, MainMenu* menu, int numLevel, bool isLevel = true, bool locked = false, int levelEndedTextPeriod = 4000);
    MainMenuItem(Core *core, Hud *hud, MainMenu* menu, int numLevel, string geometryFile, string imageFile, bool isLevel = true, bool locked = false, int levelEndedTextPeriod = 4000);
    ~MainMenuItem();
    
    void loadLevel();
    void exitLevel(int code);
    void backToMainMenu();
    void setStars(int nStars);
    void handleEvent(Event *e);
    bool Update();
};

class MainMenu : public EventHandler {
private:
    Core *core;
    Hud *hud;
    Scene *scene;
    SceneEntity *menu;
    Sound *sndRotate, *sndSelect, *sndSelectLocked;
    bool executingItem, showingInstructions;
    int numLevels, selectedItem;
    Number angleRotLeft, incYaw, itemGap;
    Color colActive, colInactive, colSelected, colLocked, colSelectedLocked;
public:
    int numMenuItems;
    Number itemSize, itemBordW, menuRad;
    deque<MainMenuItem*> items;
    
    MainMenu(Core *core, Sound *sndRotate, Sound *sndSelect, Sound *sndSelectLocked, int numLevels = 5, int numOthers = 2, int selectedItem = 0, Number incYaw = 3.0, Number itemSize = 50, Number itemBordW = 3, Number itemGap = 7, Color colActive = Color(1.0, 0.8, 0.0, 1.0), Color colInactive = Color(0.0, 0.3, 0.5, 1.0), Color colSelected = Color(1.0, 0.0, 0.0, 1.0), Color colLocked = Color(0.3, 0.3, 0.3, 1.0), Color colSelectedLocked = Color(0.2, 0.2, 0.2, 1.0), Scene *scn = NULL, SceneEntity *mn = NULL);
    ~MainMenu();
    
    void loadMenu();
    void unloadMenu();
    void unlockNextLevel(int level);
    void lockAll();
    void recolorBorders();
    void rotateMenu(bool left);
    void rotateTo(int target);
    void showInstructions();
    void handleEvent(Event *e);
    bool Update();
};

#endif
