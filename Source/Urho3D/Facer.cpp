#include "Precompiled.h"
#include "Engine/Application.h"
#include "Graphics/Graphics.h"
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
#include "Math/MathDefs.h"
#include "Core/Mutex.h"
#include "Graphics/GraphicsEvents.h"
#include "Graphics/RenderSurface.h"
#include "Graphics/Technique.h"
#include <SDL/SDL.h>
#include <stdio.h>

//#define FACER_ENABLED

const char* rotate_bone_name = "rabbit2:Bip01_Head";
const char* head_bone_name = "rabbit2:Bip01_Head";
static int s_x = -128, s_y = -128, s_w = 128, s_h = 128;

#if __cplusplus
extern "C" {
#endif

#ifdef FACER_ENABLED
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

typedef struct gpu_size_t{
    uint32_t    width;
    uint32_t    height;
}gpu_size_t;
gpu_size_t GetFrameSize();
#endif

extern void Urho3D_Post_Update();

#if __cplusplus
}   // Extern C
#endif


namespace Urho3D
{

#if defined(IOS) || defined(__EMSCRIPTEN__)
    // Code for supporting SDL_iPhoneSetAnimationCallback() and emscripten_set_main_loop_arg()
#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
    void Facer_RunFrame(void* data)
    {
        static_cast<Engine*>(data)->RunFrame();
        Urho3D_Post_Update();
    }
#endif

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

enum FacialAnimationLayer
{
    kFacial_Animation_Base_Layer,
    kFacial_Animation_Action_Layer,
};

void FilterOutAnimationBones(const String& animName, ResourceCache* cache, const Vector<String>& boneNames)
{
    Animation* anim = cache->GetResource<Animation>(animName);
    if (!anim)
        return;

    const HashMap<StringHash, AnimationTrack>& tracks = anim->GetTracks();
    HashMap<StringHash, AnimationTrack>::ConstIterator i = tracks.Begin();
    for (; i!= tracks.End(); ++i)
    {
        bool found = false;
        for (unsigned j=0; j<boneNames.Size(); ++j)
        {
            if (i->first_ == StringHash(boneNames[j]))
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            anim->GetTrack(i->first_)->channelMask_ = 0;
        }
    }
}

void FillAnimationWithCurrentPose(Animation* anim, Node* node, const Vector<String>& boneNames)
{
    anim->RemoveAllTracks();
    for (unsigned i=0; i<boneNames.Size(); ++i)
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
    for (unsigned i=0; i<children.Size(); ++i)
    {
        nodeNames.Push(children[i]->GetName());
    }
    return nodeNames;
}


#ifdef FACER_ENABLED
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
        if (bone_name)
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

    Vector2 GetPositionOnFace(const vs_models_face_action_t& face, bool normalized = true) const
    {
        vs_pointf_t pt = face.face.points_array[facial_index];
        if (normalized)
        {
            Vector2 face_rect = GetFaceRect(face);
            pt.x /= face_rect.x_;
            pt.y /= face_rect.y_;
        }
        return Vector2(pt.x, pt.y);
    }

