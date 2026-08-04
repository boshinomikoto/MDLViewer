#include "StaticSceneApp.h"
#include <nfd.hpp>

namespace urho3d = Urho3D;

// ─── Camera ──────────────────────────────────────────────────────────────────

void StaticSceneApp::MoveCamera(float timeStep)
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

    urho3d::IntVector2 mouseMove = input->GetMouseMove();
    yaw_   += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    pitch_  = urho3d::Clamp(pitch_, -90.0f, 90.0f);
    cameraNode_->SetRotation(urho3d::Quaternion(pitch_, yaw_, 0.0f));

    if (input->GetKeyDown(urho3d::KEY_W)) cameraNode_->Translate(urho3d::Vector3::FORWARD * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(urho3d::KEY_S)) cameraNode_->Translate(urho3d::Vector3::BACK    * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(urho3d::KEY_A)) cameraNode_->Translate(urho3d::Vector3::LEFT    * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(urho3d::KEY_D)) cameraNode_->Translate(urho3d::Vector3::RIGHT   * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(urho3d::KEY_Q)) cameraNode_->Translate(urho3d::Vector3::DOWN    * MOVE_SPEED * timeStep);
    if (input->GetKeyDown(urho3d::KEY_E)) cameraNode_->Translate(urho3d::Vector3::UP      * MOVE_SPEED * timeStep);
}

void StaticSceneApp::HandleCameraSetting(urho3d::StringHash, urho3d::VariantMap& eventData)
{
    if (!tNode_) return;
    if (GetSubsystem<urho3d::UI>()->GetFocusElement()) return;

    int code = eventData[Urho3D::KeyDown::P_KEY].GetI32();
    urho3d::Vector3 objPos = tNode_->GetPosition();

    switch (code)
    {
    case urho3d::KEY_X:
        cameraNode_->SetPosition(urho3d::Vector3(objPos.x_, objPos.y_ + 0.5f, objPos.z_ - 3.0f));
        yaw_ = 0.0f; pitch_ = 0.0f;
        break;
    case urho3d::KEY_Y:
        cameraNode_->SetPosition(urho3d::Vector3(objPos.x_, objPos.y_ + 3.0f, objPos.z_));
        yaw_ = 0.0001f; pitch_ = 90.0f;
        break;
    case urho3d::KEY_Z:
        cameraNode_->SetPosition(urho3d::Vector3(objPos.x_ + 3.0f, objPos.y_ + 0.5f, objPos.z_));
        yaw_ = -90.001f; pitch_ = 0.0f;
        break;
    }
}

void StaticSceneApp::HandleMouseWheel(Urho3D::StringHash, Urho3D::VariantMap& eventData)
{
    float wheel = eventData[Urho3D::MouseWheel::P_WHEEL].GetFloat();
    cameraNode_->Translate(Urho3D::Vector3(0.0f, 0.0f, wheel * 0.5f));
}

// ─── Update loop ──────────────────────────────────────────────────────────────

void StaticSceneApp::HandleUpdate(urho3d::StringHash, urho3d::VariantMap& eventData)
{
    using namespace urho3d::Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    bool  typing   = GetSubsystem<urho3d::UI>()->GetFocusElement() != nullptr;

    if (!mauseVisibility) MoveCamera(timeStep);

    if (!typing && GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_TAB))
    {
        mauseVisibility = !mauseVisibility;
        bool isVisible  = !GetSubsystem<urho3d::Input>()->IsMouseVisible();
        GetSubsystem<urho3d::Input>()->SetMouseVisible(isVisible);
        GetSubsystem<urho3d::Input>()->SetMouseMode(isVisible ? urho3d::MM_FREE : urho3d::MM_RELATIVE);
    }

    if (!typing && GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_ALT))
    {
        instructionVisibility = !instructionVisibility;
        instructionText_->SetVisible(instructionVisibility);
    }

    if (!typing && GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_DELETE))
        TakenOutFuncForDeletObj();

    if (!typing && !mauseVisibility && GetSubsystem<urho3d::Input>()->GetMouseButtonPress(urho3d::MOUSEB_LEFT))
    {
        urho3d::Node* clicked = PickObject(250.0f);
        if (clicked && clicked->GetName() == "Object")
        {
            tNode_ = clicked;
            for (auto& obj : objects_)
            {
                if (obj.node == clicked) { obj.drawGeometry = !obj.drawGeometry; break; }
            }
        }
    }

    demOfCurrPos->SetText(
        "X: \t"     + urho3d::String(cameraNode_->GetPosition().x_) +
        "\nY: \t"   + urho3d::String(cameraNode_->GetPosition().y_) +
        "\nZ: \t"   + urho3d::String(cameraNode_->GetPosition().z_) +
        "\nyaw:\t"  + urho3d::String(yaw_) +
        "\npitch:\t" + urho3d::String(pitch_) +
        " objs:\t"  + urho3d::String(objects_.Size()));

    if (GetSubsystem<urho3d::Input>()->GetKeyPress(urho3d::KEY_ESCAPE))
        engine_->Exit();
}

