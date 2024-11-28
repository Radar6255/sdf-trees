#include "UseImGui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

void UseImGui::Init(GLFWwindow* window, const char* glsl_version) {
    IMGUI_CHECKVERSION();

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImGui::StyleColorsDark();
}

void UseImGui::Update(GuiData* gd) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Test");
    ImGui::Text("Magic here");
    ImGui::SliderFloat("alterSize", &gd->alterSize, -0.2f, 0.2f);
    ImGui::SliderFloat("treeChanceThresh", &gd->treeChanceThresh, 0.1f, 6.0f);

    ImGui::Checkbox("Update Terrain", &gd->updateTerrain);

    ImGui::Text("Recent render time: %.1f", gd->recentRenderTime);
    ImGui::Text("Avg render time: %.1f", gd->avgRenderTime);
    ImGui::Text("FPS: %.1f", 1000000.0f / gd->recentRenderTime);
    ImGui::Text("Recent update Rate: %.1f", gd->recentUpdateRate);
    ImGui::Text("Avg update Rate: %.1f", gd->updateRate);

    /*bool clear_color_changed = ImGui::ColorEdit3("clear color", (float*) clear_color)*/
    ImGui::End();
}

void UseImGui::Render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
void UseImGui::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
