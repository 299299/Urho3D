#pragma once
#include "Constants.h"
#include <Urho3D/Core/Timer.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/UI/BorderImage.h>

namespace Urho3D
{

Vector3 FilterPosition(const Vector3& position)
{
    float x = position.x_;
    float z = position.z_;
    float radius = COLLISION_RADIUS + 1.0f;
    x = Clamp(x, radius - WORLD_HALF_SIZE.x_, WORLD_HALF_SIZE.x_ - radius);
    z = Clamp(z, radius - WORLD_HALF_SIZE.z_, WORLD_HALF_SIZE.z_ - radius);
    return Vector3(x, position.y_, z);
}

String GetAnimationName(const String& name)
{
    return "Animations/" + name + "_Take 001.ani";
}

String FileNameToMotionName(const String& name)
{
    return name.Substring(0, name.Length() - 13);
}

// clamps an angle to the rangle of [-PI, PI]
float ClampAngle( float angle )
{
    if (angle > 180.0F)
        angle -= 360.0F;
    if (angle < -180.0F)
        angle += 360.0F;
    return angle;
}

void AddFlag(unsigned flags, unsigned flag)
{
    flags |= flag;
}

void RemoveFlag(unsigned flags, unsigned flag)
{
    flags &= ~flag;
}

bool HasFlag(unsigned flags, unsigned flag)
{
    return (flags & flag) != 0;
}

void DebugDrawDirection(DebugRenderer* debug, const Vector3& start, float angle, const Color& color, float radius = 1.0)
{
    Vector3 end = start + Vector3(Sin(angle) * radius, 0, Cos(angle) * radius);
    debug->AddLine(start, end, color, false);
}

class TimeLogger
{
public:
    const char* message_;
    unsigned time_;

    TimeLogger(const char* message)
    :message_(message)
    {
        time_ = Time::GetSystemTime();
    }

    ~TimeLogger()
    {
        unsigned dt = Time::GetSystemTime() - t;
        URHO3D_LOGINFO(String(message) + String(" time-cost: ") + String(dt) + String(" ms."));
    }
};

class Fader : public Object
{
    URHO3D_OBJECT(Fader, Object);
public:
    SharedPtr<BorderImage> image_;
    float  fadeTime_;

    Fader(Context* c)
    :Object(c)
    ,fadeTime_(0)
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Graphics* g = GetSubsystem<Graphics>();
        UI* ui = GetSubsystem<UI>();
        image_ = ui->GetRoot()->CreateChild<BorderImage>();
        image_->SetVisible(false);
        image_->SetPriority(-999);
        image_->SetOpacity(1.0F);
        image_->SetTexture(cache->GetResource<Texture2D>("Textures/fade.png"));
        image_->SetFullImageRect();
        image_->SetFixedSize(g->GetWidth(), g->GetHeight());
    }

    int Update(float dt)
    {
        Graphics* g = GetSubsystem<Graphics>();
        image_->SetFixedSize(g->GetWidth(), g->GetHeight());
        float t = image_->GetAttributeAnimationTime("Opacity");
        if (t + 0.05F >= fadeTime)
        {
            image_->SetVisible(false);
            return 1;
        }
        return 0;
    }

    void FadeIn(float duration)
    {
        fadeTime_ = duration;
        SharedPtr<ValueAnimation> alphaAnimation(new ValueAnimation(context_));
        alphaAnimation->SetKeyFrame(0.0f, Variant(1.0f));
        alphaAnimation->SetKeyFrame(duration, Variant(0.0f));
        image_->SetVisible(true);
        image_->SetAttributeAnimation("Opacity", alphaAnimation, WM_ONCE);
    }
};

}
