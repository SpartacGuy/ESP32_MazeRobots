#pragma once

enum Robot_State
{
    Connecting,
    WaitingForNodeInfo,
    ChoosingNextNode,
    WaitingForApproval,
    MovingToNextNode,
    UpdateCommonList
};