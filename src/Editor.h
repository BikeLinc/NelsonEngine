#ifndef GUI_H
#define GUI_H

#include "NelsonEngine.h"

#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/backends/imgui_impl_glfw.h"
#include "../vendor/imgui/backends/imgui_impl_opengl3.h"

struct GuiLog
{
        ImGuiTextBuffer     buffer;
        ImGuiTextFilter     filter;
        ImVector<int>       lineOffsets;
        bool                autoScroll;

        GuiLog()
        {
                autoScroll = true;
                Clear();
        }

        void    Clear()
        {
                buffer.clear();
                lineOffsets.clear();
                lineOffsets.push_back(0);
        }

        void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
        {
                int old_size = buffer.size();
                va_list args;
                va_start(args, fmt);
                buffer.appendfv(fmt, args);
                va_end(args);
                for (int new_size = buffer.size(); old_size < new_size; old_size++)
                        if (buffer[old_size] == '\n')
                                lineOffsets.push_back(old_size + 1);
        }

        void    Draw(const char* title, bool* p_open = NULL)
        {
                if (!ImGui::Begin(title, p_open))
                {
                        ImGui::End();
                        return;
                }

                // Options menu
                if (ImGui::BeginPopup("Options"))
                {
                        ImGui::Checkbox("Auto-scroll", &autoScroll);
                        ImGui::EndPopup();
                }

                // Main window
                if (ImGui::Button("Options"))
                        ImGui::OpenPopup("Options");
                ImGui::SameLine();
                bool clear = ImGui::Button("Clear");
                ImGui::SameLine();
                bool copy = ImGui::Button("Copy");
                ImGui::SameLine();
                filter.Draw("Filter", -100.0f);

                ImGui::Separator();
                ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

                if (clear)
                        Clear();
                if (copy)
                        ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                const char* buf = buffer.begin();
                const char* buf_end = buffer.end();
                if (filter.IsActive())
                {
                        // In this example we don't use the clipper when filter is enabled.
                        // This is because we don't have a random access on the result on our filter.
                        // A real application processing logs with ten of thousands of entries may want to store the result of
                        // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                        for (int line_no = 0; line_no < lineOffsets.Size; line_no++)
                        {
                                const char* line_start = buf + lineOffsets[line_no];
                                const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                                if (filter.PassFilter(line_start, line_end))
                                        ImGui::TextUnformatted(line_start, line_end);
                        }
                }
                else
                {
                        // The simplest and easy way to display the entire buffer:
                        //   ImGui::TextUnformatted(buf_begin, buf_end);
                        // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
                        // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
                        // within the visible area.
                        // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
                        // on your side is recommended. Using ImGuiListClipper requires
                        // - A) random access into your data
                        // - B) items all being the  same height,
                        // both of which we can handle since we an array pointing to the beginning of each line of text.
                        // When using the filter (in the block of code above) we don't have random access into the data to display
                        // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
                        // it possible (and would be recommended if you want to search through tens of thousands of entries).
                        ImGuiListClipper clipper;
                        clipper.Begin(lineOffsets.Size);
                        while (clipper.Step())
                        {
                                for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                                {
                                        const char* line_start = buf + lineOffsets[line_no];
                                        const char* line_end = (line_no + 1 < lineOffsets.Size) ? (buf + lineOffsets[line_no + 1] - 1) : buf_end;
                                        ImGui::TextUnformatted(line_start, line_end);
                                }
                        }
                        clipper.End();
                }
                ImGui::PopStyleVar();

                if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                        ImGui::SetScrollHereY(1.0f);

                ImGui::EndChild();
                ImGui::End();
        }
};

class Editor {
public:
        GuiLog log;

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

        void drawInspector(Scene &scene) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
                log.Draw("Console");

                // Inspector Window
		{
			ImGui::Begin("Inspector");

                        if (ImGui::CollapsingHeader("Scene"))
                        {
                                ImGui::Indent(16.0f);

                                // Add
                                if (ImGui::CollapsingHeader("Add")) {
                                        static char modelName[128] = "name";
                                        static char texPath[128] = "../res/images/chong.png";
                                        static glm::vec2 bounds = glm::vec2(1);
                                        static Transform transform;
                                        ImGui::InputText("Name", modelName, IM_ARRAYSIZE(modelName));
                                        ImGui::InputText("Texture", texPath, IM_ARRAYSIZE(texPath));
                                        ImGui::DragFloat2("Size", &bounds.x, 0.01f, -1000.0f, 1000.0f);
                                        ImGui::DragFloat3("Position", &transform.position.x, 0.01f, -10000.0f, 10000.0f);
                                        ImGui::DragFloat3("Rotation", &transform.rotation.x, 0.1f, -180.0f, 180.0f);
                                        ImGui::DragFloat3("Scale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
                                        if (ImGui::Button("Create"))
                                        {
                                                scene.add(new Model(modelName, texPath, PlaneGeometry(bounds), transform));
                                                log.AddLog("[SCENE] Add Model \'%s\'\n", modelName);

                                        }
                                }

                                // Scene Properties
                                if (ImGui::CollapsingHeader("Properties")) {
                                        ImGui::Text("Viewport Color");
                                        ImGui::Indent(16.0f);
                                        ImGui::ColorEdit4("Clear Color", &scene.color.x);
                                        ImGui::Unindent(16.0f);
                                }

                                // Scene Models
                                for (int i = 0; i < scene.models.size(); i++) {
                                        ImGui::PushID(i);
                                        char characterBuffer[32];
                                        Model& model = *scene.models[i];
                                        sprintf(characterBuffer, model.name, i + 1);
                                        if (ImGui::CollapsingHeader(characterBuffer)) {
                                                ImGui::Text("Transform");
                                                ImGui::Indent(16.0f);
                                                ImGui::DragFloat3("Position", &model.transform.position.x, 0.01f, -10000.0f, 10000.0f);
                                                ImGui::DragFloat3("Rotation", &model.transform.rotation.x, 0.1f, -180.0f, 180.0f);
                                                ImGui::DragFloat3("Scale", &model.transform.scale.x, 0.01f, -10.0f, 10.0f);
                                                ImGui::Unindent(16.0f);


                                                ImGui::Text("Material"); 
                                                ImGui::Indent(16.0f);
                                                static char str0[128] = "images/*.png";
                                                ImGui::InputText("Texture", str0, IM_ARRAYSIZE(str0));
                                                ImGui::SameLine();
                                                if (ImGui::Button("Load"))
                                                {
                                                        log.AddLog("[ASSET]Load Texture From \'%s\'\n", str0);
                                                }
                                                ImGui::Unindent(16.0f);
                                        }
                                        ImGui::PopID();
                                }
                                

                                ImGui::Unindent(16.0f);
                        }
                        
                        ImGui::Separator();

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
