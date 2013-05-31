#include "Current.h"

#define VEL_CAM_Z       0.1
#define VEL_PLAYER_INC  0.15
#define VEL_MAX         0.25
#define VEL_ROT_AWARD   50
#define BB_HALF_DEPTH   5
#define BB_FRTH_LIMIT   currentZ + BB_HALF_DEPTH
#define BB_CLSR_LIMIT   currentZ - BB_HALF_DEPTH
#define BB_TOP_LIMIT    DrawScene::wallHeight - playerRad
#define BB_BTM_LIMIT    playerRad

Current::Current(PolycodeView *view) {
	core = new POLYCODE_CORE(view, 640,480,false,false,0,0,90);
	CoreServices::getInstance()->getResourceManager()->addDirResource("default", false);
    
	scene = new CollisionScene();
    DrawScene::drawScene(scene, player, &walls, &obstacles, "geometry.xml");
    scene->getDefaultCamera()->setPosition(-20, 50, -30);
	scene->getDefaultCamera()->lookAt(Vector3(0,1,5));
    
    ScenePrimitive *t = new ScenePrimitive(ScenePrimitive::TYPE_TORUS, 1, 0.6, 30, 30);
    t->setColor(1, 0.3, 0.4, 1);
    t->setPosition(-3, 5, 20);
    t->setPitch(-90);
    scene->addCollisionChild(t);

    totalElapsed = 0;
    playerRad = player->getMesh()->getRadius();
    playerVeloc = Vector3(0, 0, VEL_CAM_Z);
    currentZ = player->getPosition().z;
	alpha = 0;
	beta = PI/2;
    beta = 0;
	mouse_clicked = false;
	left_pressed = false;
	right_pressed = false;
    up_pressed = false;
    down_pressed = false;
	further_pressed = false;
	closer_pressed = false;
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEMOVE);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	core->getInput()->addEventListener(this, InputEvent::EVENT_KEYDOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_KEYUP);
    
}

Current::~Current() {
}

void Current::recomputePlayerVeloc() {
    if(left_pressed == right_pressed){
        playerVeloc.x = 0;
    }else if(left_pressed){
        playerVeloc.x = VEL_PLAYER_INC;
    }else{
        playerVeloc.x = -VEL_PLAYER_INC;
    }
    if(up_pressed == down_pressed){
        playerVeloc.y = 0;
    }else if(up_pressed){
        playerVeloc.y = VEL_PLAYER_INC;
    }else{
        playerVeloc.y = -VEL_PLAYER_INC;
    }
    if(further_pressed == closer_pressed){
        playerVeloc.z = VEL_CAM_Z;
    }else if(further_pressed){
        playerVeloc.z = VEL_CAM_Z + VEL_PLAYER_INC;
    }else{
        playerVeloc.z = VEL_CAM_Z - VEL_PLAYER_INC;
    }
}

bool Current::checkPlayerCollision(ScenePrimitive *obstacle) {
    CollisionResult res = scene->testCollision(player, obstacle);
    Vector3 normal = res.colNormal, oldPos = player->getPosition();//, oldVel = playerVeloc;
    if(abs(normal.x) < 10e-6) normal.x = 0;
    if(abs(normal.y) < 10e-6) normal.y = 0;
    if(abs(normal.z) < 10e-6) normal.z = 0;
    normal.Normalize();
    
    if(res.collided && res.colNormal.length() > 0) {
        if(res.colNormal.angleBetween(playerVeloc)*180/PI < 91){
            playerVeloc = normal.crossProduct(playerVeloc).crossProduct(normal);
        }
        player->setPosition(oldPos + res.colNormal*res.colDist + playerVeloc);
        return true;
    }

    return false;
}

void Current::keepPlayerWithinBB(){
    Vector3 pos = player->getPosition();
    
    currentZ += VEL_CAM_Z;
    if(pos.z > BB_FRTH_LIMIT){
        player->setPositionZ(BB_FRTH_LIMIT);
    }else if(pos.z < BB_CLSR_LIMIT){
        player->setPositionZ(BB_CLSR_LIMIT);
    }
    if(pos.y > BB_TOP_LIMIT){
        player->setPositionY(BB_TOP_LIMIT);
    }else if(pos.y <  BB_BTM_LIMIT){
        player->setPositionY(BB_BTM_LIMIT);
    }
}