// ─── Load / Delete ───────────────────────────────────────────────────────────

void StaticSceneApp::HandleButtonPress(urho3d::StringHash, urho3d::VariantMap&)
{
    urho3d::String mdlPath = mdlLine_->GetText().Empty() ? "Models/Box.mdl" : mdlLine_->GetText();
    urho3d::String xmlPath = xmlLine_->GetText();
    urho3d::Node*  newNode = loadMDLObject(GetSubsystem<urho3d::ResourceCache>(), xmlPath, mdlPath);

    if (objects_.Size() == 0)
    {
        cameraNode_->SetPosition(urho3d::Vector3(0.0f, 0.5f, -3.0f));
        yaw_ = 0.0f; pitch_ = 0.0f;
    }
    if (newNode)
    {
        objects_.Push({ urho3d::SharedPtr<urho3d::Node>(newNode), false });
        tNode_ = newNode;
    }
}

void StaticSceneApp::HandleDeleteButtonPress(urho3d::StringHash, urho3d::VariantMap&)
{
    TakenOutFuncForDeletObj();
}

void StaticSceneApp::HandleMDLButtonPress(urho3d::StringHash, urho3d::VariantMap&)
{
    nfdu8char_t*      outPath = nullptr;
    nfdfilteritem_t   filter  = { "Model files", "mdl" };
    nfdopendialogu8args_t args = {};
    args.filterList  = &filter;
    args.filterCount = 1;

    if (NFD_OpenDialogU8_With(&outPath, &args) == NFD_OKAY)
    {
        mdlLine_->SetText(urho3d::String(outPath));
        NFD_FreePathU8(outPath);
    }
}

void StaticSceneApp::HandleXMLButtonPress(urho3d::StringHash, urho3d::VariantMap&)
{
    nfdu8char_t*      outPath = nullptr;
    nfdfilteritem_t   filter  = { "Material files", "xml" };
    nfdopendialogu8args_t args = {};
    args.filterList  = &filter;
    args.filterCount = 1;

    if (NFD_OpenDialogU8_With(&outPath, &args) == NFD_OKAY)
    {
        xmlLine_->SetText(urho3d::String(outPath));
        NFD_FreePathU8(outPath);
    }
}

// ─── CheckBox ────────────────────────────────────────────────────────────────

void StaticSceneApp::HandleCheckBox(Urho3D::StringHash, Urho3D::VariantMap& eventData)
{
    auto* sender = static_cast<urho3d::CheckBox*>(eventData[urho3d::Toggled::P_ELEMENT].GetPtr());

    auto resetSlider = [](bool& flag, urho3d::Slider* sl, float& last, bool& guard)
    {
        flag  = !flag;
        guard = true;
        sl->SetValue(0.0f);
        last  = 0.0f;
        guard = false;
    };

    if      (sender == x_Neg)        resetSlider(x_nagativeCoord,    x_PosSlider,      lastX_,         isProgrammaticChange);
    else if (sender == y_Neg)        resetSlider(y_nagativeCoord,    y_PosSlider,      lastY_,         isProgrammaticChange);
    else if (sender == z_Neg)        resetSlider(z_nagativeCoord,    z_PosSlider,      lastZ_,         isProgrammaticChange);
    else if (sender == x_ScaleNeg)   resetSlider(x_nagativeScale,    x_ScaleSlider,    lastScaleX_,    isProgrammaticChangeS);
    else if (sender == y_ScaleNeg)   resetSlider(y_nagativeScale,    y_ScaleSlider,    lastScaleY_,    isProgrammaticChangeS);
    else if (sender == z_ScaleNeg)   resetSlider(z_nagativeScale,    z_ScaleSlider,    lastScaleZ_,    isProgrammaticChangeS);
    else if (sender == x_RotationNeg) resetSlider(x_nagativeRotation, x_RotationSlider, lastRotationX_, isProgrammaticChangeR);
    else if (sender == y_RotationNeg) resetSlider(y_nagativeRotation, y_RotationSlider, lastRotationY_, isProgrammaticChangeR);
    else if (sender == z_RotationNeg) resetSlider(z_nagativeRotation, z_RotationSlider, lastRotationZ_, isProgrammaticChangeR);
}

