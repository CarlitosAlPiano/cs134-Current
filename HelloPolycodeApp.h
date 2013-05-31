#include <Polycode.h>
#include "PolycodeView.h"
#include "Polycode3DPhysics.h"
#include "DrawScene.h"

using namespace Polycode;

class HelloPolycodeApp : public EventHandler {
public:
    HelloPolycodeApp(PolycodeView *view);
    ~HelloPolycodeApp();
    void handleEvent(Event *e);
    bool Update();
    
private:
    Core *core;
    
    bool left_pressed, right_pressed;
    bool further_pressed, closer_pressed;
    bool up_pressed, down_pressed;
    
    CollisionScene *scene;
    deque<ScenePrimitive*> walls;
    deque<ScenePrimitive*> obstacles;
    ScenePrimitive *player, *playerBB;
    Vector3 playerVeloc, centerPlayerBB;
    Number playerRad;
    Number totalElapsed;
    
    void recomputePlayerVeloc();
    bool checkPlayerCollision(ScenePrimitive *obstacle);
    void keepPlayerWithinBB();
};