    Vector2 GetFaceRect(const vs_models_face_action_t& face) const
    {
        Vector2 ret;
        ret.x_ = face.face.points_array[32].x - face.face.points_array[0].x;
        ret.y_ = face.face.points_array[16].y - face.face.points_array[43].y;
        return ret;
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

    String base_animation;

    FacialBoneManager()
    :face_node(NULL)
    ,rotate_bone_node(NULL)
    {
        facial_bones[kFacial_Jaw] = (FacialBone(kFacial_Jaw, 16, "rabbit2:FcFX_Jaw"));
        facial_bones[kFacial_Nose] = (FacialBone(kFacial_Nose, 46, NULL));
        facial_bones[kFacial_Nose_Left] = (FacialBone(kFacial_Nose_Left, 83, "rabbit2:FcFX_Nose_L"));
        facial_bones[kFacial_Node_Right] = (FacialBone(kFacial_Node_Right, 82, "rabbit2:FcFX_Nose_R"));

        facial_bones[kFacial_Mouth_Bottom] = (FacialBone(kFacial_Mouth_Bottom, 102, "rabbit2:FcFX_Mouth_08"));
        facial_bones[kFacial_Mouth_Top] = (FacialBone(kFacial_Mouth_Top, 98, "rabbit2:FcFX_Mouth_010"));
        facial_bones[kFacial_Mouth_Left] = (FacialBone(kFacial_Mouth_Left, 90, "rabbit2:FcFX_Mouth_05"));
        facial_bones[kFacial_Mouth_Right] = (FacialBone(kFacial_Mouth_Right, 84, "rabbit2:FcFX_Mouth_012"));

        facial_bones[kFacial_EyeBall_Left] = (FacialBone(kFacial_EyeBall_Left, 105, "rabbit2:FcFX_Eye_L"));
        facial_bones[kFacial_EyeBall_Right] = (FacialBone(kFacial_EyeBall_Right, 104, "rabbit2:FcFX_Eye_R"));
        facial_bones[kFacial_EyeTop_Left] = (FacialBone(kFacial_EyeTop_Left, 75, "rabbit2:FcFX_EyLd_Top_L"));
        facial_bones[kFacial_EyeTop_Right] = (FacialBone(kFacial_EyeTop_Right, 72, "rabbit2:FcFX_EyLd_Top_R"));
        facial_bones[kFacial_EyeBottom_Left] = (FacialBone(kFacial_EyeBottom_Left, 76, "rabbit2:FcFX_EyLd_Bottom_L"));
        facial_bones[kFacial_EyeBottom_Right] = (FacialBone(kFacial_EyeBottom_Right, 73, "rabbit2:FcFX_EyLd_Bottom_R"));

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

        for (unsigned i=0; i<kFacial_Bone_Num; ++i)
        {
            facial_bones[i].LoadNode(face_node);
        }

        Vector<String> boneNames = GetChildNodeNames(face_node);
        Vector<String> mouthBones;
        Vector<String> leftEyeBones;
        Vector<String> rightEyeBones;

        for (unsigned i=0; i<boneNames.Size(); ++i)
        {
            if (boneNames[i].Contains("Mouth"))
                mouthBones.Push(boneNames[i]);
        }
        mouthBones.Push("rabbit2:FcFX_Jaw");

        for (unsigned i=0; i<boneNames.Size(); ++i)
        {
            if (boneNames[i].Contains("Ey") && boneNames[i].EndsWith("_L") && boneNames[i] != facial_bones[kFacial_EyeBall_Left].bone_name)
            {
                leftEyeBones.Push(boneNames[i]);
            }
            if (boneNames[i].Contains("Ey") && boneNames[i].EndsWith("_R") && boneNames[i] != facial_bones[kFacial_EyeBall_Right].bone_name)
            {
                rightEyeBones.Push(boneNames[i]);
            }
        }
        Vector<String> leftEyeBalls; leftEyeBalls.Push(facial_bones[kFacial_EyeBall_Left].bone_name);
        Vector<String> rightEyeBalls; rightEyeBalls.Push(facial_bones[kFacial_EyeBall_Right].bone_name);

        facial_attributes[kFacial_MouseOpenness].animation = (CreatePoseAnimation("Models/rabbit_mouse_open.mdl", mouthBones, scene)->GetName());
        facial_attributes[kFacial_EyeCloseness_Left].animation = (CreatePoseAnimation("Models/rabbit_eye_close_L.mdl", leftEyeBones, scene)->GetName());
        facial_attributes[kFacial_EyeCloseness_Right].animation = (CreatePoseAnimation("Models/rabbit_eye_close_R.mdl", rightEyeBones, scene)->GetName());

        facial_attributes[kFacial_MouseOpenness].value = 0;
        facial_attributes[kFacial_EyeCloseness_Left].value = 0;
        facial_attributes[kFacial_EyeCloseness_Right].value = 0;
        facial_attributes[kFacial_EyePositionLeft_Left].value = 0.5F;
        facial_attributes[kFacial_EyePositionRight_Left].value = 0.5F;
        facial_attributes[kFacial_EyePositionLeft_Right].value = 0.5F;
        facial_attributes[kFacial_EyePositionRight_Right].value = 0.5F;

        base_animation = "Animations/rabbit_ear_motion_Take 001.ani";
        AnimationController* ac = face_node->GetComponent<AnimationController>();
        ac->PlayExclusive(base_animation, kFacial_Animation_Base_Layer, true, 0.25F);
        ac->SetSpeed(base_animation, 0.5F);

        for (unsigned i=0; i<kFacial_Attribute_Num; ++i)
        {
            if (!facial_attributes[i].animation.Empty())
            {
                ac->Play(facial_attributes[i].animation, kFacial_Animation_Action_Layer, true, 0);
                ac->SetWeight(facial_attributes[i].animation, facial_attributes[i].value);
            }
        }
    }

    void DebugDraw(DebugRenderer* debug)
    {
        for (unsigned i=0; i<kFacial_Bone_Num; ++i)
        {
            facial_bones[i].DebugDraw(debug);
        }
    }

    void Update(float dt, Text* text)
    {
        vs_models_human_action_t* t = GetFacerAction();
        String s = "No Face detected";

        if (t && t->face_count > 0)
        {
            const vs_models_face_action_t& f = t->faces[0];
            float yaw = f.face.yaw;
            float pitch = f.face.pitch;
            float roll = f.face.roll;
            s = "";
            //s = "yaw=" + String(yaw) + "\npitch=" + String(pitch) + "\nroll=" + String(roll);

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

            //s += String("\nb1=" + String(b1) + " b2=" + String(b2));
            //s += String("\nb3=" + String(b3)+  " b4=" + String(b4));

            const float z_offset = -15.0F;
            Quaternion q(-yaw, roll, pitch + z_offset);
            rotate_bone_node->SetRotation(q);

            float w1 = facial_attributes[kFacial_MouseOpenness].value;
            float w2 = facial_attributes[kFacial_EyeCloseness_Left].value;
            const float speed = 5.0F;

            if (f.face_action & VS_EYE_BLINK)
            {
                s += "\nBlink";
                w2 += speed * dt;
            }
            else
            {
                w2 -= speed * dt;
            }

            if (f.face_action & VS_MOUTH_AH)
            {
                s += "\nMouth_AH";
                w1 += speed * dt;
            }
            else
            {
                w1 -= speed * dt;
            }

            facial_attributes[kFacial_MouseOpenness].value = Clamp(w1, 0.0F, 1.0F);
            facial_attributes[kFacial_EyeCloseness_Left].value = Clamp(w2, 0.0F, 1.0F);
            facial_attributes[kFacial_EyeCloseness_Right].value = facial_attributes[kFacial_EyeCloseness_Left].value;

            facial_attributes[kFacial_EyePositionRight_Left].value = 1.0F - facial_attributes[kFacial_EyePositionLeft_Left].value;
            facial_attributes[kFacial_EyePositionRight_Right].value = 1.0F - facial_attributes[kFacial_EyePositionLeft_Right].value;
        }

        text->SetText(s);

        AnimationController* ac = face_node->GetComponent<AnimationController>();
        for (unsigned i=0; i<kFacial_Attribute_Num; ++i)
        {
            if (!facial_attributes[i].animation.Empty())
            {
                ac->Play(facial_attributes[i].animation, kFacial_Animation_Action_Layer, true, 0);
                ac->SetWeight(facial_attributes[i].animation, facial_attributes[i].value);
            }
        }
    }
};
#endif
    
class FacePlayer : public Application
{
    URHO3D_OBJECT(FacePlayer, Application);
public:
    FacePlayer(Context* context):Application(context),renderToTexture_(false),manualUpdate_(false){};

