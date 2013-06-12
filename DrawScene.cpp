#include "DrawScene.h"

#define ERROR(s)        {cout << "Error: " << s << ". Exiting application.\n"; exit(EXIT_FAILURE);}
#define DEF_MUSIC_EN    true
#define DEF_CAM_RADIUS  40
#define DEF_CAM_ROT     -100*PI/180
#define DEF_CAM_ELEV    20*PI/180
#define CAM_MIN_RADIUS  20
#define CAM_MAX_ELEV    (PI/2 - 0.01)
#define DEF_FOG_DENS    0.02
#define DEF_FOG_COLOR   Color(0.0, 0.0, 0.2, 1.0)
#define DEF_OBST_BORDER 1
#define DEF_ENMY_BORDER 1
#define DEF_WALL_BORDER 3
#define DEF_WALL_WIDTH  0.0
#define DEF_WALL_HEIGHT 10
#define DEF_WALL_COLOR  Color(0.0, 0.6, 0.8, 1.0)
#define DEF_WL_BORD_COL Color(0.0, 0.3, 0.5, 1.0)
#define DEF_OBST_COLOR  Color(0.0, 1.0, 0.3, 1.0)
#define DEF_OBS_BRD_COL Color(0.0, 0.5, 0.5, 1.0)
#define DEF_ENMY_COLOR  Color(1.0, 0.0, 0.1, 1.0)
#define DEF_ENM_BRD_COL Color(1.0, 0.4, 0.4, 1.0)
#define DEF_COIN_SM_COL Color(1.0, 0.8, 0.0, 1.0)
#define DEF_COIN_LG_COL Color(1.0, 0.5, 0.1, 1.0)
#define COIN_SM_PARAMS  ScenePrimitive::TYPE_CYLINDER, 0.1, 1, 30
#define COIN_LG_PARAMS  ScenePrimitive::TYPE_CYLINDER, 0.15, 1.1, 30
#define DEF_PLAY_TYPE   ScenePrimitive::TYPE_SPHERE
#define DEF_PLAY_SHAPE  CollisionSceneEntity::SHAPE_SPHERE
#define DEF_PLAY_SIZE   1.25, 25, 25
#define DEF_PLAY_POS    Vector3(0, 1.5, 0)
#define DEF_PLAY_COLOR  Color(1.0, 1.0, 0.0, 1.0)
#define DEF_PL_DEAD_COL Color(1.0, 0.0, 0.0, 1.0)

Number Wall::width = DEF_WALL_WIDTH, Wall::height = DEF_WALL_HEIGHT;
int Coin::valueSm = 1, Coin::valueLg = 3;
Number Coin::rotationVel = 50;
Sound *Coin::sndCatch = NULL;
deque<Wall*>* DrawScene::walls = NULL;
deque<Obstacle*>* DrawScene::obstacles = NULL;
deque<Enemy*>* DrawScene::enemies = NULL;
deque<Coin*>* DrawScene::coins = NULL;
ScenePrimitive* DrawScene::player = NULL;

bool DrawScene::backgndMusicEn = DEF_MUSIC_EN;
Number DrawScene::iniCamRad = DEF_CAM_RADIUS, DrawScene::iniCamElev = DEF_CAM_ELEV, DrawScene::iniCamRot = DEF_CAM_ROT;
Number Wall::defBorder = DEF_WALL_BORDER, Obstacle::defBorder = DEF_OBST_BORDER, Enemy::defBorder = DEF_ENMY_BORDER;
Color Wall::defColor = DEF_WALL_COLOR, Wall::defBordColor = DEF_WL_BORD_COL;
Color Obstacle::defColor = DEF_OBST_COLOR, Obstacle::defBordColor = DEF_OBS_BRD_COL;
Color Enemy::defColor = DEF_ENMY_COLOR, Enemy::defBordColor = DEF_ENM_BRD_COL;
Color Coin::colorSm = DEF_COIN_SM_COL, Coin::colorLg = DEF_COIN_LG_COL;
Color Player::defColor = DEF_PLAY_COLOR, Player::deadColor = DEF_PL_DEAD_COL;

