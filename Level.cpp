#include "Level.h"

#define VEL_PLAYER_INC  0.15
#define VEL_RET_HOME    0.1
#define CAM_VEL_Z       (DrawScene::speedFactor/(riverWidth*Wall::height))
#define CAM_INC_RAD     2
#define CAM_INC_ROT     0.03
#define CAM_INC_ELEV    0.03
#define CAM_MIN_RAD     20
#define CAM_MAX_RAD     100
#define CAM_MAX_ELEV    PI/2 - 0.01
#define BB_HALF_DEPTH   5
#define BB_FRTH_LIMIT   (currentZ + BB_HALF_DEPTH)
#define BB_CLSR_LIMIT   (currentZ - BB_HALF_DEPTH)
#define BB_TOP_LIMIT    (Wall::height - playerRad)
#define BB_BTM_LIMIT    playerRad
#define TIME_BTWN_COLL  0.5
#define SHOW_WIDTH_LINE false
#define SHOW_HOME_LINE  false
#define TIME_RETRN_HOME 500
#define TIME_FORCE_HOME 10000

Level::Level(Core *core, Hud *hud, MainMenuItem *mainMenu, string& geometryFile) : core(core), hud(hud), mainMenu(mainMenu) {
    loadLevel(geometryFile);
}

Level::~Level() {
    while(walls.size() > 0){
        walls.front()->~Wall();
        walls.pop_front();
    }
    while(obstacles.size() > 0){
        obstacles.front()->~Obstacle();
        obstacles.pop_front();
    }
    while(enemies.size() > 0){
        enemies.front()->~Enemy();
        enemies.pop_front();
    }
    while(coins.size() > 0){
        coins.front()->~Coin();
        coins.pop_front();
    }
    player->~ScenePrimitive();
    if(bgndMusic) bgndMusic->~Sound();
    tmrBackHome->removeAllHandlers();
    tmrBackHome->~Timer();
    tmrForceHome->removeAllHandlers();
    tmrForceHome->~Timer();
    scene->ownsChildren = true;
    scene->~CollisionScene();
    scene = NULL;
    core->getInput()->removeAllHandlersForListener(this);
}

void Level::loadLevel(string& geometryFile) {
    scene = new CollisionScene();
    lRiverWidth = new SceneLine(Vector3(), Vector3());
    lRiverWidth->setColor(0.0, 1.0, 0.0, 1.0);
    if(SHOW_WIDTH_LINE) scene->addChild(lRiverWidth);
    lHome = new SceneLine(Vector3(), Vector3());
    lHome->setColor(1.0, 0.0, 0.0, 1.0);
    if(SHOW_HOME_LINE) scene->addChild(lHome);
    DrawScene::drawScene(scene, player, walls, obstacles, enemies, coins, geometryFile.c_str());
    
    /*ScenePrimitive *sp = new ScenePrimitive(ScenePrimitive::TYPE_BOX, 3, 5, 3);
    sp->setPosition(0, 0, 75);
    sp->setColor(Enemy::defColor);
    Enemy *enemy = new Enemy(sp, 10, 4, 10, 8, 0);
    enemies.push_back(enemy);
    scene->addCollisionChild(sp);*/
    
    computeRiverWidth();
    
    camRad = DrawScene::iniCamRad;
	camRot = DrawScene::iniCamRot;
	camElev = DrawScene::iniCamElev;
    paused = false;
	mouse_clicked = false;
	left_pressed = false;
	right_pressed = false;
    up_pressed = false;
    down_pressed = false;
	further_pressed = false;
	closer_pressed = false;
    further_locked = false;
    closer_locked = false;
    stateHome = HOME;
    tmrBackHome = new Timer(true, TIME_RETRN_HOME);
    tmrBackHome->Pause(true);
    tmrBackHome->addEventListener(this, Timer::EVENT_TRIGGER);
    tmrForceHome = new Timer(false, TIME_FORCE_HOME);
    tmrForceHome->Pause(true);
    playerVeloc = Vector3(0, 0, CAM_VEL_Z);
    playerRad = player->getMesh()->getRadius();
    currentZ = player->getPosition().z;
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEMOVE);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEDOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEUP);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEWHEEL_DOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_MOUSEWHEEL_UP);
	core->getInput()->addEventListener(this, InputEvent::EVENT_KEYDOWN);
	core->getInput()->addEventListener(this, InputEvent::EVENT_KEYUP);
    Coin::sndCatch = new Sound("coin.ogg");
    if(DrawScene::backgndMusicEn){
        bgndMusic = new Sound("level1_bkgnd.ogg");
        bgndMusic->Play(true);
    }else{
        bgndMusic = NULL;
    }
    totalElapsed = 0;
    lastCollision = 0;
}