    virtual void Setup()
    {
        FileSystem* filesystem = GetSubsystem<FileSystem>();
        const String commandFileName = filesystem->GetProgramDir() + "Data/CommandLine.txt";
        if (GetArguments().Empty() && filesystem->FileExists(commandFileName))
        {
            SharedPtr<File> commandFile(new File(context_, commandFileName));
            if (commandFile->IsOpen())
            {
                String commandLine = commandFile->ReadLine();
                commandFile->Close();
                ParseArguments(commandLine, false);
                Vector<String> arguments = GetArguments();
                engineParameters_ = Engine::ParseParameters(arguments);

                for (unsigned i = 0; i < arguments.Size(); ++i)
                {
                    printf("argument=%s \n", arguments[i].CString());

                    if (arguments[i].Length() > 1 && arguments[i][0] == '-')
                    {
                        String argument = arguments[i].Substring(1).ToLower();
                        String value = i + 1 < arguments.Size() ? arguments[i + 1] : String::EMPTY;



                        if (argument == "x" && !value.Empty())
                        {
                            s_x = ToInt(value);
                            ++i;
                        }
                        else if (argument == "y" && !value.Empty())
                        {
                            s_y = ToInt(value);
                            ++i;
                        }
                        else if (argument == "w" && !value.Empty())
                        {
                            s_w = ToInt(value);
                            ++i;
                        }
                        else if (argument == "h" && !value.Empty())
                        {
                            s_h = ToInt(value);
                            ++i;
                        }
                        else if (argument == "render_to_texture")
                        {
                            renderToTexture_ = true;
                        }
                        else if (argument == "manual_update")
                        {
                            manualUpdate_ = true;
                        }
                    }
                }
            }
        }

        engineParameters_[EP_ORIENTATIONS] = "LandscapeLeft LandscapeRight Portrait PortraitUpsideDown";

        if (renderToTexture_)
        {
            // s_w = 0; s_h = 0; s_x = 0; s_y = 0;
        }

        printf("++++++++++++++++ render_to_texture=%d, manual_update=%d ++++++++++++++++\n", renderToTexture_, manualUpdate_);
    }