Wall::Wall(CollisionScene *scene, Vector3 lastVertex, Vector3 newVertex, Number border, Color color, Color bordColor){
    start = Vector2(lastVertex.x, lastVertex.z);// Get the start and end points of the wall from its vertices (ignore "y" coord)
    end = Vector2(newVertex.x, newVertex.z);
    lastVertex.y = height/2;                    // Make sure to set the "y" coord to height/2 (so that wall's "y" ranges 0~height)
    newVertex.y = height/2;
    wall = new ScenePrimitive(ScenePrimitive::TYPE_BOX, height, width, newVertex.distance(lastVertex));
    wall->setPosition((lastVertex+newVertex)/2);// Set the wall's "center", color, and make it point in the direction start->end
    wall->setColor(color);
    wall->lookAt(newVertex, Vector3(0, 1, 0).crossProduct(newVertex-lastVertex));
    scene->addCollisionChild(wall, CollisionSceneEntity::SHAPE_BOX);
    drawWallBorder(scene, border, bordColor);   // After adding it to the scene, draw its border (if needed)
}

Wall::~Wall(){
    wall->~ScenePrimitive();
}

void Wall::drawWallBorder(CollisionScene *scene, Number border, Color borderCol){
    if(border <= 0) return;                     // Only render border if a positive value is read
    ScenePrimitive *wallBorder;                 // Plane with the same size and orientation as the wall specified by the given vertices
    Vector3 startPt(start.x, height/2, start.y), endPt(end.x, height/2, end.y);
    
    wallBorder = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, height, start.distance(end));
    wallBorder->setPosition((startPt+endPt)/2); // Set the border's "center", color, and make it point in the direction start->end
    wallBorder->setColor(borderCol);
    wallBorder->lookAt(endPt, Vector3(0, 1, 0).crossProduct(endPt-startPt));
    wallBorder->lineWidth = border;             // Set the desired border width
    wallBorder->renderWireframe = true;
    scene->addChild(wallBorder);                // Lastly, add the border to the scene
}

Vector2 Wall::getSegment(){
    return end-start;
}

bool Wall::intersects(const Vector2& lDelta, Vector2& pt) {
    Vector2 delta = getSegment(), lNormal(lDelta.y, -lDelta.x);
    lNormal.Normalize();
    Number t, eps=1e-12f, denom = lNormal.dot(delta);
    
    //cout << "Checking intersection between: WALL {" << start.x << "," << start.y << " -> " << end.x << "," << end.y << "} LINE {" << pt.x << "," << pt.y << "; Dir: " << lDelta.x << "," << lDelta.y << "}\n";
    
    if(denom == 0) return false;        // Parallel lines: no intersection
    t = lNormal.dot(pt-start)/denom;    // t = (l.N * (l.p1-p1))/(l.N * (p2-p1))
    if(t<-eps || t>1+eps) return false; // t must be within [0, 1] (due to precision errors, within [-eps, 1+eps])
    
    pt = start + getSegment()*t;        // pt = p1 + t(p2-p1)
    return true;
}

Obstacle::Obstacle(ScenePrimitive *obstacle, Number amplitude, Number velocity, Number offset, Vector3 movementDir) : obstacle(obstacle), amplitude(amplitude), velocity(velocity), offset(offset), movementDir(movementDir/movementDir.length()), middlePos(obstacle->getPosition()), border(NULL) {}
Obstacle::Obstacle(ScenePrimitive *obstacle, Vector3 middlePos, Number amplitude, Number velocity, Number offset, Vector3 movementDir) : obstacle(obstacle), middlePos(middlePos), amplitude(amplitude), velocity(velocity), offset(offset), movementDir(movementDir/movementDir.length()), border(NULL) {}
Obstacle::~Obstacle(){
    obstacle->~ScenePrimitive();
    if(border) border->~ScenePrimitive();
}

void Obstacle::update(Number totalElapsed){
    Vector3 oldPos = obstacle->getPosition();
    
    obstacle->setPosition(middlePos + movementDir*amplitude*sin(velocity*totalElapsed + offset));
    if(border) border->Translate(obstacle->getPosition() - oldPos); // Not only update the obstacle, but also its border!
}

Enemy::Enemy(ScenePrimitive *enemy, Number ampH, Number ampV, Number velH, Number velV, Number offsH, Number offsV, Vector3 dirH, bool loopH, bool halfV) : enemy(enemy), ampH(ampH), ampV(ampV), velH(velH), velV(velV), offsH(offsH), offsV(offsV), dirH(Vector3(dirH.x, 0, dirH.z)/Vector3(dirH.x, 0, dirH.z).length()), middlePos(enemy->getPosition()), loopH(loopH), halfV(halfV), border(NULL) {}
Enemy::Enemy(ScenePrimitive *enemy, Vector3 middlePos, Number ampH, Number ampV, Number velH, Number velV, Number offsH, Number offsV, Vector3 dirH, bool loopH, bool halfV) : enemy(enemy), ampH(ampH), ampV(ampV), velH(velH), velV(velV), offsH(offsH), offsV(offsV), dirH(Vector3(dirH.x, 0, dirH.z)/Vector3(dirH.x, 0, dirH.z).length()), middlePos(middlePos), loopH(loopH), halfV(halfV), border(NULL) {}
Enemy::~Enemy(){
    enemy->~ScenePrimitive();
    if(border) border->~ScenePrimitive();
}

