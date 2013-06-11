#include "MainMenu.h"

const int MainMenuItem::ITEM_EXIT = 0;

MainMenuItem::MainMenuItem(Core *core, MainMenu* menu, int numLevel, bool locked) : core(core), menu(menu), numLevel(numLevel), locked(locked), level(NULL), image(NULL), imageBorder(NULL) {
    stringstream ss;
    ss << numLevel;
    
    iniItem("Level " + ss.str() + ".xml", "Level " + ss.str() + ".png");
}

MainMenuItem::MainMenuItem(Core *core, MainMenu* menu, int numLevel, string geometryFile, string imageFile, bool locked) : core(core), menu(menu), numLevel(numLevel), locked(locked), level(NULL), image(NULL), imageBorder(NULL) {
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
        level = new Level(core, this, geometryFile);
    }
}

void MainMenuItem::exitLevel(int code){
    level->~Level();
    level = NULL;
    menu->loadMenu();
    switch(code) {
        case Level::EXIT_SURVIVED:
            menu->unlockNextLevel(numLevel);
            break;
        case Level::EXIT_DIED:
        default:
            // One life less
            // If no more lifes, game over!!
            break;
    }
}

bool MainMenuItem::Update(){
    if(level) return level->Update();
    return true;
}

MainMenu::MainMenu(Core *core, Sound *sndRotate, Sound *sndSelect, Sound *sndSelectLocked, int numLevels, int numOthers, int selectedItem, Number incYaw, Number itemSize, Number itemBordW, Number itemGap, Color colActive, Color colInactive, Color colSelected, Color colLocked, Color colSelectedLocked, Scene *scn, SceneEntity *mn) : core(core), sndRotate(sndRotate), sndSelect(sndSelect), sndSelectLocked(sndSelectLocked), executingItem(false), numLevels(numLevels), selectedItem(selectedItem), angleRotLeft(0), incYaw(incYaw), itemSize(itemSize), itemBordW(itemBordW), itemGap(itemGap), colActive(colActive), colInactive(colInactive), colSelected(colSelected), colLocked(colLocked), colSelectedLocked(colSelectedLocked), scene(scn), menu(mn) {
    numMenuItems = numLevels + numOthers;
    menuRad = (itemSize+itemGap)/sqrt(2*(1-cos(2*PI/numMenuItems)));
    
    for(unsigned int i=1; i<=numLevels; i++){
        items.push_back(new MainMenuItem(core, this, i, i!=1)); // All levels locked except for level 1
    }
    items.push_back(new MainMenuItem(core, this, MainMenuItem::ITEM_EXIT, "", "Exit.png"));
    
    loadMenu();
    CoreServices::getInstance()->getCore()->getInput()->addEventListener(this, InputEvent::EVENT_KEYDOWN);
    CoreServices::getInstance()->getCore()->getInput()->addEventListener(this, InputEvent::EVENT_KEYUP);
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
    
    if(e->getDispatcher() == CoreServices::getInstance()->getCore()->getInput()){
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
