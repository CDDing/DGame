#include "pch.h"
#include "Light.h"

void DDing::Light::DrawUI()
{
    if (ImGui::CollapsingHeader("Light")) {
        const char* lightTypeNames[] = { "Directional", "Point", "Spot" };  // Assuming 3 types
        const char* currentLightType = lightTypeNames[static_cast<int>(type)];

        if (ImGui::BeginCombo("Light Type", currentLightType)) {
            for (int i = 0; i < IM_ARRAYSIZE(lightTypeNames); ++i) {
                bool isSelected = (type == static_cast<LightType>(i));
                if (ImGui::Selectable(lightTypeNames[i], isSelected)) {
                    type = static_cast<LightType>(i);  // Update the LightType
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::ColorButton("Light Color", ImVec4(color.x, color.y, color.z, 1.0f), ImGuiColorEditFlags_NoTooltip, ImVec2(30, 30))) {
            // When clicked, open the color picker dialog
            ImGui::OpenPopup("Color Picker Popup");
        }

        // If the color picker popup is open, allow the user to change the color
        if (ImGui::BeginPopup("Color Picker Popup")) {
            // The color picker will expand here
            if (ImGui::ColorPicker3("##picker", &color[0], ImGuiColorEditFlags_PickerHueBar)) {
                // Color is updated when the user picks a new color
            }
            ImGui::EndPopup();
        }

        // Intensity slider for light intensity
        if (ImGui::SliderFloat("Light Intensity", &intensity, 0.0f, 10.0f)) {
           
        }
        // Add inner and outer cone sliders only for Spot lights
        if (type == LightType::eSpot) {
            ImGui::SliderFloat("Inner Cone Angle", &innerCone, 0.0f, outerCone, "%.2f¡Æ");
            ImGui::SliderFloat("Outer Cone Angle", &outerCone, innerCone, 90.0f, "%.2f¡Æ");
        }
    }
}

void DDing::Light::Update()
{
}
