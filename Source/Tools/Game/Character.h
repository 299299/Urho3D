#pragma once
#include "GameObject.h"
#include "FSM.h"
#include "Util.h"
#include "Motion.h"
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/DebugRenderer.h>

namespace Urho3D
{
class Character : public GameObject
{
public:
    Character(Context* context)
    :GameObject(context)
    ,fsm_(new FSM(context))
    ,motion_startRotation_(0)
    ,motion_deltaRotation_(0)
    ,motion_translateEnabled_(true)
    ,motion_rotateEnabled_(true)
    ,animationPickIndex_(0)
    {

    }

    AnimatedModel* GetModel()
    {
        return model_;
    }

    AnimationController* GetAnimCtl()
    {
        return ctrl_;
    }

    Node* GetRenderNode()
    {
        return renderNode_;
    }

    virtual void DelayedStart() override
    {
        renderNode_ = GetNode()->GetChild("RenderNode");
        model_ = renderNode_->GetComponent<AnimatedModel>();
        ctrl_ = renderNode_->GetComponent<AnimationController>();
        initialPostion_ = GetNode()->GetWorldPosition();
        initialRotation_ = GetNode()->GetWorldRotation();
        CommonStateFinishedOnGroud();
    }

    virtual void Reset() override
    {
        GetNode()->SetWorldPosition(initialPostion_);
        GetNode()->SetWorldRotation(initialRotation_);
        CommonStateFinishedOnGroud();
    }

    virtual void CommonStateFinishedOnGroud()
    {
        fsm_->ChangeState("StandState");
    }

    virtual void DebugDraw(DebugRenderer* debug)
    {
        fsm_->DebugDraw(debug);
        debug->AddNode(GetNode(), 0.5f, false);
    }

    void MoveTo(const Vector3& position, float dt)
    {
        GetNode()->SetWorldPosition(FilterPosition(position));
    }

    void PlayAnimation(const String& animName,
        int layer,
        bool loop = false,
        float blendTime = 0.1f,
        float startTime = 0.0f,
        float speed = 1.0f)
    {
        if (d_log)
            URHO3D_LOGINFO(GetName() + " PlayAnimation " + animName +
                " loop=" + String(loop) +
                " blendTime=" + String(blendTime) +
                " startTime=" + String(startTime) +
                " speed=" + String(speed));

        if (layer == 0 && lastAnimation_ == animName && loop)
            return;

        lastAnimation_ = animName;
        AnimationController* ctrl = GetAnimCtl();
        ctrl->StopLayer(layer, blendTime);
        ctrl->PlayExclusive(animName, layer, loop, blendTime);
        ctrl->SetSpeed(animName, speed);
        ctrl->SetTime(animName, (speed < 0) ? ctrl->GetLength(animName) : startTime);
    }

    void PlayMotion(Motion* m, float localTime = 0.0F, float blendTime = 0.1F, float speed = 1.0F)
    {
        PlayAnimation(m->animationName_, 0, m->looped_, blendTime, localTime, speed);
        motion_startPosition_ = GetNode()->GetWorldPosition();
        motion_startRotation_ = GetNode()->GetWorldRotation().EulerAngles().y_;
        motion_deltaRotation_ = 0.0F;
        motion_deltaPosition_ = Vector3::ZERO;
        motion_velocity_ = Vector3::ZERO;
        motion_translateEnabled_ = true;
        motion_rotateEnabled_ = true;
    }

