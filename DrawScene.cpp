#include "DrawScene.h"

#define ERROR(s)        {cout << "Error: " << s << ". Exiting application.\n"; exit(EXIT_FAILURE);}
#define DEF_CAM_RADIUS  40
#define DEF_CAM_ROT     -1.7
#define DEF_CAM_ELEV    0.3
#define CAM_MIN_RADIUS  20
#define CAM_MAX_ELEV    (PI/2 - 0.01)
#define DEF_OBST_BORDER 2
#define DEF_ENMY_BORDER 2
#define DEF_WALL_BORDER 5
#define DEF_WALL_WIDTH  0.1
#define DEF_WALL_HEIGHT 10
#define DEF_WALL_COLOR  Color(0.0, 0.8, 1.0, 1.0)
#define DEF_WL_BORD_COL Color(0.0, 0.5, 0.75, 1.0)
#define DEF_OBST_COLOR  Color(0.2, 0.4, 0.8, 1.0)
#define DEF_OBS_BRD_COL Color(0.6, 0.1, 0.1, 1.0)
#define DEF_ENMY_COLOR  Color(0.2, 0.4, 0.8, 1.0)
#define DEF_ENM_BRD_COL Color(0.6, 0.1, 0.1, 1.0)
#define DEF_COIN_SM_COL Color(1.0, 0.8, 0.1, 1.0)
#define DEF_COIN_LG_COL Color(1.0, 0.5, 0.2, 1.0)
#define COIN_SM_PARAMS  ScenePrimitive::TYPE_CYLINDER, 0.1, 1, 30
#define COIN_LG_PARAMS  ScenePrimitive::TYPE_CYLINDER, 0.15, 1.1, 30
#define DEF_PLAY_TYPE   ScenePrimitive::TYPE_SPHERE
#define DEF_PLAY_SHAPE  CollisionSceneEntity::SHAPE_SPHERE
#define DEF_PLAY_SIZE   1.25, 25, 25
#define DEF_PLAY_POS    Vector3(0, 1.5, 0)
#define DEF_PLAY_COLOR  Color(1.0, 1.0, 0.0, 1.0)
#define DEF_PL_DEAD_COL Color(1.0, 0.0, 0.0, 1.0)
#define DEF_FOG_COLOR   Color(0.0, 0.0, 0.2, 1.0)
#define DEF_FOG_DENS    0.005
#define DEF_MUSIC_EN    true

unsigned int Coin::valueSm = 1, Coin::valueLg = 3;
Number Coin::rotationVel = 50;
Sound *Coin::sndCatch = NULL;
deque<ScenePrimitive*>* DrawScene::walls = NULL;
deque<ScenePrimitive*>* DrawScene::obstacles = NULL;
deque<Enemy*>* DrawScene::enemies = NULL;
deque<Coin*>* DrawScene::coins = NULL;
ScenePrimitive* DrawScene::player = NULL;

bool DrawScene::backgndMusicEn = DEF_MUSIC_EN;
Number DrawScene::iniCamRad = DEF_CAM_RADIUS, DrawScene::iniCamElev = DEF_CAM_ELEV, DrawScene::iniCamRot = DEF_CAM_ROT;
Number DrawScene::wallWidth = DEF_WALL_WIDTH, DrawScene::wallHeight = DEF_WALL_HEIGHT;
Number DrawScene::wallBorder = DEF_WALL_BORDER, DrawScene::obstBorder = DEF_OBST_BORDER, DrawScene::enemyBorder = DEF_ENMY_BORDER;
Color DrawScene::wallColor = DEF_WALL_COLOR, DrawScene::wallBordColor = DEF_WL_BORD_COL;
Color DrawScene::obstColor = DEF_OBST_COLOR, DrawScene::obstBordColor = DEF_OBS_BRD_COL;
Color DrawScene::enemyColor = DEF_ENMY_COLOR, DrawScene::enemyBordColor = DEF_ENM_BRD_COL;
Color DrawScene::coinSmColor = DEF_COIN_SM_COL, DrawScene::coinLgColor = DEF_COIN_LG_COL;
Color DrawScene::playerColor = DEF_PLAY_COLOR, DrawScene::playerDeadColor = DEF_PL_DEAD_COL;

