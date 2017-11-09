#pragma once
#include "GameObject.h"
#include "FSM.h"
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Physics/RigidBody.h>

namespace Urho3D
{

class Character : public GameObject
{
public:
    Character(Context* context)
    :GameObject(context)
    ,fsm_(new FSM())
    {

    }

    AnimatedModel* GetModel()
    {
        return GetComponent<AnimatedModel>();
    }

    AnimationController* Get


    FSMPtr      fsm_;
}

}