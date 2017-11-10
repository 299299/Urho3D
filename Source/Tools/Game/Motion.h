#pragma once
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/IO/Log.h>
#include "Character.h"

namespace Urho3D
{

class Motion : public Object
{
    URHO3D_OBJECT(Motion, Object);

public:
    Motion(Context* c)
    :Object(c)
    {
    }

    void SetEndFrame(int frame)
    {
        endFrame_ = frame;
        if (endFrame_ < 0)
        {
            endFrame_ = motionKeys_.Size() - 1;
            endTime_ = animation_->GetLength();
        }
        else
        {
            endTime_ = float(endFrame_) * SEC_PER_FRAME;
        }
    }

    void GetMotion(float t, float dt, bool loop, Vector4& out_motion)
    {
        if (motionKeys_.Empty())
            return;

        float future_time = t + dt;
        if (future_time > animation_->GetLength() && loop) {
            Vector4 t1 = Vector4(0,0,0,0);
            Vector4 t2 = Vector4(0,0,0,0);
            GetMotion(t, animation_->GetLength() - t, false, t1);
            GetMotion(0, t + dt - animation_->GetLength(), false, t2);
            out_motion = t1 + t2;
        }
        else
        {
            Vector4 k1 = GetKey(t);
            Vector4 k2 = GetKey(future_time);
            out_motion = k2 - k1;
        }
    }

    Vector4 GetKey(float t)
    {
        if (motionKeys_.Empty())
            return Vector4(0, 0, 0, 0);

        unsigned i = unsigned(t * FRAME_PER_SEC);
        if (i >= motionKeys_.Size())
            i = motionKeys_.Size() - 1;
        Vector4 k1 = motionKeys_[i];
        unsigned next_i = i + 1;
        if (next_i >= motionKeys_.Size())
            next_i = motionKeys_.Size() - 1;
        if (i == next_i)
            return k1;
        Vector4 k2 = motionKeys_[next_i];
        float a = t*FRAME_PER_SEC - float(i);
        return k1.Lerp(k2, a);
    }

    Vector3 GetFuturePosition(Character* mover, float t)
    {
        Vector4 motionOut = GetKey(t);
        Node* _node = mover->GetNode();
        Vector3 v_motion(motionOut.x_, motionOut.y_, motionOut.z_);
        if (looped_)
            return _node->GetWorldRotation() * v_motion + _node->GetWorldPosition() + mover->motion_deltaPosition_;
        else
            return Quaternion(0, mover->motion_startRotation_ + mover->motion_deltaRotation_, 0) * v_motion + mover->motion_startPosition_ + mover->motion_deltaPosition_;
    }

    float GetFutureRotation(Character* mover, float t)
    {
        if (looped_)
            return ClampAngle(mover->GetNode()->GetWorldRotation().EulerAngles().y_ + mover->motion_deltaRotation_ + GetKey(t).w_);
        else
            return ClampAngle(mover->motion_startRotation_ + mover->motion_deltaRotation_ + GetKey(t).w_);
    }

    void DebugDraw(DebugRenderer* debug, Character* mover)
    {
        Node* _node = mover->GetNode();
        Vector4 tFinnal = GetKey(endTime_);
        Vector3 tLocal(tFinnal.x_, tFinnal.y_, tFinnal.z_);
        if (looped_)
        {

            debug->AddLine(_node->GetWorldRotation() * tLocal + _node->GetWorldPosition(), _node->GetWorldPosition(), Color(0.5f, 0.5f, 0.7f), false);
        }
        else
        {
            Vector3 tMotionEnd = Quaternion(0, mover->motion_startRotation_ + mover->motion_deltaRotation_, 0) * tLocal;
            debug->AddLine(tMotionEnd + mover->motion_startPosition_ ,  mover->motion_startPosition_ , Color(0.5f, 0.5f, 0.7f), false);
            DebugDrawDirection(debug, _node->GetWorldPosition(), mover->motion_startRotation_ + mover->motion_deltaRotation_ + tFinnal.w_, Color::RED, 2.0);
        }
    }

    String                  name_;
    String                  animationName_;
    StringHash              nameHash_;

    Animation*              animation_;
    PODVector<Vector4>      motionKeys_;
    float                   endTime_;
    bool                    looped_;

    Vector4                 startFromOrigin_;
    float                   endDistance_;

    int                     endFrame_;
    int                     motionFlag_;
    int                     allowMotion_;

    float                   maxHeight_;

    float                   rotateAngle_;
    bool                    processed_;
};

class AttackMotion  : public Motion
{
public:
    // ==============================================
    //   ATTACK VALUES
    // ==============================================

    float                   impactTime_;
    float                   impactDist_;
    Vector3                 impactPosition_;
    int                     type_;
    String                  boneName_;

    AttackMotion(Context* c, const String& name, int impactFrame, int _type, const String& bName)
    :Motion(c)
    {

    }
};

typedef SharedPtr<Motion> MotionPtr;


}