void Level::startTimers(){
    if(tmrBackHome->isPaused()){
        tmrBackHome->Reset();
        tmrBackHome->Pause(false);
        if(tmrForceHome->isPaused()){
            tmrForceHome->Reset();
            tmrForceHome->Pause(false);
        }
    }
}

void Level::playerSubtractHealth(Number healthSub, bool checkLastCollision){
    Number t;
    
    if(totalElapsed-lastCollision>TIME_BTWN_COLL || !checkLastCollision){
        if(checkLastCollision) lastCollision = totalElapsed;    // Keep track of the last time a collision took place
        if(healthSub <= 0) healthSub = 10 + (random()%10);      // Reduce health by 10~20 by default
        hud->decHealth(healthSub);
        if(hud->isDead()){
            if(bgndMusic) bgndMusic->~Sound();
            bgndMusic = new Sound("endLevelDied.ogg");
            bgndMusic->Play();
            mainMenu->exitLevel(Level::EXIT_DIED);
            return;
        }
        t = hud->getHealth()/hud->maxHealth;
        player->color = Player::defColor*t + Player::deadColor*(1-t);
    }
}

bool Level::checkPlayerCollision(ScenePrimitive *obstacle) {
    CollisionResult res = scene->testCollision(player, obstacle);
    Vector3 normal = res.colNormal, oldPos = player->getPosition();
    if(abs(normal.x) < 1e-6) normal.x = 0;
    if(abs(normal.y) < 1e-6) normal.y = 0;
    if(abs(normal.z) < 1e-6) normal.z = 0;
    normal.Normalize();
    
    if(res.collided && res.colNormal.length() > 0) {
        player->setPosition(oldPos + res.colNormal*res.colDist);
        if(player->getPosition().z < BB_CLSR_LIMIT){                                // If collision puts player outside BB -> Immediately subtract health
            if(normal.dot(Vector3(0,0,1)) > 0.5)    // Only if wall is a diagonal like this /\; Don't subtract health if it's like this: \/
                playerSubtractHealth(8*abs(player->getPosition().z - BB_CLSR_LIMIT)*normal.dot(Vector3(0,0,1)), false);
        }
        return true;
    }
    
    return false;
}

void Level::computeRiverWidth(){
    priority_queue<Vector2, vector<Vector2>, CompareVector2> q;
    Vector3 playerPos3 = player->getPosition();
    Vector2 dir(1, 0), pt, lastPt, playerPos(playerPos3.x, playerPos3.z);
    
    for(size_t i=0; i<walls.size(); i++){
        pt = playerPos;
        if(walls.at(i)->intersects(dir, pt)){           // From the player's position, throw a ray of dir (1, 0, 0)
            q.push(pt);                                 // Fill in the queue with all intersection points
        }
    }
    
    if(q.empty() || playerPos.x < q.top().x){           // Is player even further away than 1st wall (sorted by "x" coord)?
        playerSubtractHealth(hud->maxHealth, false);    // This should never happen. Kill player
        return;
    }
    while(!q.empty()){
        if(q.top().x > playerPos.x){                    // Iterate until a point with higher "x" than player's is found
            riverWidth = lastPt.distance(q.top());      // In that case, width = distance{last point, current point}
            lRiverWidth->setStart(Vector3(lastPt.x, playerPos3.y, lastPt.y));
            lRiverWidth->setEnd(Vector3(q.top().x, playerPos3.y, q.top().y));
            lHome->setStart(Vector3(lastPt.x, playerPos3.y, currentZ));
            lHome->setEnd(Vector3(q.top().x, playerPos3.y, currentZ));
            return;
        }
        lastPt = q.top();
        q.pop();                                        // If point wasn't the one I'm looking for, delete it
    }
    playerSubtractHealth(hud->maxHealth, false);        // This should never happen either. Kill player (it's even further away than last wall, sorted by "x" coord!)
}

