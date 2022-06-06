#include "Maze.h"

Maze::Maze()
{
    roadCounter = 0;

    start = new Node();
    NodeStatusInit(start);
    // start->explored_Status = true;
    start->direction = None;

    curr = start;

    Serial.println("curr constructor: " + String((int)curr));
    Serial.println("curr constructor direction: " + String((int)curr->direction));
    
}

void Maze::NodeStatusInit(Node* node)
{
    node->explored_Status = false;
    node->dead_End_Status = false;
    node->backtrack_Status = false;
    node->finish_Status = false;
}

Node* Maze::CreateNewNode(Direction dir, Node* node)
{
    Node* newNode = new Node();

    newNode->direction = dir;
    newNode->nrOfRodes = 0;
    newNode->backtrack_Status = false;
    NodeStatusInit(newNode);

    switch (dir)
    {
    case Left:
        newNode->left = nullptr;
        newNode->right = node;
        newNode->up = nullptr;
        newNode->down = nullptr;
        break;
    case Right:
        newNode->left = node;
        newNode->right = nullptr;
        newNode->up = nullptr;
        newNode->down = nullptr;
        break;
    case Up:
        newNode->left = nullptr;
        newNode->right = nullptr;
        newNode->up = nullptr;
        newNode->down = node;
        break;
    case Down:
        newNode->left = nullptr;
        newNode->right = nullptr;
        newNode->up = node;
        newNode->down = nullptr;
        break;

    default:
        break;
    }

    return newNode;
}

void Maze::UpdateNode(bool leftSide, bool rightSide, bool upSide, bool downSide,
                      bool deadEnd, bool finish)
{
    Serial.println("In update node");
    if (!curr->explored_Status)
    {
        if (leftSide)
        {
            Serial.println("in left node");
            curr->left = CreateNewNode(Left, curr);
            curr->nrOfRodes++;
        }
        if (rightSide)
        {
            Serial.println("in right node");
            curr->right = CreateNewNode(Right, curr);
            curr->nrOfRodes++;
        }
        if (upSide)
        {
            Serial.println("in up node");
            curr->up = CreateNewNode(Up, curr);
            curr->nrOfRodes++;
        }
        if (downSide)
        {
            Serial.println("in down node");
            curr->down = CreateNewNode(Down, curr);
            curr->nrOfRodes++;
        }
    
        curr->explored_Status = true;
        curr->dead_End_Status = deadEnd;
        curr->finish_Status = finish;
    }
}


Node* Maze::SelectUnexploredPath()
{
    Node* temp = nullptr;
    Serial.println("current in selecting unexplored path: " + String((int)curr));
    Serial.println("current left in selecting unexplored path: " + String((int)curr->left));
    Serial.println("current right in selecting unexplored path: " + String((int)curr->right));
    Serial.println("current up in selecting unexplored path: " + String((int)curr->up));
    
    if (curr->left != nullptr && !curr->left->explored_Status && !curr->left->nodeTaken)
    {
        temp = curr->left;
        Serial.println("Choosing left unexplored path");
    }
    else if (curr->right != nullptr && !curr->right->explored_Status && !curr->right->nodeTaken)
    {
        temp = curr->right;
        Serial.println("Choosing right unexplored path");
    }
    else if (curr->up != nullptr && !curr->up->explored_Status && !curr->up->nodeTaken)
    {
        temp = curr->up;
        Serial.println("Choosing up unexplored path");
    }
    else if (curr->down != nullptr && !curr->down->explored_Status && !curr->down->nodeTaken)
    {
        temp = curr->down;
        Serial.println("Choosing down unexplored path");
    }

    Serial.println("selected unexplored path: " + String((int)temp));
    return temp;
}

Node* Maze::SelectExploredPath()
{
    Node* temp = nullptr;
    Serial.println("selecting explored path left node taken value: " + String(curr->left->nodeTaken));
    Serial.println("selecting explored path right node taken value: " + String(curr->right->nodeTaken));
    Serial.println("selecting explored path up node taken value: " + String(curr->up->nodeTaken));
    //get the path which does not eventually lead to a dead end
    if (curr->left != nullptr && !curr->left->backtrack_Status && !curr->left->nodeTaken)
    {
        temp = curr->left;
    }
    else if (curr->right != nullptr && !curr->right->backtrack_Status && !curr->right->nodeTaken)
    {
        temp = curr->right;
    }
    else if (curr->up != nullptr && !curr->up->backtrack_Status && !curr->up->nodeTaken)
    {
        temp = curr->up;
    }
    else if (curr->down != nullptr && !curr->down->backtrack_Status && !curr->down->nodeTaken)
    {
        temp = curr->down;
    }

    return temp;
}

