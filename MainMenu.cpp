#include "MainMenu.h"

int Hud::points = 0, Hud::livesLeft = 2;
Number Hud::maxHealth = 100.0, Hud::health = maxHealth;
const int MainMenuItem::ITEM_EXIT = 0;

Hud::Hud(Core *core, Vector2 imgCoinSize, Vector2 imgHealthSize, Vector2 healthBarSize, Vector2 margin, Number healthBarBorderW, Number blinkThresh, Number blinkPeriod, Color colLblLivesLeft, Color colLblPoints, Color colMaxHealth, Color colMinHealth, Color colBordMaxHealth, Color colBordMinHealth) : core(core), imgCoinSize(imgCoinSize), imgHealthSize(imgHealthSize), healthBarSize(healthBarSize), margin(margin), healthBarBorderW(healthBarBorderW), blinkThresh(blinkThresh), blinkPeriod(blinkPeriod), colLblLivesLeft(colLblLivesLeft), colLblPoints(colLblPoints), colMaxHealth(colMaxHealth), colMinHealth(colMinHealth), colBordMaxHealth(colBordMaxHealth), colBordMinHealth(colBordMinHealth), tmrBlink(NULL) {
    loadHud();
}

Hud::~Hud(){
    unloadHud();
}

void Hud::loadHud(){
    hud = new Screen();
    imgHealth = new ScreenImage("HUD_health.png");
    imgHealth->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
    imgHealth->setPosition(margin.x, margin.y+6);
    imgHealth->setShapeSize(imgHealthSize.x, imgHealthSize.y);
    healthBar = new ScreenShape(ScreenShape::SHAPE_RECT, healthBarSize.x, healthBarSize.y);
    healthBar->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
    healthBar->setPosition(imgHealth->getPosition2D() + Vector2(imgHealthSize.x+margin.x, (imgHealthSize.y-healthBarSize.y)/2));
    healthBarBorder = new ScreenShape(ScreenShape::SHAPE_RECT, healthBarSize.x, healthBarSize.y);
    healthBarBorder->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
    healthBarBorder->setPosition(healthBar->getPosition2D());
    healthBarBorder->lineWidth = 2;
    healthBarBorder->renderWireframe = true;
    lblLivesLeft = new ScreenLabel("", 26);
    lblLivesLeft->setPosition(healthBar->getPosition2D().x+healthBarSize.x+margin.x, margin.y);
    lblLivesLeft->setColor(colLblLivesLeft);
    lblPoints = new ScreenLabel("999", 26);
    lblPoints->setPositionY(margin.y);
    lblPoints->setColor(colLblPoints);
    imgCoin = new ScreenImage("HUD_coin.png");
    imgCoin->setPositionMode(ScreenEntity::POSITION_TOPLEFT);
    imgCoin->setPosition(core->getXRes()-2*margin.x-lblPoints->getWidth()-imgCoinSize.x, margin.y+5);
    imgCoin->setShapeSize(imgCoinSize.x, imgCoinSize.y);
    setPoints(points);
    restoreHealth();
    setLivesLeft(livesLeft);
    hud->addChild(imgHealth);
    hud->addChild(healthBar);
    hud->addChild(healthBarBorder);
    hud->addChild(lblLivesLeft);
    hud->addChild(imgCoin);
    hud->addChild(lblPoints);
}

void Hud::unloadHud(){
    imgHealth->~ScreenImage();
    imgCoin->~ScreenImage();
    healthBar->~ScreenShape();
    healthBarBorder->~ScreenShape();
    lblLivesLeft->~ScreenLabel();
    lblPoints->~ScreenLabel();
    if(tmrBlink){
        tmrBlink->removeAllHandlers();
        tmrBlink->~Timer();
        tmrBlink = NULL;
    }
    hud->~Screen();
    hud = NULL;
}

