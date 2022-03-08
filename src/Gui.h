#ifndef GUI_H
#define GUI_H

#include "NelsonEngine.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/backends/imgui_impl_glfw.h"
#include "../vendor/imgui/backends/imgui_impl_opengl3.h"

class Gui {
public:
        
        void init(GLFWwindow* window) {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                io.Fonts->AddFontFromFileTTF("../vendor/imgui/Roboto-Medium.ttf", 16);
                ImGui::StyleColorsDark();
                style();
                ImGui_ImplGlfw_InitForOpenGL(window, true);
                ImGui_ImplOpenGL3_Init("#version 330");
        }     

        void style() {

                ImGui::GetStyle().FrameRounding = 2.0f;
                ImGui::GetStyle().GrabRounding = 1.0f;
                ImGui::GetStyle().GrabMinSize = 18;
                ImGui::GetStyle().WindowBorderSize = 0;
                ImGui::GetStyle().FramePadding = ImVec2(6.0f, 6.0f);

                ImVec4* colors = ImGui::GetStyle().Colors;
                colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
                colors[ImGuiCol_Border]                 = ImVec4(0.72f, 0.72f, 0.72f, 1.00f);
                colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
                colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.26f, 0.71f, 0.20f, 1.00f);
                colors[ImGuiCol_FrameBgActive]          = ImVec4(0.16f, 0.45f, 0.12f, 0.60f);
                colors[ImGuiCol_TitleBg]                = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
                colors[ImGuiCol_TitleBgActive]          = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
                colors[ImGuiCol_CheckMark]              = ImVec4(0.21f, 0.19f, 0.18f, 1.00f);
                colors[ImGuiCol_SliderGrab]             = ImVec4(0.21f, 0.19f, 0.18f, 1.00f);
                colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.21f, 0.19f, 0.18f, 1.00f);
                colors[ImGuiCol_Button]                 = ImVec4(0.26f, 0.71f, 0.20f, 0.60f);
                colors[ImGuiCol_ButtonHovered]          = ImVec4(0.26f, 0.71f, 0.20f, 1.00f);
                colors[ImGuiCol_ButtonActive]           = ImVec4(0.16f, 0.45f, 0.12f, 0.60f);
                colors[ImGuiCol_Header]                 = ImVec4(0.26f, 0.71f, 0.20f, 0.60f);
                colors[ImGuiCol_HeaderHovered]          = ImVec4(0.26f, 0.71f, 0.20f, 1.00f);
                colors[ImGuiCol_HeaderActive]           = ImVec4(0.16f, 0.45f, 0.12f, 0.60f);
                colors[ImGuiCol_Separator]              = ImVec4(0.26f, 0.71f, 0.20f, 0.60f);
                colors[ImGuiCol_ResizeGrip]             = ImVec4(0.26f, 0.71f, 0.20f, 0.60f);
                colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.26f, 0.71f, 0.20f, 1.00f);
                colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.16f, 0.45f, 0.12f, 0.60f);
                colors[ImGuiCol_Tab]                    = ImVec4(0.26f, 0.71f, 0.20f, 0.60f);
                colors[ImGuiCol_TabHovered]             = ImVec4(0.16f, 0.45f, 0.12f, 0.60f);
                colors[ImGuiCol_TabActive]              = ImVec4(0.26f, 0.71f, 0.20f, 1.00f);
        }

        void draw(Scene &scene) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			ImGui::Begin("Inspector");

                        if (ImGui::CollapsingHeader("Scene"))
                        {


                                ImGui::Indent(16.0f);
                                if (ImGui::CollapsingHeader("Properties")) {
                                        ImGui::Text("Viewport Color");
                                        ImGui::Indent(16.0f);
                                        ImGui::ColorEdit4("Clear Color", &scene.backgroundColor.x);
                                        ImGui::Unindent(16.0f);
                                }
                                static int count = scene.models.size();
                                for (int i = 0; i < count; i++) {
                                        ImGui::PushID(i);
                                        char buf[32];
                                        Model& model = *scene.models[i];
                                        sprintf(buf, model.name, i + 1);
                                        if (ImGui::CollapsingHeader(buf)) {
                                                ImGui::Text("Transform");
                                                ImGui::Indent(16.0f);
                                                ImGui::SliderFloat3("Position", &model.transform.position.x, -1.0f, 1.0f);
                                                ImGui::SliderFloat3("Rotation", &model.transform.rotation.x, -180.0f, 180.0f);
                                                ImGui::SliderFloat3("Scale", &model.transform.scale.x, 0.0f, 10.0f);
                                                ImGui::Unindent(16.0f);


                                                ImGui::Text("Material"); 
                                                ImGui::Indent(16.0f);
                                                static char str0[128] = "images/*.png";
                                                ImGui::InputText("Texture", str0, IM_ARRAYSIZE(str0));
                                                ImGui::SameLine();
                                                if (ImGui::Button("Load"))
                                                {
                                                        //... my_code 
                                                }
                                                ImGui::Unindent(16.0f);
                                        }
                                        ImGui::PopID();
                                }
                                ImGui::Separator();
                                ImGui::Unindent(16.0f);
                        }
                        /*
                        if (ImGui::CollapsingHeader("Sprite")) {
                                ImGui::Text("Transform");
                                ImGui::SliderFloat3("Position", &transform.position.x, -1.0f, 1.0f);
                                ImGui::SliderFloat3("Rotation", &transform.rotation.x, -180.0f, 180.0f);
                                ImGui::SliderFloat3("Scale", &transform.scale.x, 0.0f, 10.0f);


                                
                        }
                        */
                        if (ImGui::CollapsingHeader("Application")) {

                                ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
                        }
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        void terminate() {
                ImGui_ImplOpenGL3_Shutdown();
    	        ImGui_ImplGlfw_Shutdown();
    	        ImGui::DestroyContext();
        }
};

#endif /* GUI_H */
