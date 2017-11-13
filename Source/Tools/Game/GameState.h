#pragma once
#include "FSM.h"
#include "Constants.h"
#include "Motion.h"
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Scene/ValueAnimation.h>
#include <Urho3D/Scene/SceneEvents.h>

namespace Urho3D
{

enum LoadSubState
{
    LOADING_RESOURCES,
    LOADING_MOTIONS,
    LOADING_FINISHED,
};

enum GameSubState
{
    GAME_FADING,
    GAME_RUNNING,
    GAME_FAIL,
    GAME_RESTARTING,
    GAME_PAUSE,
    GAME_WIN,
};


class GameState : public State
{
public:
    GameState(Context* c)
    :State(c)
    {

    }
};

class LoadingState : public GameState
{
public:
    int                 state_;
    SharedPtr<Scene>    preloadScene_;

    LoadingState(Context* c)
    :GameState(c)
    {
        SetName("LoadingState");
    }

    void CreateLoadingUI()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        UI* ui = GetSubsystem<UI>();
        Graphics* graphics = GetSubsystem<Graphics>();

        Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/ulogo.jpg");
        SharedPtr<Sprite> logoSprite(ui->GetRoot()->CreateChild<Sprite>());
        logoSprite->SetTexture(logoTexture);
        unsigned textureWidth = logoTexture->GetWidth();
        unsigned textureHeight = logoTexture->GetHeight();
        logoSprite->SetScale(256.0F / textureWidth);
        logoSprite->SetSize(textureWidth, textureHeight);
        logoSprite->SetHotSpot(0, textureHeight);
        logoSprite->SetAlignment(HA_LEFT, VA_BOTTOM);
        logoSprite->SetPosition(graphics->GetWidth() - textureWidth/2, 0);
        logoSprite->SetOpacity(0.75F);
        logoSprite->SetPriority(-100);
        logoSprite->AddTag(UI_LOADING_TAG);

        SharedPtr<Text> text(ui->GetRoot()->CreateChild<Text>());
        text->SetName("loading_text");
        text->SetFont(cache->GetResource<Font>(UI_FONT), UI_FONT_SIZE);
        text->SetAlignment(HA_LEFT, VA_BOTTOM);
        text->SetPosition(2, 0);
        text->SetColor(Color::WHITE);
        text->SetTextEffect(TE_STROKE);
        text->AddTag(UI_LOADING_TAG);

        Texture2D* loadingTexture = cache->GetResource<Texture2D>("Textures/Loading.tga");
        SharedPtr<Sprite> loadingSprite(ui->GetRoot()->CreateChild<Sprite>());
        loadingSprite->SetTexture(loadingTexture);
        textureWidth = loadingTexture->GetWidth();
        textureHeight = loadingTexture->GetHeight();
        loadingSprite->SetSize(textureWidth, textureHeight);
        loadingSprite->SetPosition(graphics->GetWidth()/2 - textureWidth/2, graphics->GetHeight()/2 - textureHeight/2);
        loadingSprite->SetOpacity(0.0F);
        loadingSprite->SetPriority(-100);
        loadingSprite->AddTag("TAG_LOADING");
        float alphaDuration = 1.0F;
        SharedPtr<ValueAnimation> alphaAnimation(new ValueAnimation(context_));
        alphaAnimation->SetKeyFrame(0.0F, Variant(0.0F));
        alphaAnimation->SetKeyFrame(alphaDuration, Variant(1.0F));
        alphaAnimation->SetKeyFrame(alphaDuration * 2, Variant(0.0F));
        loadingSprite->SetAttributeAnimation("Opacity", alphaAnimation);
        ui->GetRoot()->AddChild(loadingSprite);
    }

    virtual void Enter(State* lastState) override
    {
        State::Enter(lastState);
        CreateLoadingUI();
        ChangeSubState(LOADING_RESOURCES);
    }

    virtual void Exit(State* nextState) override
    {
        State::Exit(nextState);
        UI* ui = GetSubsystem<UI>();
        PODVector<UIElement*> elements = ui->GetRoot()->GetChildrenWithTag("TAG_LOADING");
        for (unsigned i = 0; i < elements.Size(); ++i)
            elements[i]->Remove();
    }

