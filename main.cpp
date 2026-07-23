#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/GraphicsAPI/Texture2D.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/BorderImage.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <nfd.hpp>
#include <stdlib.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/GraphicsAPI/VertexBuffer.h>
#include <Urho3D/GraphicsAPI/IndexBuffer.h>

namespace urho3d = Urho3D;

class StaticSceneApp : public urho3d::Application
{
    URHO3D_OBJECT(StaticSceneApp, urho3d::Application)
public:
    StaticSceneApp(urho3d::Context* context) : Application(context) {}

    void Setup() override
    {
        engineParameters_[urho3d::EP_WINDOW_TITLE]   = "ModelsViewer";
        engineParameters_[urho3d::EP_WINDOW_WIDTH]   = 1920;
        engineParameters_[urho3d::EP_WINDOW_HEIGHT]  = 1080;
        engineParameters_[urho3d::EP_FULL_SCREEN]    = false;
        engineParameters_[Urho3D::EP_MULTI_SAMPLE]   = 4;
        engineParameters_[urho3d::EP_RESOURCE_PATHS] = "Data;CoreData";
    }

    void Start() override
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

        SubscribeToEvent(urho3d::E_UPDATE,     URHO3D_HANDLER(StaticSceneApp, HandleUpdate));
        SubscribeToEvent(urho3d::E_MOUSEWHEEL, URHO3D_HANDLER(StaticSceneApp, HandleMouseWheel));
        SubscribeToEvent(urho3d::E_KEYDOWN, URHO3D_HANDLER(StaticSceneApp, HandleCameraSetting));
        SubscribeToEvent(urho3d::E_POSTRENDERUPDATE, URHO3D_HANDLER(StaticSceneApp, HandleGeometryRender));
    }

    urho3d::Window* MakeWindow(const urho3d::String& title,
        urho3d::HorizontalAlignment ha,
        urho3d::VerticalAlignment   va,
        int minWidth = 370)
    {
        auto* cache_ = GetSubsystem<urho3d::ResourceCache>();
        auto* font = cache_->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf");
        auto* win = new urho3d::Window(context_);
        uiRoot_->AddChild(win);
        win->SetMinWidth(minWidth);
        win->SetLayout(urho3d::LM_VERTICAL, 7, urho3d::IntRect(7, 7, 7, 7));
        win->SetAlignment(ha, va);
        win->SetStyleAuto();

        auto* bar = win->CreateChild<urho3d::UIElement>();
        bar->SetMinSize(0, 20);
        bar->SetLayoutMode(urho3d::LM_HORIZONTAL);

        auto* titleText = bar->CreateChild<urho3d::Text>();
        titleText->SetFont(cache_->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf"), 16);
        titleText->SetText(" " + title);
        titleText->SetColor(urho3d::Color::WHITE);
        titleText->SetAlignment(urho3d::HA_LEFT, urho3d::VA_CENTER);

        return win;
    }

    void InitWindow()
    {
        window_ = MakeWindow("write a pth to .mdl and .xml", urho3d::HA_LEFT, urho3d::VA_TOP);
        panel_ = MakeWindow("Transform", urho3d::HA_RIGHT, urho3d::VA_TOP, 280);
    }

    void InitControls()
    {
        auto* cache_ = GetSubsystem<urho3d::ResourceCache>();
        auto* font = cache_->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf");
        auto* folderTex = cache_->GetResource<urho3d::Texture2D>("UI/icons/folder.png");

        auto MakeRow = [&](urho3d::UIElement* parent, int gap = 1)
            {
                auto* row = parent->CreateChild<urho3d::UIElement>();
                row->SetLayout(urho3d::LM_HORIZONTAL, gap);
                row->SetMinHeight(28);
                return row;
            };

        auto MakeLabel = [&](urho3d::UIElement* parent,
            const urho3d::String& text,
            int fontSize = 14) -> urho3d::Text*
            {
                auto* t = parent->CreateChild<urho3d::Text>();
                t->SetStyleAuto();
                t->SetFont(font, fontSize);
                t->SetText(text);
                t->SetColor(urho3d::Color::WHITE);
                return t;
            };

        auto MakeButton = [&](urho3d::UIElement* parent,
            const urho3d::String& label,
            int w, int h) -> urho3d::Button*
            {
                auto* btn = parent->CreateChild<urho3d::Button>();
                btn->SetFixedSize(w, h);
                btn->SetStyleAuto();
                auto* txt = btn->CreateChild<urho3d::Text>();
                txt->SetFont(font, 12);
                txt->SetText(label);
                txt->SetAlignment(urho3d::HA_CENTER, urho3d::VA_CENTER);
                txt->SetStyleAuto();
                return btn;
            };

        auto MakeLineEdit = [&](urho3d::UIElement* parent,
            int w, int h) -> urho3d::LineEdit*
            {
                auto* le = parent->CreateChild<urho3d::LineEdit>();
                le->SetFixedSize(w, h);
                le->SetStyleAuto();
                return le;
            };

        auto MakeSlider = [&](urho3d::UIElement* parent,
            int w, int h,
            float range, float value) -> urho3d::Slider*
            {
                auto* sl = parent->CreateChild<urho3d::Slider>();
                sl->SetFixedSize(w, h);
                sl->SetRange(range);
                sl->SetValue(value);
                sl->SetStyleAuto();
                return sl;
            };

        auto MakeCheckBoxInline = [&](urho3d::UIElement* parent,
            bool checked = false) -> urho3d::CheckBox*
            {
                auto* cb = parent->CreateChild<urho3d::CheckBox>();
                cb->SetStyleAuto();
                cb->SetFixedSize(20, 20); 
                cb->SetChecked(checked);
                return cb;
            };

        auto MakeAxisRow = [&](urho3d::UIElement* parent,
            const urho3d::String& labelText,
            float range, float value)
            -> std::tuple<urho3d::CheckBox*, urho3d::LineEdit*, urho3d::Slider*>
            {
                auto* row = MakeRow(parent);

                auto* cb = MakeCheckBoxInline(row);               
                auto* lbl = MakeLabel(row, labelText, 15);
                lbl->SetMinWidth(15);
                auto* edit = MakeLineEdit(row, 55, 25);           
                auto* sl = MakeSlider(row, 160, 25, range, value); 
                return { cb, edit, sl };
            };

        auto MakeIconButton = [&](urho3d::UIElement* parent,
            urho3d::Texture2D* tex,
            int size = 24) -> urho3d::Button*
            {
                auto* icon = parent->CreateChild<urho3d::BorderImage>();
                icon->SetFixedSize(size, size);
                icon->SetTexture(tex);
                auto* btn = icon->CreateChild<urho3d::Button>();
                btn->SetStyle("");
                btn->SetSize(size, size);
                btn->SetOpacity(0.0f);
                return btn;
            };

        auto MakeBrowseRow = [&](urho3d::UIElement* parent,
            const urho3d::String& labelText, int editWidth = 480)
            -> std::pair<urho3d::LineEdit*, urho3d::Button*>
            {
                auto* row = MakeRow(parent);
                MakeLabel(row, labelText);
                auto* edit = MakeLineEdit(row, editWidth, 24);
                auto* btn = MakeIconButton(row, folderTex);
                return { edit, btn };
            };

        auto [ml, mb] = MakeBrowseRow(window_, "MDL:");
        auto [xl, xb] = MakeBrowseRow(window_, "XML:");
        mdlLine_ = ml;
        xmlLine_ = xl;

        auto* loadBtn = MakeButton(window_, "Load", 170, 30);
        loadBtn->SetAlignment(urho3d::HA_CENTER, urho3d::VA_TOP);

        auto* deleteBtn = MakeButton(window_, "Delete", 170, 30);
        deleteBtn->SetAlignment(urho3d::HA_CENTER, urho3d::VA_TOP);

        //── Panel ─────────────────────────────────────────────
        MakeLabel(panel_, "\nLocation");
        std::tie(x_Neg, x_Edit, x_PosSlider) = MakeAxisRow(panel_, " X", 25.0f, 0.5f);
        std::tie(y_Neg, y_Edit, y_PosSlider) = MakeAxisRow(panel_, " Y", 25.0f, 0.5f);
        std::tie(z_Neg, z_Edit, z_PosSlider) = MakeAxisRow(panel_, " Z", 25.0f, 0.5f);
        auto* locButtonApply = MakeButton(panel_, "Apply", 170, 30);
        locButtonApply->SetAlignment(urho3d::HA_CENTER, urho3d::VA_TOP);

        MakeLabel(panel_, "\nScale");
        std::tie(x_ScaleNeg, x_ScaleEdit, x_ScaleSlider) = MakeAxisRow(panel_, " X", 25.0f, 0.5f);
        std::tie(y_ScaleNeg, y_ScaleEdit, y_ScaleSlider) = MakeAxisRow(panel_, " Y", 25.0f, 0.5f);
        std::tie(z_ScaleNeg, z_ScaleEdit, z_ScaleSlider) = MakeAxisRow(panel_, " Z", 25.0f, 0.5f);
        auto* scaleButtonApply = MakeButton(panel_, "Apply", 170, 30);
        scaleButtonApply->SetAlignment(urho3d::HA_CENTER, urho3d::VA_TOP);

        MakeLabel(panel_, "\nRotation");
        std::tie(x_RotationNeg, x_RotationEdit, x_RotationSlider) = MakeAxisRow(panel_, " X", 25.0f, 0.5f);
        std::tie(y_RotationNeg, y_RotationEdit, y_RotationSlider) = MakeAxisRow(panel_, " Y", 25.0f, 0.5f);
        std::tie(z_RotationNeg, z_RotationEdit, z_RotationSlider) = MakeAxisRow(panel_, " Z", 25.0f, 0.5f);
        auto* rotationButtonApply = MakeButton(panel_, "Apply", 170, 30);
        rotationButtonApply->SetAlignment(urho3d::HA_CENTER, urho3d::VA_TOP);
              
        // ── Подписки ─────────────────────────────────────────────
        //load
        SubscribeToEvent(loadBtn, urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleButtonPress));
        SubscribeToEvent(mb,      urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleMDLButtonPress));
        SubscribeToEvent(xb,      urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleXMLButtonPress));

        SubscribeToEvent(deleteBtn, urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleDeleteButtonPress));


        //pos
        SubscribeToEvent(x_Neg,          urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(y_Neg,          urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(z_Neg,          urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(x_PosSlider,    urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleTransform));
        SubscribeToEvent(y_PosSlider,    urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleTransform));
        SubscribeToEvent(z_PosSlider,    urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleTransform));

        SubscribeToEvent(locButtonApply, urho3d::E_RELEASED,      URHO3D_HANDLER(StaticSceneApp, HandleLocationButtonApply));

        //scale
        SubscribeToEvent(x_ScaleNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(y_ScaleNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(z_ScaleNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(x_ScaleSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleScaleTransform));
        SubscribeToEvent(y_ScaleSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleScaleTransform));
        SubscribeToEvent(z_ScaleSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleScaleTransform));

        SubscribeToEvent(scaleButtonApply, urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleScaleButtonApply));

        //rotation
        SubscribeToEvent(x_RotationNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(y_RotationNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(z_RotationNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
        SubscribeToEvent(x_RotationSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleRotationTransform));
        SubscribeToEvent(y_RotationSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleRotationTransform));
        SubscribeToEvent(z_RotationSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleRotationTransform));

        SubscribeToEvent(rotationButtonApply, urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleRotationButtonApply));
    }

    void ShowGlobalValues()
    {
        auto* ui = GetSubsystem<urho3d::UI>();
        auto* cache = GetSubsystem<urho3d::ResourceCache>();
        auto* font = cache->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf");
        demOfCurrPos = ui->GetRoot()->CreateChild<urho3d::Text>();
        demOfCurrPos->SetFont(font, 14);
        demOfCurrPos->SetColor(urho3d::Color::WHITE);
        demOfCurrPos->SetAlignment(urho3d::HA_LEFT, urho3d::VA_BOTTOM);
        demOfCurrPos->SetPosition(urho3d::HA_LEFT+3, urho3d::VA_BOTTOM-10);
        demOfCurrPos->SetText("");
    }

    void SetNormalizedScale(urho3d::Node* node, float targetSize)
    {
        auto* modelComponent = node->GetComponent<urho3d::StaticModel>();
        if (!modelComponent || !modelComponent->GetModel()) return;
        urho3d::BoundingBox box = modelComponent->GetModel()->GetBoundingBox();
        urho3d::Vector3 size = box.Size();

        float maxDim = urho3d::Max(size.x_, urho3d::Max(size.y_, size.z_));

        if (maxDim > 0.0f)
        {
            float scale = targetSize / maxDim;
            node->SetScale(scale);
        }
    }
    urho3d::Node* loadMDLObject(urho3d::ResourceCache* cache, urho3d::String pathToXML, urho3d::String pathToMDL)
    {
        if (!cache) return nullptr;
        urho3d::Node* node = scene_->CreateChild("Object");
        urho3d::Vector3 spawnPos = defaultObjPos;
        if (pathToMDL == "Models/Box.mdl") spawnPos.y_ = 0.5f;
        node->SetPosition(spawnPos);

        auto* tObject = node->CreateComponent<urho3d::StaticModel>();
        tObject->SetModel(cache->GetResource<urho3d::Model>(pathToMDL));
        tObject->SetMaterial(cache->GetResource<urho3d::Material>(pathToXML));

        node->CreateComponent<urho3d::RigidBody>();
        auto* shape = node->CreateComponent<urho3d::CollisionShape>();
        shape->SetTriangleMesh(tObject->GetModel());
        SetNormalizedScale(node, 1.0f);

        return node;
    }
    void SetupLight()
    {
        auto* cache = GetSubsystem<Urho3D::ResourceCache>();

        Urho3D::Node* zoneNode = scene_->CreateChild("Zone");
        auto* zone = zoneNode->CreateComponent<Urho3D::Zone>();
        zone->SetBoundingBox(Urho3D::BoundingBox(-1000.0f, 1000.0f));
        zone->SetAmbientColor(Urho3D::Color(0.2f, 0.2f, 0.2f));
        zone->SetFogColor(Urho3D::Color(0.2f, 0.2f, 0.2f));
        zone->SetFogStart(10.0f);
        zone->SetFogEnd(25.0f);

        //key light
        Urho3D::Node* keyNode = scene_->CreateChild("KeyLight");
        keyNode->SetDirection(Urho3D::Vector3(-0.5f, -1.0f, 0.8f).Normalized());
        auto* key = keyNode->CreateComponent<Urho3D::Light>();
        key->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
        key->SetColor(Urho3D::Color(1.0f, 0.95f, 0.9f));
        key->SetCastShadows(true);
        key->SetBrightness(1.0f);

        //fill light
        Urho3D::Node* fillNode = scene_->CreateChild("FillLight");
        fillNode->SetDirection(Urho3D::Vector3(0.8f, -0.3f, -0.6f).Normalized());
        auto* fill = fillNode->CreateComponent<Urho3D::Light>();
        fill->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
        fill->SetColor(Urho3D::Color(0.6f, 0.7f, 0.9f)); 
        fill->SetCastShadows(false);
        fill->SetBrightness(0.6f);

        // rim light
        Urho3D::Node* rimNode = scene_->CreateChild("RimLight");
        rimNode->SetDirection(Urho3D::Vector3(0.0f, 0.5f, -1.0f).Normalized());
        auto* rim = rimNode->CreateComponent<Urho3D::Light>();
        rim->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
        rim->SetColor(Urho3D::Color(0.9f, 0.9f, 1.0f));
        rim->SetCastShadows(false);
        rim->SetBrightness(0.4f);
    }

    void ShowControlElements()
    {
        auto* cache = GetSubsystem<urho3d::ResourceCache>();
        auto* ui = GetSubsystem<urho3d::UI>();
        auto* image = ui->GetRoot()->CreateChild<Urho3D::BorderImage>();
        image->SetTexture(cache->GetResource<Urho3D::Texture2D>("UI/icons/x4.png"));
        image->SetSize(30, 30);
        image->SetAlignment(Urho3D::HA_CENTER, Urho3D::VA_CENTER);
        image->SetPosition(0, 0);
    }

    void CreateScene()
    {
        auto* cache = GetSubsystem<urho3d::ResourceCache>();
        scene_ = new urho3d::Scene(context_);
        scene_->CreateComponent<urho3d::Octree>();
        scene_->CreateComponent<urho3d::PhysicsWorld>();
        scene_->CreateComponent<urho3d::DebugRenderer>();
  
        auto* ui = GetSubsystem<urho3d::UI>();
        SetupLight();
        ShowControlElements();

    /*===========================*/
        urho3d::Node* gridNode = scene_->CreateChild("Grid");
        auto* gridGeo      = gridNode->CreateComponent<Urho3D::CustomGeometry>();
        auto* gridGeoRed   = gridNode->CreateComponent<Urho3D::CustomGeometry>();
        auto* gridGeoGreen = gridNode->CreateComponent<Urho3D::CustomGeometry>();

        gridGeo->BeginGeometry(0, Urho3D::LINE_LIST);
        gridGeoRed->BeginGeometry(0, Urho3D::LINE_LIST);
        gridGeoGreen->BeginGeometry(0, Urho3D::LINE_LIST);

        for (int i = -50; i <= 50; ++i)
        {
            float val = (float)i;

            if (i == 0)
            {
                gridGeoGreen->DefineVertex(Urho3D::Vector3(val, 0.0f, -50.0f));
                gridGeoGreen->DefineVertex(Urho3D::Vector3(val, 0.0f, 50.0f));

                gridGeoRed->DefineVertex(Urho3D::Vector3(-50.0f, 0.0f, val));
                gridGeoRed->DefineVertex(Urho3D::Vector3( 50.0f, 0.0f, val));

                continue;
            }
            //Z
            gridGeo->DefineVertex(Urho3D::Vector3(val, 0.0f, -50.0f));
            gridGeo->DefineVertex(Urho3D::Vector3(val, 0.0f,  50.0f));
            //X
            gridGeo->DefineVertex(Urho3D::Vector3(-50.0f, 0.0f, val));
            gridGeo->DefineVertex(Urho3D::Vector3( 50.0f, 0.0f, val));
        }

        gridGeo->Commit();
        gridGeoRed->Commit();
        gridGeoGreen->Commit();
        //white
        Urho3D::SharedPtr<Urho3D::Material> mat(new Urho3D::Material(context_));
        mat->SetTechnique(0, cache->GetResource<Urho3D::Technique>("Techniques/NoTextureUnlit.xml"));
        mat->SetShaderParameter("MatDiffColor", Urho3D::Color(0.5f, 0.5f, 0.5f));

        //green
        Urho3D::SharedPtr<Urho3D::Material> colorMatGreen(new Urho3D::Material(context_));
        colorMatGreen->SetTechnique(0, cache->GetResource<Urho3D::Technique>("Techniques/NoTextureUnlit.xml"));
        colorMatGreen->SetShaderParameter("MatDiffColor", Urho3D::Color(0.0f, 1.0f, 0.0f));
        colorMatGreen->SetShaderParameter("MatEmissiveColor", Urho3D::Color(0.0f, 0.5f, 0.0f));

        //red
        Urho3D::SharedPtr<Urho3D::Material> colorMatRed(new Urho3D::Material(context_));
        colorMatRed->SetTechnique(0, cache->GetResource<Urho3D::Technique>("Techniques/NoTextureUnlit.xml"));
        colorMatRed->SetShaderParameter("MatDiffColor", Urho3D::Color(1.0f, 0.0f, 0.0f));
        colorMatRed->SetShaderParameter("MatEmissiveColor", Urho3D::Color(0.5f, 0.0f, 0.0f));

        gridGeo->SetMaterial(mat);
        gridGeoRed->SetMaterial(colorMatRed);
        gridGeoGreen->SetMaterial(colorMatGreen);
    /*===========================*/

        if (mdlLine_->GetText().Empty())
        {
            urho3d::Node* box = loadMDLObject(cache, "", "Models/Box.mdl");
            if (box) 
            { 
                objects_.Push({ urho3d::SharedPtr<urho3d::Node>(box), false });
                tNode_ = box; 
            }
        }
        cameraNode_ = scene_->CreateChild("Camera");
        cameraNode_->CreateComponent<urho3d::Camera>();
        cameraNode_->SetPosition(urho3d::Vector3(0.0f, 0.5f, -3.0f));
    }

    void CreateInstructions()
    {
        auto* cache = GetSubsystem<urho3d::ResourceCache>();
        auto* ui = GetSubsystem<urho3d::UI>();

        instructionText_ = ui->GetRoot()->CreateChild<urho3d::Text>();
        instructionText_->SetText(
            "Use WASD keys and mouse/touch to move\n"
            "Shift: slow\n"
            "X/Y/Z: snap camera to axis\n"
            "E/Q: up/down\n"
            "LBM: toggle debug geometry\n"
            "Alt: toggle this help"
        );
        instructionText_->SetFont(cache->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf"), 15);
        instructionText_->SetHorizontalAlignment(urho3d::HA_CENTER);
        instructionText_->SetVerticalAlignment(urho3d::VA_CENTER);
        instructionText_->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
    }

    void SetupViewport()
    {
        auto* renderer = GetSubsystem<urho3d::Renderer>();
        urho3d::SharedPtr<urho3d::Viewport> viewport(new urho3d::Viewport(context_, scene_, cameraNode_->GetComponent<urho3d::Camera>()));
        renderer->SetViewport(0, viewport);
    }

    void HandleCameraSetting(urho3d::StringHash eventType, urho3d::VariantMap& eventData)
    {
        if (!tNode_) return;
        if (GetSubsystem<urho3d::UI>()->GetFocusElement()) return;
        int code = eventData[Urho3D::KeyDown::P_KEY].GetI32();
        urho3d::Vector3 objPos = tNode_->GetPosition();

        switch (code)
        {
        case urho3d::KEY_X:
            cameraNode_->SetPosition(urho3d::Vector3(objPos.x_, objPos.y_ + 0.5f, objPos.z_ - 3.0f));
            yaw_ = 0.0f;
            pitch_ = 0.0f;
            break;
        case urho3d::KEY_Y:
            cameraNode_->SetPosition(urho3d::Vector3(objPos.x_, objPos.y_ + 3.0f, objPos.z_));
            yaw_ = 0.0001f;
            pitch_ = 90.0f;
            break;
        case urho3d::KEY_Z:
            cameraNode_->SetPosition(urho3d::Vector3(objPos.x_ + 3.0f, objPos.y_ + 0.5f, objPos.z_));
            yaw_ = -90.001f;
            pitch_ = 0.0f;
            break;
        }
    }

    void MoveCamera(float timeStep)
    {
        if (GetSubsystem<urho3d::UI>()->GetFocusElement()) return;
        auto* input = GetSubsystem<urho3d::Input>();

        float MOVE_SPEED        = 5.0f;
        float MOUSE_SENSITIVITY = 0.1f;

        if (input->GetKeyDown(urho3d::KEY_SHIFT))
        {
            MOVE_SPEED        = 1.0f;
            MOUSE_SENSITIVITY = 0.05f;
        }
        else
        {
            MOVE_SPEED        = 5.0f;
            MOUSE_SENSITIVITY = 0.1f;
        }

        urho3d::IntVector2 mouseMove = input->GetMouseMove();
        yaw_   += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = urho3d::Clamp(pitch_, -90.0f, 90.0f);

        cameraNode_->SetRotation(urho3d::Quaternion(pitch_, yaw_, 0.0f));

        if (input->GetKeyDown(urho3d::KEY_W)) cameraNode_->Translate(urho3d::Vector3::FORWARD * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(urho3d::KEY_S)) cameraNode_->Translate(urho3d::Vector3::BACK    * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(urho3d::KEY_A)) cameraNode_->Translate(urho3d::Vector3::LEFT    * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(urho3d::KEY_D)) cameraNode_->Translate(urho3d::Vector3::RIGHT   * MOVE_SPEED * timeStep);

        if (input->GetKeyDown(urho3d::KEY_Q)) cameraNode_->Translate(urho3d::Vector3::DOWN * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(urho3d::KEY_E)) cameraNode_->Translate(urho3d::Vector3::UP   * MOVE_SPEED * timeStep);
    }

    void HandleMouseWheel(Urho3D::StringHash, Urho3D::VariantMap& eventData)
    {
        float wheel = eventData[Urho3D::MouseWheel::P_WHEEL].GetFloat();
        cameraNode_->Translate(Urho3D::Vector3(0.0f, 0.0f, wheel * 0.5f));
    }

    void HandleDeleteButtonPress(urho3d::StringHash, urho3d::VariantMap& eventData)
    {
        if (!tNode_) return;
        unsigned i = 0;
        bool notexist = true;
        do
        {
            if (objects_[i].node == tNode_ && objects_[i].drawGeometry == true)
            {
                objects_.Erase(i);
                tNode_->Remove();
                tNode_ = nullptr;
                break;
            }
            ++i;
            if (i == objects_.Size()) notexist = false;
        } while (notexist);
    }

    void HandleButtonPress(urho3d::StringHash, urho3d::VariantMap&)
    {
        urho3d::String mdlPath = mdlLine_->GetText().Empty() ? "Models/Box.mdl" : mdlLine_->GetText();
        urho3d::String xmlPath = xmlLine_->GetText();
        urho3d::Node* newNode = loadMDLObject(GetSubsystem<urho3d::ResourceCache>(), xmlPath, mdlPath);
        if (newNode)
        {
            objects_.Push({ urho3d::SharedPtr<urho3d::Node>(newNode), false });
            tNode_ = newNode;
        }
        cameraNode_->SetPosition(urho3d::Vector3(0.0f, 0.5f, -3.0f));
        yaw_ = 0.0f;
        pitch_ = 0.0f;
    }

    void HandleMDLButtonPress(urho3d::StringHash, urho3d::VariantMap&)
    {
        nfdu8char_t* outPath = NULL;
        nfdfilteritem_t filter = { "Model files", "mdl" };

        nfdopendialogu8args_t args = { 0 };
        args.filterList = &filter;
        args.filterCount = 1;

        nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

        if (result == NFD_OKAY) 
        {
            mdlLine_->SetText(urho3d::String(outPath));
            NFD_FreePathU8(outPath);
        }
    }

    void HandleXMLButtonPress(urho3d::StringHash, urho3d::VariantMap&)
    {
        nfdu8char_t* outPath = NULL;
        nfdfilteritem_t filter = { "Material files", "xml" };

        nfdopendialogu8args_t args = { 0 };
        args.filterList = &filter;
        args.filterCount = 1;

        nfdresult_t result = NFD_OpenDialogU8_With(&outPath, &args);

        if (result == NFD_OKAY) {
            xmlLine_->SetText(urho3d::String(outPath));
            NFD_FreePathU8(outPath);
        }
    }

    void HandleCheckBox(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
    {
        bool checked = eventData[urho3d::Toggled::P_STATE].GetBool();
        auto* sender = static_cast<urho3d::CheckBox*>(eventData[urho3d::Toggled::P_ELEMENT].GetPtr());

        if (sender == x_Neg)
        {  
            x_nagativeCoord = !x_nagativeCoord;
            isProgrammaticChange = true;
            x_PosSlider->SetValue(0.0f);
            lastX_ = 0.0f;
            isProgrammaticChange = false;
        }
        if (sender == y_Neg)
        {
            y_nagativeCoord = !y_nagativeCoord;
            isProgrammaticChange = true; 
            y_PosSlider->SetValue(0.0f);
            lastY_ = 0.0f;
            isProgrammaticChange = false; 
        }
        if (sender == z_Neg)
        {
            z_nagativeCoord = !z_nagativeCoord;
            isProgrammaticChange = true; 
            z_PosSlider->SetValue(0.0f);
            lastZ_ = 0.0f;
            isProgrammaticChange = false;
        }
        if (sender == x_ScaleNeg)
        {
            x_nagativeScale = !x_nagativeScale;
            isProgrammaticChangeS = true;
            x_ScaleSlider->SetValue(0.0f);
            lastScaleX_ = 0.0f;
            isProgrammaticChangeS = false;
        }
        if (sender == y_ScaleNeg)
        {
            y_nagativeScale = !y_nagativeScale;
            isProgrammaticChangeS = true;
            y_ScaleSlider->SetValue(0.0f);
            lastScaleY_ = 0.0f;
            isProgrammaticChangeS = false;
        }
        if (sender == z_ScaleNeg)
        {
            z_nagativeScale = !z_nagativeScale;
            isProgrammaticChangeS = true;
            z_ScaleSlider->SetValue(0.0f);
            lastScaleZ_ = 0.0f;
            isProgrammaticChangeS = false;
        }
        if (sender == x_RotationNeg)
        {
            x_nagativeRotation = !x_nagativeRotation;
            isProgrammaticChangeR = true;
            x_RotationSlider->SetValue(0.0f);
            lastRotationX_ = 0.0f;
            isProgrammaticChangeR = false;
        }
        if (sender == y_RotationNeg)
        {
            y_nagativeRotation = !y_nagativeRotation;
            isProgrammaticChangeR = true;
            y_RotationSlider->SetValue(0.0f);
            lastRotationY_ = 0.0f;
            isProgrammaticChangeR = false;
        }
        if (sender == z_RotationNeg)
        {
            z_nagativeRotation = !z_nagativeRotation;
            isProgrammaticChangeR = true;
            z_RotationSlider->SetValue(0.0f);
            lastRotationZ_ = 0.0f;
            isProgrammaticChangeR = false;
        }
    }

    void HandleTransform(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
    {
        if (!tNode_) return;
        if (isProgrammaticChange) return;
        auto* slider = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
        float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();

        urho3d::Vector3 pos = tNode_->GetPosition();

        if (slider == x_PosSlider) 
        {
            float delta = currentValue - lastX_;
            if (x_nagativeCoord) delta = -delta;
            tNode_->SetPosition({ pos.x_ + delta, pos.y_, pos.z_ });
            lastX_ = currentValue; 

            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", tNode_->GetPosition().x_);
            x_Edit->SetText(urho3d::String(buf));
        }
        else if (slider == y_PosSlider) 
        {
            float delta = currentValue - lastY_;
            if (y_nagativeCoord) delta = -delta;
            tNode_->SetPosition({ pos.x_, pos.y_ + delta, pos.z_ });
            lastY_ = currentValue;

            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", tNode_->GetPosition().y_);
            y_Edit->SetText(urho3d::String(buf));
        }
        else if (slider == z_PosSlider) 
        {
            float delta = currentValue - lastZ_;
            if (z_nagativeCoord) delta = -delta;
            tNode_->SetPosition({ pos.x_, pos.y_, pos.z_ + delta });
            lastZ_ = currentValue;

            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", tNode_->GetPosition().z_);
            z_Edit->SetText(urho3d::String(buf));
        }
    }

    void HandleScaleTransform(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
    {
        if (!tNode_) return;
        if (isProgrammaticChangeS) return;

        auto* slider = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
        float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();

        urho3d::Vector3 scale = tNode_->GetScale();
        if (slider == x_ScaleSlider)
        {
            float delta = currentValue - lastScaleX_;
            if (x_nagativeScale) delta = -delta;
            tNode_->SetScale({ scale.x_ + delta, scale.y_, scale.z_ });
            lastScaleX_ = currentValue;
            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", tNode_->GetScale().x_);
            x_ScaleEdit->SetText(urho3d::String(buf));
        }
        else if (slider == y_ScaleSlider)
        {
            float delta = currentValue - lastScaleY_;
            if (y_nagativeScale) delta = -delta;
            tNode_->SetScale({ scale.x_, scale.y_ + delta, scale.z_ });
            lastScaleY_ = currentValue;
            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", tNode_->GetScale().y_);
            y_ScaleEdit->SetText(urho3d::String(buf));
        }
        else if (slider == z_ScaleSlider)
        {
            float delta = currentValue - lastScaleZ_;
            if (z_nagativeScale) delta = -delta;
            tNode_->SetScale({ scale.x_, scale.y_, scale.z_ + delta });
            lastScaleZ_ = currentValue;
            char buf[8];
            snprintf(buf, sizeof(buf), "%.2f", tNode_->GetScale().z_);
            z_ScaleEdit->SetText(urho3d::String(buf));
        }
    }

    void HandleRotationTransform(Urho3D::StringHash, Urho3D::VariantMap& eventData)
    {
        if (!tNode_) return;
        if (isProgrammaticChangeR) return;

        auto* slider = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
        float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();

        urho3d::Vector3 euler = tNode_->GetRotation().EulerAngles();

        if (slider == x_RotationSlider)
        {
            float delta = currentValue - lastRotationX_;
            if (x_nagativeRotation) delta = -delta;
            euler.x_ += delta;
            tNode_->SetRotation(urho3d::Quaternion(euler.x_, euler.y_, euler.z_));
            lastRotationX_ = currentValue;
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", euler.x_);
            x_RotationEdit->SetText(urho3d::String(buf));
        }
        else if (slider == y_RotationSlider)
        {
            float delta = currentValue - lastRotationY_;
            if (y_nagativeRotation) delta = -delta;
            euler.y_ += delta;
            tNode_->SetRotation(urho3d::Quaternion(euler.x_, euler.y_, euler.z_));
            lastRotationY_ = currentValue;
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", euler.y_);
            y_RotationEdit->SetText(urho3d::String(buf));
        }
        else if (slider == z_RotationSlider)
        {
            float delta = currentValue - lastRotationZ_;
            if (z_nagativeRotation) delta = -delta;
            euler.z_ += delta;
            tNode_->SetRotation(urho3d::Quaternion(euler.x_, euler.y_, euler.z_));
            lastRotationZ_ = currentValue;
            char buf[16];
            snprintf(buf, sizeof(buf), "%.2f", euler.z_);
            z_RotationEdit->SetText(urho3d::String(buf));
        }
    }

    void HandleLocationButtonApply(urho3d::StringHash eventType, urho3d::VariantMap& eventData)
    {
        if (!tNode_) return;
        urho3d::String xtext = x_Edit->GetText();
        urho3d::String ytext = y_Edit->GetText();
        urho3d::String ztext = z_Edit->GetText();

        xtext.Replace(',', '.');
        ytext.Replace(',', '.');
        ztext.Replace(',', '.');

        float x = atof(xtext.CString());
        float y = ytext.Empty() ? 0.5f : atof(ytext.CString());
        float z = atof(ztext.CString());

        tNode_->SetPosition(urho3d::Vector3(x, y, z));
        lastX_ = x;
        lastY_ = y;
        lastZ_ = z;
    }

    void HandleScaleButtonApply(urho3d::StringHash eventType, urho3d::VariantMap& eventData)
    {
        if (!tNode_) return;
        urho3d::String xtext = x_ScaleEdit->GetText();
        urho3d::String ytext = y_ScaleEdit->GetText();
        urho3d::String ztext = z_ScaleEdit->GetText();

        xtext.Replace(',', '.');
        ytext.Replace(',', '.');
        ztext.Replace(',', '.');

        float x = xtext.Empty() ? 1.0f : atof(xtext.CString());
        float y = ytext.Empty() ? 1.0f : atof(ytext.CString());
        float z = ztext.Empty() ? 1.0f : atof(ztext.CString());

        tNode_->SetScale(urho3d::Vector3(x, y, z));
        lastScaleX_ = x;
        lastScaleY_ = y;
        lastScaleZ_ = z;
    }

    void HandleRotationButtonApply(urho3d::StringHash eventType, urho3d::VariantMap& eventData)
    {
        if (!tNode_) return;
        urho3d::String xtext = x_RotationEdit->GetText();
        urho3d::String ytext = y_RotationEdit->GetText();
        urho3d::String ztext = z_RotationEdit->GetText();

        xtext.Replace(',', '.');
        ytext.Replace(',', '.');
        ztext.Replace(',', '.');

        float x = xtext.Empty() ? 0.0f : atof(xtext.CString());
        float y = ytext.Empty() ? 0.0f : atof(ytext.CString());
        float z = ztext.Empty() ? 0.0f : atof(ztext.CString());

        tNode_->SetRotation(urho3d::Quaternion(x, y, z));
        lastRotationX_ = x;
        lastRotationY_ = y;
        lastRotationZ_ = z;
    }
    
    urho3d::Node* PickObject(float maxDistance)
    {
        urho3d::Ray ray(cameraNode_->GetWorldPosition(), cameraNode_->GetWorldDirection());

        urho3d::Vector<urho3d::RayQueryResult> results;
        urho3d::RayOctreeQuery query(results, ray, urho3d::RAY_TRIANGLE, maxDistance, urho3d::DrawableTypes::Geometry);
        scene_->GetComponent<urho3d::Octree>()->RaycastSingle(query);

        return results.Empty() ? nullptr : results[0].drawable_->GetNode();
    }

    void HandleUpdate(urho3d::StringHash eventType, urho3d::VariantMap& eventData)
    {
        using namespace urho3d::Update;
        float timeStep = eventData[P_TIMESTEP].GetFloat();
        bool typing = GetSubsystem<urho3d::UI>()->GetFocusElement() != nullptr;
        if(mauseVisibility == false) MoveCamera(timeStep);
        if (!typing && GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_TAB))
        {
            mauseVisibility = !mauseVisibility;
            bool isVisible = !GetSubsystem<urho3d::Input>()->IsMouseVisible();

            GetSubsystem<urho3d::Input>()->SetMouseVisible(isVisible);
            GetSubsystem<urho3d::Input>()->SetMouseMode(isVisible ? urho3d::MM_FREE : urho3d::MM_RELATIVE); 
        }
        if (!typing && GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_ALT))
        {
            instructionVisibility = !instructionVisibility;
            instructionText_->SetVisible(instructionVisibility);
        }

        if (!typing && !mauseVisibility && GetSubsystem<urho3d::Input>()->GetMouseButtonPress(urho3d::MOUSEB_LEFT))
        {
            urho3d::Node* clicked = PickObject(250.0f);
            if (clicked && clicked->GetName() == "Object")
            {
                tNode_ = clicked;
                for (auto& obj : objects_)
                {
                    if (obj.node == clicked)
                    {
                        obj.drawGeometry = !obj.drawGeometry;
                        break;
                    }
                }
            }
        }
        demOfCurrPos->SetText("X: \t" + urho3d::String(cameraNode_->GetPosition().x_) +
            "\nY: \t" + urho3d::String(cameraNode_->GetPosition().y_) +
            "\nZ: \t" + urho3d::String(cameraNode_->GetPosition().z_) +
            "\nyaw:\t" + urho3d::String(yaw_) + "\npitch:\t" + urho3d::String(pitch_));

        if (GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_ESCAPE)) engine_->Exit();
    }

    void HandleGeometryRender(urho3d::StringHash, urho3d::VariantMap&)
    {
        auto* debug = scene_->GetComponent<urho3d::DebugRenderer>();
        if (!debug) return;

        for (auto& obj : objects_)
        {
            if (!obj.drawGeometry || !obj.node) continue;

            auto* staticModel = obj.node->GetComponent<urho3d::StaticModel>();
            auto* model = staticModel ? staticModel->GetModel() : nullptr;
            if (!model) continue;

            urho3d::Geometry* geom = model->GetGeometry(0, 0);
            if (!geom) continue;

            auto* vb = geom->GetVertexBuffer(0);
            auto* ib = geom->GetIndexBuffer();
            if (!vb || !ib) continue;

            const unsigned char* vertexData = reinterpret_cast<const unsigned char*>(vb->GetShadowData());
            const unsigned char* indexData = reinterpret_cast<const unsigned char*>(ib->GetShadowData());
            if (!vertexData || !indexData) continue;

            debug->AddTriangleMesh(
                vertexData, vb->GetVertexSize(),
                indexData,  ib->GetIndexSize(),
                geom->GetIndexStart(), geom->GetIndexCount(),
                obj.node->GetWorldTransform(),
                urho3d::Color(1.0f, 0.6f, 0.0f),
                true);
        }
    }

    void Stop() override {}

private:
    urho3d::Node* tNode_ = nullptr;

    struct SceneObject
    {
        urho3d::SharedPtr<urho3d::Node> node;
        bool drawGeometry = false;
    };

    urho3d::Vector<SceneObject> objects_;

    urho3d::SharedPtr<urho3d::Scene> scene_;
    urho3d::Node* cameraNode_ = nullptr;
    float yaw_   = 0.0f;
    float pitch_ = 0.0f;

    urho3d::Window* window_ = nullptr;
    urho3d::Window* panel_  = nullptr;  
    urho3d::SharedPtr<urho3d::UIElement> uiRoot_;

    urho3d::LineEdit* mdlLine_ = nullptr;
    urho3d::LineEdit* xmlLine_ = nullptr;

   /*===MOVE===*/
    urho3d::Slider* x_PosSlider = nullptr;
    urho3d::Slider* y_PosSlider = nullptr;
    urho3d::Slider* z_PosSlider = nullptr;

    urho3d::LineEdit* x_Edit = nullptr;
    urho3d::LineEdit* y_Edit = nullptr;
    urho3d::LineEdit* z_Edit = nullptr;

    urho3d::CheckBox* x_Neg = nullptr;
    urho3d::CheckBox* y_Neg = nullptr;
    urho3d::CheckBox* z_Neg = nullptr;

    bool x_nagativeCoord = false;
    bool y_nagativeCoord = false;
    bool z_nagativeCoord = false;

    float lastX_ = 0.0f;
    float lastY_ = 0.0f;
    float lastZ_ = 0.0f;

    bool isProgrammaticChange = false;
   /*===MOVE===*/

   /*===SCALE===*/
    urho3d::Slider* x_ScaleSlider = nullptr;
    urho3d::Slider* y_ScaleSlider = nullptr;
    urho3d::Slider* z_ScaleSlider = nullptr;

    urho3d::LineEdit* x_ScaleEdit = nullptr;
    urho3d::LineEdit* y_ScaleEdit = nullptr;
    urho3d::LineEdit* z_ScaleEdit = nullptr;

    urho3d::CheckBox* x_ScaleNeg = nullptr;
    urho3d::CheckBox* y_ScaleNeg = nullptr;
    urho3d::CheckBox* z_ScaleNeg = nullptr;

    bool x_nagativeScale = false;
    bool y_nagativeScale = false;
    bool z_nagativeScale = false;

    float lastScaleX_ = 0.0f;
    float lastScaleY_ = 0.0f;
    float lastScaleZ_ = 0.0f;

    bool isProgrammaticChangeS = false;
   /*===SCALE===*/

  /*===ROTATION===*/
    urho3d::CheckBox* x_RotationNeg = nullptr;
    urho3d::CheckBox* y_RotationNeg = nullptr;
    urho3d::CheckBox* z_RotationNeg = nullptr;

    urho3d::LineEdit* x_RotationEdit = nullptr;
    urho3d::LineEdit* y_RotationEdit = nullptr;
    urho3d::LineEdit* z_RotationEdit = nullptr;

    urho3d::Slider* x_RotationSlider = nullptr;
    urho3d::Slider* y_RotationSlider = nullptr;
    urho3d::Slider* z_RotationSlider = nullptr;

    bool x_nagativeRotation = false;
    bool y_nagativeRotation = false;
    bool z_nagativeRotation = false;

    float lastRotationX_ = 0.0f;
    float lastRotationY_ = 0.0f;
    float lastRotationZ_ = 0.0f;

    bool isProgrammaticChangeR = false;
  /*===ROTATION===*/

    urho3d::Vector3 defaultObjPos    = { 0.0f, 0.0f, 0.0f };
    urho3d::Vector3 defaultСameraPos = { 0.0f, 0.0f, 0.0f };
    urho3d::Text* demOfCurrPos = nullptr;

    bool mauseVisibility = false;

    urho3d::Text* instructionText_ = nullptr;
    bool instructionVisibility = true;
};
URHO3D_DEFINE_APPLICATION_MAIN(StaticSceneApp)
