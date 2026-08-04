#include "StaticSceneApp.h"

namespace urho3d = Urho3D;

StaticSceneApp::StaticSceneApp(urho3d::Context* context)
    : Application(context)
{}

void StaticSceneApp::Setup()
{
    engineParameters_[urho3d::EP_WINDOW_TITLE]   = "ModelsViewer";
    engineParameters_[urho3d::EP_WINDOW_WIDTH]   = 1920;
    engineParameters_[urho3d::EP_WINDOW_HEIGHT]  = 1070;
    engineParameters_[urho3d::EP_FULL_SCREEN]    = false;
    engineParameters_[Urho3D::EP_MULTI_SAMPLE]   = 4;
    engineParameters_[urho3d::EP_RESOURCE_PATHS] = "Data;CoreData";
}

void StaticSceneApp::Start()
{
    uiRoot_ = GetSubsystem<urho3d::UI>()->GetRoot();
    auto* cache = GetSubsystem<urho3d::ResourceCache>();
    auto* style = cache->GetResource<urho3d::XMLFile>("UI/DefaultStyle.xml");
    uiRoot_->SetDefaultStyle(style);

    InitWindow();
    InitControls();
    ShowGlobalValues();
    CreateScene();
    CreateInstructions();
    SetupViewport();

    SubscribeToEvent(urho3d::E_UPDATE,           URHO3D_HANDLER(StaticSceneApp, HandleUpdate));
    SubscribeToEvent(urho3d::E_MOUSEWHEEL,       URHO3D_HANDLER(StaticSceneApp, HandleMouseWheel));
    SubscribeToEvent(urho3d::E_KEYDOWN,          URHO3D_HANDLER(StaticSceneApp, HandleCameraSetting));
    SubscribeToEvent(urho3d::E_POSTRENDERUPDATE, URHO3D_HANDLER(StaticSceneApp, HandleGeometryRender));
}

void StaticSceneApp::Stop()
{}

// ─────────────────────────────────────────────────────────────────────────────

urho3d::Window* StaticSceneApp::MakeWindow(const urho3d::String& title,
    urho3d::HorizontalAlignment ha,
    urho3d::VerticalAlignment   va,
    int minWidth)
{
    auto* cache_ = GetSubsystem<urho3d::ResourceCache>();
    auto* font   = cache_->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf");
    auto* win    = new urho3d::Window(context_);
    uiRoot_->AddChild(win);
    win->SetMinWidth(minWidth);
    win->SetLayout(urho3d::LM_VERTICAL, 7, urho3d::IntRect(7, 7, 7, 7));
    win->SetAlignment(ha, va);
    win->SetStyleAuto();

    auto* bar = win->CreateChild<urho3d::UIElement>();
    bar->SetMinSize(0, 20);
    bar->SetLayoutMode(urho3d::LM_HORIZONTAL);

    auto* titleText = bar->CreateChild<urho3d::Text>();
    titleText->SetFont(font, 16);
    titleText->SetText(" " + title);
    titleText->SetColor(urho3d::Color::WHITE);
    titleText->SetAlignment(urho3d::HA_LEFT, urho3d::VA_CENTER);

    return win;
}

void StaticSceneApp::SetupViewport()
{
    auto* renderer = GetSubsystem<urho3d::Renderer>();
    urho3d::SharedPtr<urho3d::Viewport> viewport(
        new urho3d::Viewport(context_, scene_, cameraNode_->GetComponent<urho3d::Camera>()));
    renderer->SetViewport(0, viewport);
}