    virtual void Update(float dt) override
    {
        if (state_ == LOADING_RESOURCES)
        {

        }
        else if (state_ == LOADING_MOTIONS)
        {
            MotionManager* mgr = GetSubsystem<MotionManager>();
            UI* ui = GetSubsystem<UI>();
            Text* text = ui->GetRoot()->GetChildStaticCast<Text>(StringHash("loading_text"));
            if (text)
            {
                text->SetText(String("Loading Motions, loaded = ") + String(mgr->processedMotions_));
            }

            if (d_log)
                URHO3D_LOGINFO("============================== Motion Loading start ==============================");

            if (mgr->Update(dt))
            {
                mgr->Finish();
                ChangeSubState(LOADING_FINISHED);
                if (text)
                    text->SetText("Loading Scene Resources");
            }

            if (d_log)
                URHO3D_LOGINFO("============================== Motion Loading end ==============================");
        }
        else if (state_ == LOADING_FINISHED)
        {
            if (preloadScene_.NotNull())
                preloadScene_.Reset();
            // gGame.ChangeState("TestGameState");
        }
    }

    void ChangeSubState(int newState)
    {
        if (state_ == newState)
            return;

        URHO3D_LOGINFO("LoadingState ChangeSubState from " + String(state_) + " to " + String(newState));
        state_ = newState;

        if (newState == LOADING_RESOURCES)
        {
            preloadScene_ = new Scene(context_);
            SubscribeToEvent(preloadScene_, E_ASYNCLOADPROGRESS, URHO3D_HANDLER(LoadingState, HandleAsyncLoadProgress));
            SubscribeToEvent(preloadScene_, E_ASYNCLOADFINISHED, URHO3D_HANDLER(LoadingState, HandleAsyncLoadFinished));

            SharedPtr<File> file = GetSubsystem<ResourceCache>()->GetFile(PRELOAD_SCENE_NAME);
            preloadScene_->LoadAsyncXML(file, LOAD_RESOURCES_ONLY);
        }
        else if (newState == LOADING_MOTIONS)
        {
            MotionManager* mgr = GetSubsystem<MotionManager>();
            mgr->Start();
        }
    }

    void HandleAsyncLoadProgress(StringHash eventType, VariantMap& eventData)
    {
        UI* ui = GetSubsystem<UI>();
        Text* text = ui->GetRoot()->GetChildStaticCast<Text>(StringHash("loading_text"));
        if (text)
        {
            using namespace AsyncLoadProgress;
            float progress = eventData[P_PROGRESS].GetFloat();
            unsigned loadedResources = eventData[P_LOADEDRESOURCES].GetUInt();
            unsigned totalResources = eventData[P_TOTALRESOURCES].GetUInt();
            text->SetText(String("Loading scene ressources progress=" + String(progress) +
                " resources:" + String(loadedResources) + "/" + String(totalResources)));
        }
    }

    void HandleAsyncLoadFinished(StringHash eventType, VariantMap& eventData)
    {
        if (state_ == LOADING_RESOURCES)
            ChangeSubState(LOADING_MOTIONS);
    }
};

class InGameState : public GameState
{
public:
    SharedPtr<Scene>       scene_;
    SharedPtr<Fader>       fader_;

    int                    state_ = -1;

    InGameState(Context* c)
    :GameState(c)
    ,state_(-1)
    {
        SetName("InGameState");
    }

    virtual void Enter(State* lastState) override
    {
        State::Enter(lastState);
        CreateScene();
        SetupViewport();
        CreateUI();
        ChangeSubState(GAME_FADING);
    }

    void CreateUI()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Graphics* g = GetSubsystem<Graphics>();
        UI* ui = GetSubsystem<UI>();
        fader_ = new Fader(context_);

        int height = g->GetHeight() / 22;
        if (height > 64)
            height = 64;
        Text* messageText = ui->GetRoot()->CreateChild<Text>("message");
        messageText->SetFont(cache->GetResource<Font>(UI_FONT), UI_FONT_SIZE);
        messageText->SetAlignment(HA_CENTER, VA_CENTER);
        messageText->SetPosition(0, -height * 2 + 100);
        messageText->SetColor(Color::RED);
        messageText->SetVisible(false);

