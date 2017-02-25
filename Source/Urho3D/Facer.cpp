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

const char* rotate_bone_name = "Bip01_Spine3_$AssimpFbx$_Translation";

#if __cplusplus
extern "C" {
#endif
typedef struct st_rect_t {
    int left;   ///< 矩形最左边的坐标
    int top;    ///< 矩形最上边的坐标
    int right;  ///< 矩形最右边的坐标
    int bottom; ///< 矩形最下边的坐标
} vs_rect_t;
/// st float type point definition
typedef struct st_pointf_t {
    float x;    ///< 点的水平方向坐标，为浮点数
    float y;    ///< 点的竖直方向坐标，为浮点数
} vs_pointf_t;
typedef struct vs_models_106_t {
    vs_rect_t rect;         ///< 代表面部的矩形区域
    float score;            ///< 置信度
    vs_pointf_t points_array[106];  ///< 人脸106关键点的数组
    float yaw;              ///< 水平转角，真实度量的左负右正
    float pitch;            ///< 俯仰角，真实度量的上负下正
    float roll;             ///< 旋转角，真实度量的左负右正
    float eye_dist;         ///< 两眼间距
    int ID;                 ///< faceID
} vs_models_106_t;
typedef struct vs_models_face_action_t {
    struct vs_models_106_t face;    /// 人脸信息，包含矩形、106点、pose信息等
    unsigned int face_action;       /// 脸部动作
} vs_models_face_action_t;
#define VS_FACE_DETECT      0x00000001    ///<  人脸检测
#define VS_EYE_BLINK        0x00000002    ///<  眨眼
#define VS_MOUTH_AH         0x00000004    ///<  嘴巴大张
#define VS_HEAD_YAW         0x00000008    ///<  摇头
#define VS_HEAD_PITCH       0x00000010    ///<  点头
#define VS_BROW_JUMP        0x00000020    ///<  眉毛挑动
typedef struct vs_models_human_action_t {
    vs_models_face_action_t faces[10];   /// 检测到的人脸及动作数组
    int face_count;                                                         /// 检测到的人脸数目
} vs_models_human_action_t;

vs_models_human_action_t* GetFacerAction();
    
#if __cplusplus
}   // Extern C
#endif

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
    kFacial_Mouth_Top,
    kFacial_Mouth_Left,
    kFacial_Mouth_Right,
    kFacial_EyeBall_Left,
    kFacial_EyeBall_Right,
    kFacial_EyeTop_Left,
    kFacial_EyeTop_Right,
    kFacial_EyeBottom_Left,
    kFacial_EyeBottom_Right,
    kFacial_EyeLeft_Left,
    kFacial_EyeRight_Left,
    kFacial_EyeLeft_Right,
    kFacial_EyeRight_Right,
    kFacial_Bone_Num
};

enum FacialAttributeType
{
    kFacial_MouseOpenness,
    kFacial_EyeCloseness_Left,
    kFacial_EyeCloseness_Right,
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
    