// ─── Transform sliders ───────────────────────────────────────────────────────

void StaticSceneApp::HandleTransform(Urho3D::StringHash, Urho3D::VariantMap& eventData)
{
    if (!tNode_ || isProgrammaticChange) return;

    auto* slider       = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
    float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();
    urho3d::Vector3 pos = tNode_->GetPosition();

    auto apply = [&](float& last, bool neg, float& coord, urho3d::LineEdit* edit)
    {
        float delta = currentValue - last;
        if (neg) delta = -delta;
        coord += delta;
        last = currentValue;
        char buf[16];
        snprintf(buf, sizeof(buf), "%.2f", coord);
        edit->SetText(urho3d::String(buf));
        tNode_->SetPosition(pos);
    };

    if      (slider == x_PosSlider) { float d = currentValue - lastX_; if (x_nagativeCoord) d=-d; pos.x_+=d; lastX_=currentValue; tNode_->SetPosition(pos); char buf[16]; snprintf(buf,sizeof(buf),"%.2f",pos.x_); x_Edit->SetText(urho3d::String(buf)); }
    else if (slider == y_PosSlider) { float d = currentValue - lastY_; if (y_nagativeCoord) d=-d; pos.y_+=d; lastY_=currentValue; tNode_->SetPosition(pos); char buf[16]; snprintf(buf,sizeof(buf),"%.2f",pos.y_); y_Edit->SetText(urho3d::String(buf)); }
    else if (slider == z_PosSlider) { float d = currentValue - lastZ_; if (z_nagativeCoord) d=-d; pos.z_+=d; lastZ_=currentValue; tNode_->SetPosition(pos); char buf[16]; snprintf(buf,sizeof(buf),"%.2f",pos.z_); z_Edit->SetText(urho3d::String(buf)); }
}

void StaticSceneApp::HandleScaleTransform(Urho3D::StringHash, Urho3D::VariantMap& eventData)
{
    if (!tNode_ || isProgrammaticChangeS) return;

    auto* slider       = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
    float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();
    urho3d::Vector3 scale = tNode_->GetScale();

    if      (slider == x_ScaleSlider) { float d = currentValue - lastScaleX_; if (x_nagativeScale) d=-d; scale.x_+=d; lastScaleX_=currentValue; tNode_->SetScale(scale); char buf[16]; snprintf(buf,sizeof(buf),"%.2f",scale.x_); x_ScaleEdit->SetText(urho3d::String(buf)); }
    else if (slider == y_ScaleSlider) { float d = currentValue - lastScaleY_; if (y_nagativeScale) d=-d; scale.y_+=d; lastScaleY_=currentValue; tNode_->SetScale(scale); char buf[16]; snprintf(buf,sizeof(buf),"%.2f",scale.y_); y_ScaleEdit->SetText(urho3d::String(buf)); }
    else if (slider == z_ScaleSlider) { float d = currentValue - lastScaleZ_; if (z_nagativeScale) d=-d; scale.z_+=d; lastScaleZ_=currentValue; tNode_->SetScale(scale); char buf[16]; snprintf(buf,sizeof(buf),"%.2f",scale.z_); z_ScaleEdit->SetText(urho3d::String(buf)); }
}

