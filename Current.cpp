#include "Current.h"

#define VEL_PLAYER_INC  0.15
#define CAM_VEL_Z       0.1
#define CAM_INC_ROT     0.03
#define CAM_INC_ELEV    0.03
#define BB_HALF_DEPTH   8
#define BB_FRTH_LIMIT   currentZ + BB_HALF_DEPTH
#define BB_CLSR_LIMIT   currentZ - BB_HALF_DEPTH
#define BB_TOP_LIMIT    DrawScene::wallHeight - playerRad
#define BB_BTM_LIMIT    playerRad
#define TIME_BTWN_COLL  0.5

Current::Current(PolycodeView *view) {
	core = new POLYCODE_CORE(view, 640,480,false,false,0,0,90);
	CoreServices::getInstance()->getResourceManager()->addDirResource("default", false);
    
	scene = new CollisionScene();
    DrawScene::drawScene(scene, player, walls, obstacles, enemies, coins, "geometry.xml");
    
    camRad = DrawScene::iniCamRad;
	camRot = DrawScene::iniCamRot;
	camElev = DrawScene::iniCamElev;
	mouse_clicked = false;
	left_pressed = false;
	right_pressed = false;
	further_pressed = false;
	closer_pressed = false;
    up_pressed = false;
    down_pressed = false;
    playerVeloc = Vector3(0, 0, CAM_VEL_Z);
    playerRad = player->getMesh()->getRadius();
    currentZ = player->getPosition().z;
    totalElapsed = 0;
    lastCollision = 0;
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEMOVE);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	core->getInput()->addEventListener(this, InputEvent::EVENT_KEYDOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_KEYUP);
    Coin::sndCatch = new Sound("coin.ogg");
    Sound *s = new Sound("background.ogg");
    s->Play(true);
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
        playerVeloc.z = CAM_VEL_Z;
    }else if(further_pressed){
        playerVeloc.z = VEL_PLAYER_INC + CAM_VEL_Z;
    }else{
        playerVeloc.z =-VEL_PLAYER_INC /*+ CAM_VEL_Z*/;
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
    
    currentZ += CAM_VEL_Z;
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
                    camRot += CAM_INC_ROT;
                    if(camRot > PI) camRot -= 2*PI;
                    cout << "camRot: " << camRot << "; \t camElev: " << camElev << "\n";
                    mouse_x = inputEvent->mousePosition.x;
                }else if(inputEvent->mousePosition.x < mouse_x && mouse_clicked){
                    camRot -= CAM_INC_ROT;
                    if(camRot < -PI) camRot += 2*PI;
                    cout << "camRot: " << camRot << "; \t camElev: " << camElev << "\n";
                    mouse_x = inputEvent->mousePosition.x;
                }
                if(inputEvent->mousePosition.y > mouse_y && mouse_clicked){
                    camElev += CAM_INC_ELEV;
                    if(camElev > PI/2) camElev = PI/2;
                    cout << "camRot: " << camRot << "; \t camElev: " << camElev << "\n";
                    mouse_y = inputEvent->mousePosition.y;
                } else if(inputEvent->mousePosition.y < mouse_y && mouse_clicked) {
                    camElev -= CAM_INC_ELEV;
                    if(camElev < 0.1) camElev = 0.1;
                    cout << "camRot: " << camRot << "; \t camElev: " << camElev << "\n";
                    mouse_y = inputEvent->mousePosition.y;
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
    
    keepPlayerWithinBB();
	scene->getDefaultCamera()->setPositionX(camRad*cos(camElev)*cos(camRot));
    scene->getDefaultCamera()->setPositionY(camRad*sin(camElev) + 10);
	scene->getDefaultCamera()->setPositionZ(camRad*cos(camElev)*sin(camRot) + currentZ);
    scene->getDefaultCamera()->lookAt(Vector3(0, DrawScene::wallHeight/2, currentZ));

    for(size_t i=0; i<coins.size(); i++){
        if(coins.at(i)->coin->visible){
            coins.at(i)->update(totalElapsed);
            CollisionResult res = scene->testCollision(player, coins.at(i)->coin);
            if(res.collided){
                coins.at(i)->catchCoin();
                // Increase points!
            }
        }
    }
    for(size_t i=0; i<enemies.size(); i++){
        enemies.at(i)->update(totalElapsed);
        if(checkPlayerCollision(enemies.at(i)->enemy)){
            if(totalElapsed-lastCollision > TIME_BTWN_COLL){
                player->color.r -= 0.2;
                lastCollision = totalElapsed;
            }
            //return core->updateAndRender();
        }
    }
    for(size_t i=0; i<obstacles.size(); i++){
        if(checkPlayerCollision(obstacles.at(i))){
            if(totalElapsed-lastCollision > TIME_BTWN_COLL){
                player->color.r -= 0.2;
                lastCollision = totalElapsed;
            }
            //return core->updateAndRender();
        }
    }
    for(size_t i=0; i<walls.size(); i++){
        if(checkPlayerCollision(walls.at(i))){
            //return core->updateAndRender();
        }
    }
    
	player->setPosition(player->getPosition() + playerVeloc);
	
    return core->updateAndRender();
}