String Maze::GetDecision()
{
    if (curr->finish_Status)
    {
        return "FinishReached";
    }

    if (curr->dead_End_Status)
    {
        nextLocation = curr;
        curr->backtrack_Status = true;
        Backtrack(&nextLocation);
        // roadCounter++;
        return GetPositionPath(nextLocation);
    }

    Node* tempLocation = nullptr;
    tempLocation = SelectUnexploredPath();

    if (tempLocation != nullptr)
    {
        nextLocation = tempLocation;
        Serial.println("In get decision the path to the next point is: " + GetPositionPath(nextLocation));
    }
    else
    {
        tempLocation = SelectExploredPath();
        if (tempLocation == nullptr)
        {
            return "No Paths Available";
        }
        else
        {
            if (!InBackTracking(tempLocation))
            {
                nextLocation = tempLocation;
                // Serial.println("In get decision the path to the next point is: " + GetPositionPath(nextLocation));
            }
            else
            {
                curr->backtrack_Status = true;
                nextLocation = curr;
                Backtrack(&nextLocation);
                // roadCounter++;
                return GetPositionPath(nextLocation);
            }
        }
    }
    // roadCounter++;
    return GetPositionPath(nextLocation);
}

bool Maze::MoveNext(bool response)
{
    //if other cars approved, then move to next location
    Serial.println("curr MoveNext: " + String((int)curr));
    Serial.println("curr MoveNext direction: " + String((int)curr->direction));
    Serial.println("nextLocation MoveNext direction: " + String((int)curr->direction));
    
    if (response)
    {
        ResetTakenNodes(curr);
        curr = nextLocation;
        return true;
    } 
    
    roadCounter++;
    Serial.println("RoadCounter is: " + String(roadCounter));
    Serial.println("Current nr of roads is: " + String(curr->nrOfRodes));
    //if all possible rodes taken, start ask from the first one again
    if (roadCounter >= curr->nrOfRodes)
    {
        roadCounter = 0;
        ResetTakenNodes(curr);
    }
    else
    {
        nextLocation->nodeTaken = true; //if cant move than its taken
    }
    
    nextLocation = curr;
    return false;
}

void Maze::ResetTakenNodes(Node* node)
{
    Serial.println("Resetting the taken nodes");
    if (node->left != nullptr) 
    {
        Serial.println("reset taken node left");
        node->left->nodeTaken = false;
    }
    if (node->right != nullptr) 
    {
        node->right->nodeTaken = false;
    }
    if (node->up != nullptr) 
    {
        node->up->nodeTaken = false;
    }
    if (node->down != nullptr) 
    {
        node->down->nodeTaken = false;
    }
    Serial.println("Left node taken status: " + String(node->left->nodeTaken));
    Serial.println("Left curr taken status: " + String(curr->left->nodeTaken));
}

Direction Maze::Backtrack(Node** givenNode)
{
    //Serial.println("backtracking");
    Direction dir = Up;
    switch ((*givenNode)->direction)
    {
    case Left:
        (*givenNode) = (*givenNode)->right;
        dir = Right;
        break;
    case Right:
        (*givenNode) = (*givenNode)->left;
        dir = Left;
        break;
    case Up:
        (*givenNode) = (*givenNode)->down;
        dir = Down;
        break;
    case Down:
        (*givenNode) = (*givenNode)->up;
        dir = Up;
        break;

    default:
        break;
    }

    return dir;
}

int Maze::InBackTracking(Node* nextNode)
{
    int result = 0;
    //if opposite direction of curr is same as the next node, then backtrack
    switch (curr->direction)
    {
    case Left:
        if (curr->right == nextNode)
        {
            result = 1;
        }
        break;
    case Right:
        if (curr->left == nextNode)
        {
            result = 1;
        }
        break;
    case Up:
        if (curr->down == nextNode)
        {
            result = 1;
        }
        break;
    case Down:
        if (curr->up == nextNode)
        {
            result = 1;
        }
        break;    

    default:
        break;
    }
    
    return result;
}

String Maze::GetPositionPath(Node* selectedNode)
{
    String pos = "";
    Node* tempNode = selectedNode;
    while (tempNode != start)
    {
        pos += ToString(Backtrack(&tempNode));
    }
    
    //reversing the string, so the path will start from beggining of maze
    int stringSize = pos.length();
    for (int i = 0; i < stringSize / 2; i++)
    {
        std::swap(pos[i], pos[stringSize-i-1]);
    }

    return pos;
}

String Maze::ToString(Direction dir)
{
    String convertedDirection = "";
    //reversing the direction, so we have it from start to current point
    //and not from current point to start
    switch (dir)
    {
    case Left:
        convertedDirection = "R";
        break;
    case Right:
        convertedDirection = "L";
        break;
    case Up:
        convertedDirection = "D";
        break;
    case Down:
        convertedDirection = "U";
        break;
    
    default:
        break;
    }
    
    return convertedDirection;
}

String Maze::GetCurrentPosition()
{
    return GetPositionPath(curr);
}

Direction Maze::GetDirection()
{
    return curr->direction;
}