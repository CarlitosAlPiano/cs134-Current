#include "DrawScene.h"

#define ERROR(s)        {cout << "Error: " << s << ". Exiting application.\n"; exit(1);}
#define DEF_WALL_COLOR  Color(0.0, 0.8, 1.0, 1.0)
#define DEF_WL_BORD_COL Color(0.0, 0.5, 0.75, 1.0)
#define DEF_WALL_BORDER 4
#define DEF_WALL_WIDTH  0.1
#define DEF_WALL_HEIGHT 10
#define DEF_OBST_COLOR  Color(1.0, 0.0, 0.0, 1.0)
#define DEF_OBS_BRD_COL Color(1.0, 0.8, 0.4, 1.0)
#define DEF_ENMY_COLOR  Color(0.9, 0.4, 0.8, 1.0)
#define DEF_ENM_BRD_COL Color(1.0, 0.2, 1.0, 1.0)
#define DEF_COIN_SM_COL Color(1.0, 0.8, 0.1, 1.0)
#define DEF_COIN_LG_COL Color(1.0, 0.5, 0.2, 1.0)
#define COIN_SM_PARAMS  ScenePrimitive::TYPE_CYLINDER, 0.1, 1, 30
#define COIN_LG_PARAMS  ScenePrimitive::TYPE_CYLINDER, 0.15, 1.1, 30
#define DEF_PLAY_TYPE   ScenePrimitive::TYPE_SPHERE
#define DEF_PLAY_SHAPE  CollisionSceneEntity::SHAPE_SPHERE
#define DEF_PLAY_SIZE   1.5, 25, 25
#define DEF_PLAY_POS    Vector3(0, 1.5, 0)
#define DEF_PLAY_COLOR  Color(1.0, 1.0, 0.0, 1.0)
#define N_SEGMENTS      30   

Number Coin::valueSm = 1, Coin::valueLg = 3, Coin::rotationVel = 50;
Sound *Coin::sndCatch = NULL;
deque<ScenePrimitive*>* DrawScene::walls;
deque<ScenePrimitive*>* DrawScene::obstacles;
deque<Enemy*>* DrawScene::enemies;
deque<Coin*>* DrawScene::coins;
ScenePrimitive* DrawScene::player = NULL;
Number DrawScene::wallHeight = DEF_WALL_HEIGHT;

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
        coin->setColor(DEF_COIN_SM_COL);
    }else{
        coin = new ScenePrimitive(COIN_LG_PARAMS);
        coin->setColor(DEF_COIN_LG_COL);
    }
    coin->setPosition(pos);
    coin->setPitch(-90);
    scene->addCollisionChild(coin);
}

