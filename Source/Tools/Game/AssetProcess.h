#pragma once
#include "Constants.h"
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/Skeleton.h>
#include <Urho3D/Graphics/Model.h>

namespace Urho3D
{
enum RootMotionFlag
{
    kMotion_None= 0,
    kMotion_X   = (1 << 0),
    kMotion_Y   = (1 << 1),
    kMotion_Z   = (1 << 2),
    kMotion_R   = (1 << 3),

    kMotion_Ext_Rotate_From_Start = (1 << 4),
    kMotion_Ext_Debug_Dump = (1 << 5),
    kMotion_Ext_Adjust_Y = (1 << 6),
    kMotion_Ext_Foot_Based_Height = (1 << 7),

    kMotion_XZR = kMotion_X | kMotion_Z | kMotion_R,
    kMotion_YZR = kMotion_Y | kMotion_Z | kMotion_R,
    kMotion_XYR = kMotion_X | kMotion_Y | kMotion_R,

    kMotion_XZ  = kMotion_X | kMotion_Z,
    kMotion_XR  = kMotion_X | kMotion_R,
    kMotion_ZR  = kMotion_Z | kMotion_R,
    kMotion_XY  = kMotion_X | kMotion_Y,
    kMotion_YZ  = kMotion_Y | kMotion_Z,
    kMotion_XYZ = kMotion_XZ | kMotion_Y,
    kMotion_ALL = kMotion_XZR | kMotion_Y,
};


class MotionRig
{
public:
    MotionRig(const char* rigName, Scene* processScene)
    :processScene_(processScene)
    ,rigName_(rigName)
    {
        processNode_ = processScene_->CreateChild(rigName_ + "_Character");
        processNode_->SetWorldRotation(Quaternion(0, 180, 0));

        AnimatedModel* am = processNode_->CreateComponent<AnimatedModel>();
        ResourceCache* cache = processNode_->GetSubsystem<ResourceCache>();
        am->SetModel(cache->GetResource<Model>(rigName_));

        Bone* bone = am->GetSkeleton().GetBone(ROTATE_BONE_NAME);
        rotateBoneInitQ_ = bone->initialRotation_;

        pelvisRightAxis_ = rotateBoneInitQ_ * Vector3(1, 0, 0);
        pelvisRightAxis_.Normalize();

        translateNode_ = processNode_->GetChild(TRANSLATE_BONE_NAME, true);
        rotateNode_ = processNode_->GetChild(ROTATE_BONE_NAME, true);
        pelvisOrign_ = am->GetSkeleton().GetBone(TRANSLATE_BONE_NAME)->initialPosition_;

        left_foot_to_ground_height_ = processNode_->GetChild(L_FOOT, true)->GetWorldPosition().y_;
        right_foot_to_ground_height_ = processNode_->GetChild(R_FOOT, true)->GetWorldPosition().y_;

        alignNode_ = processScene_->CreateChild(rigName_ + "_Align");
        alignNode_->SetWorldRotation(Quaternion(0, 180, 0));
        AnimatedModel* am2 = alignNode_->CreateComponent<AnimatedModel>();
        am2->SetModel(am->GetModel());

        URHO3D_LOGINFO(rigName_ + " pelvisRightAxis=" + pelvisRightAxis_.ToString() + " pelvisOrign=" + pelvisOrign_.ToString());
    }

    ~MotionRig()
    {
        processNode_->Remove();
        alignNode_->Remove();
    }

    Vector3 GetProjectedAxis(Node* node, const Vector3& axis) const
    {
        Vector3 p = node->GetWorldRotation() * axis;
        p.Normalize();
        Vector3 ret = processNode_->GetWorldRotation().Inverse() * p;
        ret.Normalize();
        ret.y_ = 0;
        return ret;
    }

    Quaternion GetRotationInXZPlane(const Quaternion& startLocalRot, const Quaternion& curLocalRot)
    {
        rotateNode_->SetRotation(startLocalRot);
        Vector3 startAxis = GetProjectedAxis(rotateNode_, pelvisRightAxis_);
        rotateNode_->SetRotation(curLocalRot);
        Vector3 curAxis = GetProjectedAxis(rotateNode_, pelvisRightAxis_);
        return Quaternion(startAxis, curAxis);
    }

    void RotateAnimation(const String& animName, float rotateAngle)
    {
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);
        if (!anim)
            return;

