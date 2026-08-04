#pragma once
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/CustomGeometry.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/GraphicsAPI/IndexBuffer.h>
#include <Urho3D/GraphicsAPI/Texture2D.h>
#include <Urho3D/GraphicsAPI/VertexBuffer.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
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
#include <stdlib.h>

class StaticSceneApp : public Urho3D::Application
{
    URHO3D_OBJECT(StaticSceneApp, Urho3D::Application)
public:
    StaticSceneApp(Urho3D::Context* context);

    void Setup() override;
    void Start() override;
    void Stop()  override;

private:
    // ── Core ──────────────────────────────────────────────
    Urho3D::Window* MakeWindow(const Urho3D::String& title,
                               Urho3D::HorizontalAlignment ha,
                               Urho3D::VerticalAlignment   va,
                               int minWidth = 370);
    void SetupViewport();

    // ── Scene ─────────────────────────────────────────────
    void CreateScene();
    void SetupLight();
    void ShowControlElements();

    // ── UI ────────────────────────────────────────────────
    void InitWindow();
    void InitControls();
    void ShowGlobalValues();
    void CreateInstructions();

    // ── Utils ─────────────────────────────────────────────
    Urho3D::Node* loadMDLObject(Urho3D::ResourceCache* cache,
                                Urho3D::String pathToXML,
                                Urho3D::String pathToMDL);
    void SetNormalizedScale(Urho3D::Node* node, float targetSize);
    void TakenOutFuncForDeletObj();
    Urho3D::Node* PickObject(float maxDistance);

    // ── Handlers ──────────────────────────────────────────
    void MoveCamera(float timeStep);
    void HandleUpdate         (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleMouseWheel     (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleCameraSetting  (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleButtonPress    (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleDeleteButtonPress(Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleMDLButtonPress (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleXMLButtonPress (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleCheckBox       (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleTransform      (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleScaleTransform (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleRotationTransform(Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleColorTransform (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleLocationButtonApply(Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleScaleButtonApply   (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleRotationButtonApply(Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleColorButtonApply   (Urho3D::StringHash, Urho3D::VariantMap& eventData);
    void HandleGeometryRender (Urho3D::StringHash, Urho3D::VariantMap& eventData);

    // ── Scene state ───────────────────────────────────────
    Urho3D::SharedPtr<Urho3D::Scene>     scene_;
    Urho3D::Node*                        cameraNode_ = nullptr;
    Urho3D::Node*                        tNode_      = nullptr;

    struct SceneObject
    {
        Urho3D::SharedPtr<Urho3D::Node> node;
        bool drawGeometry = false;
    };
    Urho3D::Vector<SceneObject> objects_;

    float yaw_   = 0.0f;
    float pitch_ = 0.0f;
    bool  mauseVisibility = false;

    Urho3D::Vector3 defaultObjPos = { 0.0f, 0.0f, 0.0f };

    // ── UI state ──────────────────────────────────────────
    Urho3D::SharedPtr<Urho3D::UIElement> uiRoot_;
    Urho3D::Window*   window_     = nullptr;
    Urho3D::Window*   panel_      = nullptr;
    Urho3D::Window*   colorPanel_ = nullptr;

    Urho3D::LineEdit* mdlLine_ = nullptr;
    Urho3D::LineEdit* xmlLine_ = nullptr;
    Urho3D::Text*     demOfCurrPos    = nullptr;
    Urho3D::Text*     instructionText_ = nullptr;
    bool              instructionVisibility = true;

    // Position
    Urho3D::CheckBox* x_Neg = nullptr; Urho3D::LineEdit* x_Edit = nullptr; Urho3D::Slider* x_PosSlider = nullptr;
    Urho3D::CheckBox* y_Neg = nullptr; Urho3D::LineEdit* y_Edit = nullptr; Urho3D::Slider* y_PosSlider = nullptr;
    Urho3D::CheckBox* z_Neg = nullptr; Urho3D::LineEdit* z_Edit = nullptr; Urho3D::Slider* z_PosSlider = nullptr;
    bool  x_nagativeCoord = false, y_nagativeCoord = false, z_nagativeCoord = false;
    float lastX_ = 0.0f, lastY_ = 0.0f, lastZ_ = 0.0f;
    bool  isProgrammaticChange = false;

    // Scale
    Urho3D::CheckBox* x_ScaleNeg = nullptr; Urho3D::LineEdit* x_ScaleEdit = nullptr; Urho3D::Slider* x_ScaleSlider = nullptr;
    Urho3D::CheckBox* y_ScaleNeg = nullptr; Urho3D::LineEdit* y_ScaleEdit = nullptr; Urho3D::Slider* y_ScaleSlider = nullptr;
    Urho3D::CheckBox* z_ScaleNeg = nullptr; Urho3D::LineEdit* z_ScaleEdit = nullptr; Urho3D::Slider* z_ScaleSlider = nullptr;
    bool  x_nagativeScale = false, y_nagativeScale = false, z_nagativeScale = false;
    float lastScaleX_ = 0.0f, lastScaleY_ = 0.0f, lastScaleZ_ = 0.0f;
    bool  isProgrammaticChangeS = false;

    // Rotation
    Urho3D::CheckBox* x_RotationNeg = nullptr; Urho3D::LineEdit* x_RotationEdit = nullptr; Urho3D::Slider* x_RotationSlider = nullptr;
    Urho3D::CheckBox* y_RotationNeg = nullptr; Urho3D::LineEdit* y_RotationEdit = nullptr; Urho3D::Slider* y_RotationSlider = nullptr;
    Urho3D::CheckBox* z_RotationNeg = nullptr; Urho3D::LineEdit* z_RotationEdit = nullptr; Urho3D::Slider* z_RotationSlider = nullptr;
    bool  x_nagativeRotation = false, y_nagativeRotation = false, z_nagativeRotation = false;
    float lastRotationX_ = 0.0f, lastRotationY_ = 0.0f, lastRotationZ_ = 0.0f;
    bool  isProgrammaticChangeR = false;

    // Color
    Urho3D::LineEdit* r_Edit = nullptr; Urho3D::Slider* r_Slider = nullptr;
    Urho3D::LineEdit* g_Edit = nullptr; Urho3D::Slider* g_Slider = nullptr;
    Urho3D::LineEdit* b_Edit = nullptr; Urho3D::Slider* b_Slider = nullptr;
    Urho3D::LineEdit* a_Edit = nullptr; Urho3D::Slider* a_Slider = nullptr;
    Urho3D::Vector4   color  = { 0.0f, 0.0f, 0.0f, 1.0f };
};
