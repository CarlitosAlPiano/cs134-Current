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

class Wall {
public:
    ScenePrimitive *wall;
    Vector2 start, end;
    static Number defBorder, width, height;
    static Color defColor, defBordColor;
    
    Wall(CollisionScene *scene, Vector3 lastVertex, Vector3 newVertex, Number border, Color color, Color bordColor);
    
    void drawWallBorder(CollisionScene *scene, Number border, Color borderCol);
    Vector2 getSegment();
    bool intersects(const Vector2& l, Vector2& pt);
};

class Obstacle {
public:
    static Number defBorder;
    static Color defColor, defBordColor;
};

class Enemy {
public:
    ScenePrimitive *enemy, *border;
    Number amplitude, velocity, offset;
    Vector3 movementDir, middlePos;
    static Number defBorder;
    static Color defColor, defBordColor;
    
    Enemy(ScenePrimitive *enemy, Number amplitude = 3, Number velocity = 1, Number offset = PI/2, Vector3 movementDir = Vector3(1, 0, 0));
    Enemy(ScenePrimitive *enemy, Vector3 middlePos, Number amplitude = 3, Number velocity = 1, Number offset = PI/2, Vector3 movementDir = Vector3(1, 0, 0));
    
    void update(Number totalElapsed);
};

class Coin {
public:
    ScenePrimitive *coin;
    bool hasSmallValue;
    Number offset;
    static unsigned int valueSm, valueLg;
    static Number rotationVel;
    static Sound *sndCatch;
    static Color colorSm, colorLg;
    
    Coin(CollisionScene *scene, Vector3 pos, bool hasSmallValue = true, Number offset = 0);

    void catchCoin(unsigned int &points);
    void update(Number totalElapsed);
};

class Player {
public:
    static Color defColor, deadColor;
};

class DrawScene {
private:
    static deque<Wall*> *walls;
    static deque<ScenePrimitive*> *obstacles;
    static deque<Enemy*> *enemies;
    static deque<Coin*> *coins;
    static ScenePrimitive *player;

    static bool fileExists(const char *strFile);
    static void getColor(Color& color, xml_node<> *node);
    static void getColor(Color& color, xml_node<> *node, string prefix);
    static void getPosition(Vector3& pos, xml_node<> *node);
    static void getMovementDir(Vector3& dir, xml_node<> *node);
    static Number getNumber(const char* attrName, xml_node<> *node, Number defValue = 0.0, bool absVal = true);
    static Number getWidth(xml_node<> *node, Number defValue = 0.0);
    static Number getHeight(xml_node<> *node, Number defValue = 0.0);
    static Number getDepth(xml_node<> *node, Number defValue = 0.0);
    static Number getRadius(xml_node<> *node, Number defValue = 0.0);
    static Number getRadius2(xml_node<> *node, Number defValue = 0.0);
    static Number getBorder(xml_node<> *node, Number defValue = 0.0);
    static Number getAmplitude(xml_node<> *node, Number defValue = 0.0);
    static Number getVelocity(xml_node<> *node, Number defValue = 0.0);
    static Number getOffset(xml_node<> *node, Number defValue = 0.0);
    static bool isAttributeTrue(const char* attrName, xml_node<> *node, bool defValue = false);
    static bool hasLargeValue(xml_node<> *node);
    static bool isTransparent(xml_node<> *node);
    static bool isGoal(xml_node<> *node);
    static ScenePrimitive* iniPrimitive(int& shape, xml_node<> *node, bool boxReversed = false, Number numSegments = 30);
    static ScenePrimitive* iniShapeBorder(CollisionScene *scene, Number border, int shape, Vector3 pos, Color col, xml_node<> *node, bool boxReversed = false);
    static void drawWalls(CollisionScene *scene, xml_node<> *ndWalls);
    static void drawObstacles(CollisionScene *scene, xml_node<> *ndObstactles);
    static void drawEnemies(CollisionScene *scene, xml_node<> *ndEnemies);
    static void drawCoins(CollisionScene *scene, xml_node<> *ndCoins);
    static void drawPlayer(CollisionScene *scene, xml_node<> *ndPlayer);
    static void setupScene(CollisionScene *scene, xml_node<> *ndScene);
public:
    static bool backgndMusicEn;
    static Number iniCamRad, iniCamRot, iniCamElev;

    static void drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<Wall*>& wlls, deque<ScenePrimitive*>& obstcls, deque<Enemy*>& enemies, deque<Coin*>& coins, const char *strFile);
};

#endif