    Vector2 GetPositionOnFace(const vs_models_face_action_t& face) const
    {
        vs_pointf_t pt = face.face.points_array[facial_index];
        return Vector2(pt.x, pt.y);
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
    Node* rotate_bone_node;

    FacialBoneManager()
    :face_node(NULL)
    ,rotate_bone_node(NULL)
    {
        facial_bones[kFacial_Jaw] = (FacialBone(kFacial_Jaw, 16, "FcFX_Jaw"));
        facial_bones[kFacial_Nose] = (FacialBone(kFacial_Nose, 46, NULL));
        facial_bones[kFacial_Nose_Left] = (FacialBone(kFacial_Nose_Left, 83, "FcFX_Nose_L"));
        facial_bones[kFacial_Node_Right] = (FacialBone(kFacial_Node_Right, 82, "FcFX_Nose_R"));

        facial_bones[kFacial_Mouth_Bottom] = (FacialBone(kFacial_Mouth_Bottom, 102, "FcFX_Mouth_07"));
        facial_bones[kFacial_Mouth_Top] = (FacialBone(kFacial_Mouth_Top, 98, "FcFX_Mouth_03"));
        facial_bones[kFacial_Mouth_Left] = (FacialBone(kFacial_Mouth_Left, 90, "FcFX_Mouth_05"));
        facial_bones[kFacial_Mouth_Right] = (FacialBone(kFacial_Mouth_Right, 84, "FcFX_Mouth_01"));

        facial_bones[kFacial_EyeBall_Left] = (FacialBone(kFacial_EyeBall_Left, 105, "FcFX_Eye_L"));
        facial_bones[kFacial_EyeBall_Right] = (FacialBone(kFacial_EyeBall_Right, 104, "FcFX_Eye_R"));
        facial_bones[kFacial_EyeTop_Left] = (FacialBone(kFacial_EyeTop_Left, 75, "FcFX_EyLd_Top_L"));
        facial_bones[kFacial_EyeTop_Right] = (FacialBone(kFacial_EyeTop_Right, 72, "FcFX_EyLd_Top_R"));
        facial_bones[kFacial_EyeBottom_Left] = (FacialBone(kFacial_EyeBottom_Left, 76, "FcFX_EyLd_Bottom_L"));
        facial_bones[kFacial_EyeBottom_Right] = (FacialBone(kFacial_EyeBottom_Right, 73, "FcFX_EyLd_Bottom_R"));
        
        facial_bones[kFacial_EyeLeft_Left] = (FacialBone(kFacial_EyeLeft_Left, 61, NULL));
        facial_bones[kFacial_EyeRight_Left] = (FacialBone(kFacial_EyeRight_Left, 58, NULL));
        
        facial_bones[kFacial_EyeLeft_Right] = (FacialBone(kFacial_EyeLeft_Right, 55, NULL));
        facial_bones[kFacial_EyeRight_Right] = (FacialBone(kFacial_EyeRight_Right, 52, NULL));

        facial_bones[kFacial_ForeHead] = (FacialBone(kFacial_ForeHead, 43, NULL));
    }

    void Init(Scene* scene)
    {
        face_node = scene->GetChild("Head", true);
        
        AnimatedModel* am = face_node->GetComponent<AnimatedModel>();
        am->SetCastShadows(false);
        
        Bone* b = am->GetSkeleton().GetBone(rotate_bone_name);
        b->animated_ = false;
        rotate_bone_node = face_node->GetChild(rotate_bone_name, true);
        
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
        facial_attributes[kFacial_EyeCloseness_Left].animation = (CreatePoseAnimation("Models/head_eye_close_L.mdl", leftEyeBones, scene)->GetName());
        facial_attributes[kFacial_EyeCloseness_Right].animation = (CreatePoseAnimation("Models/head_eye_close_R.mdl", rightEyeBones, scene)->GetName());
        facial_attributes[kFacial_EyePositionLeft_Left].animation = (CreatePoseAnimation("Models/head_eyeball_L_L.mdl", leftEyeBalls, scene)->GetName());
        facial_attributes[kFacial_EyePositionRight_Left].animation = (CreatePoseAnimation("Models/head_eyeball_L_R.mdl", leftEyeBalls, scene)->GetName());
        facial_attributes[kFacial_EyePositionLeft_Right].animation = (CreatePoseAnimation("Models/head_eyeball_R_L.mdl", rightEyeBalls, scene)->GetName());
        facial_attributes[kFacial_EyePositionRight_Right].animation = (CreatePoseAnimation("Models/head_eyeball_R_R.mdl", rightEyeBalls, scene)->GetName());
        
        facial_attributes[kFacial_MouseOpenness].value = 0;
        facial_attributes[kFacial_EyeCloseness_Left].value = 0;
        facial_attributes[kFacial_EyeCloseness_Right].value = 0;
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

    void Update(float dt, Text* text)
    {
        vs_models_human_action_t* t = GetFacerAction();
        if (t && t->face_count > 0)
        {
            const vs_models_face_action_t& f = t->faces[0];
            float yaw = f.face.yaw;
            float pitch = f.face.pitch;
            float roll = f.face.roll;
            String s = "yaw=" + String(yaw) + "\npitch=" + String(pitch) + "\nroll=" + String(roll);
            
            const float h = f.face.points_array[16].y - f.face.points_array[0].y;
            
            Vector2 mouse_top = facial_bones[kFacial_Mouth_Top].GetPositionOnFace(f);
            Vector2 mouse_bottom = facial_bones[kFacial_Mouth_Bottom].GetPositionOnFace(f);
            float mouth_h = mouse_bottom.y_ - mouse_top.y_;
            s += String("\nm_h=" + String(mouth_h));
            const float mouth_h_max = 0.1F;
            
            Vector2 eye_top = facial_bones[kFacial_EyeTop_Left].GetPositionOnFace(f);
            Vector2 eye_bottom = facial_bones[kFacial_EyeBottom_Left].GetPositionOnFace(f);
            float l_eye_h = (eye_bottom.y_ - eye_top.y_)/h;
            s += String("\nle_h=" + String(l_eye_h));
            
            eye_top = facial_bones[kFacial_EyeTop_Right].GetPositionOnFace(f);
            eye_bottom = facial_bones[kFacial_EyeBottom_Right].GetPositionOnFace(f);
            float r_eye_h = (eye_bottom.y_ - eye_top.y_)/h;
            s += String("\nre_h=" + String(r_eye_h));
            
            float eye1 = facial_bones[kFacial_EyeBall_Left].GetPositionOnFace(f).x_;
            float eye2 = facial_bones[kFacial_EyeLeft_Left].GetPositionOnFace(f).x_;
            float eye3 = facial_bones[kFacial_EyeRight_Left].GetPositionOnFace(f).x_;
            float b1 = fabs(eye1 - eye3);
            float b2 = fabs(eye1 - eye2);
            
            eye1 = facial_bones[kFacial_EyeBall_Right].GetPositionOnFace(f).x_;
            eye2 = facial_bones[kFacial_EyeLeft_Right].GetPositionOnFace(f).x_;
            eye3 = facial_bones[kFacial_EyeRight_Right].GetPositionOnFace(f).x_;
            float b3 = fabs(eye1 - eye3);
            float b4 = fabs(eye1 - eye2);
            
            s += String("\nb1=" + String(b1) + " b2=" + String(b2));
            s += String("\nb3=" + String(b3)+  " b4=" + String(b4));
            
            const float eye_h_min = 0.05F;
            const float eye_h_max = 0.07F;
            const float eye_h_range = eye_h_max - eye_h_min;
            float l_eye_h_to_min = fmax(0.0F, l_eye_h - eye_h_min);
            float r_eye_h_to_min = fmax(0.0F, r_eye_h - eye_h_min);
        
            facial_attributes[kFacial_EyeCloseness_Left].value = 1.0F - (l_eye_h_to_min / eye_h_range);
            facial_attributes[kFacial_EyeCloseness_Left].value = 1.0F - (r_eye_h_to_min / eye_h_range);
            
            facial_attributes[kFacial_MouseOpenness].value = fmin(1.0F, mouth_h / mouth_h_max);
            
            Quaternion q(-pitch, yaw, roll);
            rotate_bone_node->SetRotation(q);
            
            text->SetText(s);
        }
        else
        {
            text->SetText("No Face detected");
        }

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
    FacePlayer(Context* context):Application(context){};

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
        CreateUI();
        SetupViewport();
        SubscribeToEvents();
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
        cameraNode_->SetPosition(Vector3(0.0f, 0.55f, -0.5f));
        mgr_.Init(scene_);
        cameraNode_->LookAt(mgr_.face_node->GetChild("Bip01_Head", true)->GetWorldPosition());
    }
    
    void CreateUI()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        debugText_ = new Text(context_);
        debugText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 10);
        debugText_->SetColor(Color(0.0f, 1.0f, 0.0f));
        debugText_->SetHorizontalAlignment(HA_LEFT);
        debugText_->SetVerticalAlignment(VA_TOP);
        debugText_->SetText("Test");
        GetSubsystem<UI>()->GetRoot()->AddChild(debugText_);
    }

    void SetupViewport()
    {
        Renderer* renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);
    }
    
    void SubscribeToEvents()
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FacePlayer, HandleUpdate));
    }
    
    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();
        mgr_.Update(timeStep, debugText_);
    }

private:
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Text> debugText_;
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
