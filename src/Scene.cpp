#include "StaticSceneApp.h"

namespace urho3d = Urho3D;

void StaticSceneApp::SetupLight()
{
    auto* cache = GetSubsystem<Urho3D::ResourceCache>();

    Urho3D::Node* zoneNode = scene_->CreateChild("Zone");
    auto* zone = zoneNode->CreateComponent<Urho3D::Zone>();
    zone->SetBoundingBox(Urho3D::BoundingBox(-1000.0f, 1000.0f));
    zone->SetAmbientColor(Urho3D::Color(0.2f, 0.2f, 0.2f));
    zone->SetFogColor(Urho3D::Color(0.2f, 0.2f, 0.2f));
    zone->SetFogStart(10.0f);
    zone->SetFogEnd(25.0f);

    // Key light
    Urho3D::Node* keyNode = scene_->CreateChild("KeyLight");
    keyNode->SetDirection(Urho3D::Vector3(-0.5f, -1.0f, 0.8f).Normalized());
    auto* key = keyNode->CreateComponent<Urho3D::Light>();
    key->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
    key->SetColor(Urho3D::Color(1.0f, 0.95f, 0.9f));
    key->SetCastShadows(true);
    key->SetBrightness(1.0f);

    // Fill light
    Urho3D::Node* fillNode = scene_->CreateChild("FillLight");
    fillNode->SetDirection(Urho3D::Vector3(0.8f, -0.3f, -0.6f).Normalized());
    auto* fill = fillNode->CreateComponent<Urho3D::Light>();
    fill->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
    fill->SetColor(Urho3D::Color(0.6f, 0.7f, 0.9f));
    fill->SetCastShadows(false);
    fill->SetBrightness(0.6f);

    // Rim light
    Urho3D::Node* rimNode = scene_->CreateChild("RimLight");
    rimNode->SetDirection(Urho3D::Vector3(0.0f, 0.5f, -1.0f).Normalized());
    auto* rim = rimNode->CreateComponent<Urho3D::Light>();
    rim->SetLightType(Urho3D::LIGHT_DIRECTIONAL);
    rim->SetColor(Urho3D::Color(0.9f, 0.9f, 1.0f));
    rim->SetCastShadows(false);
    rim->SetBrightness(0.4f);
}

void StaticSceneApp::ShowControlElements()
{
    auto* cache = GetSubsystem<urho3d::ResourceCache>();
    auto* ui    = GetSubsystem<urho3d::UI>();
    auto* image = ui->GetRoot()->CreateChild<Urho3D::BorderImage>();
    image->SetTexture(cache->GetResource<Urho3D::Texture2D>("UI/icons/x4.png"));
    image->SetSize(30, 30);
    image->SetAlignment(Urho3D::HA_CENTER, Urho3D::VA_CENTER);
    image->SetPosition(0, 0);
}

void StaticSceneApp::CreateScene()
{
    auto* cache = GetSubsystem<urho3d::ResourceCache>();
    scene_ = new urho3d::Scene(context_);
    scene_->CreateComponent<urho3d::Octree>();
    scene_->CreateComponent<urho3d::PhysicsWorld>();
    scene_->CreateComponent<urho3d::DebugRenderer>();

    SetupLight();
    ShowControlElements();

    /*=== Grid ===*/
    urho3d::Node* gridNode    = scene_->CreateChild("Grid");
    auto* gridGeo             = gridNode->CreateComponent<Urho3D::CustomGeometry>();
    auto* gridGeoRed          = gridNode->CreateComponent<Urho3D::CustomGeometry>();
    auto* gridGeoGreen        = gridNode->CreateComponent<Urho3D::CustomGeometry>();

    gridGeo->BeginGeometry(0,      Urho3D::LINE_LIST);
    gridGeoRed->BeginGeometry(0,   Urho3D::LINE_LIST);
    gridGeoGreen->BeginGeometry(0, Urho3D::LINE_LIST);

    for (int i = -50; i <= 50; ++i)
    {
        float val = (float)i;
        if (i == 0)
        {
            gridGeoGreen->DefineVertex(Urho3D::Vector3(val,   0.0f, -50.0f));
            gridGeoGreen->DefineVertex(Urho3D::Vector3(val,   0.0f,  50.0f));
            gridGeoRed->DefineVertex(Urho3D::Vector3(-50.0f, 0.0f,  val));
            gridGeoRed->DefineVertex(Urho3D::Vector3( 50.0f, 0.0f,  val));
            continue;
        }
        gridGeo->DefineVertex(Urho3D::Vector3(val,   0.0f, -50.0f));
        gridGeo->DefineVertex(Urho3D::Vector3(val,   0.0f,  50.0f));
        gridGeo->DefineVertex(Urho3D::Vector3(-50.0f, 0.0f, val));
        gridGeo->DefineVertex(Urho3D::Vector3( 50.0f, 0.0f, val));
    }

    gridGeo->Commit();
    gridGeoRed->Commit();
    gridGeoGreen->Commit();

    // White grid material
    Urho3D::SharedPtr<Urho3D::Material> mat(new Urho3D::Material(context_));
    mat->SetTechnique(0, cache->GetResource<Urho3D::Technique>("Techniques/NoTextureUnlit.xml"));
    mat->SetShaderParameter("MatDiffColor", Urho3D::Color(0.5f, 0.5f, 0.5f));

    // Green (Z axis)
    Urho3D::SharedPtr<Urho3D::Material> colorMatGreen(new Urho3D::Material(context_));
    colorMatGreen->SetTechnique(0, cache->GetResource<Urho3D::Technique>("Techniques/NoTextureUnlit.xml"));
    colorMatGreen->SetShaderParameter("MatDiffColor",    Urho3D::Color(0.0f, 1.0f, 0.0f));
    colorMatGreen->SetShaderParameter("MatEmissiveColor", Urho3D::Color(0.0f, 0.5f, 0.0f));

    // Red (X axis)
    Urho3D::SharedPtr<Urho3D::Material> colorMatRed(new Urho3D::Material(context_));
    colorMatRed->SetTechnique(0, cache->GetResource<Urho3D::Technique>("Techniques/NoTextureUnlit.xml"));
    colorMatRed->SetShaderParameter("MatDiffColor",    Urho3D::Color(1.0f, 0.0f, 0.0f));
    colorMatRed->SetShaderParameter("MatEmissiveColor", Urho3D::Color(0.5f, 0.0f, 0.0f));

    gridGeo->SetMaterial(mat);
    gridGeoRed->SetMaterial(colorMatRed);
    gridGeoGreen->SetMaterial(colorMatGreen);
    /*=== Grid ===*/

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
