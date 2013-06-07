#include <Polycode.h>
#include "PolycodeView.h"
#include "Polycode3DPhysics.h"
#include "DrawScene.h"
#include <queue>

using namespace Polycode;

class CompareVector2 {
public:
    bool operator() (const Vector2& v1, const Vector2& v2) const{
        return (v1.x > v2.x);
    }
};

class Current : public EventHandler {
private:
    Core *core;

    Number camRad, camElev, camRot;
    bool mouse_clicked;
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
    deque<ScenePrimitive*> obstacles;
    deque<Enemy*> enemies;
    deque<Coin*> coins;
    ScenePrimitive *player;
    Vector3 playerVeloc;
    Number playerRad, currentZ, riverWidth, playerHealth;
    Number totalElapsed, lastCollision;
    unsigned int points;
    
    void startTimers();
    void playerSubtractHealth(Number healthSub = 0.0, bool checkLastCollision = true);
    bool checkPlayerCollision(ScenePrimitive *obstacle);
    void computeRiverWidth();
    void recomputePlayerVeloc();
    void keepPlayerHome();
    void keepPlayerWithinBB();

public:
    Current(PolycodeView *view);
    ~Current();
    void handleEvent(Event *e);
    bool Update();
};