    int Move(Motion* motion, float dt)
    {
        AnimationController* ctrl = GetAnimCtl();
        Node* _node = GetNode();
        float localTime = ctrl->GetTime(motion->animationName_);
        float speed = ctrl->GetSpeed(motion->animationName_);
        float absSpeed = Abs(speed);

        if (absSpeed < 0.001F)
            return 0;

        dt *= absSpeed;
        if (motion->looped_ || speed < 0)
        {
            Vector4 motionOut = Vector4(0, 0, 0, 0);
            motion->GetMotion(localTime, dt, motion->looped_, motionOut);
            if (!motion->looped_)
            {
                if (localTime < SEC_PER_FRAME)
                    motionOut = Vector4(0, 0, 0, 0);
            }

            if (motion_rotateEnabled_)
                _node->Yaw(motionOut.w_);

            if (motion_translateEnabled_)
            {
                Vector3 tLocal(motionOut.x_, motionOut.y_, motionOut.z_);
                // tLocal = tLocal * ctrl.GetWeight(animationName_);
                Vector3 tWorld = _node->GetWorldRotation() * tLocal + _node->GetWorldPosition() + motion_velocity_ * dt;
                MoveTo(tWorld, dt);
            }

            return (speed < 0 && localTime < 0.001F) ? 1 : 0;
        }
        else
        {
            Vector4 motionOut = motion->GetKey(localTime);
            if (motion_rotateEnabled_)
                _node->SetWorldRotation(Quaternion(0, motion_startRotation_ + motionOut.w_ + motion_deltaRotation_, 0));

            if (motion_translateEnabled_)
            {
                motion_deltaPosition_ += motion_velocity_ * dt;
                Vector3 tWorld = Quaternion(0, motion_startRotation_ + motion_deltaRotation_, 0) * Vector3(motionOut.x_, motionOut.y_, motionOut.z_) + motion_startPosition_ + motion_deltaPosition_;
                MoveTo(tWorld, dt);
            }

            bool bFinished = (speed > 0) ? localTime >= motion->endTime_ : (localTime < 0.001F);
            return bFinished ? 1 : 0;
        }
    }

    Vector3 GetFuturePosition(Motion* motion, float t)
    {
        Vector4 motionOut = motion->GetKey(t);
        Vector3 v_motion(motionOut.x_, motionOut.y_, motionOut.z_);
        if (motion->looped_)
            return GetNode()->GetWorldRotation() * v_motion + GetNode()->GetWorldPosition() + motion_deltaPosition_;
        else
            return Quaternion(0, motion_startRotation_ + motion_deltaRotation_, 0) * v_motion + motion_startPosition_ + motion_deltaPosition_;
    }

    float GetFutureRotation(Motion* motion, float t)
    {
        Vector4 motionOut = motion->GetKey(t);
        if (motion->looped_)
            return ClampAngle(GetNode()->GetWorldRotation().EulerAngles().y_ + motion_deltaRotation_ + motionOut.w_);
        else
            return ClampAngle(motion_startRotation_ + motion_deltaRotation_ + motionOut.w_);
    }

    void DebugDraw(DebugRenderer* debug, Motion* motion)
    {
        Vector4 tFinnal = motion->GetKey(motion->endTime_);
        Vector3 tLocal(tFinnal.x_, tFinnal.y_, tFinnal.z_);
        if (motion->looped_)
        {
            debug->AddLine(GetNode()->GetWorldRotation() * tLocal + GetNode()->GetWorldPosition(), GetNode()->GetWorldPosition(), Color(0.5f, 0.5f, 0.7f), false);
        }
        else
        {
            Vector3 tMotionEnd = Quaternion(0, motion_startRotation_ + motion_deltaRotation_, 0) * tLocal;
            debug->AddLine(tMotionEnd + motion_startPosition_ ,  motion_startPosition_ , Color(0.5f, 0.5f, 0.7f), false);
            DebugDrawDirection(debug, GetNode()->GetWorldPosition(), motion_startRotation_ + motion_deltaRotation_ + tFinnal.w_, Color::RED, 2.0);
        }
    }

    float GetAngle() const
    {
        Vector3 dir = GetNode()->GetWorldRotation() * Vector3(0, 0, 1.0F);
        return Atan2(dir.x_, dir.z_);
    }

    FSMPtr                  fsm_;
    Node*                   renderNode_;
    AnimatedModel*          model_;
    AnimationController*    ctrl_;

    Vector3                 initialPostion_;
    Quaternion              initialRotation_;

    String                  lastAnimation_;

    int                     animationPickIndex_;

    // ==============================================
    //   DYNAMIC VALUES For Motion
    // ==============================================
    Vector3                 motion_startPosition_;
    float                   motion_startRotation_;

    float                   motion_deltaRotation_;
    Vector3                 motion_deltaPosition_;
    Vector3                 motion_velocity_;

    bool                    motion_translateEnabled_;
    bool                    motion_rotateEnabled_;
};

}