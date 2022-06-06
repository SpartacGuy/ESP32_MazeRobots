#pragma once
#include "Structs.h"
#include <Arduino.h>

class Maze
{
private:
    Node* start;
    Node* curr;
    Node* nextLocation;
    Node* nodeFoundByOther;
    int roadCounter;

    Node* CreateNewNode(Direction dir, Node* node);
    void NodeStatusInit(Node* node);
    Direction Backtrack(Node** node);
    Node* SelectUnexploredPath();
    Node* SelectExploredPath();
    int InBackTracking(Node* node);
    String ToString(Direction dir);
    Node* FindNodeToUpdate(String path);
    String GetPositionPath(Node* node);
    void ResetTakenNodes(Node* curr);
public:
    Maze();
    ~Maze();
    void UpdateNode(bool leftSide, bool rightSide,
                    bool upSide, bool downSide, bool dEndStatus, bool fStatus);
    void UpdateOtherRobotNode(String path, bool leftSide, bool rightSide,
                              bool upSide, bool downSide, bool deadEnd, bool finish);
    String GetDecision();
    Node* GetCurrent(); //only for debugging
    Node* GetStart();
    String GetCurrentPosition();
    Direction GetDirection();
    bool MoveNext(bool response);
    String GetCurrentInfo();
};