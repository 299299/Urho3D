#include "Precompiled.h"
#include "Engine/Application.h"
#ifdef IOS
#include "Graphics/Graphics.h"
#include <SDL/SDL.h>
#endif
#include "DebugNew.h"
#include "Engine/EngineDefs.h"
#include "UI/Sprite.h"
#include "UI/Text.h"
#include "UI/UI.h"
#include "Graphics/Texture2D.h"
#include "Scene/Scene.h"
#include "Graphics/Renderer.h"
#include "Resource/ResourceCache.h"
#include "IO/FileSystem.h"
#include "Input/Input.h"
#include "UI/Font.h"
#include "Graphics/StaticModel.h"
#include "Graphics/Camera.h"
#include "Graphics/Model.h"
#include "Graphics/Octree.h"
#include "Graphics/Animation.h"
#include "Graphics/AnimatedModel.h"
#include "Graphics/AnimationController.h"
#include "Container/Vector.h"
#include "Container/Str.h"
#include "IO/Log.h"
#include "Scene/SceneEvents.h"
#include "Core/CoreEvents.h"

namespace Urho3D
{

void FillAnimationWithCurrentPose(Animation* anim, Node* node, const Vector<String>& boneNames)
{
    anim->RemoveAllTracks();
    for (uint i=0; i<boneNames.Size(); ++i)
    {
        Node* n = node->GetChild(boneNames[i], true);
        if (!n)
        {
            URHO3D_LOGDEBUG("FillAnimationWithCurrentPose can not find bone " + boneNames[i]);
            continue;
        }
        AnimationTrack* track = anim->CreateTrack(boneNames[i]);
        track->channelMask_ = CHANNEL_POSITION | CHANNEL_ROTATION;
        AnimationKeyFrame kf;
        kf.time_ = 0.0f;
        kf.position_ = n->GetPosition();
        kf.rotation_ = n->GetRotation();
        track->AddKeyFrame(kf);
    }
}

SharedPtr<Animation> CreatePoseAnimation(const String& modelName, const Vector<String>& boneNames, Scene* scene)
{
    ResourceCache* cache = scene->GetSubsystem<ResourceCache>();
    Model* model = cache->GetResource<Model>(modelName);
    if (!model)
        return SharedPtr<Animation>();

    Node* n = scene->CreateChild("Temp_Node");
    AnimatedModel* am = n->CreateComponent<AnimatedModel>();
    am->SetModel(model);

    SharedPtr<Animation> anim(new Animation(scene->GetContext()));
    anim->SetName(modelName + "_ani");
    FillAnimationWithCurrentPose(anim, n, boneNames);
    cache->AddManualResource(anim);
    n->Remove();

    return anim;
}

Vector<String> GetChildNodeNames(Node* node)
{
    Vector<String> nodeNames;
    nodeNames.Push(node->GetName());

    PODVector<Node*> children = node->GetChildren(true);
    for (uint i=0; i<children.Size(); ++i)
    {
        nodeNames.Push(children[i]->GetName());
    }

    return nodeNames;
}

enum FacialBoneType
{
    kFacial_ForeHead,
    kFacial_Nose,
    kFacial_Nose_Left,
    kFacial_Node_Right,
    kFacial_Jaw,
    kFacial_Mouth_Bottom,
    kFacial_Mouth_Up,
    kFacial_Mouth_Left,
    kFacial_Mouth_Right,
    kFacial_EyeBall_Left,
    kFacial_EyeBall_Right,
    kFacial_EyeTop_Left,
    kFacial_EyeTop_Right,
    kFacial_EyeBottom_Left,
    kFacial_EyeBottom_Right,
    kFacial_EyeLeft,
    kFacial_EyeRight,
    kFacial_Bone_Num
};

enum FacialAttributeType
{
    kFacial_MouseOpenness,
    kFacial_EyeOpenness_Left,
    kFacial_EyeOpenness_Right,
    kFacial_EyePositionLeft_Left,
    kFacial_EyePositionRight_Left,
    kFacial_EyePositionLeft_Right,
    kFacial_EyePositionRight_Right,
    kFacial_Attribute_Num
};


struct FacialBone
{
    FacialBone() {};

    FacialBone(int b_type, int b_index, const char* name)
    :facial_bone_type(b_type)
    ,facial_index(b_index)
    ,bone_node(NULL)
    ,bone_name(name)
    {
    }

    void LoadNode(Node* node)
    {
        if (!bone_name)
        {
            bone_node = node->GetChild(bone_name, true);
        }
    }

    void DebugDraw(DebugRenderer* debug)
    {
        if (bone_node)
        {
            // debug.AddCross(bone_node.worldPosition, 0.01, GREEN, false);
        }
    }