void Hud::updateTextWithNum(ScreenLabel*lbl, int num){
    stringstream ss;
    
    ss << num;
    lbl->setText(ss.str());
}

int Hud::getPoints(){
    return points;
}

int Hud::getLivesLeft(){
    return livesLeft;
}

Number Hud::getHealth(){
    return health;
}

bool Hud::isDead(){
    return (health <= 0.0);
}

bool Hud::isGameOver(){
    return (livesLeft < 0);
}

void Hud::setPoints(int pts){
    static Number posX = core->getXRes()-margin.x;
    
    points = pts;
    updateTextWithNum(lblPoints, pts);
    lblPoints->setPositionX(posX-lblPoints->getWidth());
}

void Hud::setLivesLeft(int lives){
    livesLeft = lives;
    if(livesLeft <= 0){
        lblLivesLeft->setText((livesLeft == 0)? "Last life!":"Game over!!");
        lblLivesLeft->setColor(colMinHealth);
    }else{
        updateTextWithNum(lblLivesLeft, lives);
        lblLivesLeft->setColor(colLblLivesLeft);
    }
}

void Hud::setHealth(Number healthPts){
    Number t;
    
    health = max(min(healthPts, maxHealth), -0.1);
    t = health/maxHealth;
    healthBar->setShapeSize(t*healthBarSize.x, healthBarSize.y);
    healthBar->setColor(colMaxHealth*t + colMinHealth*(1-t));
    healthBarBorder->setColor(colBordMaxHealth*t + colBordMinHealth*(1-t));
    if(t<blinkThresh && !tmrBlink){
        tmrBlink = new Timer(true, blinkPeriod);
        tmrBlink->addEventListener(this, Timer::EVENT_TRIGGER);
    }
}

void Hud::incPoints(int inc){
    setPoints(points+inc);
}

void Hud::decPoints(int dec){
    setPoints(points-dec);
}

void Hud::incLivesLeft(int inc){
    setLivesLeft(livesLeft+inc);
}

void Hud::decLivesLeft(int dec){
    setLivesLeft(livesLeft-dec);
}

void Hud::restoreHealth(){
    setHealth(maxHealth);
    healthBar->visible = true;
    if(tmrBlink){
        tmrBlink->removeAllHandlers();
        tmrBlink->~Timer();
        tmrBlink = NULL;
    }
}

void Hud::decHealth(int dec){
    setHealth(health-dec);
}

void Hud::handleEvent(Event *e){
    if(e->getDispatcher()==tmrBlink && e->getEventCode()==Timer::EVENT_TRIGGER){
        healthBar->visible = !healthBar->visible;
    }
}

MainMenuItem::MainMenuItem(Core *core, Hud *hud, MainMenu* menu, int numLevel, bool locked) : core(core), hud(hud), menu(menu), numLevel(numLevel), locked(locked), level(NULL), image(NULL), imageBorder(NULL) {
    stringstream ss;
    ss << numLevel;
    
    iniItem("Level " + ss.str() + ".xml", "Level " + ss.str() + ".png");
}

MainMenuItem::MainMenuItem(Core *core, Hud *hud, MainMenu* menu, int numLevel, string geometryFile, string imageFile, bool locked) : core(core), menu(menu), numLevel(numLevel), locked(locked), level(NULL), image(NULL), imageBorder(NULL) {
    iniItem(geometryFile, imageFile);
}

MainMenuItem::~MainMenuItem(){
    image->~ScenePrimitive();
    imageBorder->~ScenePrimitive();
    if(level) level->~Level();
}

