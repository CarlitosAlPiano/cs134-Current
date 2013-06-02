#ifndef DRAW_SCENE_INCLUDED
#define DRAW_SCENE_INCLUDED

#include <Polycode.h>
#include "PolyCollisionScene.h"
#include "PolyCollisionSceneEntity.h"
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <deque>

using namespace Polycode;
using namespace rapidxml;
using namespace std;

class Enemy {
public:
    ScenePrimitive *enemy, *border;
    Number amplitude, velocity, offset;
    Vector3 movementDir, middlePos;
    
    Enemy(ScenePrimitive *enemy, Number amplitude = 3, Number velocity = 1, Number offset = PI/2, Vector3 movementDir = Vector3(1, 0, 0));
    Enemy(ScenePrimitive *enemy, Vector3 middlePos, Number amplitude = 3, Number velocity = 1, Number offset = PI/2, Vector3 movementDir = Vector3(1, 0, 0));
    
    void update(Number totalElapsed);
};

class Coin {
public:
    ScenePrimitive *coin;
    bool hasSmallValue;
    Number offset;
    static Number valueSm, valueLg, rotationVel;
    static Sound *sndCatch;
    
    Coin(CollisionScene *scene, Vector3 pos, bool hasSmallValue = true, Number offset = 0);

    void catchCoin();
    void update(Number totalElapsed);
};

class DrawScene {
private:
    static deque<ScenePrimitive*> *walls;
    static deque<ScenePrimitive*> *obstacles;
    static deque<Enemy*> *enemies;
    static deque<Coin*> *coins;
    static ScenePrimitive *player;

    static bool fileExists(const char *strFile);
    static void getColor(Color& color, xml_node<> *node);
    static void getPosition(Vector3& pos, xml_node<> *node);
    static void getMovementDir(Vector3& dir, xml_node<> *node);
    static Number getNumber(const char* attrName, xml_node<> *node, Number defValue = 0.0);
    static Number getWidth(xml_node<> *node);
    static Number getHeight(xml_node<> *node);
    static Number getDepth(xml_node<> *node);
    static Number getRadius(xml_node<> *node);
    static Number getRadius2(xml_node<> *node);
    static Number getBorder(xml_node<> *node);
    static Number getAmplitude(xml_node<> *node);
    static Number getVelocity(xml_node<> *node);
    static Number getOffset(xml_node<> *node);
    static bool hasLargeValue(xml_node<> *node);
    static bool isTransparent(xml_node<> *node);
    static ScenePrimitive* iniPrimitive(int& shape, xml_node<> *node);
    static ScenePrimitive* iniShapeBorder(CollisionScene *scene, int shape, Vector3 pos, Color col, xml_node<> *node);
    static void drawWallBorder(CollisionScene *scene, Vector3 lastVertex, Vector3 newVertex);
    static void drawWalls(CollisionScene *scene, const Color& wallCol, xml_node<> *node);
    static void drawObstacles(CollisionScene *scene, xml_node<> *node);
    static void drawEnemies(CollisionScene *scene, xml_node<> *node);
    static void drawCoins(CollisionScene *scene, xml_node<> *node);
    static void drawPlayer(CollisionScene *scene, xml_node<> *node);
public:
    static Number wallHeight;
    static void drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<ScenePrimitive*>& wlls, deque<ScenePrimitive*>& obstcls, deque<Enemy*>& enemies, deque<Coin*>& coins, const char *strFile);
};

#endif