void Level::recomputePlayerVelocZ() {
    if(further_pressed == closer_pressed){
        playerVeloc.z = CAM_VEL_Z + VEL_RET_HOME*(stateHome==RET_FROM_BACK) - VEL_RET_HOME*(stateHome==RET_FROM_FRONT);
    }else if(further_pressed){
        playerVeloc.z = VEL_PLAYER_INC + CAM_VEL_Z - VEL_RET_HOME*tmrForceHome->getElapsedf()*(stateHome==RET_FROM_FRONT);
    }else{
        playerVeloc.z =-VEL_PLAYER_INC + CAM_VEL_Z + VEL_RET_HOME*tmrForceHome->getElapsedf()*(stateHome==RET_FROM_BACK);
    }
}

void Level::recomputePlayerVeloc() {
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
    recomputePlayerVelocZ();
}

void Level::keepPlayerHome(){
    Number z = player->getPosition().z;
    
    if(z > BB_FRTH_LIMIT){
        player->setPositionZ(BB_FRTH_LIMIT);
        startTimers();
    }else if(z > currentZ){
        switch(stateHome) {
            case HOME:
                if(further_pressed == closer_pressed){
                    startTimers();
                }
                break;
            case RET_FROM_BACK:
                stateHome = HOME;
                player->setPositionZ(currentZ);
                tmrBackHome->Pause(true);
                tmrForceHome->Pause(true);
                if(closer_pressed){
                    closer_locked = true;
                    closer_pressed = false;
                }
            case RET_FROM_FRONT:
            default:
                recomputePlayerVelocZ();
                break;
        }
    }else if(z < BB_CLSR_LIMIT){
        player->setPositionZ(BB_CLSR_LIMIT);
        startTimers();
    }else if(z < currentZ){
        switch(stateHome) {
            case HOME:
                if(further_pressed == closer_pressed){
                    startTimers();
                }
                break;
            case RET_FROM_FRONT:
                stateHome = HOME;
                player->setPositionZ(currentZ);
                tmrBackHome->Pause(true);
                tmrForceHome->Pause(true);
                if(further_pressed){
                    further_locked = true;
                    further_pressed = false;
                }
            case RET_FROM_BACK:
            default:
                recomputePlayerVelocZ();
        }
    }
}

void Level::keepPlayerWithinBB(){
    Vector3 pos = player->getPosition();
    
    currentZ += CAM_VEL_Z;
    recomputePlayerVelocZ();
    keepPlayerHome();
    if(pos.y > BB_TOP_LIMIT){
        player->setPositionY(BB_TOP_LIMIT);
    }else if(pos.y <  BB_BTM_LIMIT){
        player->setPositionY(BB_BTM_LIMIT);
    }
}

void Level::updatePlayerPos(){
    computeRiverWidth();        // Compute the width of the river in this point
    if(!scene) return;          // This happens when player was found dead in computeRiverWidth()
    recomputePlayerVelocZ();    // Recompute player's "z" velocity based on new river width
    player->setPosition(player->getPosition() + playerVeloc);
    keepPlayerWithinBB();       // Update currentZ and keep player inside the BB
}