void Coin::catchCoin(){
    coin->visible = false;
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

Number DrawScene::getNumber(const char* attrName, xml_node<> *node, Number defValue){
    xml_attribute<> *attr;
    
    attr = node->first_attribute(attrName, 0, false);
    if(!attr) return defValue;
    
    return abs(atof(attr->value()));
}

Number DrawScene::getWidth(xml_node<> *node){
    return getNumber("w", node);
}

Number DrawScene::getHeight(xml_node<> *node){
    return getNumber("h", node);
}

Number DrawScene::getDepth(xml_node<> *node){
    return getNumber("d", node);
}

Number DrawScene::getRadius(xml_node<> *node){
    return getNumber("rad", node);
}

Number DrawScene::getRadius2(xml_node<> *node){
    return getNumber("rad2", node);
}

Number DrawScene::getBorder(xml_node<> *node){
    return getNumber("bord", node);
}

Number DrawScene::getAmplitude(xml_node<> *node){
    return getNumber("amp", node, 10.0);
}

Number DrawScene::getVelocity(xml_node<> *node){
    return getNumber("vel", node, 1.0);
}

Number DrawScene::getOffset(xml_node<> *node){
    return getNumber("offs", node);
}

bool DrawScene::hasLargeValue(xml_node<> *node){
    return node->first_attribute("lrg", 0, false);
}

bool DrawScene::isTransparent(xml_node<> *node){
    return node->first_attribute("t", 0, false);
}

ScenePrimitive* DrawScene::iniPrimitive(int& shape, xml_node<> *node){
    xml_attribute<> *attr;
    string type;
    Number a1, a2, a3;
    
    attr = node->first_attribute("type", 0, false); // Read the primitive's type from the node attributes
    if(!attr) return NULL;                          // Exit if type is not present
    type = string(attr->value());                   // Store the uppercase version of the type
    transform(type.begin(), type.end(), type.begin(), ::toupper);
    
    if(type.compare("BOX") == 0){                   // Initialize the primitive depending on the type read
        shape = CollisionSceneEntity::SHAPE_BOX;
        a1 = getWidth(node);
        a2 = getHeight(node);
        a3 = getDepth(node);
        
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
        }else{
            return new ScenePrimitive(ScenePrimitive::TYPE_PLANE, a1, a3);
        }
    }else if(type.compare("SPHERE") == 0){
        shape = CollisionSceneEntity::SHAPE_SPHERE;
        a1 = getRadius(node);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_SPHERE, a1, N_SEGMENTS, N_SEGMENTS);
    }else if(type.compare("CYLINDER") == 0){
        shape = CollisionSceneEntity::SHAPE_CYLINDER;
        a1 = getHeight(node);
        a2 = getRadius(node);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_CYLINDER, a1, a2, N_SEGMENTS);
    }else if(type.compare("UNCAPPED_CYLINDER") == 0){
        shape = CollisionSceneEntity::SHAPE_CYLINDER;
        a1 = getHeight(node);
        a2 = getRadius(node);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_UNCAPPED_CYLINDER, a1, a2, N_SEGMENTS);
    }else if(type.compare("CONE") == 0){
        shape = CollisionSceneEntity::SHAPE_CONE;
        a1 = getHeight(node);
        a2 = getRadius(node);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_CONE, a1, a2, N_SEGMENTS);
    }else if(type.compare("TORUS") == 0){
        shape = CollisionSceneEntity::SHAPE_SPHERE;
        a1 = getRadius(node);
        a2 = getRadius2(node);
        
        return new ScenePrimitive(ScenePrimitive::TYPE_TORUS, a1, a2, N_SEGMENTS, N_SEGMENTS);
    }
    
    return NULL;
}

ScenePrimitive* DrawScene::iniShapeBorder(CollisionScene *scene, int shape, Vector3 pos, Color col, xml_node<> *node){
    ScenePrimitive *primBorder = NULL;
    Number border = getBorder(node);
    
    if(border == 0.0) return NULL;                  // If border wasn't specified, exit
    
    switch(shape){                                  // Depending on the shape of the obstacle, we require different shapes for the borders
        case CollisionSceneEntity::SHAPE_BOX:
        case CollisionSceneEntity::SHAPE_PLANE:
            primBorder = iniPrimitive(shape, node);
            primBorder->getMesh()->setMeshType(Mesh::LINE_LOOP_MESH);
            break;
        case CollisionSceneEntity::SHAPE_CYLINDER:
            primBorder = new ScenePrimitive(ScenePrimitive::TYPE_CYLINDER, getHeight(node), getRadius(node), 360);
            primBorder->getMesh()->setMeshType(Mesh::POINT_MESH);
            break;
        case CollisionSceneEntity::SHAPE_CONE:
            primBorder = new ScenePrimitive(ScenePrimitive::TYPE_CONE, getHeight(node), getRadius(node), 360);
            primBorder->getMesh()->setMeshType(Mesh::POINT_MESH);
            break;
        case CollisionSceneEntity::SHAPE_SPHERE:
        default:
            primBorder = NULL;
            break;
    }
    
    if(primBorder){                                 // If the entity requires a border, add a wireframe shape to the scene
        primBorder->lineWidth = border;
        primBorder->renderWireframe = true;
        primBorder->setColor(col);
        primBorder->setPosition(pos);
    }
    
    return primBorder;
}

void DrawScene::drawWallBorder(CollisionScene *scene, Vector3 lastVertex, Vector3 newVertex){
    ScenePrimitive *wallBorder; // Plane with the same size and orientation as the wall specified by the given vertices

    wallBorder = new ScenePrimitive(ScenePrimitive::TYPE_PLANE, DEF_WALL_HEIGHT, newVertex.distance(lastVertex));
    wallBorder->setPosition((lastVertex+newVertex)/2);
    wallBorder->setColor(DEF_WL_BORD_COL);
    wallBorder->lookAt(newVertex, Vector3(0, 1, 0).crossProduct(newVertex-lastVertex));
    wallBorder->lineWidth = DEF_WALL_BORDER;
    wallBorder->renderWireframe = true;
    scene->addChild(wallBorder);
}

