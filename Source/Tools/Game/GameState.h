#pragma once
#include "FSM.h"
#include "Constants.h"
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Sprite.h>

namespace Urho3D
{

enum LoadSubState
{
    LOADING_RESOURCES,
    LOADING_MOTIONS,
    LOADING_FINISHED,
};

class GameState : public State
{
public:
    GameState(Context* c)
    :State(c)
    {

    }
}

class LoadingState : public GameState
{
public:
    int                 state_;
    int                 numLoadedResources_;
    SharedPtr<Scene>    preloadScene_;

    LoadingState(Context* c)
    :GameState(c)
    ,numLoadedResources_(0)
    {
        SetName("LoadingState");
    }

    void CreateLoadingUI()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        UI* ui = GetSubsystem<UI>();
        Graphics* graphics = GetSubsystem<Graphics>();

        Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/ulogo.jpg");
        SharedPtr<Sprite> logoSprite(new Sprite(context_));
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
        logoSprite->AddTag("TAG_LOADING");
        ui->AddChild(logoSprite);

        SharedPtr<Text> text(new Text(context_));
        text->SetFont(cache.GetResource("Font", UI_FONT), UI_FONT_SIZE);
        text->SetAlignment(HA_LEFT, VA_BOTTOM);
        text->SetPosition(2, 0);
        text->color = Color(1, 1, 1);
        text->SetTextEffect(TE_STROKE);
        text->AddTag("TAG_LOADING");
        ui->AddChild(text);

        Texture2D* loadingTexture = cache->GetResource<Texture2D>("Textures/Loading.tga");
        SharedPtr<Sprite> loadingSprite(new Sprite(context_));
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
        alphaAnimation.SetKeyFrame(0.0F, Variant(0.0F));
        alphaAnimation.SetKeyFrame(alphaDuration, Variant(1.0F));
        alphaAnimation.SetKeyFrame(alphaDuration * 2, Variant(0.0F));
        loadingSprite->SetAttributeAnimation("Opacity", alphaAnimation);
        ui->AddChild(loadingSprite);
    }

    void Enter(State@ lastState)
    {
        State::Enter(lastState);
        if (!engine.headless)
            CreateLoadingUI();
        ChangeSubState(LOADING_RESOURCES);
    }

    void Exit(State@ nextState)
    {
        State::Exit(nextState);
        Array<UIElement@>@ elements = ui.root.GetChildrenWithTag("TAG_LOADING");
        for (uint i = 0; i < elements.length; ++i)
            elements[i].Remove();
    }

    void Update(float dt)
    {
        if (state == LOADING_RESOURCES)
        {

        }
        else if (state == LOADING_MOTIONS)
        {
            Text@ text = ui.root.GetChild("loading_text");
            if (text !is null)
                text.text = "Loading Motions, loaded = " + gMotionMgr.processedMotions;

            if (d_log)
                Print("============================== Motion Loading start ==============================");

            if (gMotionMgr.Update(dt))
            {
                gMotionMgr.Finish();
                ChangeSubState(LOADING_FINISHED);
                if (text !is null)
                    text.text = "Loading Scene Resources";
            }

            if (d_log)
                Print("============================== Motion Loading end ==============================");
        }
        else if (state == LOADING_FINISHED)
        {
            if (preloadScene !is null)
                preloadScene.Remove();
            preloadScene = null;
            gGame.ChangeState("TestGameState");
        }
    }

    void ChangeSubState(int newState)
    {
        if (state == newState)
            return;

        Print("LoadingState ChangeSubState from " + state + " to " + newState);
        state = newState;

        if (newState == LOADING_RESOURCES)
        {
            preloadScene = Scene();
            preloadScene.LoadAsyncXML(cache.GetFile("Scenes/animation.xml"), LOAD_RESOURCES_ONLY);
        }
        else if (newState == LOADING_MOTIONS)
            gMotionMgr.Start();
    }
}

}