void Enemy::update(Number totalElapsed){
    Vector3 oldPos = enemy->getPosition();
    Number moveH = fmod(velH*totalElapsed/ampH + offsH/90.0, 4);
    Number factor = min(1.0,moveH) - min(2.0, max(0.0,moveH-1)) + max(0.0,moveH-3);
    
    enemy->setPositionX(middlePos.x + ampH*dirH.x*factor);
    enemy->setPositionZ(middlePos.z + ampH*dirH.z*factor);
    if(halfV){
        enemy->setPositionY(middlePos.y + ampV*abs(sin(velV*totalElapsed + offsV)));
    }else{
        enemy->setPositionY(middlePos.y + ampV*sin(velV*totalElapsed + offsV));
    }
    
    if(border) border->Translate(enemy->getPosition() - oldPos);    // Not only update the enemy, but also its border!
}

Coin::Coin(CollisionScene *scene, Vector3 pos, bool hasSmallValue, Number offset) : hasSmallValue(hasSmallValue), offset(offset) {
    if(hasSmallValue){
        coin = new ScenePrimitive(COIN_SM_PARAMS);
        coin->setColor(colorSm);
    }else{
        coin = new ScenePrimitive(COIN_LG_PARAMS);
        coin->setColor(colorLg);
    }
    coin->setPosition(pos);
    coin->setPitch(-90);    // Vertical
    scene->addCollisionChild(coin);
}

Coin::~Coin(){
    coin->~ScenePrimitive();
}

int Coin::catchCoin(){
    coin->visible = false;
    sndCatch->Play();
    return (hasSmallValue)? valueSm:valueLg;
}

void Coin::update(Number totalElapsed){
    coin->setYaw(rotationVel*(totalElapsed + offset));  // Rotate the coin based on the total time elapsed
}

bool DrawScene::fileExists(const char *strFile){
    ifstream f(strFile);
    bool ret = f;

    f.close();

    return ret;
}

void DrawScene::getColor(Color& color, xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("r", 0, false);
    if(attr) color.r = atof(attr->value());
    attr = node->first_attribute("g", 0, false);
    if(attr) color.g = atof(attr->value());
    attr = node->first_attribute("b", 0, false);
    if(attr) color.b = atof(attr->value());
    attr = node->first_attribute("a", 0, false);
    if(attr) color.a = atof(attr->value());
}

void DrawScene::getColor(Color& color, xml_node<> *node, string prefix){
    xml_attribute<> *attr;
    
    attr = node->first_attribute((prefix+"R").c_str(), 0, false);
    if(attr) color.r = atof(attr->value());
    attr = node->first_attribute((prefix+"G").c_str(), 0, false);
    if(attr) color.g = atof(attr->value());
    attr = node->first_attribute((prefix+"B").c_str(), 0, false);
    if(attr) color.b = atof(attr->value());
    attr = node->first_attribute((prefix+"A").c_str(), 0, false);
    if(attr) color.a = atof(attr->value());
}

void DrawScene::getPosition(Vector3& pos, xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("x", 0, false);
    if(attr) pos.x = atof(attr->value());
    attr = node->first_attribute("y", 0, false);
    if(attr) pos.y = atof(attr->value());
    attr = node->first_attribute("z", 0, false);
    if(attr) pos.z = atof(attr->value());
}

void DrawScene::getMovementDir(Vector3& dir, xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("dirX", 0, false);
    if(attr) dir.x = atof(attr->value());
    attr = node->first_attribute("dirY", 0, false);
    if(attr) dir.y = atof(attr->value());
    attr = node->first_attribute("dirZ", 0, false);
    if(attr) dir.z = atof(attr->value());
    
    dir.Normalize();
}

Number DrawScene::getNumber(const char* attrName, xml_node<> *node, Number defValue, bool absVal){
    xml_attribute<> *attr = node->first_attribute(attrName, 0, false);
    if(!attr) return defValue;
    
    return (absVal)? abs(atof(attr->value())):atof(attr->value());  // Return the absolute value of the attribute unless absVal is false
}

