#pragma once
#include "Constants.h"
#include <Urho3D/Math/MathDefs.h>

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

}
