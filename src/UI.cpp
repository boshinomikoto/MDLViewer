#include "StaticSceneApp.h"
#include <initializer_list>

namespace urho3d = Urho3D;

void StaticSceneApp::InitWindow()
{
    window_     = MakeWindow("write a pth to .mdl and .xml", urho3d::HA_LEFT,  urho3d::VA_TOP);
    panel_      = MakeWindow("Transform",                    urho3d::HA_RIGHT, urho3d::VA_TOP,    280);
    colorPanel_ = MakeWindow("Color",                        urho3d::HA_RIGHT, urho3d::VA_BOTTOM, 500);
}

void StaticSceneApp::InitControls()
{
    auto* cache_    = GetSubsystem<urho3d::ResourceCache>();
    auto* font      = cache_->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf");
    auto* folderTex = cache_->GetResource<urho3d::Texture2D>("UI/icons/folder.png");

    // ── Lambdas ───────────────────────────────────────────────────────────────

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
        auto* row  = MakeRow(parent);
        auto* cb   = MakeCheckBoxInline(row);
        auto* lbl  = MakeLabel(row, labelText, 15);
        lbl->SetMinWidth(15);
        auto* edit = MakeLineEdit(row, 55, 25);
        auto* sl   = MakeSlider(row, 160, 25, range, value);
        return { cb, edit, sl };
    };

    auto MakeAxisColorRow = [&](urho3d::UIElement* parent,
        const urho3d::String& labelText,
        float range, float value)
        -> std::tuple<urho3d::LineEdit*, urho3d::Slider*>
    {
        auto* row  = MakeRow(parent);
        auto* lbl  = MakeLabel(row, labelText, 15);
        lbl->SetMinWidth(15);
        auto* edit = MakeLineEdit(row, 55, 25);
        auto* sl   = MakeSlider(row, 570, 25, range, value);
        return { edit, sl };
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
        const urho3d::String& labelText,
        int editWidth = 480)
        -> std::pair<urho3d::LineEdit*, urho3d::Button*>
    {
        auto* row  = MakeRow(parent);
        MakeLabel(row, labelText);
        auto* edit = MakeLineEdit(row, editWidth, 24);
        auto* btn  = MakeIconButton(row, folderTex);
        return { edit, btn };
    };

    auto MakeButtonRow = [&](urho3d::UIElement* parent,
        std::initializer_list<urho3d::Button*> values)
        -> urho3d::UIElement*
    {
        auto* rowLayout = parent->CreateChild<urho3d::UIElement>();
        rowLayout->SetLayout(urho3d::LM_HORIZONTAL, 4);
        rowLayout->SetHorizontalAlignment(urho3d::HA_CENTER);
        rowLayout->SetMinHeight(28);
        for (auto* val : values) rowLayout->AddChild(val);
        return rowLayout;
    };

    // ── Load panel ───────────────────────────────────────────────────────────
    auto [ml, mb] = MakeBrowseRow(window_, "MDL:");
    auto [xl, xb] = MakeBrowseRow(window_, "XML:");
    mdlLine_ = ml;
    xmlLine_ = xl;

    auto* deleteBtn = MakeButton(window_, "Delete", 170, 30);
    auto* loadBtn   = MakeButton(window_, "Load",   170, 30);
    MakeButtonRow(window_, { deleteBtn, loadBtn });

    // ── Color panel ──────────────────────────────────────────────────────────
    MakeLabel(colorPanel_, "\nRGBA");
    std::tie(r_Edit, r_Slider) = MakeAxisColorRow(colorPanel_, "R", 100.f, 0.f);
    std::tie(g_Edit, g_Slider) = MakeAxisColorRow(colorPanel_, "G", 100.f, 0.f);
    std::tie(b_Edit, b_Slider) = MakeAxisColorRow(colorPanel_, "B", 100.f, 0.f);
    std::tie(a_Edit, a_Slider) = MakeAxisColorRow(colorPanel_, "A", 100.f, 0.f);
    auto* colorButtonApply = MakeButton(colorPanel_, "Apply", 170, 30);
    colorButtonApply->SetAlignment(urho3d::HA_CENTER, urho3d::VA_TOP);

    // ── Transform panel ──────────────────────────────────────────────────────
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

    // ── Event subscriptions ──────────────────────────────────────────────────
    // Load
    SubscribeToEvent(loadBtn,   urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleButtonPress));
    SubscribeToEvent(mb,        urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleMDLButtonPress));
    SubscribeToEvent(xb,        urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleXMLButtonPress));
    // Delete
    SubscribeToEvent(deleteBtn, urho3d::E_RELEASED, URHO3D_HANDLER(StaticSceneApp, HandleDeleteButtonPress));

    // Position
    SubscribeToEvent(x_Neg,       urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(y_Neg,       urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(z_Neg,       urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(x_PosSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleTransform));
    SubscribeToEvent(y_PosSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleTransform));
    SubscribeToEvent(z_PosSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleTransform));
    SubscribeToEvent(locButtonApply, urho3d::E_RELEASED,   URHO3D_HANDLER(StaticSceneApp, HandleLocationButtonApply));

    // Scale
    SubscribeToEvent(x_ScaleNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(y_ScaleNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(z_ScaleNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(x_ScaleSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleScaleTransform));
    SubscribeToEvent(y_ScaleSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleScaleTransform));
    SubscribeToEvent(z_ScaleSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleScaleTransform));
    SubscribeToEvent(scaleButtonApply, urho3d::E_RELEASED,   URHO3D_HANDLER(StaticSceneApp, HandleScaleButtonApply));

    // Rotation
    SubscribeToEvent(x_RotationNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(y_RotationNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(z_RotationNeg,    urho3d::E_TOGGLED,       URHO3D_HANDLER(StaticSceneApp, HandleCheckBox));
    SubscribeToEvent(x_RotationSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleRotationTransform));
    SubscribeToEvent(y_RotationSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleRotationTransform));
    SubscribeToEvent(z_RotationSlider, urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleRotationTransform));
    SubscribeToEvent(rotationButtonApply, urho3d::E_RELEASED,   URHO3D_HANDLER(StaticSceneApp, HandleRotationButtonApply));

    // Color
    SubscribeToEvent(r_Slider,       urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleColorTransform));
    SubscribeToEvent(g_Slider,       urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleColorTransform));
    SubscribeToEvent(b_Slider,       urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleColorTransform));
    SubscribeToEvent(a_Slider,       urho3d::E_SLIDERCHANGED, URHO3D_HANDLER(StaticSceneApp, HandleColorTransform));
    SubscribeToEvent(colorButtonApply, urho3d::E_RELEASED,    URHO3D_HANDLER(StaticSceneApp, HandleColorButtonApply));
}