void Level::handleEvent(Event *e) {
    static Number mouse_x = 0, mouse_y = 0;
    static int bgndMusicOffset = 0;
    
    if(e->getDispatcher() == core->getInput()) {
        InputEvent *inputEvent = (InputEvent*)e;
        
        switch(e->getEventCode()) {
            case InputEvent::EVENT_MOUSEMOVE:
                if(mouse_clicked){
                    if(inputEvent->mousePosition.x > mouse_x){
                        camRot += CAM_INC_ROT;
                        if(camRot > PI) camRot -= 2*PI;
                        mouse_x = inputEvent->mousePosition.x;
                    }else if(inputEvent->mousePosition.x < mouse_x){
                        camRot -= CAM_INC_ROT;
                        if(camRot < -PI) camRot += 2*PI;
                        mouse_x = inputEvent->mousePosition.x;
                    }
                    if(inputEvent->mousePosition.y > mouse_y){
                        camElev += CAM_INC_ELEV;
                        if(camElev > CAM_MAX_ELEV) camElev = CAM_MAX_ELEV;
                        mouse_y = inputEvent->mousePosition.y;
                    }else if(inputEvent->mousePosition.y < mouse_y){
                        camElev -= CAM_INC_ELEV;
                        if(camElev < 0) camElev = 0;
                        mouse_y = inputEvent->mousePosition.y;
                    }
                }
                break;
            case InputEvent::EVENT_MOUSEDOWN:
                mouse_clicked = true;
                break;
            case InputEvent::EVENT_MOUSEUP:
                mouse_clicked = false;
                break;
            case InputEvent::EVENT_MOUSEWHEEL_DOWN:
                camRad += CAM_INC_RAD;
                if(camRad > CAM_MAX_RAD) camRad = CAM_MAX_RAD;
                break;
            case InputEvent::EVENT_MOUSEWHEEL_UP:
                camRad -= CAM_INC_RAD;
                if(camRad < CAM_MIN_RAD) camRad = CAM_MIN_RAD;
                break;
            case InputEvent::EVENT_KEYDOWN:
                switch (inputEvent->keyCode()) {
                    case KEY_LEFT:
                        left_pressed = true;
                        if(paused) hud->showPrevInstructPage();
                        break;
                    case KEY_RIGHT:
                        right_pressed = true;
                        if(paused) hud->showNextInstructPage();
                        break;
                    case KEY_UP:
                        up_pressed = true;
                        break;
                    case KEY_DOWN:
                        down_pressed = true;
                        break;
                    case 'w':
                    case 'W':
                        if(!further_locked) further_pressed = true;
                        break;
                    case 's':
                    case 'S':
                        if(!closer_locked) closer_pressed = true;
                        break;
                    case KEY_ESCAPE:
                        paused = !paused;
                        if(paused){
                            hud->showInstructions(2);
                        }else{
                            hud->hideInstructions();
                        }
                        if(bgndMusic){
                            if(paused){
                                bgndMusicOffset = bgndMusic->getOffset();
                                bgndMusic->Stop();
                            }else{
                                bgndMusic->Play(true);
                                bgndMusic->setOffset(bgndMusicOffset);
                            }
                        }
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
                        up_pressed = false;
                        break;
                    case KEY_DOWN:
                        down_pressed = false;
                        break;
                    case 'w':
                    case 'W':
                        further_pressed = false;
                        further_locked = false;
                        break;
                    case 's':
                    case 'S':
                        closer_pressed = false;
                        closer_locked = false;
                        break;
                }
                recomputePlayerVeloc();
                break;
        }
    }else if(e->getDispatcher()==tmrBackHome && e->getEventCode()==Timer::EVENT_TRIGGER){
        tmrBackHome->Pause(true);
        if(player->getPosition().z < currentZ){
            stateHome = RET_FROM_BACK;
            recomputePlayerVeloc();
        }else if(player->getPosition().z > currentZ){
            stateHome = RET_FROM_FRONT;
            recomputePlayerVeloc();
        }
    }
}

bool Level::Update() {
    if(paused) return core->updateAndRender();
	Number elapsed = core->getElapsed();
    totalElapsed += elapsed;
    
	scene->getDefaultCamera()->setPositionX(camRad*cos(camElev)*cos(camRot));
    scene->getDefaultCamera()->setPositionY(camRad*sin(camElev) + Wall::height);
	scene->getDefaultCamera()->setPositionZ(camRad*cos(camElev)*sin(camRot) + currentZ);
    scene->getDefaultCamera()->lookAt(Vector3(0, Wall::height/2, currentZ));
    
    if(checkPlayerCollision(walls.front()->wall)){  // Check if player "collided" with goal wall
        if(bgndMusic) bgndMusic->~Sound();
        bgndMusic = new Sound("endLevelSurvived.ogg");
        bgndMusic->Play();
        mainMenu->exitLevel(Level::EXIT_SURVIVED);
        return core->updateAndRender();
    }
    for(size_t i=0; i<coins.size(); i++){           // COINS
        if(coins.at(i)->coin->visible){
            coins.at(i)->update(totalElapsed);
            CollisionResult res = scene->testCollision(player, coins.at(i)->coin);
            if(res.collided) hud->incPoints(coins.at(i)->catchCoin());
        }
    }
    for(size_t i=0; i<enemies.size(); i++){         // ENEMIES
        enemies.at(i)->update(totalElapsed);
        if(checkPlayerCollision(enemies.at(i)->enemy)){
            playerSubtractHealth();
            if(hud->isDead()) return core->updateAndRender();
        }
    }
    for(size_t i=0; i<obstacles.size(); i++){       // OBSTACLES
        obstacles.at(i)->update(totalElapsed);
        if(checkPlayerCollision(obstacles.at(i)->obstacle)){
            playerSubtractHealth();
            if(hud->isDead()) return core->updateAndRender();
        }
    }
    for(size_t i=1; i<walls.size(); i++){           // WALLS
        checkPlayerCollision(walls.at(i)->wall);
    }

    updatePlayerPos();                              // PLAYER
	
    return core->updateAndRender();
}