void MainMenuItem::iniItem(string geomFile, string imageFile){
    Number th = (numLevel-1)*2*PI/menu->numMenuItems;
    
    geometryFile = geomFile;
    image = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, menu->itemSize, menu->itemSize);
    image->setPosition(Vector3(-menu->menuRad*sin(th), 0, -menu->menuRad*cos(th)));
    image->setRoll(180);
    image->setPitch(-90);
    image->setYaw(th*180/PI);
    image->loadTexture(imageFile);
    image->backfaceCulled = false;
    if(menu->itemBordW > 0){
        imageBorder = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, menu->itemSize, menu->itemSize);
        imageBorder->setPosition(image->getPosition());
        imageBorder->setRoll(image->getRoll());
        imageBorder->setPitch(image->getPitch());
        imageBorder->setYaw(image->getYaw());
        imageBorder->lineWidth = menu->itemBordW;
        imageBorder->renderWireframe = true;
    }else{
        imageBorder = NULL;
    }
}

void MainMenuItem::loadLevel(){
    if(numLevel == ITEM_EXIT){
        exit(EXIT_SUCCESS);
    }else{
        level = new Level(core, hud, this, geometryFile);
    }
}

void MainMenuItem::exitLevel(int code){
    if(level) level->~Level();
    level = NULL;
    menu->loadMenu();
    hud->restoreHealth();
    switch(code) {
        case Level::EXIT_SURVIVED:
            menu->unlockNextLevel(numLevel);
            break;
        case Level::EXIT_DIED:
        default:
            hud->decLivesLeft(1);
            if(hud->isGameOver()) menu->lockAll();
            break;
    }
}

bool MainMenuItem::Update(){
    return level->Update();
}

MainMenu::MainMenu(Core *core, Sound *sndRotate, Sound *sndSelect, Sound *sndSelectLocked, int numLevels, int numOthers, int selectedItem, Number incYaw, Number itemSize, Number itemBordW, Number itemGap, Color colActive, Color colInactive, Color colSelected, Color colLocked, Color colSelectedLocked, Scene *scn, SceneEntity *mn) : core(core), sndRotate(sndRotate), sndSelect(sndSelect), sndSelectLocked(sndSelectLocked), executingItem(false), numLevels(numLevels), selectedItem(selectedItem), angleRotLeft(0), incYaw(incYaw), itemSize(itemSize), itemBordW(itemBordW), itemGap(max(itemGap,0.5)), colActive(colActive), colInactive(colInactive), colSelected(colSelected), colLocked(colLocked), colSelectedLocked(colSelectedLocked), scene(scn), menu(mn), hud(NULL) {
    numMenuItems = numLevels + numOthers;
    menuRad = (itemSize+this->itemGap)/(2*tan(PI/numMenuItems));
    hud = new Hud(core);
    
    for(unsigned int i=1; i<=numLevels; i++){
        items.push_back(new MainMenuItem(core, hud, this, i, i!=1)); // All levels locked except for level 1
    }
    items.push_back(new MainMenuItem(core, hud, this, MainMenuItem::ITEM_EXIT, "", "Exit.png"));
    
    loadMenu();
    core->getInput()->addEventListener(this, InputEvent::EVENT_KEYDOWN);
    core->getInput()->addEventListener(this, InputEvent::EVENT_KEYUP);
}

MainMenu::~MainMenu(){
    while(items.size() > 0){
        items.front()->~MainMenuItem();
        items.pop_front();
    }
    if(sndRotate) sndRotate->~Sound();
    menu->ownsChildren = true;
    unloadMenu();
}

void MainMenu::loadMenu(){
    executingItem = false;
    if(!scene) scene = new Scene();
    if(!menu) menu = new SceneEntity();
    
    for(size_t i=0; i<items.size(); i++){
        menu->addChild(items.at(i)->image);
        menu->addChild(items.at(i)->imageBorder);
    }
    menu->setPosition(0, 0, menuRad);
    menu->setYaw(-selectedItem*360.0/numMenuItems);
    menu->ownsChildren = false;
    scene->addChild(menu);
    scene->getDefaultCamera()->setPosition(0, itemSize, -2*itemSize);
    scene->getDefaultCamera()->lookAt(Vector3(0,0,menuRad/2));
    recolorBorders();
}