Number DrawScene::getWidth(xml_node<> *node, Number defValue){
    return getNumber("w", node, defValue);
}

Number DrawScene::getHeight(xml_node<> *node, Number defValue){
    return getNumber("h", node, defValue);
}

Number DrawScene::getDepth(xml_node<> *node, Number defValue){
    return getNumber("d", node, defValue);
}

Number DrawScene::getRadius(xml_node<> *node, Number defValue){
    return getNumber("rad", node, defValue);
}

Number DrawScene::getRadius2(xml_node<> *node, Number defValue){
    return getNumber("rad2", node, defValue);
}

Number DrawScene::getBorder(xml_node<> *node, Number defValue){
    return getNumber("bord", node, defValue);
}

Number DrawScene::getAmplitude(xml_node<> *node, Number defValue){
    return getNumber("amp", node, defValue);
}

Number DrawScene::getVelocity(xml_node<> *node, Number defValue){
    return getNumber("vel", node, defValue);
}

Number DrawScene::getOffset(xml_node<> *node, Number defValue){
    return getNumber("offs", node, defValue, false)*PI/180;
}

bool DrawScene::isAttributeTrue(const char* attrName, xml_node<> *node, bool defValue){
    xml_attribute<> *attr = node->first_attribute(attrName, 0, false);
    if(!attr) return defValue;                                  // If attribute doesn't exist -> defValue
    
    string val = string(attr->value());
    transform(val.begin(), val.end(), val.begin(), ::tolower);  // Store the lowercase version of the value
    return (atof(attr->value())==1 || val.compare("true")==0);  // Valid values: '1' and 'true'
}

bool DrawScene::hasLargeValue(xml_node<> *node){
    return isAttributeTrue("lrg", node);
}

bool DrawScene::isTransparent(xml_node<> *node){
    return isAttributeTrue("t", node);
}

bool DrawScene::isGoal(xml_node<> *node){
    return isAttributeTrue("goal", node);
}

ScenePrimitive* DrawScene::iniPrimitive(int& shape, xml_node<> *node,  bool boxReversed, Number numSegments){
    xml_attribute<> *attr;
    string type;
    Number a1, a2, a3;
    
    attr = node->first_attribute("type", 0, false); // Read the primitive's type from the node attributes
    if(!attr) return NULL;                          // Exit if type is not present
    type = string(attr->value());                   // Store the uppercase version of the type
    transform(type.begin(), type.end(), type.begin(), ::toupper);
    
    if(type.compare("BOX") == 0){                   // Initialize the primitive depending on the type read
        shape = CollisionSceneEntity::SHAPE_BOX;
        a1 = getWidth(node, 1);
        a2 = getHeight(node, 1);
        a3 = getDepth(node, 1);                     // NOTE: boxReversed is a fix to this issue: when method lookAt is called on a box entity the..
                                                    // ..width and the depth values get 'reversed', so we reverse them again if boxReversed is true
        if(boxReversed) return new ScenePrimitive(ScenePrimitive::TYPE_BOX, a3, a2, a1);
        return new ScenePrimitive(ScenePrimitive::TYPE_BOX, a1, a2, a3);
    }else if(type.compare("PLANE") == 0){
        shape = CollisionSceneEntity::SHAPE_PLANE;
        a1 = getWidth(node);
        a2 = getHeight(node);
        a3 = getDepth(node);
        
        if(a2 != 0.0){
            ScenePrimitive *aux = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, a1, a2);
            aux->setPitch(-90);
            return aux;
        }else if(a3 != 0.0){
            return new ScenePrimitive(ScenePrimitive::TYPE_PLANE, a1, a3);
        }else{
            return NULL;
        }
    }else if(type.compare("SPHERE") == 0){
        shape = CollisionSceneEntity::SHAPE_SPHERE;
        a1 = getRadius(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_SPHERE, a1, numSegments, numSegments);
    }else if(type.compare("CYLINDER") == 0){
        shape = CollisionSceneEntity::SHAPE_CYLINDER;
        a1 = getHeight(node, Wall::height);
        a2 = getRadius(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_CYLINDER, a1, a2, numSegments);
    }else if(type.compare("UNCAPPED_CYLINDER") == 0){
        shape = CollisionSceneEntity::SHAPE_CYLINDER;
        a1 = getHeight(node, Wall::height);
        a2 = getRadius(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_UNCAPPED_CYLINDER, a1, a2, numSegments);
    }else if(type.compare("CONE") == 0){
        shape = CollisionSceneEntity::SHAPE_CONE;
        a1 = getHeight(node, Wall::height);
        a2 = getRadius(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_CONE, a1, a2, numSegments);
    }else if(type.compare("TORUS") == 0){
        shape = CollisionSceneEntity::SHAPE_SPHERE;
        a1 = getRadius(node, 2);
        a2 = getRadius2(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_TORUS, a1, a2, numSegments, numSegments);
    }
    
    return NULL;
}