Enemy::Enemy(ScenePrimitive *enemy, Number amplitude, Number velocity, Number offset, Vector3 movementDir) : enemy(enemy), amplitude(amplitude), velocity(velocity), offset(offset), movementDir(movementDir/movementDir.length()), middlePos(enemy->getPosition()) {}

Enemy::Enemy(ScenePrimitive *enemy, Vector3 middlePos, Number amplitude, Number velocity, Number offset, Vector3 movementDir) : enemy(enemy), middlePos(middlePos), amplitude(amplitude), velocity(velocity), offset(offset), movementDir(movementDir/movementDir.length()) {}

void Enemy::update(Number totalElapsed){
    Vector3 oldPos = enemy->getPosition();
    
    enemy->setPosition(middlePos + movementDir*amplitude*sin(velocity*totalElapsed + offset));
    if(border) border->Translate(enemy->getPosition() - oldPos);
}

Coin::Coin(CollisionScene *scene, Vector3 pos, bool hasSmallValue, Number offset) : hasSmallValue(hasSmallValue), offset(offset) {
    if(hasSmallValue){
        coin = new ScenePrimitive(COIN_SM_PARAMS);
        coin->setColor(DrawScene::coinSmColor);
    }else{
        coin = new ScenePrimitive(COIN_LG_PARAMS);
        coin->setColor(DrawScene::coinLgColor);
    }
    coin->setPosition(pos);
    coin->setPitch(-90);
    scene->addCollisionChild(coin);
}

void Coin::catchCoin(unsigned int &points){
    coin->visible = false;
    points += (hasSmallValue)? valueSm:valueLg;
    cout << "Points: " << points << "\n";
    sndCatch->Play();
}

void Coin::update(Number totalElapsed){
    coin->setYaw(rotationVel*(totalElapsed + offset));
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
    xml_attribute<> *attr;
    
    attr = node->first_attribute(attrName, 0, false);
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
    return getNumber("offs", node, defValue)*PI/180;
}

bool DrawScene::hasLargeValue(xml_node<> *node){
    return node->first_attribute("lrg", 0, false);
}

bool DrawScene::isTransparent(xml_node<> *node){
    return node->first_attribute("t", 0, false);
}

