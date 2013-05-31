#include "DrawScene.h"

#define ERROR(s)        {cout << "Error: " << s << ". Exiting application.\n"; exit(1);}
#define DEF_WALL_COLOR  Color(0.0, 0.8, 1.0, 1.0)
#define DEF_WALL_BORDER 4
#define DEF_WALL_WIDTH  0.1
#define DEF_WALL_HEIGHT 10
#define DEF_BB_WIDTH    100
#define DEF_BB_DEPTH    3
#define DEF_WL_BORD_COL Color(0.0, 0.5, 0.75, 1.0)
#define DEF_BORD_COLOR  Color(1.0, 0.8, 0.4, 1.0)
#define DEF_OBST_COLOR  Color(1.0, 0.0, 0.0, 1.0)
#define DEF_PLAY_TYPE   ScenePrimitive::TYPE_SPHERE
#define DEF_PLAY_SHAPE  CollisionSceneEntity::SHAPE_SPHERE
#define DEF_PLAY_SIZE   1.5, 25, 25
#define DEF_PLAY_POS    Vector3(0, 1.5, 0)
#define DEF_PLAY_COLOR  Color(1.0, 1.0, 0.0, 1.0)
#define N_SEGMENTS      30   

deque<ScenePrimitive*>* DrawScene::walls;
deque<ScenePrimitive*>* DrawScene::obstacles;
ScenePrimitive* DrawScene::player = NULL;
Number DrawScene::wallHeight = DEF_WALL_HEIGHT;

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


Number DrawScene::getWidth(xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("w", 0, false);
    if(!attr) return 0.0;
    
    return abs(atof(attr->value()));
}

Number DrawScene::getHeight(xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("h", 0, false);
    if(!attr) return 0.0;
    
    return abs(atof(attr->value()));
}

Number DrawScene::getDepth(xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("d", 0, false);
    if(!attr) return 0.0;
    
    return abs(atof(attr->value()));
}

Number DrawScene::getRadius(xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("rad", 0, false);
    if(!attr) return 0.0;
    
    return abs(atof(attr->value()));
}

Number DrawScene::getRadius2(xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("rad2", 0, false);
    if(!attr) return 0.0;
    
    return abs(atof(attr->value()));
}

Number DrawScene::getBorder(xml_node<> *node){
    xml_attribute<> *attr;
    
    attr = node->first_attribute("bord", 0, false);
    if(!attr) return 0.0;
    
    return abs(atof(attr->value()));
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

void DrawScene::drawObstBorder(CollisionScene *scene, int shape, Vector3 pos, xml_node<> *node){
    ScenePrimitive *primBorder = NULL;
    Number border = getBorder(node);

    if(border == 0.0) return;   // If border wasn't specified, exit
    
    switch(shape){              // Depending on the shape of the obstacle, we require different shapes for the borders
        case CollisionSceneEntity::SHAPE_BOX:
        case CollisionSceneEntity::SHAPE_PLANE:
            primBorder = iniPrimitive(shape, node);
            primBorder->getMesh()->setMeshType(Mesh::LINE_LOOP_MESH);
            primBorder->setPosition(pos);
            break;
        case CollisionSceneEntity::SHAPE_CYLINDER:
            primBorder = new ScenePrimitive(ScenePrimitive::TYPE_CYLINDER, 0.05, getRadius(node), 360);
            primBorder->getMesh()->setMeshType(Mesh::POINT_MESH);
            primBorder->setPosition(pos + Vector3(0, getHeight(node)/2, 0));    // Upper circle
            primBorder->lineWidth = border;
            primBorder->setColor(DEF_BORD_COLOR);
            scene->addChild(primBorder);
        case CollisionSceneEntity::SHAPE_CONE:
            primBorder = new ScenePrimitive(ScenePrimitive::TYPE_CYLINDER, 0.05, getRadius(node), 360);
            primBorder->getMesh()->setMeshType(Mesh::POINT_MESH);
            primBorder->setPosition(pos - Vector3(0, getHeight(node)/2, 0));    // Lower circle
            break;
        case CollisionSceneEntity::SHAPE_SPHERE:
        default:
            primBorder = NULL;
            break;
    }
    
    if(primBorder){             // If the obstacle requires a border, add a wireframe shape to the scene
        primBorder->lineWidth = border;
        primBorder->renderWireframe = true;
        primBorder->setColor(DEF_BORD_COLOR);
        scene->addChild(primBorder);
    }
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
    ScenePrimitive *scnPrimitive = NULL;
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
            drawObstBorder(scene, shape, pos, node);            // Draw the obstacle's border
        }else{
            cout << "Incorrect format: the attribute 'type' of the tag 'obstacle' must exist and be one of these: BOX, PLANE, SPHERE, CYLINDER, UNCAPPED_CYLINDER, CONE, TORUS. Obstacle skipped.\n";
        }
        node = node->next_sibling();                            // Get the next obstacle
    } while(node);                                              // Until there are no more obstacles
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

void DrawScene::drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<ScenePrimitive*> *wlls, deque<ScenePrimitive*> *obstcls, const char *strFile){
    if(!fileExists(strFile)) ERROR("Error: File not found (" << strFile << ")");    // Make sure the file exists
    file<> file(strFile);                                       // Open the xml file
    xml_document<> doc;
    xml_node<> *geometry, *n_walls, *n_obstacles, *node;
    Color wallCol = DEF_WALL_COLOR;
    
    walls = wlls;
    obstacles = obstcls;
    walls->clear();                                             // Clear the walls stored
    obstacles->clear();                                         // Clear the obstacles stored
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
    
    /* PLAYER */
    node = geometry->first_node("player", 0, false);            // Find the tag 'player'
    drawPlayer(scene, node);                                    // Read and draw the player
    
    plyr = player;
}