ScenePrimitive* DrawScene::iniShapeBorder(CollisionScene *scene, Number border, int shape, Vector3 pos, Color col, xml_node<> *node, bool boxReversed){
    if(border == 0.0) return NULL;                  // If border wasn't specified, exit
    ScenePrimitive *primBorder = NULL;
    
    switch(shape){                                  // Depending on the shape of the obstacle, we require different shapes for the borders
        case CollisionSceneEntity::SHAPE_BOX:
        case CollisionSceneEntity::SHAPE_PLANE:
            primBorder = iniPrimitive(shape, node, boxReversed);
            primBorder->getMesh()->setMeshType(Mesh::LINE_LOOP_MESH);
            primBorder->lineWidth = border;
            primBorder->renderWireframe = true;
            break;
        case CollisionSceneEntity::SHAPE_CYLINDER:
        case CollisionSceneEntity::SHAPE_CONE:
            primBorder = iniPrimitive(shape, node, false, 300);
            primBorder->getMesh()->setMeshType(Mesh::POINT_MESH);
            break;
        case CollisionSceneEntity::SHAPE_SPHERE:
        default:
            primBorder = NULL;
            break;
    }
    
    if(primBorder){                                 // If the entity requires a border, add a wireframe shape to the scene
        primBorder->setColor(col);
        primBorder->setPosition(pos);
    }
    
    return primBorder;
}

void DrawScene::drawWalls(CollisionScene *scene, xml_node<> *ndWalls){
    if(!ndWalls) ERROR("Incorrect format: One child of 'geometry' must be 'walls'");
    Wall *wall = NULL;
    Vector3 lastVertex, newVertex;
    Number border;
    Color color, bordColor;
    bool goalSaved = false;                     // True when a wall with the attribute 'goal' is read
    xml_node<> *node = ndWalls->first_node("vertex", 0, false); // Get 1st vertex and make sure there are at least 3 vertices (2 walls)
    if(!node || !node->next_sibling() || !node->next_sibling()->next_sibling()) ERROR("Incorrect format: The tag 'walls' must have at least 3 'vertex' children");
    
    getColor(Wall::defColor, ndWalls);          // Configure parameters by default (values in case not specified)
    getColor(Wall::defBordColor, ndWalls, "bord");
    Wall::defBorder = getBorder(ndWalls, Wall::defBorder);
    Wall::width = getWidth(ndWalls, Wall::width);
    Wall::height = getHeight(ndWalls, Wall::height);
    
    getPosition(lastVertex, node);              // Read the first vertex
    while((node = node->next_sibling()) != 0){  // While there are vertices to read
        getPosition(newVertex, node);           // Read the current vertex
        color = Wall::defColor;
        getColor(color, node);                  // Get the color of the wall (if given)
        border = getBorder(node, Wall::defBorder);
        bordColor = Wall::defBordColor;
        getColor(bordColor, node, "bord");
        if(!isTransparent(node)){               // Draw wall if is not transparent
            if(isGoal(node) && !goalSaved){     // Only allow one goal to be saved (at walls[0])
                wall = new Wall(scene, lastVertex, newVertex, 0, color, bordColor);
                goalSaved = true;
                wall->wall->visible = false;    // Invisible wall!
                walls->push_front(wall);        // Identify the goal by pushing the wall to the front
            }else{
                wall = new Wall(scene, lastVertex, newVertex, border, color, bordColor);
                walls->push_back(wall);         // Store pointer in the back of the queue walls
            }
        }
        lastVertex = newVertex;                 // In the next iteration, lastVertex will be the current vertex
    }
    
    ScenePrimitive *sp = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, 5000, 5000);
    sp->setPositionY(-0.2);
    sp->setColor(1.0, 1.0, 0.5, 1.0);
    scene->addChild(sp);
}

