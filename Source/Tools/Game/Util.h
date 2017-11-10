#pragma once
#include "Constants.h"
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Graphics/DebugRenderer.h>

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

}
