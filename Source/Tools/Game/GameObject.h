#pragma once
#include <Urho3D/Scene/LogicComponent.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/IO/Log.h>

namespace Urho3D
{

class GameObject : public LogicComponent
{
    URHO3D_OBJECT(GameObject, LogicComponent);

public:
    GameObject(Context* context)
    :LogicComponent(context)
    ,flags_(0)
    ,duration_(-1.0F)
    {

    }

    virtual void Update(float timeStep) override
    {
        // Disappear when duration expired
        if (duration_ > 0.0F)
        {
            duration_ -= timeStep;
            if (duration_ <= 0.0F)
                GetNode()->Remove();
        }
    }

    const String& GetName() const
    {
        return GetNode()->GetName();
    }

    virtual void Reset() {};

    virtual String GetDebugText() const { return GetName() + " duration:" + String(duration_); };

    unsigned flags_;
    float duration_;
};


}