/*void DrawScene::drawObstacles(CollisionScene *scene, xml_node<> *ndObstacles){
    if(!ndObstacles) ERROR("Incorrect format: One child of 'geometry' must be 'obstacles'");
    ScenePrimitive *scnPrimitive = NULL, *obsBorder = NULL;
    Vector3 pos;
    Number border;
    Color color, bordCol;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    xml_node<> *node = ndObstacles->first_node("obstacle", 0, false);   // Get the first obstacle
    if(!node) ERROR("Incorrect format: The tag 'obstacles' must have at least 1 'obstacle' child");
    
    getColor(Obstacle::defColor, ndObstacles);                  // Configure parameters by default (values in case not specified)
    getColor(Obstacle::defBordColor, ndObstacles, "bord");
    Obstacle::defBorder = getBorder(ndObstacles, Obstacle::defBorder);
    
    do{
        scnPrimitive = iniPrimitive(shape, node);               // Initialize the obstacle (create instance of ScenePrimitive)
        if(scnPrimitive){
            color = Obstacle::defColor;
            getColor(color, node);                              // Get its color
            getPosition(pos, node);                             // And position
            scnPrimitive->setColor(color);                      // Apply those settings
            scnPrimitive->setPosition(pos);
            scnPrimitive->backfaceCulled = false;
            obstacles->push_back(scnPrimitive);                 // Store pointer in the queue obstacles
            scene->addCollisionChild(obstacles->back(), shape); // And add the obstacle to the scene
            bordCol = Obstacle::defBordColor;
            getColor(bordCol, node, "bord");
            border = getBorder(node, Obstacle::defBorder);      // Read border setting or use obstBorder by default
            obsBorder = iniShapeBorder(scene, border, shape, pos, bordCol, node);
            if(obsBorder) scene->addChild(obsBorder);           // Draw the obstacle's border
        }else{
            cout << "Incorrect format: the attribute 'type' of the tag 'obstacle' must exist and be one of these: BOX, PLANE, SPHERE, CYLINDER, UNCAPPED_CYLINDER, CONE, TORUS. Obstacle skipped.\n";
        }
        node = node->next_sibling();                            // Get the next obstacle
    } while(node);                                              // Until there are no more obstacles
}*/

void DrawScene::drawObstacles(CollisionScene *scene, xml_node<> *ndObstacles){
    if(!ndObstacles) ERROR("Incorrect format: One child of 'geometry' must be 'obstacles'");
    ScenePrimitive *scnPrimitive = NULL;
    Obstacle *obstacle = NULL;
    Vector3 pos, dir, lookAt;
    Number border;
    Color color, bordCol;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    xml_node<> *node = ndObstacles->first_node("obstacle", 0, false);   // Get the first obstacle
    if(!node) ERROR("Incorrect format: The tag 'obstacles' must have at least 1 'obstacle' child");
    
    getColor(Obstacle::defColor, ndObstacles);                          // Configure parameters by default (values in case not specified)
    getColor(Obstacle::defBordColor, ndObstacles, "bord");
    Obstacle::defBorder = getBorder(ndObstacles, Obstacle::defBorder);
    
    do{
        scnPrimitive = iniPrimitive(shape, node);                       // Initialize the obstacle (create instance of ScenePrimitive)
        if(scnPrimitive){
            color = Obstacle::defColor;                                 // Default color
            pos = Vector3(0, 0, 0);                                     // Default position
            dir = Vector3(1, 0, 0);                                     // Default movement direction
            getColor(color, node);                                      // Get its color
            getPosition(pos, node);                                     // Position
            getMovementDir(dir, node);                                  // And direction of movement
            lookAt = Vector3(0, 0, 1).crossProduct(dir);
            if(lookAt.length() > 0) scnPrimitive = iniPrimitive(shape, node, true);
            scnPrimitive->setColor(color);                              // Apply those settings
            scnPrimitive->setPosition(pos);
            scnPrimitive->backfaceCulled = false;
            obstacle = new Obstacle(scnPrimitive);                      // Create a new obstacle and configure its parameters
            obstacle->movementDir = dir;                                // Configure obstacle's parameters
            obstacle->amplitude = getAmplitude(node);
            obstacle->velocity = getVelocity(node);
            obstacle->offset = getOffset(node);
            bordCol = Obstacle::defBordColor;
            getColor(bordCol, node, "bord");
            border = getBorder(node, Obstacle::defBorder);              // Read border setting or use defBorder by default
            obstacle->border = iniShapeBorder(scene, border, shape, pos, bordCol, node, lookAt.length()>0);
            if(obstacle->border){
                scene->addChild(obstacle->border);
                if(lookAt.length() > 0) obstacle->border->lookAt(pos + dir, lookAt);
            }
            if(lookAt.length() > 0) obstacle->obstacle->lookAt(pos + dir, lookAt);
            obstacles->push_back(obstacle);                             // Store pointer in the queue enemies
            scene->addCollisionChild(obstacles->back()->obstacle, shape);//And add the enemy to the scene
        }else{
            cout << "Incorrect format: the attribute 'type' of the tag 'enemy' must exist and be one of these: BOX, PLANE, SPHERE, CYLINDER, UNCAPPED_CYLINDER, CONE, TORUS. Enemy skipped.\n";
        }
        node = node->next_sibling();                                // Get the next enemy
    } while(node);                                                  // Until there are no more enemies
}