void DrawScene::drawWalls(CollisionScene *scene, const Color& wallCol, xml_node<> *node){
    ScenePrimitive *scnPrimitive;
    Vector3 lastVertex(0, DEF_WALL_HEIGHT/2, 0), newVertex(0, DEF_WALL_HEIGHT/2, 0);
    Color color;
    /*Mesh *mesh = new Mesh(Mesh::POINT_MESH);
    Polygon *poly = new Polygon();*/
    
    // Make sure there are at least 3 vertices (2 walls)
    if(!node || !node->next_sibling() || !node->next_sibling()->next_sibling()) ERROR("Incorrect format: The tag 'walls' must have at least 3 'vertex' children");
    getPosition(lastVertex, node);              // Read the first vertex
    
    while((node = node->next_sibling()) != 0){  // While there are vertices to read
        getPosition(newVertex, node);           // Read the current vertex
        color = wallCol;
        getColor(color, node);                  // Get the color of the wall (if given)
        if(!isTransparent(node)){               // Draw wall if is not transparent
            scnPrimitive = new ScenePrimitive(ScenePrimitive::TYPE_BOX, DEF_WALL_HEIGHT, DEF_WALL_WIDTH, newVertex.distance(lastVertex));
            scnPrimitive->setPosition((lastVertex+newVertex)/2);
            scnPrimitive->setColor(color);
            scnPrimitive->lookAt(newVertex, Vector3(0, 1, 0).crossProduct(newVertex-lastVertex));
            walls->push_back(scnPrimitive);     // Store pointer in the queue walls
            scene->addCollisionChild(walls->back(), CollisionSceneEntity::SHAPE_BOX);
            drawWallBorder(scene, lastVertex, newVertex);
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

void DrawScene::drawObstacles(CollisionScene *scene, xml_node<> *node){
    ScenePrimitive *scnPrimitive = NULL, *obsBorder = NULL;
    Color obstacleCol;
    Vector3 pos;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    
    if(!node) ERROR("Incorrect format: The tag 'obstacles' must have at least 1 'obstacle' child");
    
    do{
        scnPrimitive = iniPrimitive(shape, node);               // Initialize the obstacle (create instance of ScenePrimitive)
        if(scnPrimitive){
            obstacleCol = DEF_OBST_COLOR;
            getColor(obstacleCol, node);                        // Get its color
            getPosition(pos, node);                             // And position
            scnPrimitive->setColor(obstacleCol);                // Apply those settings
            scnPrimitive->setPosition(pos);
            scnPrimitive->backfaceCulled = false;
            obstacles->push_back(scnPrimitive);                 // Store pointer in the queue obstacles
            scene->addCollisionChild(obstacles->back(), shape); // And add the obstacle to the scene
            obsBorder = iniShapeBorder(scene, shape, pos, DEF_OBS_BRD_COL, node);
            if(obsBorder) scene->addChild(obsBorder);           //Draw the obstacle's border
        }else{
            cout << "Incorrect format: the attribute 'type' of the tag 'obstacle' must exist and be one of these: BOX, PLANE, SPHERE, CYLINDER, UNCAPPED_CYLINDER, CONE, TORUS. Obstacle skipped.\n";
        }
        node = node->next_sibling();                            // Get the next obstacle
    } while(node);                                              // Until there are no more obstacles
}

void DrawScene::drawEnemies(CollisionScene *scene, xml_node<> *node){
    ScenePrimitive *scnPrimitive = NULL;
    Enemy *enemy = NULL;
    Color enemyCol;
    Vector3 pos, dir, lookAt;
    int shape = CollisionSceneEntity::SHAPE_BOX;
    
    if(!node) ERROR("Incorrect format: The tag 'enemies' must have at least 1 'enemy' child");
    
    do{
        scnPrimitive = iniPrimitive(shape, node);                   // Initialize the obstacle (create instance of ScenePrimitive)
        if(scnPrimitive){
            enemyCol = DEF_ENMY_COLOR;                              // Default color
            pos = Vector3(0, 0, 0);                                 // Default position
            getColor(enemyCol, node);                               // Get its color
            getPosition(pos, node);                                 // And position
            scnPrimitive->setColor(enemyCol);                       // Apply those settings
            scnPrimitive->setPosition(pos);
            enemy = new Enemy(scnPrimitive);                        // Create a new enemy and configure its parameters
            dir = Vector3(1, 0, 0);                                 // Default movement direction
            getMovementDir(dir, node);                              // Get direction of enemy's movement
            enemy->movementDir = dir;                               // Configure enemy's parameters
            enemy->amplitude = getAmplitude(node);
            enemy->velocity = getVelocity(node);
            enemy->offset = getOffset(node);
            enemy->border = iniShapeBorder(scene, shape, pos, DEF_ENM_BRD_COL, node);
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

void DrawScene::drawCoins(CollisionScene *scene, xml_node<> *node){
    Vector3 pos;
    
    if(!node) ERROR("Incorrect format: The tag 'coins' must have at least 1 'coin' child");
    
    do{
        pos = Vector3(0, 0, 0);         // Default position
        getPosition(pos, node);         // Get position
        coins->push_back(new Coin(scene, pos, !hasLargeValue(node), getOffset(node)));
        node = node->next_sibling();    // Get the next coin
    } while(node);                      // Until there are no more coins
}

void DrawScene::drawPlayer(CollisionScene *scene, xml_node<> *node){
    Color playerCol = DEF_PLAY_COLOR;
    Vector3 pos = DEF_PLAY_POS;
    int shape = DEF_PLAY_SHAPE;
    
    if(node){
        player = iniPrimitive(shape, node);     // Initialize the player (create instance of ScenePrimitive)
        getColor(playerCol, node);              // Get its color
        getPosition(pos, node);                 // And position
    }
    if(!player) player = new ScenePrimitive(DEF_PLAY_TYPE, DEF_PLAY_SIZE);
    
    player->setColor(playerCol);                // Apply those settings
    player->setPosition(pos);
    scene->addCollisionChild(player, shape);    // Add the player to the scene
}

void DrawScene::drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<ScenePrimitive*>& wlls, deque<ScenePrimitive*>& obstcls, deque<Enemy*>& enms, deque<Coin*>& cns, const char *strFile){
    if(!fileExists(strFile)) ERROR("Error: File not found (" << strFile << ")");    // Make sure the file exists
    file<> file(strFile);                                       // Open the xml file
    xml_document<> doc;
    xml_node<> *geometry, *n_walls, *n_obstacles, *n_enemies, *n_coins, *node;
    Color wallCol = DEF_WALL_COLOR;
    
    walls = &wlls;
    obstacles = &obstcls;
    enemies = &enms;
    coins = &cns;
    walls->clear();                                             // Clear the walls stored
    obstacles->clear();                                         // Clear the obstacles stored
    enemies->clear();                                           // Clear the enemies stored
    coins->clear();                                             // Clear the coins stored
    doc.parse<0>(file.data());                                  // Read the xml file with the info of the geometry
    geometry = doc.first_node("geometry", 0, false);            // Make sure the top element is 'geometry'
    if(!geometry) ERROR("Incorrect format: Top element must be 'geometry', not '" << doc.first_node()->name() << "'");
    
    /* WALLS */
    n_walls = geometry->first_node("walls", 0, false);          // Find the tag 'walls'
    if(!n_walls) ERROR("Incorrect format: One child of 'geometry' must be 'walls'");
    getColor(wallCol, n_walls);                                 // Get the color of the walls
    node = n_walls->first_node("vertex", 0, false);             // Read the first vertex
    drawWalls(scene, wallCol, node);                            // Read and draw the walls
    
    /* OBSTACLES */
    n_obstacles = geometry->first_node("obstacles", 0, false);  // Find the tag 'obstacles'
    if(!n_obstacles) ERROR("Incorrect format: One child of 'geometry' must be 'obstacles'");
    node = n_obstacles->first_node("obstacle", 0, false);       // Read the first obstacle
    drawObstacles(scene, node);                                 // Read and draw the obstacles
    
    /* ENEMIES */
    n_enemies = geometry->first_node("enemies", 0, false);      // Find the tag 'enemies'
    if(!n_enemies) ERROR("Incorrect format: One child of 'geometry' must be 'enemies'");
    node = n_enemies->first_node("enemy", 0, false);            // Read the first enemy
    drawEnemies(scene, node);                                   // Read and draw the enemies
    
    /* COINS */
    n_coins = geometry->first_node("coins", 0, false);          // Find the tag 'coins'
    if(!n_coins) ERROR("Incorrect format: One child of 'geometry' must be 'coins'");
    node = n_coins->first_node("coin", 0, false);               // Read the first coin
    drawCoins(scene, node);                                     // Read and draw the coins
    
    /* PLAYER */
    node = geometry->first_node("player", 0, false);            // Find the tag 'player'
    drawPlayer(scene, node);                                    // Read and draw the player
    
    plyr = player;
}
