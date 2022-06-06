#pragma once
#include "Enums.h"

class Node
{
    public:
    Direction direction;
    bool explored_Status;
    bool dead_End_Status;
    bool finish_Status;
    bool backtrack_Status;
    bool nodeTaken;
    int nrOfRodes;
    Node* left;
    Node* right;
    Node* up;
    Node* down;
};