void DrawScene::drawEnemies(CollisionScene *scene, xml_node<> *ndEnemies){
    /*if(!ndEnemies) ERROR("Incorrect format: One child of 'geometry' must be 'enemies'");
    ScenePrimitive *scnPrimitive = NULL;
    Enemy *enemy = NULL;
    Vector3 pos, dir, lookAt;
    Number border;
    Color color, bordCol;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    xml_node<> *node = ndEnemies->first_node("enemy", 0, false);    // Get the first enemy
    if(!node) ERROR("Incorrect format: The tag 'enemies' must have at least 1 'enemy' child");
    
    getColor(Enemy::defColor, ndEnemies);                           // Configure parameters by default (values in case not specified)
    getColor(Enemy::defBordColor, ndEnemies, "bord");
    Enemy::defBorder = getBorder(ndEnemies, Enemy::defBorder);
    
    do{
        scnPrimitive = iniPrimitive(shape, node);                   // Initialize the enemy (create instance of ScenePrimitive)
        if(scnPrimitive){
            color = Enemy::defColor;                                // Default color
            pos = Vector3(0, 0, 0);                                 // Default position
            dir = Vector3(1, 0, 0);                                 // Default movement direction
            getColor(color, node);                                  // Get its color
            getPosition(pos, node);                                 // Position
            getMovementDir(dir, node);                              // And direction of movement
            lookAt = Vector3(0, 0, 1).crossProduct(dir);
            if(lookAt.length() > 0) scnPrimitive = iniPrimitive(shape, node, true);
            scnPrimitive->setColor(color);                          // Apply those settings
            scnPrimitive->setPosition(pos);
            scnPrimitive->backfaceCulled = false;
            enemy = new Enemy(scnPrimitive);                        // Create a new enemy and configure its parameters
            enemy->movementDir = dir;                               // Configure enemy's parameters
            enemy->amplitude = getAmplitude(node);
            enemy->velocity = getVelocity(node);
            enemy->offset = getOffset(node);
            bordCol = Enemy::defBordColor;
            getColor(bordCol, node, "bord");
            border = getBorder(node, Enemy::defBorder);             // Read border setting or use enemyBorder by default
            enemy->border = iniShapeBorder(scene, border, shape, pos, bordCol, node, lookAt.length()>0);
            if(enemy->border){
                scene->addChild(enemy->border);
                if(lookAt.length() > 0) enemy->border->lookAt(pos + dir, lookAt);
            }
            if(lookAt.length() > 0) enemy->enemy->lookAt(pos + dir, lookAt);
            enemies->push_back(enemy);                              // Store pointer in the queue enemies
            scene->addCollisionChild(enemies->back()->enemy, shape);// And add the enemy to the scene
        }else{
            cout << "Incorrect format: the attribute 'type' of the tag 'enemy' must exist and be one of these: BOX, PLANE, SPHERE, CYLINDER, UNCAPPED_CYLINDER, CONE, TORUS. Enemy skipped.\n";
        }
        node = node->next_sibling();                                // Get the next enemy
    } while(node);                                                  // Until there are no more enemies*/
}

void DrawScene::drawCoins(CollisionScene *scene, xml_node<> *ndCoins){
    if(!ndCoins) ERROR("Incorrect format: One child of 'geometry' must be 'coins'");
    Vector3 pos;
    xml_node<> *node = ndCoins->first_node("coin", 0, false);   // Get the first coin
    if(!node) ERROR("Incorrect format: The tag 'coins' must have at least 1 'coin' child");
    
    getColor(Coin::colorSm, ndCoins, "sm"); // Configure parameters by default (values in case not specified)
    getColor(Coin::colorLg, ndCoins, "lg");
    Coin::valueSm = getNumber("ptsSm", ndCoins, Coin::valueSm);
    Coin::valueLg = getNumber("ptsLg", ndCoins, Coin::valueLg);
    Coin::rotationVel = getNumber("rotV", ndCoins, Coin::rotationVel);
    
    do{
        pos = Vector3(0, 0, 0);             // Default position
        getPosition(pos, node);             // Get position
        coins->push_back(new Coin(scene, pos, !hasLargeValue(node), getOffset(node)));
        node = node->next_sibling();        // Get the next coin
    } while(node);                          // Until there are no more coins
}