    virtual void Start()
    {
#ifdef FACER_EANBLED
        Graphics* g = GetSubsystem<Graphics>();
        gpu_size_t f_size = GetFrameSize();
        printf("graphics-width=%d,height=%d gpu-width=%d,height=%d\n", g->GetWidth(), g->GetHeight(), f_size.width, f_size.height);
#endif

        CreateScene();
        CreateUI();
        SetupViewport();
        SubscribeToEvents();
    }

    void GetRenderTexture(int* out_w, int* out_h, void* data)
    {
        if (!renderToTexture_)
            return;

        MutexLock _l(lock_);
        *out_w = renderTexture_->GetWidth();
        *out_h = renderTexture_->GetHeight();
        if (data)
            renderTexture_->GetData(0, data);
    }

    unsigned GetRenderTextureId()
    {
        if (!renderToTexture_)
            return 0;

        return renderTexture_->GetGPUObjectName();
    }

    void Update()
    {
        if (!manualUpdate_)
            return;
        engine_->RunFrame();
        Urho3D_Post_Update();
    }

    int RunEngine()
    {
#if !defined(__GNUC__) || __EXCEPTIONS
        try
        {
#endif
            Setup();
            if (exitCode_)
                return exitCode_;

            if (!engine_->Initialize(engineParameters_))
            {
                ErrorExit();
                return exitCode_;
            }

            Start();
            if (exitCode_)
                return exitCode_;

            // Platforms other than iOS and Emscripten run a blocking main loop
#if !defined(IOS) && !defined(__EMSCRIPTEN__)
            while (!engine_->IsExiting())
                engine_->RunFrame();

            Stop();
            // iOS will setup a timer for running animation frames so eg. Game Center can run. In this case we do not
            // support calling the Stop() function, as the application will never stop manually
#else
#if defined(IOS)
            if (!manualUpdate_)
                SDL_iPhoneSetAnimationCallback(GetSubsystem<Graphics>()->GetWindow(), 1, Facer_RunFrame, engine_);
#elif defined(__EMSCRIPTEN__)
            emscripten_set_main_loop_arg(RunFrame, engine_, 0, 1);
#endif
#endif

            return exitCode_;
#if !defined(__GNUC__) || __EXCEPTIONS
        }
        catch (std::bad_alloc&)
        {
            ErrorDialog(GetTypeName(), "An out-of-memory error occurred. The application will now exit.");
            return EXIT_FAILURE;
        }
#endif
    }

private:
    void CreateScene()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        scene_ = new Scene(context_);
        SharedPtr<File> file = cache->GetFile("Scenes/Head.xml");
        scene_->LoadXML(*file);
#ifdef FACER_ENABLED
        mgr_.Init(scene_);
#endif
        
        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->CreateComponent<Camera>();
        cameraNode_->SetPosition(Vector3(0.0f, 0.55f, -1.5f));

