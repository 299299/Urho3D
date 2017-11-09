#pragma once
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Ptr.h>

namespace Urho3D
{
class DebugRenderer;

static const unsigned RE_ENTER_FLAG = (1 << 0);

class State : public RefCounted
{
public:
    State()
    :timeInState_(0.0F)
    ,flag_(0)
    {
    }

    virtual ~State() {};

    void SetName(const String& name)
    {
        name_ = name;
        nameHash_ = StringHash(name);
    }

    virtual void Enter(State* lastState)
    {
        timeInState_ = 0;
    }

    virtual void Exit(State* nextState)
    {
        timeInState_ = 0;
    }

    virtual void Update(float dt)
    {
        timeInState_ += dt;
    }

    virtual void FixedUpdate(float dt)
    {
    }

    virtual void DebugDraw(DebugRenderer* debug)
    {
    }

    virtual String GetDebugText() const
    {
        return " name=" + name_ + " timeInState=" + String(timeInState_) + "\n";
    }

    String name_;
    StringHash nameHash_;
    float timeInState_;
    int flag_;
};

typedef SharedPtr<State> StatePtr;

class FSM
{
public:
    void AddState(StatePtr state)
    {
        states_[state->nameHash_] = state;
    }

    StatePtr FindState(const String& name)
    {
        return FindState(StringHash(name));
    }

    StatePtr FindState(StringHash nameHash)
    {
        StatePtr ret;
        states_.TryGetValue(nameHash, ret);
        return ret;
    }

    bool ChangeState(StringHash nameHash)
    {
        StatePtr newState = FindState(nameHash);

        if (!newState)
        {
            Print("new-state not found " + nameHash.ToString());
            return false;
        }

        if (currentState is newState) {
            // Print("same state !!!");
            if (!currentState.CanReEntered())
                return false;
            currentState.Exit(newState);
            currentState.Enter(newState);
        }

        State@ oldState = currentState;
        if (oldState !is null)
            oldState.Exit(newState);

        if (newState !is null)
            newState.Enter(oldState);

        @currentState = @newState;

        String oldStateName = "null";
        if (oldState !is null)
            oldStateName = oldState.name;

        String newStateName = "null";
        if (newState !is null)
            newStateName = newState.name;

        if (d_log)
            Print("FSM Change State " + oldStateName + " -> " + newStateName);

        return true;
    }

    bool ChangeState(const String& name)
    {
        return ChangeState(StringHash(name));
    }

    HashMap<StringHash, StatePtr>     states_;
    StatePtr                          currentState_;
};

}