void DrawScene::drawPlayer(CollisionScene *scene, xml_node<> *ndPlayer){
    Vector3 pos = DEF_PLAY_POS;
    int shape = DEF_PLAY_SHAPE;
    
    player = NULL;
    if(ndPlayer){
        player = iniPrimitive(shape, ndPlayer); // Initialize the player (create instance of ScenePrimitive)
        getColor(Player::defColor, ndPlayer);   // Get its color
        getColor(Player::deadColor, ndPlayer, "dead");
        getPosition(pos, ndPlayer);             // And position
    }
    if(!player) player = new ScenePrimitive(DEF_PLAY_TYPE, DEF_PLAY_SIZE);
    
    player->setColor(Player::defColor);         // Apply those settings
    player->setPosition(pos);
    scene->addCollisionChild(player, shape);    // Add the player to the scene
}

void DrawScene::setupScene(CollisionScene *scene, xml_node<> *ndScene){
    if(!ndScene) return;                                                        // This node is optional. Exit if not present
    xml_node<> *node;
    
    node = ndScene->first_node("sound", 0, false);                              // Get the tag 'sound'
    if(node){
        backgndMusicEn = isAttributeTrue("enbl", node, backgndMusicEn);         // Enable background music if attribute 'enbl' is 1
    }
    
    node = ndScene->first_node("camera", 0, false);                             // Get the tag 'camera'
    if(node){
        iniCamRad = getNumber("rad", node, iniCamRad);                          // Configure camera's radius (spherical coords)
        iniCamRot = getNumber("rot", node, iniCamRot*180/PI, false)*PI/180;     // Configure camera's initial rotation
        iniCamElev = getNumber("elev", node, iniCamElev*180/PI)*PI/180;         // Configure camera's initial elevation
        if(iniCamRad < CAM_MIN_RADIUS) iniCamRad = CAM_MIN_RADIUS;              // Establish a minimum radius for the camera
        if(iniCamElev < 0) iniCamElev = 0;
        if(iniCamElev > CAM_MAX_ELEV) iniCamElev = CAM_MAX_ELEV;
    }
    
    node = ndScene->first_node("fog", 0, false);                                // Get the tag 'fog'
    if(node && getNumber("enbl", node)==1){
        scene->enableFog(true);                                                 // A bit of (blue) foggy effect to simulate water
        scene->setFogProperties(Renderer::FOG_EXP2, DEF_FOG_COLOR, getNumber("dens", node, DEF_FOG_DENS), -1000, 3000);
    }else{
        scene->enableFog(false);                                                // Foggy effect disabled
    }
}

void DrawScene::drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<Wall*>& wlls, deque<Obstacle*>& obstcls, deque<Enemy*>& enms, deque<Coin*>& cns, const char *strFile){
    if(!fileExists(strFile)) ERROR("Error: File not found (" << strFile << ")");
    file<> file(strFile);                                               // Open the xml file after making sure it exists
    xml_document<> doc;
    xml_node<> *geometry;
    
    walls = &wlls;
    obstacles = &obstcls;
    enemies = &enms;
    coins = &cns;
    walls->clear();                                                     // Clear the walls stored
    obstacles->clear();                                                 // Clear the obstacles stored
    enemies->clear();                                                   // Clear the enemies stored
    coins->clear();                                                     // Clear the coins stored
    doc.parse<0>(file.data());                                          // Read the xml file with the info of the geometry
    geometry = doc.first_node("geometry", 0, false);                    // Make sure the top element is 'geometry'
    if(!geometry) ERROR("Incorrect format: Top element must be 'geometry', not '" << doc.first_node()->name() << "'");

    setupScene(scene, geometry->first_node("scene", 0, false));         // Set up scene, camera, etc.
    drawWalls(scene, geometry->first_node("walls", 0, false));          // Walls
    drawObstacles(scene, geometry->first_node("obstacles", 0, false));  // Obstacles
    drawEnemies(scene, geometry->first_node("enemies", 0, false));      // Enemies
    drawCoins(scene, geometry->first_node("coins", 0, false));          // Coins
    drawPlayer(scene, geometry->first_node("player", 0, false));        // Player
    
    plyr = player;
}
