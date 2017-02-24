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

namespace Urho3D
{
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

        // Construct a search path to find the resource prefix with two entries:
        // The first entry is an empty path which will be substituted with program/bin directory -- this entry is for binary when it is still in build tree
        // The second and third entries are possible relative paths from the installed program/bin directory to the asset directory -- these entries are for binary when it is in the Urho3D SDK installation location
        if (!engineParameters_.Contains(EP_RESOURCE_PREFIX_PATHS))
            engineParameters_[EP_RESOURCE_PREFIX_PATHS] = ";../share/Resources;../share/Urho3D/Resources";
    }

    virtual void Start()
    {
        CreateLogo();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        SharedPtr<Text> helloText(new Text(context_));
        helloText->SetText("Hello World from Urho3D!");
        helloText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 10);
        helloText->SetColor(Color(0.0f, 1.0f, 0.0f));
        helloText->SetHorizontalAlignment(HA_CENTER);
        helloText->SetVerticalAlignment(VA_CENTER);
        GetSubsystem<UI>()->GetRoot()->AddChild(helloText);
        
        Graphics* g = GetSubsystem<Graphics>();
        SDL_HideWindow(g->GetWindow());
    }

    virtual void Stop()
    {

    }

    void CreateLogo()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Texture2D* logoTexture = cache->GetResource<Texture2D>("Textures/FishBoneLogo.png");
        if (!logoTexture)
            return;
        UI* ui = GetSubsystem<UI>();
        logoSprite_ = ui->GetRoot()->CreateChild<Sprite>();
        logoSprite_->SetTexture(logoTexture);

        int textureWidth = logoTexture->GetWidth();
        int textureHeight = logoTexture->GetHeight();
        logoSprite_->SetScale(256.0f / textureWidth);
        logoSprite_->SetSize(textureWidth, textureHeight);
        logoSprite_->SetHotSpot(textureWidth, textureHeight);
        logoSprite_->SetAlignment(HA_RIGHT, VA_BOTTOM);
        logoSprite_->SetOpacity(0.9f);
        logoSprite_->SetPriority(-100);
    }

private:
    SharedPtr<Sprite> logoSprite_;
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
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

void TestFUCK()
{
    printf("FUCK\n");
    SDL_main(0, 0);
}

#if __cplusplus
}   // Extern C
#endif