void Current::handleEvent(Event *e) {
    static Number mouse_x = 0, mouse_y = 0;

    if(e->getDispatcher() == core->getInput()) {
        InputEvent *inputEvent = (InputEvent*)e;
        
        switch(e->getEventCode()) {
            case InputEvent::EVENT_MOUSEMOVE:
                if(inputEvent->mousePosition.x > mouse_x && mouse_clicked){
                    beta -= 0.03;
                    mouse_x = inputEvent->mousePosition.x;
                }else if(inputEvent->mousePosition.x < mouse_x && mouse_clicked){
                    beta += 0.03;
                    mouse_x = inputEvent->mousePosition.x;
                }
                if(inputEvent->mousePosition.y > mouse_y && mouse_clicked){
                    //alpha += 0.5;
                    //mouse_y = inputEvent->mousePosition.y;
                } else if(inputEvent->mousePosition.y < mouse_y && mouse_clicked) {
                    //alpha -= 0.01;
                    //mouse_y = inputEvent->mousePosition.y;
                }
                break;
            case InputEvent::EVENT_MOUSEDOWN:
                mouse_clicked = true;
                break;
            case InputEvent::EVENT_MOUSEUP:
                mouse_clicked = false;
                break;
            case InputEvent::EVENT_KEYDOWN:
                switch (inputEvent->keyCode()) {
                    case KEY_LEFT:
                        left_pressed = true;
                        break;
                    case KEY_RIGHT:
                        right_pressed = true;
                        break;
                    case KEY_UP:
                        further_pressed = true;
                        break;
                    case KEY_DOWN:
                        closer_pressed = true;
                        break;
                    case 'w':
                    case 'W':
                        up_pressed = true;
                        break;
                    case 's':
                    case 'S':
                        down_pressed = true;
                        break;
                }
                recomputePlayerVeloc();
                break;
            case InputEvent::EVENT_KEYUP:
                switch (inputEvent->keyCode()) {
                    case KEY_LEFT:
                        left_pressed = false;
                        break;
                    case KEY_RIGHT:
                        right_pressed = false;
                        break;
                    case KEY_UP:
                        further_pressed = false;
                        break;
                    case KEY_DOWN:
                        closer_pressed = false;
                        break;
                    case 'w':
                    case 'W':
                        up_pressed = false;
                        break;
                    case 's':
                    case 'S':
                        down_pressed = false;
                        break;
                }
                recomputePlayerVeloc();
                break;
        }
    }
}

bool Current::Update() {
	Number elapsed = core->getElapsed();
    totalElapsed += elapsed;
    
    scene->getEntity(scene->getNumEntities()-1)->setYaw(VEL_ROT_AWARD*totalElapsed);

    /*Vector3 newCamPos = scene->getDefaultCamera()->getPosition();
    newCamPos.x = -15 * cos(beta) * cos(alpha) + 20;
	newCamPos.y = -15 * cos(beta) * sin(alpha) + 50;
	newCamPos.z = -15 * sin(beta);// + currentZ;
    
    scene->getDefaultCamera()->setPosition(newCamPos);
    scene->getDefaultCamera()->lookAt(Vector3(0, DrawScene::wallHeight/2, 20));*/
    //scene->getDefaultCamera()->lookAt(Vector3(0, DrawScene::wallHeight/2, currentZ));
    scene->getDefaultCamera()->setPositionZ(scene->getDefaultCamera()->getPosition().z + VEL_CAM_Z);
    keepPlayerWithinBB();
    for(size_t i=0; i<obstacles.size(); i++){
        if(checkPlayerCollision(obstacles.at(i))){
            player->color.r -= 0.1;
            return core->updateAndRender();
        }
    }
    for(size_t i=0; i<walls.size(); i++){
        if(checkPlayerCollision(walls.at(i))){
            return core->updateAndRender();
        }
    }
    
	player->setPosition(player->getPosition() + playerVeloc);
	
    return core->updateAndRender();
}