ScenePrimitive* DrawScene::iniPrimitive(int& shape, xml_node<> *node, Number numSegments){
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
        a3 = getDepth(node, 1);
        
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
        a1 = getHeight(node, DrawScene::wallHeight);
        a2 = getRadius(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_CYLINDER, a1, a2, numSegments);
    }else if(type.compare("UNCAPPED_CYLINDER") == 0){
        shape = CollisionSceneEntity::SHAPE_CYLINDER;
        a1 = getHeight(node, DrawScene::wallHeight);
        a2 = getRadius(node, 1);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_UNCAPPED_CYLINDER, a1, a2, numSegments);
    }else if(type.compare("CONE") == 0){
        shape = CollisionSceneEntity::SHAPE_CONE;
        a1 = getHeight(node, DrawScene::wallHeight);
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

ScenePrimitive* DrawScene::iniShapeBorder(CollisionScene *scene, Number border, int shape, Vector3 pos, Color col, xml_node<> *node){
    if(border == 0.0) return NULL;                  // If border wasn't specified, exit
    ScenePrimitive *primBorder = NULL;
    
    switch(shape){                                  // Depending on the shape of the obstacle, we require different shapes for the borders
        case CollisionSceneEntity::SHAPE_BOX:
        case CollisionSceneEntity::SHAPE_PLANE:
            primBorder = iniPrimitive(shape, node);
            primBorder->getMesh()->setMeshType(Mesh::LINE_LOOP_MESH);
            primBorder->lineWidth = border;
            primBorder->renderWireframe = true;
            break;
        case CollisionSceneEntity::SHAPE_CYLINDER:
        case CollisionSceneEntity::SHAPE_CONE:
            primBorder = iniPrimitive(shape, node, 300*border);
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

void DrawScene::drawWallBorder(CollisionScene *scene, Vector3 lastVertex, Vector3 newVertex, xml_node<> *node){
    ScenePrimitive *wallBorder; // Plane with the same size and orientation as the wall specified by the given vertices
    Number border = getBorder(node, DrawScene::wallBorder);
    Color borderCol = DrawScene::wallBordColor;
    getColor(borderCol, node, "bord");
    if(border <= 0) return;     // Only render border if a positive value is read

    wallBorder = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, DrawScene::wallHeight, newVertex.distance(lastVertex));
    wallBorder->setPosition((lastVertex+newVertex)/2);
    wallBorder->setColor(borderCol);
    wallBorder->lookAt(newVertex, Vector3(0, 1, 0).crossProduct(newVertex-lastVertex));
    wallBorder->lineWidth = border;
    wallBorder->renderWireframe = true;
    scene->addChild(wallBorder);
}

void DrawScene::drawWalls(CollisionScene *scene, xml_node<> *ndWalls){
    if(!ndWalls) ERROR("Incorrect format: One child of 'geometry' must be 'walls'");
    ScenePrimitive *scnPrimitive = NULL;
    Vector3 lastVertex(0, wallHeight/2, 0), newVertex(0, wallHeight/2, 0);
    Color color;
    xml_node<> *node = ndWalls->first_node("vertex", 0, false); // Get 1st vertex and make sure there are at least 3 vertices (2 walls)
    if(!node || !node->next_sibling() || !node->next_sibling()->next_sibling()) ERROR("Incorrect format: The tag 'walls' must have at least 3 'vertex' children");
    
    getColor(DrawScene::wallColor, ndWalls);    // Configure parameters by default (values in case not specified)
    getColor(DrawScene::wallBordColor, ndWalls, "bord");
    DrawScene::wallBorder = getBorder(ndWalls, DrawScene::wallBorder);
    DrawScene::wallWidth = getWidth(ndWalls, DrawScene::wallWidth);
    DrawScene::wallHeight = getHeight(ndWalls, DrawScene::wallHeight);
    
    /*Mesh *mesh = new Mesh(Mesh::POINT_MESH);
    Polygon *poly = new Polygon();*/
    
    getPosition(lastVertex, node);              // Read the first vertex
    lastVertex.y = wallHeight/2;
    while((node = node->next_sibling()) != 0){  // While there are vertices to read
        getPosition(newVertex, node);           // Read the current vertex
        newVertex.y = wallHeight/2;
        color = wallColor;
        getColor(color, node);                  // Get the color of the wall (if given)
        if(!isTransparent(node)){               // Draw wall if is not transparent
            scnPrimitive = new ScenePrimitive(ScenePrimitive::TYPE_BOX, wallHeight, wallWidth, newVertex.distance(lastVertex));
            scnPrimitive->setPosition((lastVertex+newVertex)/2);
            scnPrimitive->setColor(color);
            scnPrimitive->lookAt(newVertex, Vector3(0, 1, 0).crossProduct(newVertex-lastVertex));
            walls->push_back(scnPrimitive);     // Store pointer in the queue walls
            scene->addCollisionChild(walls->back(), CollisionSceneEntity::SHAPE_BOX);
            drawWallBorder(scene, lastVertex, newVertex, node);
        }
        //poly->addVertex(lastVertex.x, 0, lastVertex.z);
        lastVertex = newVertex;                 // In the next iteration, lastVertex will be the current vertex
    }
    /*poly->addVertex(lastVertex.x, 0, lastVertex.z);
    mesh->addPolygon(poly);
    SceneMesh *sm = new SceneMesh(mesh);
    sm->setColor(1.0,0.0,1.0,1.0);
    scene->addChild(sm);*/
}

void DrawScene::drawObstacles(CollisionScene *scene, xml_node<> *ndObstacles){
    if(!ndObstacles) ERROR("Incorrect format: One child of 'geometry' must be 'obstacles'");
    ScenePrimitive *scnPrimitive = NULL, *obsBorder = NULL;
    Color obstacleCol, obstacleBordCol;
    Vector3 pos;
    Number border;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    xml_node<> *node = ndObstacles->first_node("obstacle", 0, false);   // Get the first obstacle
    if(!node) ERROR("Incorrect format: The tag 'obstacles' must have at least 1 'obstacle' child");
    
    getColor(DrawScene::obstColor, ndObstacles);                // Configure parameters by default (values in case not specified)
    getColor(DrawScene::obstBordColor, ndObstacles, "bord");
    DrawScene::obstBorder = getBorder(ndObstacles, DrawScene::obstBorder);
    
    do{
        scnPrimitive = iniPrimitive(shape, node);               // Initialize the obstacle (create instance of ScenePrimitive)
        if(scnPrimitive){
            obstacleCol = obstColor;
            getColor(obstacleCol, node);                        // Get its color
            getPosition(pos, node);                             // And position
            scnPrimitive->setColor(obstacleCol);                // Apply those settings
            scnPrimitive->setPosition(pos);
            scnPrimitive->backfaceCulled = false;
            obstacles->push_back(scnPrimitive);                 // Store pointer in the queue obstacles
            scene->addCollisionChild(obstacles->back(), shape); // And add the obstacle to the scene
            obstacleBordCol = obstBordColor;
            getColor(obstacleBordCol, node, "bord");
            border = getBorder(node, obstBorder);               // Read border setting or use obstBorder by default
            obsBorder = iniShapeBorder(scene, border, shape, pos, obstacleBordCol, node);
            if(obsBorder) scene->addChild(obsBorder);           //Draw the obstacle's border
        }else{
            cout << "Incorrect format: the attribute 'type' of the tag 'obstacle' must exist and be one of these: BOX, PLANE, SPHERE, CYLINDER, UNCAPPED_CYLINDER, CONE, TORUS. Obstacle skipped.\n";
        }
        node = node->next_sibling();                            // Get the next obstacle
    } while(node);                                              // Until there are no more obstacles
}

void DrawScene::drawEnemies(CollisionScene *scene, xml_node<> *ndEnemies){
    if(!ndEnemies) ERROR("Incorrect format: One child of 'geometry' must be 'enemies'");
    ScenePrimitive *scnPrimitive = NULL;
    Enemy *enemy = NULL;
    Color enemyCol, enemyBordCol;
    Vector3 pos, dir, lookAt;
    Number border;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    xml_node<> *node = ndEnemies->first_node("enemy", 0, false);    // Get the first enemy
    if(!node) ERROR("Incorrect format: The tag 'enemies' must have at least 1 'enemy' child");
    
    getColor(DrawScene::enemyColor, ndEnemies);                     // Configure parameters by default (values in case not specified)
    getColor(DrawScene::enemyBordColor, ndEnemies, "bord");
    DrawScene::enemyBorder = getBorder(ndEnemies, DrawScene::enemyBorder);
    
    do{
        scnPrimitive = iniPrimitive(shape, node);                   // Initialize the obstacle (create instance of ScenePrimitive)
        if(scnPrimitive){
            enemyCol = enemyColor;                                  // Default color
            pos = Vector3(0, 0, 0);                                 // Default position
            getColor(enemyCol, node);                               // Get its color
            getPosition(pos, node);                                 // And position
            scnPrimitive->setColor(enemyCol);                       // Apply those settings
            scnPrimitive->setPosition(pos);
            enemy = new Enemy(scnPrimitive);                        // Create a new enemy and configure its parameters
            dir = Vector3(1, 0, 0);                                 // Default movement direction
            getMovementDir(dir, node);                              // Get direction of enemy's movement
            enemy->movementDir = dir;                               // Configure enemy's parameters
            enemy->amplitude = getAmplitude(node, 5);
            enemy->velocity = getVelocity(node, 1);
            enemy->offset = getOffset(node);
            enemyBordCol = enemyBordColor;
            getColor(enemyBordCol, node, "bord");
            border = getBorder(node, enemyBorder);                  // Read border setting or use enemyBorder by default
            enemy->border = iniShapeBorder(scene, border, shape, pos, enemyBordCol, node);
            lookAt = Vector3(0, 0, 1).crossProduct(dir);
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
    } while(node);                                                  // Until there are no more enemies
}

void DrawScene::drawCoins(CollisionScene *scene, xml_node<> *ndCoins){
    if(!ndCoins) ERROR("Incorrect format: One child of 'geometry' must be 'coins'");
    Vector3 pos;
    xml_node<> *node = ndCoins->first_node("coin", 0, false);   // Get the first coin
    if(!node) ERROR("Incorrect format: The tag 'coins' must have at least 1 'coin' child");
    
    getColor(DrawScene::coinSmColor, ndCoins, "sm");            // Configure parameters by default (values in case not specified)
    getColor(DrawScene::coinLgColor, ndCoins, "lg");
    Coin::valueSm = getNumber("ptsSm", ndCoins, Coin::valueSm);
    Coin::valueLg = getNumber("ptsLg", ndCoins, Coin::valueLg);
    Coin::rotationVel = getNumber("rotV", ndCoins, Coin::rotationVel);
    
    do{
        pos = Vector3(0, 0, 0);         // Default position
        getPosition(pos, node);         // Get position
        coins->push_back(new Coin(scene, pos, !hasLargeValue(node), getOffset(node)));
        node = node->next_sibling();    // Get the next coin
    } while(node);                      // Until there are no more coins
}

void DrawScene::drawPlayer(CollisionScene *scene, xml_node<> *ndPlayer){
    Vector3 pos = DEF_PLAY_POS;
    int shape = DEF_PLAY_SHAPE;
    
    if(ndPlayer){
        player = iniPrimitive(shape, ndPlayer); // Initialize the player (create instance of ScenePrimitive)
        getColor(playerColor, ndPlayer);        // Get its color
        getColor(playerDeadColor, ndPlayer, "dead");
        getPosition(pos, ndPlayer);             // And position
    }
    if(!player) player = new ScenePrimitive(DEF_PLAY_TYPE, DEF_PLAY_SIZE);
    
    player->setColor(playerColor);              // Apply those settings
    player->setPosition(pos);
    scene->addCollisionChild(player, shape);    // Add the player to the scene
}

void DrawScene::setupScene(CollisionScene *scene, xml_node<> *ndScene){
    xml_node<> *node;
    
    CoreServices::getInstance()->getRenderer()->setClippingPlanes(0.1, 5000);   // Set far plane very far: avoid weird visual effects!
    if(!ndScene) return;                                                        // This node is optional. Exit if not present
    
    node = ndScene->first_node("sound", 0, false);                              // Get the tag 'sound'
    backgndMusicEn = (getNumber("enbl", node, backgndMusicEn) == 1);            // Enable background music if attribute 'enbl' is 1
    
    node = ndScene->first_node("camera", 0, false);                             // Get the tag 'camera'
    iniCamRad = getNumber("rad", node, iniCamRad);                              // Configure camera's radius (spherical coords)
    iniCamRot = getNumber("rot", node, iniCamRot, false)*PI/180;                // Configure camera's initial rotation
    iniCamElev = getNumber("elev", node, iniCamElev)*PI/180;                    // Configure camera's initial elevation
    if(iniCamRad < CAM_MIN_RADIUS) iniCamRad = CAM_MIN_RADIUS;                  // Establish a minimum radius for the camera
    if(iniCamElev < 0) iniCamElev = 0;
    if(iniCamElev > CAM_MAX_ELEV) iniCamElev = CAM_MAX_ELEV;
    
    node = ndScene->first_node("fog", 0, false);                                // Get the tag 'fog'
    if(getNumber("enbl", node) == 1){
        scene->enableFog(true);                                                 // A bit of (blue) foggy effect to simulate water
        scene->setFogProperties(Renderer::FOG_EXP2, DEF_FOG_COLOR, getNumber("dens", node, DEF_FOG_DENS), -1000, 3000);
    }else{
        scene->enableFog(false);                                                // Foggy effect disabled
    }
}

void DrawScene::drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<ScenePrimitive*>& wlls, deque<ScenePrimitive*>& obstcls, deque<Enemy*>& enms, deque<Coin*>& cns, const char *strFile){
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