    int facial_bone_type;
    int facial_index;
    Node* bone_node;
    const char* bone_name;
};

struct FacialAttribute
{
    String animation;
    float value;
};


struct FacialBoneManager
{
    FacialBone facial_bones[kFacial_Bone_Num];
    FacialAttribute facial_attributes[kFacial_Attribute_Num];
    Node* face_node;

    FacialBoneManager()
    :face_node(NULL)
    {
        facial_bones[kFacial_Jaw] = (FacialBone(kFacial_Jaw, 16, "FcFX_Jaw"));
        facial_bones[kFacial_Nose] = (FacialBone(kFacial_Nose, 46, NULL));
        facial_bones[kFacial_Nose_Left] = (FacialBone(kFacial_Nose_Left, 83, "FcFX_Nose_L"));
        facial_bones[kFacial_Node_Right] = (FacialBone(kFacial_Node_Right, 82, "FcFX_Nose_R"));

        facial_bones[kFacial_Mouth_Bottom] = (FacialBone(kFacial_Mouth_Bottom, 102, "FcFX_Mouth_07"));
        facial_bones[kFacial_Mouth_Up] = (FacialBone(kFacial_Mouth_Up, 98, "FcFX_Mouth_03"));
        facial_bones[kFacial_Mouth_Left] = (FacialBone(kFacial_Mouth_Left, 90, "FcFX_Mouth_05"));
        facial_bones[kFacial_Mouth_Right] = (FacialBone(kFacial_Mouth_Right, 84, "FcFX_Mouth_01"));

        facial_bones[kFacial_EyeBall_Left] = (FacialBone(kFacial_EyeBall_Left, 105, "FcFX_Eye_L"));
        facial_bones[kFacial_EyeBall_Right] = (FacialBone(kFacial_EyeBall_Right, 104, "FcFX_Eye_R"));
        facial_bones[kFacial_EyeTop_Left] = (FacialBone(kFacial_EyeTop_Left, 75, "FcFX_EyLd_Top_L"));
        facial_bones[kFacial_EyeTop_Right] = (FacialBone(kFacial_EyeTop_Right, 72, "FcFX_EyLd_Top_R"));
        facial_bones[kFacial_EyeBottom_Left] = (FacialBone(kFacial_EyeBottom_Left, 76, "FcFX_EyLd_Bottom_L"));
        facial_bones[kFacial_EyeBottom_Right] = (FacialBone(kFacial_EyeBottom_Right, 73, "FcFX_EyLd_Bottom_R"));
        facial_bones[kFacial_EyeLeft] = (FacialBone(kFacial_EyeLeft, 61, NULL));
        facial_bones[kFacial_EyeRight] = (FacialBone(kFacial_EyeRight, 52, NULL));

        facial_bones[kFacial_ForeHead] = (FacialBone(kFacial_ForeHead, 43, NULL));
    }

    void Init(Scene* scene)
    {
        face_node = scene->GetChild("Head", true);
        for (uint i=0; i<kFacial_Bone_Num; ++i)
        {
            facial_bones[i].LoadNode(face_node);
        }

        Vector<String> jawBones = GetChildNodeNames(face_node->GetChild("FcFX_Jaw", true));
        Vector<String> mouseBones;
        for (uint i=0; i<jawBones.Size(); ++i)
            mouseBones.Push(jawBones[i]);
        Vector<String> boneNames = GetChildNodeNames(face_node);
        Vector<String> leftEyeBones;
        Vector<String> rightEyeBones;
        for (uint i=0; i<boneNames.Size(); ++i)
        {
            if (boneNames[i].EndsWith("_L") && boneNames[i] != "FcFx_Eye_L")
            {
                leftEyeBones.Push(boneNames[i]);
            }
            if (boneNames[i].EndsWith("_R") && boneNames[i] != "FcFx_Eye_R")
            {
                rightEyeBones.Push(boneNames[i]);
            }
        }
        Vector<String> leftEyeBalls; leftEyeBalls.Push("FcFx_Eye_L");
        Vector<String> rightEyeBalls; rightEyeBalls.Push("FcFx_Eye_R");

        facial_attributes[kFacial_MouseOpenness].animation = (CreatePoseAnimation("Models/head_mouse_open.mdl", mouseBones, scene)->GetName());
        facial_attributes[kFacial_EyeOpenness_Left].animation = (CreatePoseAnimation("Models/head_eye_close_L.mdl", leftEyeBones, scene)->GetName());
        facial_attributes[kFacial_EyeOpenness_Right].animation = (CreatePoseAnimation("Models/head_eye_close_R.mdl", rightEyeBones, scene)->GetName());
        facial_attributes[kFacial_EyePositionLeft_Left].animation = (CreatePoseAnimation("Models/head_eyeball_L_L.mdl", leftEyeBalls, scene)->GetName());
        facial_attributes[kFacial_EyePositionRight_Left].animation = (CreatePoseAnimation("Models/head_eyeball_L_R.mdl", leftEyeBalls, scene)->GetName());
        facial_attributes[kFacial_EyePositionLeft_Right].animation = (CreatePoseAnimation("Models/head_eyeball_R_L.mdl", rightEyeBalls, scene)->GetName());
        facial_attributes[kFacial_EyePositionRight_Right].animation = (CreatePoseAnimation("Models/head_eyeball_R_R.mdl", rightEyeBalls, scene)->GetName());
        
        facial_attributes[kFacial_MouseOpenness].value = 0;
        facial_attributes[kFacial_EyeOpenness_Left].value = 0;
        facial_attributes[kFacial_EyeOpenness_Right].value = 0;
        facial_attributes[kFacial_EyePositionLeft_Left].value = 0.5F;
        facial_attributes[kFacial_EyePositionRight_Left].value = 0.5F;
        facial_attributes[kFacial_EyePositionLeft_Right].value = 0.5F;
        facial_attributes[kFacial_EyePositionRight_Right].value = 0.5F;

    }

