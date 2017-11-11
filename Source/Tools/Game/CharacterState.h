#pragma once
#include "Character.h"

namespace Urho3D
{

class CharacterState : public State
{
public:
    Character*                  ownner_;
    float                       animSpeed_;
    float                       blendTime_;
    float                       startTime_;
    bool                        combatReady_;
    unsigned                    characterFlags_;

    CharacterState(Character* c)
    :State(c->GetContext())
    ,ownner_(c)
    ,animSpeed_(1.0F)
    ,blendTime_(0.2F)
    ,startTime_(0.0F)
    ,characterFlags_(0)
    {
    }

    virtual ~CharacterState()
    {
    }

    float GetThreatScore() const
    {
        return 0.0f;
    }

    virtual void Enter(State* lastState) override
    {
        AddFlag(ownner_->flags_, characterFlags_);
        State::Enter(lastState);
    }

    virtual void Exit(State* nextState) override
    {
        RemoveFlag(ownner_->flags_, characterFlags_);
        State::Exit(nextState);
    }
};


class SingleAnimationState : CharacterState
{
    String animation_;
    bool looped_;
    float stateTime_;

    SingleAnimationState(Character* c)
    :CharacterState(c)
    ,looped_(false)
    ,stateTime_(-1.0F)
    {
    }

    virtual void Update(float dt) override
    {
        bool finished = false;
        if (looped_)
        {
            if (stateTime_ > 0 && timeInState_ > stateTime_)
                finished = true;
        }
        else
        {
            if (animSpeed_ < 0)
            {
                finished = ownner_->GetAnimCtl()->GetTime(animation_) < 0.0001F;
            }
            else
                finished = ownner_->GetAnimCtl()->IsAtEnd(animation_);
        }

        if (finished)
            OnMotionFinished();

        CharacterState::Update(dt);
    }

    virtual void Enter(State* lastState) override
    {
        ownner_->PlayAnimation(animation_, 0, looped_, blendTime_, startTime_, animSpeed_);
        CharacterState::Enter(lastState);
    }

    virtual void OnMotionFinished()
    {
        ownner_->CommonStateFinishedOnGroud();
    }

    void SetMotion(const String& name)
    {
        animation_ = GetAnimationName(name);
    }
};


class SingleMotionState : CharacterState
{
    Motion* motion_;

    SingleMotionState(Character* c)
    :CharacterState(c)
    ,motion_(NULL)
    {
    }

    virtual void Update(float dt) override
    {
        if (ownner_->Move(motion_, dt) == 1)
            OnMotionFinished();
        CharacterState::Update(dt);
    }

    virtual void Enter(State* lastState) override
    {
        ownner_->PlayMotion(motion_, startTime_, blendTime_, animSpeed_);
        CharacterState::Enter(lastState);
    }

    virtual void DebugDraw(DebugRenderer* debug) override
    {
        ownner_->DebugDraw(debug, motion_);
    }

    void SetMotion(const String& name)
    {
        motion_ = GetSubsystem<MotionManager>()->FindMotion(StringHash(name));
    }

    virtual void OnMotionFinished()
    {
        // Print(ownner.GetName() + " state:" + name + " finshed motion:" + motion.animationName);
        ownner_->CommonStateFinishedOnGroud();
    }
};

}