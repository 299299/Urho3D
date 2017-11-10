#pragma once
#include "GameObject.h"
#include "FSM.h"
#include "Util.h"
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

    FSMPtr                  fsm_;
    Node*                   renderNode_;
    AnimatedModel*          model_;
    AnimationController*    ctrl_;

    Vector3                 initialPostion_;
    Quaternion              initialRotation_;

    String                  lastAnimation_;
};

}