    void DebugDraw(DebugRenderer* debug)
    {
        for (uint i=0; i<kFacial_Bone_Num; ++i)
        {
            facial_bones[i].DebugDraw(debug);
        }
    }

    void Update(float dt)
    {
        AnimationController* ac = face_node->GetComponent<AnimationController>();

        facial_attributes[kFacial_EyePositionRight_Left].value = 1.0F - facial_attributes[kFacial_EyePositionLeft_Left].value;
        facial_attributes[kFacial_EyePositionRight_Right].value = 1.0F - facial_attributes[kFacial_EyePositionLeft_Right].value;

        for (uint i=0; i<kFacial_Attribute_Num; ++i)
        {
            ac->Play(facial_attributes[i].animation, 0, false, 0);
            ac->SetWeight(facial_attributes[i].animation, facial_attributes[i].value);
        }
    }
};

class FacePlayer : public Application
{
    URHO3D_OBJECT(FacePlayer, Application);
public:
    FacePlayer(Context* context):Application(context)
    {
    }

    virtual void Setup()
    {
        // Modify engine startup parameters
        engineParameters_[EP_WINDOW_TITLE] = GetTypeName();
        engineParameters_[EP_LOG_NAME]     = GetSubsystem<FileSystem>()->GetAppPreferencesDir("urho3d", "logs") + GetTypeName() + ".log";
        engineParameters_[EP_FULL_SCREEN]  = false;
        engineParameters_[EP_HEADLESS]     = false;
        engineParameters_[EP_SOUND]        = false;
        if (!engineParameters_.Contains(EP_RESOURCE_PREFIX_PATHS))
            engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../share/Resources;../share/Urho3D/Resources";
    }

    virtual void Start()
    {
        Graphics* g = GetSubsystem<Graphics>();
        printf("graphics-width=%d, height=%d]n", g->GetWidth(), g->GetHeight());

        CreateScene();
        SetupViewport();
    }

    virtual void Stop()
    {

    }

private:
    void CreateScene()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        scene_ = new Scene(context_);

        SharedPtr<File> file = cache->GetFile("Scenes/Head.xml");
        scene_->LoadXML(*file);

        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->CreateComponent<Camera>();

        cameraNode_->SetPosition(Vector3(0.0f, 0.5f, -0.5f));
        
        mgr_.Init(scene_);
    }

    void SetupViewport()
    {
        Renderer* renderer = GetSubsystem<Renderer>();

        // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
        // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
        // use, but now we just use full screen and default render path configured in the engine command line options
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);
    }
    
    void SubscribeToEvents()
    {
        // Subscribe HandleUpdate() function for processing update events
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FacePlayer, HandleUpdate));
    }
    
    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();
    }

private:
    SharedPtr<Sprite> logoSprite_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    FacialBoneManager mgr_;
};
}

#if __cplusplus
extern "C" {
#endif

int SDL_main(int argc, char** argv)
{
    SDL_SetMainReady();
    Urho3D::Context* context = new Urho3D::Context();
    Urho3D::FacePlayer* application = new Urho3D::FacePlayer(context);
    return application->Run();
}

void Urho3D_Init()
{
    printf("Urho3D_Init\n");
    SDL_main(0, 0);
}

#if __cplusplus
}   // Extern C
#endif