        Text* statusText = ui->GetRoot()->CreateChild<Text>("status");;
        statusText->SetFont(cache->GetResource<Font>(UI_FONT), UI_FONT_SIZE);
        statusText->SetAlignment(HA_LEFT, VA_TOP);
        statusText->SetPosition(0, 0);
        statusText->SetColor(Color(1, 1, 0));
        statusText->SetVisible(true);
    }

    virtual void Update(float dt) override
    {
        switch (state_)
        {
        case GAME_FADING:
            {
                if (fader_->Update(dt) == 1)
                {
                    ChangeSubState(GAME_RUNNING);
                }
            }
            break;
        }
        GameState::Update(dt);
    }

    void ChangeSubState(int newState)
    {
        if (state_ == newState)
            return;

        int oldState = state_;
        URHO3D_LOGINFO("InGameState ChangeSubState from " + String(oldState) + " to " + String(newState));
        state_ = newState;
        timeInState_ = 0.0F;

        switch (newState)
        {
        case GAME_RUNNING:
            {

            }
            break;

        case GAME_FADING:
            {
                fader_->FadeIn(3.0F);
            }
            break;
        }
    }

    void SetupViewport()
    {
        Node* cameraNode = scene_->GetChild(CAMERA_NODE_NAME, true);
        Camera* cam = cameraNode->GetComponent<Camera>();
        Renderer* renderer = GetSubsystem<Renderer>();
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cam));
        renderer->SetViewport(0, viewport);
    }

    void CreateScene()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        scene_ = new Scene(context_);
        unsigned t = Time::GetSystemTime();
        {
            TimeLogger _l("Game scene load");
            scene_->LoadXML(*cache->GetFile(GAME_SCENE_NAME));
        }

        Node* cameraNode = scene_->CreateChild(CAMERA_NODE_NAME);
        cameraNode->CreateComponent<Camera>();

        Node* tmpPlayerNode = scene_->GetChild("player", true);
        Vector3 playerPos;
        Quaternion playerRot;
        if (tmpPlayerNode)
        {
            playerPos = tmpPlayerNode->GetWorldPosition();
            playerRot = tmpPlayerNode->GetWorldRotation();
            playerPos.y_ = 0;
            tmpPlayerNode->Remove();
        }

        //Node@ playerNode = CreateCharacter("player", "bruce_w", "Bruce", playerPos, playerRot);
        //audio.listener = playerNode.GetChild(HEAD, true).CreateComponent("SoundListener");
        //playerId = playerNode.id;

        // preprocess current scene
        PODVector<unsigned> nodes_to_remove;
        for (unsigned i=0; i<scene_->GetNumChildren(); ++i)
        {
            Node* _node = scene_->GetChild(i);
            if (_node->GetName().StartsWith("thug"))
            {
                nodes_to_remove.Push(_node->GetID());
                //Vector3 v = _node.worldPosition;
                //v.y = 0;
                //em.enemyResetPositions.Push(v);
                //em.enemyResetRotations.Push(_node.worldRotation);
                //++enemyNum;
            }
            else if (_node->GetName().StartsWith("preload_"))
                nodes_to_remove.Push(_node->GetID());
            else if (_node->GetName().StartsWith("light"))
            {
                Light* light = _node->GetComponent<Light>();
                //if (render_features & RF_SHADOWS == 0)
                //    light.castShadows = false;
                light->SetShadowBias(BiasParameters(0.00025F, 0.5F));
                light->SetShadowCascade(CascadeParameters(10.0F, 50.0F, 200.0F, 0.0F, 0.8F));
            }
        }

        for (unsigned i=0; i<nodes_to_remove.Size(); ++i)
            scene_->GetNode(nodes_to_remove[i])->Remove();

        //Vector3 v_pos = playerNode->GetWorldPosition();
        //cameraNode.position = Vector3(v_pos.x, 10.0f, -10);
        //cameraNode.LookAt(Vector3(v_pos.x, 4, 0));

        //gCameraMgr.Start(cameraNode);
        //gCameraMgr.SetCameraController("Debug");
        //gCameraMgr.SetCameraController("ThirdPerson");
        //gCameraMgr.SetCameraController("LookAt");

        //Node@ floor = scene_.GetChild("floor", true);
        //StaticModel@ model = floor.GetComponent("StaticModel");
        // WORLD_HALF_SIZE = model.boundingBox.halfSize * floor.worldScale;
    }

    virtual String GetDebugText() const override
    {
        return  String(" name=") + name_ + " timeInState=" + String(timeInState_) +
               " state=" + String(state_) + "\n";
    }
};

}