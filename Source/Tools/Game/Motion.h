#pragma once
#include "AssetProcess.h"
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Container/List.h>

namespace Urho3D
{
class Motion : public Object
{
    URHO3D_OBJECT(Motion, Object);

public:
    Motion(Context* c)
    :Object(c)
    ,rigIndex_(0)
    {
    }

    virtual ~Motion()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        cache->ReleaseResource<Animation>(animationName_);
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
        if (processed_)
            return;

        unsigned startTime = Time::GetSystemTime();
        animationName_ = GetAnimationName(name_);
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        animation_ = cache->GetResource<Animation>(animationName_);
        if (!animation_)
            return;

        rotateAngle_ = rig->ProcessAnimation(animationName_, motionFlag_, allowMotion_, rotateAngle_, motionKeys_, startFromOrigin_);
        SetEndFrame(endFrame_);

        if (!motionKeys_.Empty())
        {
            Vector4 v = motionKeys_[0];
            Vector4 diff = motionKeys_[endFrame_ - 1] - motionKeys_[0];
            endDistance_ = Vector3(diff.x_, diff.y_, diff.z_).Length();
        }

        processed_ = true;

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

    int                     rigIndex_;
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

enum MotionLoadingState
{
    kMotionLoadingStart = 0,
    kMotionLoadingAnimations,
    kMotionLoadingMotions,
    kMotionLoadingFinished
};

class MotionManager : public Object
{
    URHO3D_OBJECT(MotionManager, Object);
public:
    MotionManager(Context* c)
    :Object(c)
    ,processedAnimations_(0)
    ,state_(kMotionLoadingStart)
    {
    }

    void AddMotion(MotionPtr motion)
    {
        motions_[motion->nameHash_] = motion;
        unprocessedMotions_.Push(motion);
    }

    Motion* FindMotion(StringHash nameHash)
    {
        MotionPtr ret;
        motions_.TryGetValue(nameHash, ret);
        return ret.Get();
    }

    void AddAnimation(const String& animName)
    {
        animations_.Push(animName);
    }

    void Start()
    {
        assetProcessTime_ = Time::GetSystemTime();
        processScene_ = new Scene(context_);
        AddMotions();
        state_ = kMotionLoadingMotions;
    }

    void Stop()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        motions_.Clear();
        for (unsigned i=0; i<animations_.Size();++i)
            cache->ReleaseResource<Animation>(animations_[i]);
    }

    void Finish()
    {
        unsigned t = Time::GetSystemTime();
        AddTriggers();
        processScene_.Reset();

        URHO3D_LOGINFO("************************************************************************************************");
        URHO3D_LOGINFO("Motion Process time-cost=" + String(Time::GetSystemTime() - assetProcessTime_)
            + " ms num-of-motions=" + String(motions_.Size()));
        URHO3D_LOGINFO("************************************************************************************************");
    }

    bool Update(float dt)
    {
        if (state_ == kMotionLoadingFinished)
            return true;

        if (state_ == kMotionLoadingMotions)
        {
            unsigned t = Time::GetSystemTime();
            int processedMotions = 0;
            while(!unprocessedMotions_.Empty())
            {
                MotionPtr m = unprocessedMotions_.Front();
                m->Process(rigs_[m->rigIndex_].Get());
                unprocessedMotions_.PopFront();
                ++processedMotions;

                unsigned time_diff = Time::GetSystemTime() - t;
                if (time_diff >= PROCESS_TIME_PER_FRAME)
                    break;
            }

            URHO3D_LOGINFO("MotionManager Process this frame time=" +
                String(Time::GetSystemTime() - t) + " ms " + " processedMotions=" + String(processedMotions));
            if (unprocessedMotions_.Empty())
                state_ = kMotionLoadingAnimations;
        }
        else if (state_ == kMotionLoadingAnimations)
        {
            ResourceCache* cache = GetSubsystem<ResourceCache>();
            unsigned t = Time::GetSystemTime();
            for (unsigned i=processedAnimations_; i<animations_.Size(); ++i)
            {
                cache->GetResource("Animation", GetAnimationName(animations_[i]));
                ++processedAnimations_;
                unsigned time_diff = Time::GetSystemTime() - t;
                if (time_diff >= PROCESS_TIME_PER_FRAME)
                    break;
            }

            URHO3D_LOGINFO("MotionManager Process this frame time=" +
                String(Time::GetSystemTime() - t) + " ms " + " processedAnimations_=" + String(processedAnimations_));

            if (processedAnimations_ >= animations_.Size())
            {
                state_ = kMotionLoadingFinished;
                return true;
            }
        }
        return false;
    }

    MotionPtr CreateMotion(const String& name,
        int rigIndex,
        unsigned motionFlag = kMotion_XZR,
        unsigned allowMotion = kMotion_ALL,
        int endFrame = -1,
        bool loop = false,
        float rotateAngle = 361)
    {

    }

    void AddRigs()
    {
    }

    void AddMotions()
    {
    }

    void AddTriggers()
    {
    }

    Vector<MotionRigPtr>               rigs_;
    HashMap<StringHash, MotionPtr>     motions_;
    Vector<String>                     animations_;

    List<MotionPtr>                    unprocessedMotions_;

    SharedPtr<Scene>                   processScene_;

    unsigned                           assetProcessTime_;
    unsigned                           processedMotions_;
    unsigned                           processedAnimations_;
    int                                state_;
};


}