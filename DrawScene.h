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

class DrawScene {
private:
    static deque<ScenePrimitive*> *walls;
    static deque<ScenePrimitive*> *obstacles;
    static ScenePrimitive *player;

    static bool fileExists(const char *strFile);
    static void getColor(Color& color, xml_node<> *node);
    static void getPosition(Vector3& pos, xml_node<> *node);
    static Number getWidth(xml_node<> *node);
    static Number getHeight(xml_node<> *node);
    static Number getDepth(xml_node<> *node);
    static Number getRadius(xml_node<> *node);
    static Number getRadius2(xml_node<> *node);
    static Number getBorder(xml_node<> *node);
    static bool isTransparent(xml_node<> *node);
    static ScenePrimitive* iniPrimitive(int& shape, xml_node<> *node);
    static void drawWallBorder(CollisionScene *scene, Vector3 lastVertex, Vector3 newVertex);
    static void drawObstBorder(CollisionScene *scene, int shape, Vector3 pos, xml_node<> *node);
    static void drawWalls(CollisionScene *scene, const Color& wallCol, xml_node<> *node);
    static void drawObstacles(CollisionScene *scene, xml_node<> *node);
    static void drawPlayer(CollisionScene *scene, xml_node<> *node);
public:
    static Number wallHeight;
    static void drawScene(CollisionScene *scene, ScenePrimitive*& plyr, deque<ScenePrimitive*> *wlls, deque<ScenePrimitive*> *obstcls, const char *strFile);
};

#endif