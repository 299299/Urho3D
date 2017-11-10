#pragma once
#include <Urho3D/Resource/Animation.h>
#include <Urho3D/IO/Log.h>

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