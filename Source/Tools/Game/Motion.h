#pragma once
#include "AssetProcess.h"
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Resource/ResourceCache.h>

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

    void DebugDraw(DebugRenderer* debug)
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

    Vector3 GetStartPos()
    {
        return Vector3(startFromOrigin_.x_, startFromOrigin_.y_, startFromOrigin_.z_);
    }

    float GetStartRot()
    {
        return -rotateAngle_;
    }

    void Process(MotionRig* rig)
    {
        if (processed)
            return;

        unsigned startTime = Time::GetSystemTime();
        animationName_ = GetAnimationName(name_);
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        animation_ = cache->GetResource("Animation", animationName_);
        if (!animation_)
            return;

        rotateAngle_ = rig->ProcessAnimation(animationName_, motionFlag_, allowMotion_, rotateAngle_, motionKeys_, startFromOrigin_);
        SetEndFrame(endFrame_);

        if (!motionKeys_.Empty())
        {
            Vector4 v = motionKeys_[0];
            Vector4 diff = motionKeys_[endFrame_ - 1] - motionKeys_[0];
            endDistance_ = Vector3(diff.x_, diff.y_, diff.z_).length;
        }

        processed = true;

        if (d_log)
        {
            unsigned endTime = Time::GetSystemTime();
            URHO3D_LOGINFO("Motion " + name_ + " endDistance="  + String(endDistance_) + " startFromOrigin=" + startFromOrigin_.ToString()  + " timeCost=" + String(endTime - startTime) + " ms");
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