void StaticSceneApp::ShowGlobalValues()
{
    auto* ui    = GetSubsystem<urho3d::UI>();
    auto* cache = GetSubsystem<urho3d::ResourceCache>();
    auto* font  = cache->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf");

    demOfCurrPos = ui->GetRoot()->CreateChild<urho3d::Text>();
    demOfCurrPos->SetFont(font, 14);
    demOfCurrPos->SetColor(urho3d::Color::WHITE);
    demOfCurrPos->SetAlignment(urho3d::HA_LEFT, urho3d::VA_BOTTOM);
    demOfCurrPos->SetPosition(urho3d::HA_LEFT + 3, urho3d::VA_BOTTOM - 10);
    demOfCurrPos->SetText("");
}

void StaticSceneApp::CreateInstructions()
{
    auto* cache = GetSubsystem<urho3d::ResourceCache>();
    auto* ui    = GetSubsystem<urho3d::UI>();

    instructionText_ = ui->GetRoot()->CreateChild<urho3d::Text>();
    instructionText_->SetText(
        "Use WASD keys and mouse/touch to move\n"
        "Shift: slow\n"
        "X/Y/Z: snap camera to axis\n"
        "E/Q: up/down\n"
        "LBM: toggle debug geometry\n"
        "DEL: delete object\n"
        "Alt: toggle this help"
    );
    instructionText_->SetFont(cache->GetResource<urho3d::Font>("Fonts/Anonymous Pro.ttf"), 15);
    instructionText_->SetHorizontalAlignment(urho3d::HA_CENTER);
    instructionText_->SetVerticalAlignment(urho3d::VA_CENTER);
    instructionText_->SetPosition(0, ui->GetRoot()->GetHeight() / 3);
}
