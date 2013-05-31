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
    ScenePrimitive *player;
    Vector3 playerVeloc;
    Number playerRad, currentZ;
    Number totalElapsed;
    
    void recomputePlayerVeloc();
    bool checkPlayerCollision(ScenePrimitive *obstacle);
    void keepPlayerWithinBB();

public:
    Current(PolycodeView *view);
    ~Current();
    void handleEvent(Event *e);
    bool Update();
};

class Enemy {
public:
    Number amplitude, velocity, offset;
    Vector3 movementDir, middlePos;
    ScenePrimitive *enemy;
    
    Enemy(ScenePrimitive *enemy, Number amplitude = 3, Number velocity = 1, Number offset = PI/4, Vector3 movementDir = Vector3(1, 0, 0));
    Enemy(ScenePrimitive *enemy, Vector3 middlePos, Number amplitude = 3, Number velocity = 1, Number offset = PI/4, Vector3 movementDir = Vector3(1, 0, 0));
    
    void update(Number totalElapsed);
};