        AnimationTrack* translateTrack = anim->GetTrack(String(TRANSLATE_BONE_NAME));
        AnimationTrack* rotateTrack = anim->GetTrack(String(ROTATE_BONE_NAME));
        Quaternion q(0, rotateAngle, 0);

        if (rotateTrack)
        {
            for (unsigned i=0; i<rotateTrack->GetNumKeyFrames(); ++i)
            {
                AnimationKeyFrame kf = rotateTrack->keyFrames_[i];
                rotateNode_->SetRotation(kf.rotation_);
                Quaternion wq = rotateNode_->GetWorldRotation();
                wq = q * wq;
                rotateNode_->SetWorldRotation(wq);
                kf.rotation_ = rotateNode_->GetRotation();
                rotateTrack->keyFrames_[i] = kf;
            }
        }
        if (translateTrack)
        {
            for (unsigned i=0; i<translateTrack->GetNumKeyFrames(); ++i)
            {
                AnimationKeyFrame kf = translateTrack->keyFrames_[i];
                kf.position_ = q * kf.position_;
                translateTrack->keyFrames_[i] = kf;
            }
        }
    }

    void TranslateAnimation(const String& animName, const Vector3& diff)
    {
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);
        if (!anim)
            return;

        AnimationTrack* translateTrack = anim->GetTrack(String(TRANSLATE_BONE_NAME));
        if (translateTrack)
        {
            for (unsigned i=0; i<translateTrack->GetNumKeyFrames(); ++i)
            {
                AnimationKeyFrame kf = translateTrack->keyFrames_[i];
                kf.position_ += diff;
                translateTrack->keyFrames_[i] = kf;
            }
        }
    }

    void CollectBoneWorldPositions(const String& animName, const String& boneName, PODVector<Vector3>& outPositions)
    {
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);
        if (!anim)
            return;

        AnimationTrack* track = anim->GetTrack(boneName);
        if (!track)
            return;

        AnimatedModel* am = alignNode_->GetComponent<AnimatedModel>();
        am->RemoveAllAnimationStates();
        AnimationState* state = am->AddAnimationState(anim);
        state->SetWeight(1.0F);
        state->SetLooped(false);

        outPositions.Resize(track->GetNumKeyFrames());
        Node* boneNode = alignNode_->GetChild(boneName, true);

        for (unsigned i=0; i<track->GetNumKeyFrames(); ++i)
        {
            state->SetTime(track->keyFrames_[i].time_);
            state->Apply();
            alignNode_->MarkDirty();
            outPositions[i] = boneNode->GetWorldPosition();
        }
    }

    Vector3 GetBoneWorldPosition(const String& animName, const String& boneName, float t)
    {
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);
        if (!anim)
            return Vector3::ZERO;
        AnimatedModel* am = alignNode_->GetComponent<AnimatedModel>();
        am->RemoveAllAnimationStates();
        AnimationState* state = am->AddAnimationState(anim);
        state->SetWeight(1.0F);
        state->SetLooped(false);
        state->SetTime(t);
        state->Apply();
        alignNode_->MarkDirty();
        return alignNode_->GetChild(boneName, true)->GetWorldPosition();
    }

    void FixAnimationOrigin(const String& animName, unsigned motionFlag)
    {
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);
        if (!anim)
            return;

        AnimationTrack* translateTrack = anim->GetTrack(String(TRANSLATE_BONE_NAME));
        if (!translateTrack)
            return;

        unsigned translateFlag = 0;
        Vector3 position = translateTrack->keyFrames_[0].position_ - pelvisOrign_;
        const float minDist = 0.5F;
        if (Abs(position.x_) > minDist) {
            if (d_log)
                URHO3D_LOGINFO(animName + " Need reset x position");
            AddFlag(translateFlag, kMotion_X);
        }
        if (Abs(position.y_) > 2.0f && HasFlag(motionFlag, kMotion_Ext_Adjust_Y)) {
            if (d_log)
                URHO3D_LOGINFO(animName + " Need reset y position");
            AddFlag(translateFlag, kMotion_Y);
        }
        if (Abs(position.z_) > minDist) {
            if (d_log)
                URHO3D_LOGINFO(animName + " Need reset z position");
            AddFlag(translateFlag, kMotion_Z);
        }
        if (d_log)
            URHO3D_LOGINFO("t-diff-position=" + position.ToString());

        if (translateFlag == 0)
            return;

        Vector3 firstKeyPos = translateTrack->keyFrames_[0].position_;
        translateNode_->SetPosition(firstKeyPos);
        Vector3 currentWS = translateNode_->GetWorldPosition();
        Vector3 oldWS = currentWS;

        if (HasFlag(translateFlag, kMotion_X))
            currentWS.x_ = pelvisOrign_.x_;
        if (HasFlag(translateFlag, kMotion_Y))
            currentWS.y_ = pelvisOrign_.y_;
        if (HasFlag(translateFlag, kMotion_Z))
            currentWS.z_ = pelvisOrign_.z_;

        translateNode_->SetWorldPosition(currentWS);
        Vector3 currentLS = translateNode_->GetPosition();
        Vector3 originDiffLS = currentLS - firstKeyPos;
        TranslateAnimation(animName, originDiffLS);
    }

    float ProcessAnimation(const String& animName,
        int motionFlag,
        int allowMotion,
        float rotateAngle,
        PODVector<Vector4>& outKeys,
        Vector4& startFromOrigin)
    {
        if (d_log)
        {
            URHO3D_LOGINFO("---------------------------------------------------------------------------------------");
            URHO3D_LOGINFO("Processing animation " + animName);
        }

        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);
        if (!anim)
            return 0;

        AnimationTrack* translateTrack = anim->GetTrack(String(TRANSLATE_BONE_NAME));
        if (!translateTrack)
            return 0;

        AnimationTrack* rotateTrack = anim->GetTrack(String(ROTATE_BONE_NAME));
        Quaternion flipZ_Rot(0, 180, 0);
        bool cutRotation = HasFlag(motionFlag, kMotion_Ext_Rotate_From_Start);
        bool dump = HasFlag(motionFlag, kMotion_Ext_Debug_Dump);
        float firstRotateFromRoot = 0;
        bool flip = false;
        bool footBased = HasFlag(motionFlag, kMotion_Ext_Foot_Based_Height);
        int translateFlag = 0;

        // ==============================================================
        // pre process key frames
        if (rotateTrack && rotateAngle > 360)
        {
            firstRotateFromRoot = GetRotationInXZPlane(rotateBoneInitQ_, rotateTrack->keyFrames_[0].rotation_).EulerAngles().y_;
            if (Abs(firstRotateFromRoot) > 75)
            {
                if (d_log)
                    URHO3D_LOGINFO(animName + " Need to flip rotate track since object is start opposite, rotation=" + String(firstRotateFromRoot));
                flip = true;
            }
            startFromOrigin.w_ = firstRotateFromRoot;
        }

        // get start offset
        translateNode_->SetPosition(translateTrack->keyFrames_[0].position_);
        Vector3 t_ws1 = translateNode_->GetWorldPosition();
        translateNode_->SetPosition(pelvisOrign_);
        Vector3 t_ws2 = translateNode_->GetWorldPosition();
        Vector3 diff = t_ws1 - t_ws2;
        startFromOrigin.x_ = diff.x_;
        startFromOrigin.y_ = diff.y_;
        startFromOrigin.z_ = diff.z_;

        if (rotateAngle < 360)
            RotateAnimation(animName, rotateAngle);
        else if (flip)
            RotateAnimation(animName, 180);

        FixAnimationOrigin(animName, motionFlag);

        if (rotateTrack)
        {
            for (unsigned i=0; i<rotateTrack->GetNumKeyFrames(); ++i)
            {
                Quaternion q = GetRotationInXZPlane(rotateBoneInitQ_, rotateTrack->keyFrames_[i].rotation_);
                if (d_log)
                {
                    if (i == 0 || i == rotateTrack->GetNumKeyFrames() - 1)
                        URHO3D_LOGINFO("frame=" + String(i) + " rotation from identical in xz plane=" + q.EulerAngles().ToString());
                }
                if (i == 0)
                    firstRotateFromRoot = q.EulerAngles().y_;
            }
        }

        outKeys.Resize(translateTrack->GetNumKeyFrames());

        bool rotateMotion = HasFlag(motionFlag, kMotion_R);
        // process rotate key frames first
        if (rotateMotion && rotateTrack)
        {
            Quaternion lastRot = rotateTrack->keyFrames_[0].rotation_;
            float rotateFromStart = cutRotation ? firstRotateFromRoot : 0;
            for (unsigned i=0; i<rotateTrack->GetNumKeyFrames(); ++i)
            {
                AnimationKeyFrame kf = rotateTrack->keyFrames_[i];
                Quaternion q = GetRotationInXZPlane(lastRot, kf.rotation_);
                lastRot = kf.rotation_;
                outKeys[i].w_ = rotateFromStart;
                rotateFromStart += q.EulerAngles().y_;

                if (dump)
                    URHO3D_LOGINFO("rotation from last frame = " + String(q.EulerAngles().y_) + " rotateFromStart=" + String(rotateFromStart));

                q = Quaternion(0, rotateFromStart, 0).Inverse();

                Quaternion wq = rotateNode_->GetWorldRotation();
                wq = q * wq;
                rotateNode_->SetWorldRotation(wq);
                kf.rotation_ = rotateNode_->GetRotation();

                rotateTrack->keyFrames_[i] = kf;
            }
        }

        RemoveFlag(motionFlag, kMotion_R);
        if (motionFlag != 0)
        {
            Vector3 firstKeyPos = translateTrack->keyFrames_[0].position_;
            for (unsigned i=0; i<translateTrack->GetNumKeyFrames(); ++i)
            {
                AnimationKeyFrame kf = translateTrack->keyFrames_[i];
                translateNode_->SetPosition(firstKeyPos);
                Vector3 t1_ws = translateNode_->GetWorldPosition();
                translateNode_->SetPosition(kf.position_);
                Vector3 t2_ws = translateNode_->GetWorldPosition();
                Vector3 translation = t2_ws - t1_ws;
                if (HasFlag(motionFlag, kMotion_X))
                {
                    outKeys[i].x_ = translation.x_;
                    t2_ws.x_  = t1_ws.x_;
                }
                if (HasFlag(motionFlag, kMotion_Y) != 0 && !footBased)
                {
                    outKeys[i].y_ = translation.y_;
                    t2_ws.y_ = t1_ws.y_;
                }
                if (HasFlag(motionFlag, kMotion_Z))
                {
                    outKeys[i].z_ = translation.z_;
                    t2_ws.z_ = t1_ws.z_;
                }

                translateNode_->SetWorldPosition(t2_ws);
                Vector3 local_pos = translateNode_->GetPosition();
                kf.position_ = local_pos;
                translateTrack->keyFrames_[i] = kf;
            }
        }

        if (footBased)
        {
            PODVector<Vector3> leftFootPositions;
            PODVector<Vector3> rightFootPositions;
            CollectBoneWorldPositions(animName, L_FOOT, leftFootPositions);
            CollectBoneWorldPositions(animName, R_FOOT, rightFootPositions);
            PODVector<float> ground_heights;
            ground_heights.Resize(leftFootPositions.Size());
            for (unsigned i=0; i<translateTrack->GetNumKeyFrames(); ++i)
            {
                AnimationKeyFrame kf = translateTrack->keyFrames_[i];
                float ground_y = 0;
                if (rightFootPositions[i].y_ < leftFootPositions[i].y_)
                    ground_y = rightFootPositions[i].y_ - right_foot_to_ground_height_;
                else
                    ground_y = leftFootPositions[i].y_ - left_foot_to_ground_height_;
                kf.position_.y_ -= ground_y;
                translateTrack->keyFrames_[i] = kf;
                ground_heights[i] = ground_y;
            }

            if (HasFlag(motionFlag, kMotion_Y))
            {
                for (unsigned i=0; i<ground_heights.Size(); ++i)
                    outKeys[i].y_ = ground_heights[i] - ground_heights[0];
            }
        }

        for (unsigned i=0; i<outKeys.Size(); ++i)
        {
            if (HasFlag(allowMotion, kMotion_X))
                outKeys[i].x_ = 0;
            if (HasFlag(allowMotion, kMotion_Y))
                outKeys[i].y_ = 0;
            if (HasFlag(allowMotion, kMotion_Z))
                outKeys[i].z_ = 0;
            if (HasFlag(allowMotion, kMotion_R))
                outKeys[i].w_ = 0;
        }

        if (dump)
        {
            for (unsigned i=0; i<outKeys.Size(); ++i)
            {
                URHO3D_LOGINFO("Frame " + String(i) + " motion-key=" + outKeys[i].ToString());
            }
        }

        if (d_log)
            URHO3D_LOGINFO("---------------------------------------------------------------------------------------");

        if (rotateAngle < 360)
            return rotateAngle;
        else
            return flip ? 180 : 0;
    }

    Scene* processScene_;
    String  rigName_;

    Node* processNode_;
    Node* translateNode_;
    Node* rotateNode_;

    Vector3 pelvisRightAxis_;
    Quaternion rotateBoneInitQ_;
    Vector3 pelvisOrign_;

    float left_foot_to_ground_height_;
    float right_foot_to_ground_height_;

    Node* alignNode_;
};



}