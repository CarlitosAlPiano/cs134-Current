#ifndef LEVEL_INCLUDED
#define LEVEL_INCLUDED

#include "DrawScene.h"          // Draw the scene of the level
#include "MainMenu.h"           // Reference to main menu (notify when level is finished)
#include <queue>                // Priority queues

class MainMenuItem;

class CompareVector2 {
public:
    bool operator() (const Vector2& v1, const Vector2& v2) const{
        return (v1.x > v2.x);
    }
};

class Level : public EventHandler {
private:
    Core *core;
    MainMenuItem *mainMenu;
    
    Number camRad, camElev, camRot;
    bool dead, mouse_clicked;
    bool left_pressed, right_pressed;
    bool up_pressed, down_pressed;
    bool further_pressed, closer_pressed;
    bool further_locked, closer_locked;
    enum {HOME, RET_FROM_FRONT, RET_FROM_BACK} stateHome;
    Timer *tmrBackHome, *tmrForceHome;
    
    CollisionScene *scene;
    Sound *bgndMusic;
    SceneLine *lRiverWidth, *lHome;
    deque<Wall*> walls;
    deque<Obstacle*> obstacles;
    deque<Enemy*> enemies;
    deque<Coin*> coins;
    ScenePrimitive *player;
    Vector3 playerVeloc;
    Number playerRad, currentZ, riverWidth, playerHealth;
    Number totalElapsed, lastCollision;
    unsigned int points;
    
    void loadLevel(string& geometryFile);
    void startTimers();
    void playerSubtractHealth(Number healthSub = 0.0, bool checkLastCollision = true);
    bool checkPlayerCollision(ScenePrimitive *obstacle);
    void computeRiverWidth();
    void recomputePlayerVeloc();
    void keepPlayerHome();
    void keepPlayerWithinBB();
    
public:
    static const int EXIT_DIED = 0;
    static const int EXIT_SURVIVED = 1;

    Level(Core *core, MainMenuItem *mainMenu, string& geometryFile);
    ~Level();
    void handleEvent(Event *e);
    bool Update();
};

#endif