void StaticSceneApp::HandleRotationTransform(Urho3D::StringHash, Urho3D::VariantMap& eventData)
{
    if (!tNode_ || isProgrammaticChangeR) return;

    auto* slider       = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
    float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();
    urho3d::Vector3 euler = tNode_->GetRotation().EulerAngles();

    if (slider == x_RotationSlider)
    {
        float d = currentValue - lastRotationX_; if (x_nagativeRotation) d=-d;
        euler.x_ += d; lastRotationX_ = currentValue;
        tNode_->SetRotation(urho3d::Quaternion(euler.x_, euler.y_, euler.z_));
        char buf[16]; snprintf(buf, sizeof(buf), "%.2f", euler.x_); x_RotationEdit->SetText(urho3d::String(buf));
    }
    else if (slider == y_RotationSlider)
    {
        float d = currentValue - lastRotationY_; if (y_nagativeRotation) d=-d;
        euler.y_ += d; lastRotationY_ = currentValue;
        tNode_->SetRotation(urho3d::Quaternion(euler.x_, euler.y_, euler.z_));
        char buf[16]; snprintf(buf, sizeof(buf), "%.2f", euler.y_); y_RotationEdit->SetText(urho3d::String(buf));
    }
    else if (slider == z_RotationSlider)
    {
        float d = currentValue - lastRotationZ_; if (z_nagativeRotation) d=-d;
        euler.z_ += d; lastRotationZ_ = currentValue;
        tNode_->SetRotation(urho3d::Quaternion(euler.x_, euler.y_, euler.z_));
        char buf[16]; snprintf(buf, sizeof(buf), "%.2f", euler.z_); z_RotationEdit->SetText(urho3d::String(buf));
    }
}

void StaticSceneApp::HandleColorTransform(Urho3D::StringHash, Urho3D::VariantMap& eventData)
{
    if (!tNode_ || isProgrammaticChange) return;

    auto* slider       = static_cast<urho3d::Slider*>(eventData[urho3d::SliderChanged::P_ELEMENT].GetPtr());
    float currentValue = eventData[urho3d::SliderChanged::P_VALUE].GetFloat();
    float normalized   = currentValue / 100.f;

    if (slider == r_Slider)      { color.x_ = normalized; char buf[16]; snprintf(buf,sizeof(buf),"%.2f",currentValue*2.55f); r_Edit->SetText(urho3d::String(buf)); }
    else if (slider == g_Slider) { color.y_ = normalized; char buf[16]; snprintf(buf,sizeof(buf),"%.2f",currentValue*2.55f); g_Edit->SetText(urho3d::String(buf)); }
    else if (slider == b_Slider) { color.z_ = normalized; char buf[16]; snprintf(buf,sizeof(buf),"%.2f",currentValue*2.55f); b_Edit->SetText(urho3d::String(buf)); }
    else if (slider == a_Slider) { color.w_ = normalized; char buf[16]; snprintf(buf,sizeof(buf),"%.2f",currentValue);       a_Edit->SetText(urho3d::String(buf)); }

    auto* staticModel = tNode_->GetComponent<urho3d::StaticModel>();
    if (staticModel && staticModel->GetMaterial(0))
    {
        auto* mat = staticModel->GetMaterial(0);
        mat->SetTechnique(0, GetSubsystem<urho3d::ResourceCache>()->GetResource<urho3d::Technique>("Techniques/NoTextureAlpha.xml"));
        mat->SetShaderParameter("MatDiffColor", urho3d::Color(color.x_, color.y_, color.z_, color.w_));
    }
}

// ─── Apply buttons ───────────────────────────────────────────────────────────

void StaticSceneApp::HandleLocationButtonApply(urho3d::StringHash, urho3d::VariantMap&)
{
    if (!tNode_) return;
    urho3d::String xt = x_Edit->GetText(); xt.Replace(',', '.');
    urho3d::String yt = y_Edit->GetText(); yt.Replace(',', '.');
    urho3d::String zt = z_Edit->GetText(); zt.Replace(',', '.');
    float x = xt.Empty() ? 0.0f : atof(xt.CString());
    float y = yt.Empty() ? 0.5f : atof(yt.CString());
    float z = zt.Empty() ? 0.0f : atof(zt.CString());
    tNode_->SetPosition(urho3d::Vector3(x, y, z));
    lastX_ = x; lastY_ = y; lastZ_ = z;
}

