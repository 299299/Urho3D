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
        processNode = processScene_->CreateChild(rig + "_Character");
        processNode-SetWorldRotation(Quaternion(0, 180, 0));

        AnimatedModel* am = processNode->CreateComponent<AnimatedModel>();
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        am->SetModel(cache->GetResource<Model>(rigName));

        Bone* bone = am->GetSkeleton().GetBone(ROTATE_BONE_NAME);
        rotateBoneInitQ_ = bone.initialRotation_;

        pelvisRightAxis_ = rotateBoneInitQ_ * Vector3(1, 0, 0);
        pelvisRightAxis_.Normalize();

        translateNode_ = processNode->GetChild(TRANSLATE_BONE_NAME, true);
        rotateNode_ = processNode->GetChild(ROTATE_BONE_NAME, true);
        pelvisOrign_ = am->GetSkeleton().GetBone(TRANSLATE_BONE_NAME).initialPosition_;

        left_foot_to_ground_height = processNode->GetChild(L_FOOT, true).GetWorldPosition().y;
        right_foot_to_ground_height = processNode->GetChild(R_FOOT, true).GetWorldPosition().y;

        alignNode = processScene_->CreateChild(rig + "_Align");
        alignNode->SetWorldRotation(Quaternion(0, 180, 0));
        AnimatedModel* am2 = alignNode->CreateComponent<AnimatedModel>();
        am2->SetModel(am->GetModel());

        URHO3D_LOGINFO(rigName + " pelvisRightAxis=" + pelvisRightAxis_.ToString() + " pelvisOrign=" + pelvisOrign_.ToString());
    }

    ~MotionRig()
    {
        processNode->Remove();
        alignNode->Remove();
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
        Vector3 startAxis = GetProjectedAxis(rotateNode, pelvisRightAxis_);
        rotateNode_->SetRotation(curLocalRot);
        Vector3 curAxis = GetProjectedAxis(rig, rotateNode, pelvisRightAxis_);
        return Quaternion(startAxis, curAxis);
    }

    void RotateAnimation(const String& animName, float rotateAngle)
    {
        ResourceCache* cache = processScene_->GetSubsystem<ResourceCache>();
        Animation* anim = cache->GetResource<Animation>(animName);

        AnimationTrack@ translateTrack = anim.tracks[TranslateBoneName];
        AnimationTrack@ rotateTrack = anim.tracks[RotateBoneName];
        Quaternion q(0, rotateAngle, 0);
        Node@ rotateNode = curRig.rotateNode;

        if (rotateTrack !is null)
        {
            for (uint i=0; i<rotateTrack.numKeyFrames; ++i)
            {
                AnimationKeyFrame kf(rotateTrack.keyFrames[i]);
                rotateNode.rotation = kf.rotation;
                Quaternion wq = rotateNode.worldRotation;
                wq = q * wq;
                rotateNode.worldRotation = wq;
                kf.rotation = rotateNode.rotation;
                rotateTrack.keyFrames[i] = kf;
            }
        }
        if (translateTrack !is null)
        {
            for (uint i=0; i<translateTrack.numKeyFrames; ++i)
            {
                AnimationKeyFrame kf(translateTrack.keyFrames[i]);
                kf.position = q * kf.position;
                translateTrack.keyFrames[i] = kf;
            }
        }
    }

    Scene* processScene_;
    String  rigName_;

    Node* processNode_;
    Node* translateNode_;
    Node* rotateNode_;

    Vector3 pelvisRightAxis_;
    Quaternion rotateBoneInitQ_;
    Vector3 pelvisOrign_;

    float left_foot_to_ground_height;
    float right_foot_to_ground_height;

    Node* alignNode;
};



void TranslateAnimation(Animation* anim, const Vector3& diff)
{
    AnimationTrack* translateTrack = anim.tracks[TranslateBoneName];
    if (translateTrack !is null)
    {
        for (uint i=0; i<translateTrack.numKeyFrames; ++i)
        {
            AnimationKeyFrame kf(translateTrack.keyFrames[i]);
            kf.position += diff;
            translateTrack.keyFrames[i] = kf;
        }
    }
}


}