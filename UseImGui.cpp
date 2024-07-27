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
void UseImGui::Update() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static float f = 0.0f;

    ImGui::Begin("Test");
    ImGui::Text("Magic here");
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    
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
