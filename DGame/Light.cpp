#include "pch.h"
#include "Light.h"

void DDing::Light::DrawUI()
{
    if (ImGui::CollapsingHeader("Light")) {

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

    }
}

void DDing::Light::Update()
{
}