void MainMenu::unloadMenu(){
    if(scene){
        scene->ownsChildren = true;
        CoreServices::getInstance()->getSceneManager()->removeScene(scene);
        scene = NULL;
        menu = NULL;
    }
}

void MainMenu::unlockNextLevel(int level){
    if(level < numLevels){
        items.at(level)->locked = false;
        rotateTo(level);
    }
}

void MainMenu::lockAll(){
    for(size_t i=0; i<items.size()-1; i++){
        items.at(i)->locked = true;
    }
    rotateTo(numMenuItems-1);
}

void MainMenu::recolorBorders(){
    for(size_t i=0; i<items.size(); i++){
        MainMenuItem *item = items.at(i);
        
        if(item->imageBorder){
            if(item->locked){
                item->image->setColor(0.5, 0.5, 0.5, 1.0);
                item->imageBorder->setColor(colLocked);
            }else{
                item->image->setColor(1.0, 1.0, 1.0, 1.0);
                if(selectedItem == i){
                    item->imageBorder->setColor(colActive);
                }else{
                    item->imageBorder->setColor(colInactive);
                }
            }
        }
    }
}

void MainMenu::rotateMenu(bool left){
    if(left){
        angleRotLeft += 360.0/numMenuItems;
        selectedItem = (numMenuItems + selectedItem-1)%numMenuItems;
    }else{
        angleRotLeft -= 360.0/numMenuItems;
        selectedItem = (selectedItem+1)%numMenuItems;
    }
    if(sndRotate) sndRotate->Play();
    recolorBorders();
}

void MainMenu::rotateTo(int target){
    int dist1 = abs(selectedItem-target), dist2 = numMenuItems-dist1;
    bool dist1Positive = (selectedItem-target > 0);
    
    if(dist1 < dist2){
        for(unsigned int i=0; i<dist1; i++){
            rotateMenu(dist1Positive);
        }
    }else{
        for(unsigned int i=0; i<dist2; i++){
            rotateMenu(!dist1Positive);
        }
    }
}

void MainMenu::handleEvent(Event *e){
    if(executingItem) return;
    InputEvent *inputEvent = (InputEvent*)e;
    
    if(e->getDispatcher() == core->getInput()){
        switch(e->getEventCode()) {
            case InputEvent::EVENT_KEYDOWN:
                if(inputEvent->keyCode() == KEY_RETURN) {
                    if(items.at(selectedItem)->locked){
                        items.at(selectedItem)->imageBorder->setColor(colSelectedLocked);
                        sndSelectLocked->Play();
                    }else{
                        items.at(selectedItem)->imageBorder->setColor(colSelected);
                        sndSelect->Play();
                    }
                }
                break;
            case InputEvent::EVENT_KEYUP:
                switch (inputEvent->keyCode()) {
                    case KEY_LEFT:
                        rotateMenu(true);
                        break;
                    case KEY_RIGHT:
                        rotateMenu(false);
                        break;
                    case KEY_RETURN:
                        if(!items.at(selectedItem)->locked){
                            executingItem = true;
                            items.at(selectedItem)->loadLevel();
                            unloadMenu();
                        }
                        break;
                    case KEY_ESCAPE:
                        rotateTo(numMenuItems-1);
                        break;
                }
                recolorBorders();
                break;
        }
    }
}

bool MainMenu::Update(){
    if(executingItem) return items.at(selectedItem)->Update();

    if(angleRotLeft >= incYaw){
        menu->setYaw(menu->getYaw()+incYaw);
        angleRotLeft -= incYaw;
    }else if(angleRotLeft <= -incYaw){
        menu->setYaw(menu->getYaw()-incYaw);
        angleRotLeft += incYaw;
    }else if(angleRotLeft != 0.0){
        menu->setYaw(menu->getYaw()+angleRotLeft);
        angleRotLeft = 0;
    }
    
    return core->updateAndRender();
}