void StaticSceneApp::HandleScaleButtonApply(urho3d::StringHash, urho3d::VariantMap&)
{
    if (!tNode_) return;
    urho3d::String xt = x_ScaleEdit->GetText(); xt.Replace(',', '.');
    urho3d::String yt = y_ScaleEdit->GetText(); yt.Replace(',', '.');
    urho3d::String zt = z_ScaleEdit->GetText(); zt.Replace(',', '.');
    float x = xt.Empty() ? 1.0f : atof(xt.CString());
    float y = yt.Empty() ? 1.0f : atof(yt.CString());
    float z = zt.Empty() ? 1.0f : atof(zt.CString());
    tNode_->SetScale(urho3d::Vector3(x, y, z));
    lastScaleX_ = x; lastScaleY_ = y; lastScaleZ_ = z;
}

void StaticSceneApp::HandleRotationButtonApply(urho3d::StringHash, urho3d::VariantMap&)
{
    if (!tNode_) return;
    urho3d::String xt = x_RotationEdit->GetText(); xt.Replace(',', '.');
    urho3d::String yt = y_RotationEdit->GetText(); yt.Replace(',', '.');
    urho3d::String zt = z_RotationEdit->GetText(); zt.Replace(',', '.');
    float x = xt.Empty() ? 0.0f : atof(xt.CString());
    float y = yt.Empty() ? 0.0f : atof(yt.CString());
    float z = zt.Empty() ? 0.0f : atof(zt.CString());
    tNode_->SetRotation(urho3d::Quaternion(x, y, z));
    lastRotationX_ = x; lastRotationY_ = y; lastRotationZ_ = z;
}

void StaticSceneApp::HandleColorButtonApply(urho3d::StringHash, urho3d::VariantMap&)
{
    if (!tNode_) return;
    urho3d::String xt = r_Edit->GetText(); xt.Replace(',', '.');
    urho3d::String yt = g_Edit->GetText(); yt.Replace(',', '.');
    urho3d::String zt = b_Edit->GetText(); zt.Replace(',', '.');
    urho3d::String wt = a_Edit->GetText(); wt.Replace(',', '.');
    float x = xt.Empty() ? 255.0f : atof(xt.CString());
    float y = yt.Empty() ? 255.0f : atof(yt.CString());
    float z = zt.Empty() ? 255.0f : atof(zt.CString());
    float w = wt.Empty() ? 100.0f : atof(wt.CString());

    r_Slider->SetValue(x); g_Slider->SetValue(y); b_Slider->SetValue(z); a_Slider->SetValue(w);

    color.x_ = x / 255.0f;
    color.y_ = y / 255.0f;
    color.z_ = z / 255.0f;
    color.w_ = w / 100.0f;

    auto* staticModel = tNode_->GetComponent<urho3d::StaticModel>();
    if (staticModel && staticModel->GetMaterial(0))
        staticModel->GetMaterial(0)->SetShaderParameter("MatDiffColor",
            urho3d::Color(color.x_, color.y_, color.z_, color.w_));
}

// ─── Debug geometry ──────────────────────────────────────────────────────────

void StaticSceneApp::HandleGeometryRender(urho3d::StringHash, urho3d::VariantMap&)
{
    auto* debug = scene_->GetComponent<urho3d::DebugRenderer>();
    if (!debug) return;

    for (auto& obj : objects_)
    {
        if (!obj.drawGeometry || !obj.node) continue;

        auto* staticModel = obj.node->GetComponent<urho3d::StaticModel>();
        auto* model       = staticModel ? staticModel->GetModel() : nullptr;
        if (!model) continue;

        urho3d::Geometry* geom = model->GetGeometry(0, 0);
        if (!geom) continue;

        auto* vb = geom->GetVertexBuffer(0);
        auto* ib = geom->GetIndexBuffer();
        if (!vb || !ib) continue;

        const unsigned char* vertexData = reinterpret_cast<const unsigned char*>(vb->GetShadowData());
        const unsigned char* indexData  = reinterpret_cast<const unsigned char*>(ib->GetShadowData());
        if (!vertexData || !indexData) continue;

        debug->AddTriangleMesh(
            vertexData, vb->GetVertexSize(),
            indexData,  ib->GetIndexSize(),
            geom->GetIndexStart(), geom->GetIndexCount(),
            obj.node->GetWorldTransform(),
            urho3d::Color(1.0f, 0.6f, 0.0f), true);
    }
}
