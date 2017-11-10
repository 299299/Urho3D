#pragma once
#include <Urho3D/Resource/Animation.h>

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


class Motion : public Object
{
URHO3D_OBJECT(Motion, Object);

public:
    Motion(Context* c)
    :Object(c)
    {

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


}