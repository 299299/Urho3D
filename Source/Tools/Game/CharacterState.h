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


class SingleAnimationState : public CharacterState
{
public:
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


class SingleMotionState : public CharacterState
{
public:
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

class MultiAnimationState : public CharacterState
{
public:
    Vector<String> animations_;
    bool looped_;
    float stateTime_;
    int selectIndex_;

    MultiAnimationState(Character* c)
    :CharacterState(c)
    ,looped_(false)
    ,stateTime_(-1)
    ,selectIndex_(0)
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
                finished = ownner_->GetAnimCtl()->GetTime(animations_[selectIndex_]) < 0.0001F;
            }
            else
                finished = ownner_->GetAnimCtl()->IsAtEnd(animations_[selectIndex_]);
        }

        if (finished)
            OnMotionFinished();

        CharacterState::Update(dt);
    }

    virtual void Enter(State* lastState) override
    {
        selectIndex_ = PickIndex();
        ownner_->PlayAnimation(animations_[selectIndex_], 0, looped_, blendTime_, startTime_, animSpeed_);
        CharacterState::Enter(lastState);
    }

    virtual void OnMotionFinished()
    {
        ownner_->CommonStateFinishedOnGroud();
    }

    void AddMotion(const String& name)
    {
        animations_.Push(GetAnimationName(name));
    }

    virtual int PickIndex() const
    {
        return ownner_->animationPickIndex_;
    }
};


class MultiMotionState : public CharacterState
{
public:
    Vector<Motion*> motions_;
    int selectIndex_;

    MultiMotionState(Character* c)
    :CharacterState(c)
    ,selectIndex_(0)
    {
    }

    virtual void Update(float dt) override
    {
        int ret = ownner_->Move(motions_[selectIndex_], dt);
        if (ret == 1)
            OnMotionFinished();
        else if (ret == 2)
            OnMotionAlignTimeOut();
        CharacterState::Update(dt);
    }

    virtual void Enter(State* lastState) override
    {
        Start();
        CharacterState::Enter(lastState);
    }

    void Start()
    {
        selectIndex_ = PickIndex();
        if (selectIndex_ >= int(motions_.Size()))
        {
            URHO3D_LOGERROR("ERROR: a large animation index=" + String(selectIndex_) + " name:" + ownner_->GetName());
            selectIndex_ = 0;
        }

        ownner_->PlayMotion(motions_[selectIndex_], startTime_, blendTime_, animSpeed_);
    }

    virtual void DebugDraw(DebugRenderer* debug) override
    {
        ownner_->DebugDraw(debug, motions_[selectIndex_]);
    }

    virtual int PickIndex() const
    {
        return ownner_->animationPickIndex_;
    }

    virtual String GetDebugText() const override
    {
        return " name=" + name_ + " timeInState=" + String(timeInState_) +
               " current motion=" + motions_[selectIndex_]->animationName_ + "\n";
    }

    void AddMotion(const String& name)
    {
        motions_.Push(GetSubsystem<MotionManager>()->FindMotion(StringHash(name)));
    }

    virtual void OnMotionFinished()
    {
        // Print(ownner.GetName() + " state:" + name + " finshed motion:" + motions[selectIndex].animationName);
        ownner_->CommonStateFinishedOnGroud();
    }

    virtual void OnMotionAlignTimeOut()
    {
    }
};


class CharacterAlignState : public CharacterState
{
public:
    StringHash  nextStateName_;
    String      alignAnimation_;
    Vector3     targetPosition_;
    float       targetRotation_;
    Vector3     movePerSec_;
    float       rotatePerSec_;
    float       alignTime_;

    CharacterAlignState(Character* c)
    :CharacterState(c)
    ,alignTime_(0.2F)
    {
        SetName("AlignState");
    }

    void Start(StringHash nextState,
        const Vector3& tPos,
        float tRot,
        float duration,
        const String& anim = "")
    {
        nextStateName_ = nextState;
        targetPosition_ = tPos;
        targetRotation_ = tRot;
        alignTime_ = duration;
        alignAnimation_ = anim;

        Vector3 curPos = ownner_->GetNode()->GetWorldPosition();
        float curAngle = ownner_->GetAngle();
        movePerSec_ = (tPos - curPos) / duration;
        rotatePerSec_ = ClampAngle(tRot - curAngle) / duration;

        if (anim != "")
        {
            ownner_->PlayAnimation(anim, 0, true);
        }
    }

    virtual void Update(float dt) override
    {
        ownner_->MoveTo(ownner_->GetNode()->GetWorldPosition() + movePerSec_ * dt, dt);
        ownner_->GetNode()->Yaw(rotatePerSec_ * dt);
        CharacterState::Update(dt);
        if (timeInState_ >= alignTime_)
            OnAlignTimeOut();
    }

    virtual void DebugDraw(DebugRenderer* debug) override
    {
        DebugDrawDirection(debug, ownner_->GetNode()->GetWorldPosition(), targetRotation_, Color::RED, 2.0f);
        debug->AddCross(targetPosition_, 0.5F, Color::YELLOW, false);
    }

    void OnAlignTimeOut()
    {
        ownner_->GetNode()->SetWorldPosition(targetPosition_);
        ownner_->GetNode()->SetWorldRotation(Quaternion(0, targetRotation_, 0));
        ownner_->fsm_->ChangeState(nextStateName_);
    }
};

}