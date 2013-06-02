#include <Polycode.h>
#include "PolycodeView.h"
#include "Polycode3DPhysics.h"
#include "DrawScene.h"

using namespace Polycode;

class Current : public EventHandler {
private:
    Core *core;
    
    Number alpha, beta;
    bool mouse_clicked;
    bool left_pressed, right_pressed;
    bool further_pressed, closer_pressed;
    bool up_pressed, down_pressed;

    CollisionScene *scene;
    deque<ScenePrimitive*> walls;
    deque<ScenePrimitive*> obstacles;
    deque<Enemy*> enemies;
    deque<Coin*> coins;
    ScenePrimitive *player;
    Vector3 playerVeloc;
    Number playerRad, currentZ;
    Number totalElapsed, lastCollision;
    
    void recomputePlayerVeloc();
    bool checkPlayerCollision(ScenePrimitive *obstacle);
    void keepPlayerWithinBB();

public:
    Current(PolycodeView *view);
    ~Current();
    void handleEvent(Event *e);
    bool Update();
};