        rttCameraNode_ = scene_->CreateChild("RttCamera");
        rttCameraNode_->CreateComponent<Camera>();
        rttCameraNode_->SetPosition(cameraNode_->GetPosition());
    }

    void CreateUI()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        debugText_ = new Text(context_);
        debugText_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 20);
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
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        if (renderToTexture_)
        {
            Graphics* g = GetSubsystem<Graphics>();
            renderTexture_ = new Texture2D(context_);
            SharedPtr<Viewport> rttViewPort(new Viewport(context_, scene_, rttCameraNode_->GetComponent<Camera>()));
            renderTexture_->SetSize(g->GetWidth(), g->GetHeight(), Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
            renderTexture_->SetFilterMode(FILTER_BILINEAR);
            renderTexture_->GetRenderSurface()->SetViewport(0, rttViewPort);

            /*SharedPtr<Material> renderMaterial(new Material(context_));
            renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffUnlit.xml"));
            renderMaterial->SetTexture(TU_DIFFUSE, renderTexture_);
            renderMaterial->SetDepthBias(BiasParameters(-0.001f, 0.0f));

            Node* screenNode = scene_->CreateChild("Screen");
            screenNode->SetPosition(Vector3(0.0f, 10.0f, -0.27f));
            screenNode->SetRotation(Quaternion(-90.0f, 0.0f, 0.0f));
            screenNode->SetScale(Vector3(20.0f, 0.0f, 15.0f));
            StaticModel* screenObject = screenNode->CreateComponent<StaticModel>();
            screenObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
            screenObject->SetMaterial(renderMaterial);*/
        }
    }

    void SubscribeToEvents()
    {
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FacePlayer, HandleUpdate));
        if (renderToTexture_)
            SubscribeToEvent(E_ENDRENDERING, URHO3D_HANDLER(FacePlayer, HanldeEndRendering));
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();
#ifdef FACER_ENABLED
        mgr_.Update(timeStep, debugText_);
#endif
    }

    void HanldeEndRendering(StringHash eventType, VariantMap& eventData)
    {
        MutexLock _l(lock_);
        Graphics* g = GetSubsystem<Graphics>();
        if (renderTexture_->GetWidth() != g->GetWidth() || renderTexture_->GetHeight() != g->GetHeight())
        {
            renderTexture_->SetSize(g->GetWidth(), g->GetHeight(), Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
        }
    }

private:
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<Text> debugText_;
    SharedPtr<Node> rttCameraNode_;
    SharedPtr<Texture2D> renderTexture_;
#ifdef FACER_ENABLED
    FacialBoneManager mgr_;
#endif
    Mutex lock_;
    bool renderToTexture_;
    bool manualUpdate_;
};
}

#if __cplusplus
extern "C" {
#endif

static Urho3D::FacePlayer* g_app = NULL;
void* g_eagl_ctx = NULL;

void GetEngineWindowRect(int* x, int *y, int* w, int* h)
{
    *x = s_x;
    *y = s_y;
    *w = s_w;
    *h = s_h;
}

int SDL_main(int argc, char** argv)
{
    SDL_SetMainReady();
    Urho3D::Context* context = new Urho3D::Context();
    g_app = new Urho3D::FacePlayer(context);
    return g_app->RunEngine();
}

void Urho3D_Init()
{
    if (g_app != NULL)
    {
        printf ("======================== g_app not NULL !!!! ======================== \n");
        return;
    }
    printf("Urho3D_Init\n");
    SDL_main(0, 0);
}

void Urho3D_GetRenderTexture(int* out_w, int* out_h, void* data)
{
    if (!g_app)
        return;
    g_app->GetRenderTexture(out_w, out_h, data);
}

unsigned Urho3D_GetRenderTextureId()
{
    if (!g_app)
        return 0;
    return g_app->GetRenderTextureId();
}

void Urho3D_Update()
{
    if (!g_app)
        return;
    g_app->Update();
}

void* Urho3D_GetContext()
{
    return g_eagl_ctx;
}

#if __cplusplus
}
#endif
