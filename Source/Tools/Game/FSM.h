#pragma once
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Container/HashMap.h>
#include <Urho3D/Container/Ptr.h>
#include <Urho3D/Core/Object.h>
#include <Urho3D/IO/Log.h>

namespace Urho3D
{
class DebugRenderer;

static const unsigned STATE_FLAG_RE_ENTER = (1 << 0);

class State : public Object
{
    URHO3D_OBJECT(State, Object);

public:
    State(Context* c)
    :Object(c)
    ,timeInState_(0.0F)
    ,flags_(0)
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

    virtual void DebugDraw(DebugRenderer* debug)
    {
    }

    virtual String GetDebugText() const
    {
        return " name=" + name_ + " timeInState=" + String(timeInState_) + "\n";
    }

    void AddFlag(int flag)
    {
        flags_ |= flag;
    }

    void RemoveFlag(int flag)
    {
        flags_ &= ~flag;
    }

    bool HasFlag(int flag)
    {
        return (flags_ & flag) != 0;
    }

    String name_;
    StringHash nameHash_;
    float timeInState_;
    unsigned flags_;
};

typedef SharedPtr<State> StatePtr;

class FSM : public Object
{
    URHO3D_OBJECT(FSM, Object);

public:
    FSM(Context* c)
    :Object(c)
    {

    }

    void AddState(StatePtr state)
    {
        states_[state->nameHash_] = state;
    }

    StatePtr FindState(const char* name)
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

        if (newState.Null())
        {
            URHO3D_LOGERROR("new-state not found " + nameHash.ToString());
            return false;
        }

        if (currentState_ == newState)
        {
            if (!currentState_->HasFlag(STATE_FLAG_RE_ENTER))
                return false;
            currentState_->Exit(newState.Get());
            currentState_->Enter(newState.Get());
        }

        StatePtr oldState = currentState_;
        if (oldState.NotNull())
            oldState->Exit(newState.Get());

        if (newState.NotNull())
            newState->Enter(oldState.Get());

        currentState_ = newState;

        if (d_log)
        {
            URHO3D_LOGINFO("FSM Change State " +
                   (oldState.NotNull() ? oldState->name_ : "null") + " -> " +
                   (newState.NotNull() ? newState->name_ : "null"));
        }

        return true;
    }

    bool ChangeState(const char* name)
    {
        return ChangeState(StringHash(name));
    }

    void DebugDraw(DebugRenderer* debug)
    {
        if (currentState_.NotNull())
            currentState_->DebugDraw(debug);
    }

    HashMap<StringHash, StatePtr>     states_;
    StatePtr                          currentState_;
};

typedef SharedPtr<FSM> FSMPtr;

}
