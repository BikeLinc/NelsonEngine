#ifndef GUI_H
#define GUI_H

#include "Nelson.h"
#include "SimpleJson.h"

#include <cctype>
#include <cstdio>
#include <filesystem>

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

        void DrawBody(float reservedBottomHeight = 0.0f) {
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
                const float childHeight = (reservedBottomHeight > 0.0f) ? -reservedBottomHeight : 0.0f;
                ImGui::BeginChild("scrolling", ImVec2(0, childHeight), false, ImGuiWindowFlags_HorizontalScrollbar);

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
        }

        void Draw(const char* title, bool* p_open = NULL, ImGuiWindowFlags windowFlags = 0)
        {
                if (!ImGui::Begin(title, p_open, windowFlags))
                {
                        ImGui::End();
                        return;
                }

                DrawBody();
                ImGui::End();
        }
};

class Editor : public System {
public:
        enum class PrimitiveType {
                Plane,
                Cube,
                Sphere
        };

        GuiLog log;
        Entity* selectedEntity = nullptr;
        bool sceneRootSelected = true;
        float leftPanelWidth = 280.0f;
        float rightPanelWidth = 320.0f;
        float bottomPanelHeight = 230.0f;
        float toolbarHeight = 56.0f;
        unsigned int sceneTextureID = 0;
        int sceneTextureWidth = 0;
        int sceneTextureHeight = 0;
        std::string currentProjectName = "default";
        std::string sceneDialogDirectory = "projects/default/scenes";
        int workspaceTabIndex = 0;
        int requestedWorkspaceTabIndex = -1;
        char sceneJsonPath[256] = "projects/default/scenes/default.scene.json";
        char consoleCommandBuffer[512] = "";

        Editor(MessageBus* bus) : System({ ENGINE_EVENT, EDITOR_EVENT, CONSOLE_EVENT}, bus) {}

        void init(GLFWwindow* window) {
                IMGUI_CHECKVERSION();
                ImGui::CreateContext();
                ImGuiIO& io = ImGui::GetIO(); (void)io;
                auto fileExists = [](const char* path) -> bool {
                        std::ifstream f(path);
                        return f.good();
                };

                const char* fontPaths[] = {
                        "vendor/imgui/Roboto-Medium.ttf",
                        "../vendor/imgui/Roboto-Medium.ttf"
                };

                bool loadedCustomFont = false;
                for (const char* path : fontPaths) {
                        if (fileExists(path)) {
                                if (io.Fonts->AddFontFromFileTTF(path, 16.0f) != nullptr) {
                                        loadedCustomFont = true;
                                        break;
                                }
                        }
                }

                if (!loadedCustomFont) {
                        io.Fonts->AddFontDefault();
                        std::cout << "[Editor] Warning: Could not load Roboto-Medium.ttf; using default ImGui font." << std::endl;
                }
                ImGui::StyleColorsDark();
                style();
                ImGui_ImplGlfw_InitForOpenGL(window, true);
                ImGui_ImplOpenGL3_Init("#version 330");
                postMessage(Message({ CONSOLE_EVENT }, "[Editor] Ready."));
        }

        void style() {
                ImVec4* colors = ImGui::GetStyle().Colors;
                colors[ImGuiCol_Text] = ImVec4(0.62f, 0.65f, 0.71f, 1.00f);
                colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
                colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.15f, 0.17f, 1.00f);
                colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
                colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_FrameBg] = ImVec4(0.21f, 0.23f, 0.27f, 1.00f);
                colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
                colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
                colors[ImGuiCol_TitleBg] = ImVec4(0.13f, 0.15f, 0.17f, 1.00f);
                colors[ImGuiCol_TitleBgActive] = ImVec4(0.13f, 0.15f, 0.17f, 1.00f);
                colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
                colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
                colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
                colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
                colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
                colors[ImGuiCol_CheckMark] = ImVec4(0.62f, 0.65f, 0.71f, 1.00f);
                colors[ImGuiCol_SliderGrab] = ImVec4(0.62f, 0.65f, 0.71f, 1.00f);
                colors[ImGuiCol_SliderGrabActive] = ImVec4(0.62f, 0.65f, 0.71f, 1.00f);
                colors[ImGuiCol_Button] = ImVec4(0.21f, 0.23f, 0.27f, 1.00f);
                colors[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
                colors[ImGuiCol_ButtonActive] = ImVec4(0.28f, 0.38f, 0.59f, 1.00f);
                colors[ImGuiCol_Header] = ImVec4(0.21f, 0.23f, 0.27f, 1.00f);
                colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
                colors[ImGuiCol_HeaderActive] = ImVec4(0.28f, 0.38f, 0.59f, 1.00f);
                colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_SeparatorHovered] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_SeparatorActive] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
                colors[ImGuiCol_ResizeGrip] = ImVec4(0.21f, 0.23f, 0.27f, 1.00f);
                colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
                colors[ImGuiCol_ResizeGripActive] = ImVec4(0.28f, 0.38f, 0.59f, 1.00f);
                colors[ImGuiCol_Tab] = ImVec4(0.21f, 0.23f, 0.27f, 1.00f);
                colors[ImGuiCol_TabHovered] = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
                colors[ImGuiCol_TabActive] = ImVec4(0.28f, 0.38f, 0.59f, 1.00f);
                colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
                colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
                colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
                colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
                colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
                colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
                colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
                colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
                colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
                colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
                colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
                colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
                colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
                colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
                colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
                colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
                colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        }

        void update() override {

        }

        void draw(Scene &scene) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
                captureSceneFramebuffer();
                ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                const float minSidePanel = 180.0f;
                const float minBottomPanel = 120.0f;
                const float minScene = 220.0f;
                const float panelGap = 0.0f;
                const float statusBarHeight = 52.0f;

                leftPanelWidth = std::max(minSidePanel, leftPanelWidth);
                rightPanelWidth = std::max(minSidePanel, rightPanelWidth);
                bottomPanelHeight = std::max(minBottomPanel, bottomPanelHeight);
                toolbarHeight = std::max(56.0f, toolbarHeight);

                float menuBarHeight = 0.0f;
                if (ImGui::BeginMainMenuBar()) {
                        menuBarHeight = ImGui::GetWindowSize().y;
                        ImGui::Text("Nelson Engine");
                        if (ImGui::BeginMenu("File")) {
                                if (ImGui::MenuItem("New Scene")) {
                                        scene.clear();
                                        scene.name = "scene";
                                        scene.color = glm::vec4(0.25f, 0.25f, 0.35f, 1.0f);
                                        sceneRootSelected = true;
                                        selectedEntity = nullptr;
                                        log.AddLog("[SCENE] Created new scene.\n");
                                }
                                if (ImGui::MenuItem("Open Scene JSON...")) {
                                        requestedWorkspaceTabIndex = 1;
                                }
                                if (ImGui::MenuItem("Save Scene JSON")) {
                                        saveSceneAtPath(scene, sceneJsonPath);
                                }
                                if (ImGui::MenuItem("Save Scene JSON As...")) {
                                        requestedWorkspaceTabIndex = 1;
                                }
                                ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Edit")) {
                                ImGui::MenuItem("Undo");
                                ImGui::MenuItem("Redo");
                                ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("View")) {
                                ImGui::MenuItem("Hierarchy");
                                ImGui::MenuItem("Inspector");
                                ImGui::MenuItem("Console");
                                ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Help")) {
                                ImGui::MenuItem("About");
                                ImGui::EndMenu();
                        }
                        ImGui::EndMainMenuBar();
                }
                if (menuBarHeight <= 0.0f) {
                        menuBarHeight = ImGui::GetFrameHeight();
                }

                const float contentTop = menuBarHeight + toolbarHeight;
                const float verticalSpace = std::max(100.0f, displaySize.y - contentTop - statusBarHeight - panelGap * 2.0f);
                bottomPanelHeight = std::min(bottomPanelHeight, std::max(minBottomPanel, verticalSpace - minScene - panelGap));

                float mainHeight = verticalSpace - bottomPanelHeight - panelGap;
                float consoleHeight = bottomPanelHeight;
                const float consoleY = contentTop + mainHeight + panelGap;
                const float statusBarY = consoleY + consoleHeight + panelGap;

                if (leftPanelWidth + rightPanelWidth > displaySize.x - minScene - panelGap * 2.0f) {
                        const float maxSideTotal = std::max(minScene, displaySize.x - minScene - panelGap * 2.0f);
                        const float sideScale = maxSideTotal / std::max(1.0f, leftPanelWidth + rightPanelWidth);
                        leftPanelWidth *= sideScale;
                        rightPanelWidth *= sideScale;
                }

                const float sceneX = leftPanelWidth + panelGap;
                const float sceneWidth = std::max(minScene, displaySize.x - leftPanelWidth - rightPanelWidth - panelGap * 2.0f);
                const float inspectorX = sceneX + sceneWidth + panelGap;

                ImGuiWindowFlags dockedWindowFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
                static char primitiveName[128] = "Model";
                static char primitiveTexturePath[128] = "res/images/default_white.png";
                static glm::vec2 primitiveBounds = glm::vec2(1.0f);
                static Transform primitiveTransform;
                static char loadName[128] = "Model";
                static Transform loadTransform;
                static char objPath[256] = "projects/default/assets/models/sponza/sponza.obj";
                static char mtlDir[256] = "projects/default/assets/models/sponza/";

                ImGui::SetNextWindowPos(ImVec2(0.0f, menuBarHeight));
                ImGui::SetNextWindowSize(ImVec2(displaySize.x, toolbarHeight));
                if (ImGui::Begin("Toolbar", nullptr, dockedWindowFlags | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
                        ImGui::Button("Play");
                        ImGui::SameLine();
                        ImGui::Button("Pause");
                        ImGui::SameLine();
                        ImGui::Button("Stop");
                        ImGui::SameLine();
                        if (ImGui::Button("Create Primitive")) {
                                ImGui::OpenPopup("Create Primitive");
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Load OBJ")) {
                                ImGui::OpenPopup("Load OBJ");
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Scene Files")) {
                                requestedWorkspaceTabIndex = 1;
                        }
                        ImGui::SameLine();
                        ImGui::Checkbox("Wireframe", &scene.wireframeMode);
                        ImGui::SameLine();
                        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
                }
                ImGui::End();

                if (ImGui::BeginPopupModal("Create Primitive", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::InputText("Name", primitiveName, IM_ARRAYSIZE(primitiveName));
                        ImGui::InputText("Texture", primitiveTexturePath, IM_ARRAYSIZE(primitiveTexturePath));
                        ImGui::DragFloat2("Size", &primitiveBounds.x, 0.01f, 0.1f, 1000.0f);
                        ImGui::DragFloat3("Position", &primitiveTransform.position.x, 0.1f, -1000.0f, 1000.0f);
                        ImGui::DragFloat3("Rotation", &primitiveTransform.rotation.x, 0.1f, -180.0f, 180.0f);
                        ImGui::DragFloat3("Scale", &primitiveTransform.scale.x, 0.001f, 0.001f, 100.0f);

                        if (ImGui::Button("Create", ImVec2(140.0f, 0.0f))) {
                                scene.add(new Model(primitiveName, primitiveTexturePath, primitiveBounds, primitiveTransform));
                                log.AddLog("[SCENE] Add Model '%s'\n", primitiveName);
                                ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) {
                                ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                }

                if (ImGui::BeginPopupModal("Load OBJ", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::InputText("Name", loadName, IM_ARRAYSIZE(loadName));
                        ImGui::InputText("OBJ Path", objPath, IM_ARRAYSIZE(objPath));
                        ImGui::InputText("MTL Dir", mtlDir, IM_ARRAYSIZE(mtlDir));
                        ImGui::DragFloat3("Position", &loadTransform.position.x, 0.1f, -1000.0f, 1000.0f);
                        ImGui::DragFloat3("Rotation", &loadTransform.rotation.x, 0.1f, -180.0f, 180.0f);
                        ImGui::DragFloat3("Scale", &loadTransform.scale.x, 0.001f, 0.001f, 100.0f);

                        if (ImGui::Button("Load", ImVec2(140.0f, 0.0f))) {
                                Model* mdl = new Model(loadName, loadTransform);
                                if (mdl->LoadOBJ(objPath, mtlDir)) {
                                        scene.add(mdl);
                                        log.AddLog("[SCENE] Loaded OBJ '%s'\n", objPath);
                                } else {
                                        delete mdl;
                                        log.AddLog("[ERROR] Failed to load OBJ '%s'\n", objPath);
                                }
                                ImGui::CloseCurrentPopup();
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Cancel", ImVec2(140.0f, 0.0f))) {
                                ImGui::CloseCurrentPopup();
                        }
                        ImGui::EndPopup();
                }

                ImGui::SetNextWindowPos(ImVec2(0.0f, contentTop));
                ImGui::SetNextWindowSize(ImVec2(leftPanelWidth, mainHeight));
                if (ImGui::Begin("Hierarchy", nullptr, dockedWindowFlags | ImGuiWindowFlags_NoResize)) {
                        auto drawHierarchyCreateMenu = [&](Entity* parent) {
                                if (ImGui::BeginMenu("Create")) {
                                        if (ImGui::MenuItem("Empty Entity")) {
                                                addEmptyEntityToScene(scene, parent);
                                        }
                                        if (ImGui::BeginMenu("Primitive")) {
                                                if (ImGui::MenuItem("Plane")) {
                                                        addPrimitiveToScene(scene, PrimitiveType::Plane, parent);
                                                }
                                                if (ImGui::MenuItem("Cube")) {
                                                        addPrimitiveToScene(scene, PrimitiveType::Cube, parent);
                                                }
                                                if (ImGui::MenuItem("Sphere")) {
                                                        addPrimitiveToScene(scene, PrimitiveType::Sphere, parent);
                                                }
                                                ImGui::EndMenu();
                                        }
                                        ImGui::EndMenu();
                                }
                        };

                        std::function<void(Entity*)> drawEntityNode = [&](Entity* entity) {
                                if (entity == nullptr) {
                                        return;
                                }
                                ImGuiTreeNodeFlags entityFlags = ImGuiTreeNodeFlags_SpanAvailWidth;
                                if (entity == selectedEntity && !sceneRootSelected) {
                                        entityFlags |= ImGuiTreeNodeFlags_Selected;
                                }
                                if (entity->children.empty()) {
                                        entityFlags |= ImGuiTreeNodeFlags_Leaf;
                                }

                                const bool openEntity = ImGui::TreeNodeEx(entity, entityFlags, "%s", entity->name.c_str());
                                if (ImGui::IsItemClicked()) {
                                        selectedEntity = entity;
                                        sceneRootSelected = false;
                                }
                                if (ImGui::BeginPopupContextItem("EntityHierarchyContext")) {
                                        drawHierarchyCreateMenu(entity);
                                        ImGui::Separator();
                                        if (!entity->hasRenderable) {
                                                if (ImGui::BeginMenu("Add Renderable Component")) {
                                                        if (ImGui::MenuItem("Plane")) {
                                                                attachRenderableComponent(entity, PrimitiveType::Plane);
                                                        }
                                                        if (ImGui::MenuItem("Cube")) {
                                                                attachRenderableComponent(entity, PrimitiveType::Cube);
                                                        }
                                                        if (ImGui::MenuItem("Sphere")) {
                                                                attachRenderableComponent(entity, PrimitiveType::Sphere);
                                                        }
                                                        ImGui::EndMenu();
                                                }
                                        } else if (ImGui::MenuItem("Remove Renderable Component")) {
                                                detachRenderableComponent(entity);
                                        }
                                        if (ImGui::MenuItem("Delete Entity")) {
                                                Entity* toDelete = entity;
                                                if (scene.removeEntity(toDelete)) {
                                                        if (selectedEntity == toDelete) {
                                                                selectedEntity = nullptr;
                                                                sceneRootSelected = true;
                                                        }
                                                }
                                        }
                                        ImGui::EndPopup();
                                }

                                if (openEntity) {
                                        for (Entity* child : entity->children) {
                                                drawEntityNode(child);
                                        }
                                        ImGui::TreePop();
                                }
                        };

                        ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth;
                        bool openScene = ImGui::TreeNodeEx("Scene", sceneFlags);
                        if (ImGui::IsItemClicked()) {
                                sceneRootSelected = true;
                                selectedEntity = nullptr;
                        }
                        if (ImGui::BeginPopupContextItem("SceneHierarchyContext")) {
                                drawHierarchyCreateMenu(nullptr);
                                ImGui::EndPopup();
                        }

                        if (openScene) {
                                for (Entity* entity : scene.entities) {
                                        drawEntityNode(entity);
                                }
                                ImGui::TreePop();
                        }

                        // Right-clicking empty hierarchy space opens create menu too.
                        if (ImGui::BeginPopupContextWindow("HierarchyWindowContext", ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight)) {
                                drawHierarchyCreateMenu(nullptr);
                                ImGui::EndPopup();
                        }
                }
                ImGui::End();

                ImGui::SetNextWindowPos(ImVec2(sceneX, contentTop));
                ImGui::SetNextWindowSize(ImVec2(sceneWidth, mainHeight));
                if (ImGui::Begin("Workspace", nullptr, dockedWindowFlags | ImGuiWindowFlags_NoResize)) {
                        if (ImGui::BeginTabBar("WorkspaceTabs")) {
                                const int desiredWorkspaceTab = requestedWorkspaceTabIndex;
                                requestedWorkspaceTabIndex = -1;
                                ImGuiTabItemFlags sceneTabFlags = 0;
                                ImGuiTabItemFlags sceneFilesTabFlags = 0;
                                if (desiredWorkspaceTab == 0) {
                                        sceneTabFlags |= ImGuiTabItemFlags_SetSelected;
                                } else if (desiredWorkspaceTab == 1) {
                                        sceneFilesTabFlags |= ImGuiTabItemFlags_SetSelected;
                                }

                                if (ImGui::BeginTabItem("Scene", nullptr, sceneTabFlags)) {
                                        workspaceTabIndex = 0;
                                        ImVec2 regionSize = ImGui::GetContentRegionAvail();
                                        if (sceneTextureID != 0 && regionSize.x > 1.0f && regionSize.y > 1.0f && sceneTextureWidth > 0 && sceneTextureHeight > 0) {
                                                const float textureAspect = static_cast<float>(sceneTextureWidth) / static_cast<float>(sceneTextureHeight);
                                                const float regionAspect = regionSize.x / regionSize.y;
                                                ImVec2 drawSize = regionSize;
                                                if (regionAspect > textureAspect) {
                                                        drawSize.x = regionSize.y * textureAspect;
                                                } else {
                                                        drawSize.y = regionSize.x / textureAspect;
                                                }

                                                ImVec2 cursorPos = ImGui::GetCursorPos();
                                                cursorPos.x += (regionSize.x - drawSize.x) * 0.5f;
                                                cursorPos.y += (regionSize.y - drawSize.y) * 0.5f;
                                                ImGui::SetCursorPos(cursorPos);
                                                ImGui::Image(reinterpret_cast<void*>(static_cast<intptr_t>(sceneTextureID)),
                                                             drawSize, ImVec2(0, 1), ImVec2(1, 0));
                                        } else {
                                                ImGui::Text("Viewport unavailable.");
                                        }
                                        ImGui::EndTabItem();
                                }
                                if (ImGui::BeginTabItem("Scene Files", nullptr, sceneFilesTabFlags)) {
                                        workspaceTabIndex = 1;
                                        ImGui::Text("Load/Save Scene JSON");
                                        const std::string projectRoot = detectProjectRootPath().lexically_normal().generic_string();
                                        ImGui::TextWrapped("Project Root: %s", projectRoot.c_str());
                                        ImGui::Text("Current Project: %s", currentProjectName.c_str());
                                        ImGui::SameLine();
                                        if (ImGui::Button("Use Project Scenes Dir")) {
                                                std::filesystem::path projectSceneDir = currentProjectScenesPath();
                                                sceneDialogDirectory = projectSceneDir.generic_string();
                                        }
                                        ImGui::Separator();
                                        ImGui::InputText("Scene Path", sceneJsonPath, IM_ARRAYSIZE(sceneJsonPath));

                                        const std::string resolvedReadPath = resolvePathForRead(sceneJsonPath);
                                        const std::string resolvedWritePath = resolvePathForWrite(sceneJsonPath);
                                        ImGui::TextWrapped("Load From: %s", resolvedReadPath.c_str());
                                        ImGui::TextWrapped("Save To: %s", resolvedWritePath.c_str());

                                        if (ImGui::Button("Load Scene", ImVec2(140.0f, 0.0f))) {
                                                loadSceneAtPath(scene, resolvedReadPath);
                                        }
                                        ImGui::SameLine();
                                        if (ImGui::Button("Save Scene", ImVec2(140.0f, 0.0f))) {
                                                saveSceneAtPath(scene, resolvedWritePath);
                                        }
                                        ImGui::Separator();
                                        drawSceneFileBrowser(sceneJsonPath, IM_ARRAYSIZE(sceneJsonPath), "workspace");
                                        ImGui::EndTabItem();
                                }
                                ImGui::EndTabBar();
                        }
                }
                ImGui::End();

                ImGui::SetNextWindowPos(ImVec2(inspectorX, contentTop));
                ImGui::SetNextWindowSize(ImVec2(rightPanelWidth, mainHeight));
                if (ImGui::Begin("Inspector", nullptr, dockedWindowFlags | ImGuiWindowFlags_NoResize)) {
                        if (sceneRootSelected) {
                                ImGui::Text("Scene");
                                ImGui::Separator();
                                ImGui::Text("Viewport");
                                ImGui::ColorEdit4("Clear Color", &scene.color.x);
                                ImGui::Separator();
                                ImGui::Text("Entity Count: %d", static_cast<int>(scene.entities.size()));
                                ImGui::Text("Renderable: %d", scene.renderableCount());
                        } else if (selectedEntity != nullptr) {
                                Entity* entity = selectedEntity;
                                ImGui::Text("%s", entity->name.c_str());
                                ImGui::Separator();
                                ImGui::DragInt("Order", &entity->order);
                                ImGui::DragFloat3("Position", &entity->transform.position.x, 0.1f, -1000.0f, 1000.0f);
                                ImGui::DragFloat3("Rotation", &entity->transform.rotation.x, 0.1f, -180.0f, 180.0f);
                                ImGui::DragFloat3("Scale", &entity->transform.scale.x, 0.001f, 0.001f, 100.0f);
                                ImGui::Separator();
                                const bool wasRenderable = entity->hasRenderable;
                                ImGui::Checkbox("Renderable", &entity->hasRenderable);
                                if (!wasRenderable && entity->hasRenderable) {
                                        attachRenderableComponent(entity, PrimitiveType::Cube);
                                } else if (wasRenderable && !entity->hasRenderable) {
                                        detachRenderableComponent(entity);
                                }

                                if (entity->hasRenderable) {
                                        ImGui::ColorEdit4("Tint", &entity->material.tint.x);
                                        ImGui::SliderFloat("Metallic", &entity->material.metallic, 0.0f, 1.0f);
                                        ImGui::SliderFloat("Roughness", &entity->material.roughness, 0.0f, 1.0f);
                                        ImGui::Checkbox("Wireframe (Entity)", &entity->material.wireframe);
                                } else {
                                        ImGui::Checkbox("Show Origin Marker", &entity->showOriginMarker);
                                        ImGui::ColorEdit4("Marker Color", &entity->material.tint.x);
                                }
                        } else {
                                ImGui::Text("Select an entity in Hierarchy.");
                        }
                }
                ImGui::End();

                ImGui::SetNextWindowPos(ImVec2(0.0f, consoleY));
                ImGui::SetNextWindowSize(ImVec2(displaySize.x, consoleHeight));
                if (ImGui::Begin("Console", nullptr, dockedWindowFlags | ImGuiWindowFlags_NoResize)) {
                        const float commandBarHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;
                        log.DrawBody(commandBarHeight);
                        ImGui::Separator();
                        ImGui::SetNextItemWidth(-70.0f);
                        const bool submitted = ImGui::InputTextWithHint(
                                "##ConsoleCommand",
                                "save | saveas \"scene\" | load \"scene\" | project \"name\" | help",
                                consoleCommandBuffer,
                                IM_ARRAYSIZE(consoleCommandBuffer),
                                ImGuiInputTextFlags_EnterReturnsTrue);
                        ImGui::SameLine();
                        const bool runClicked = ImGui::Button("Run");
                        if (submitted || runClicked) {
                                const std::string command = trimWhitespace(consoleCommandBuffer);
                                if (!command.empty()) {
                                        executeConsoleCommand(scene, command);
                                }
                                consoleCommandBuffer[0] = '\0';
                        }
                }
                ImGui::End();

                ImGui::SetNextWindowPos(ImVec2(0.0f, statusBarY));
                ImGui::SetNextWindowSize(ImVec2(displaySize.x, statusBarHeight));
                if (ImGui::Begin("Status", nullptr, dockedWindowFlags | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {
                        ImGui::AlignTextToFramePadding();
                        ImGui::Text("Engine: Running");
                        ImGui::SameLine();
                        ImGui::Text("| FPS: %.1f", ImGui::GetIO().Framerate);
                        ImGui::SameLine();
                        ImGui::Text("| Entities: %d", static_cast<int>(scene.entities.size()));
                        ImGui::SameLine();
                        if (sceneRootSelected) {
                                ImGui::Text("| Selected: Scene");
                        } else {
                                ImGui::Text("| Selected: %s", selectedEntity ? selectedEntity->name.c_str() : "<none>");
                        }
                }
                ImGui::End();

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        void terminate() {
                if (sceneTextureID != 0) {
                        glDeleteTextures(1, &sceneTextureID);
                        sceneTextureID = 0;
                }
                ImGui_ImplOpenGL3_Shutdown();
    	        ImGui_ImplGlfw_Shutdown();
    	        ImGui::DestroyContext();
        }

        void onNotify(Message message) {
                        std::time_t t = std::time(0);   // get time now
                        std::tm* now = std::localtime(&t);
                        log.AddLog("[%d-%d-%d %d:%d:%d] > %s\n", (now->tm_mon + 1), now->tm_mday,(now->tm_year + 1900), (now->tm_hour), (now->tm_min), (now->tm_sec),message.getEvent().c_str());
                        printf("[%d-%d-%d %d:%d:%d] > %s\n", (now->tm_mon + 1), now->tm_mday, (now->tm_year + 1900), (now->tm_hour), (now->tm_min), (now->tm_sec), message.getEvent().c_str());
        }

private:
        static std::string primitiveBaseName(PrimitiveType type) {
                switch (type) {
                case PrimitiveType::Plane:
                        return "Plane";
                case PrimitiveType::Cube:
                        return "Cube";
                case PrimitiveType::Sphere:
                        return "Sphere";
                }
                return "Model";
        }

        static std::string makeUniqueModelName(const Scene& scene, const std::string& baseName) {
                std::function<bool(const Entity*, const std::string&)> existsInTree =
                        [&](const Entity* entity, const std::string& candidate) -> bool {
                                if (entity == nullptr) {
                                        return false;
                                }
                                if (entity->name == candidate) {
                                        return true;
                                }
                                for (const Entity* child : entity->children) {
                                        if (existsInTree(child, candidate)) {
                                                return true;
                                        }
                                }
                                return false;
                        };
                auto exists = [&](const std::string& candidate) -> bool {
                        for (const Entity* entity : scene.entities) {
                                if (existsInTree(entity, candidate)) {
                                        return true;
                                }
                        }
                        return false;
                };

                if (!exists(baseName)) {
                        return baseName;
                }

                int suffix = 1;
                while (true) {
                        const std::string candidate = baseName + " " + std::to_string(suffix);
                        if (!exists(candidate)) {
                                return candidate;
                        }
                        suffix++;
                }
        }

        bool attachRenderableComponent(Entity* entity, PrimitiveType type) {
                if (entity == nullptr) {
                        return false;
                }
                if (entity->hasRenderable && entity->renderable.model != nullptr) {
                        return true;
                }

                const std::string baseName = primitiveBaseName(type);
                const char* defaultTexturePath = "res/images/default_white.png";
                Model* model = nullptr;
                if (type == PrimitiveType::Plane) {
                        model = new Model(entity->name.c_str(), defaultTexturePath, glm::vec2(1.0f), entity->transform);
                } else if (type == PrimitiveType::Cube) {
                        model = new Model(entity->name.c_str(), defaultTexturePath, CubeGeometry(), entity->transform);
                } else if (type == PrimitiveType::Sphere) {
                        model = new Model(entity->name.c_str(), defaultTexturePath, SphereGeometry(), entity->transform);
                }
                if (model == nullptr) {
                        return false;
                }
                entity->renderable.model = model;
                entity->hasRenderable = true;
                entity->showOriginMarker = false;
                log.AddLog("[ECS] Added RenderableComponent (%s) to '%s'\n", baseName.c_str(), entity->name.c_str());
                return true;
        }

        bool detachRenderableComponent(Entity* entity) {
                if (entity == nullptr || !entity->hasRenderable) {
                        return true;
                }
                if (entity->renderable.model != nullptr) {
                        entity->renderable.model->destroy();
                        delete entity->renderable.model;
                        entity->renderable.model = nullptr;
                }
                entity->hasRenderable = false;
                entity->showOriginMarker = true;
                log.AddLog("[ECS] Removed RenderableComponent from '%s'\n", entity->name.c_str());
                return true;
        }

        bool addPrimitiveToScene(Scene& scene, PrimitiveType type, Entity* parent = nullptr) {
                const std::string baseName = primitiveBaseName(type);
                const std::string entityName = makeUniqueModelName(scene, baseName);
                Entity* entity = scene.createEmptyEntity(entityName, Transform(), parent);
                if (entity == nullptr) {
                        return false;
                }
                if (!attachRenderableComponent(entity, type)) {
                        scene.removeEntity(entity);
                        return false;
                }
                selectedEntity = entity;
                sceneRootSelected = false;
                log.AddLog("[SCENE] Added %s '%s'%s\n",
                        baseName.c_str(),
                        entityName.c_str(),
                        parent ? " as child" : "");
                return true;
        }

        bool addEmptyEntityToScene(Scene& scene, Entity* parent = nullptr) {
                const std::string entityName = makeUniqueModelName(scene, "Entity");
                Entity* entity = scene.createEmptyEntity(entityName, Transform(), parent);
                if (entity == nullptr) {
                        return false;
                }
                entity->material.tint = glm::vec4(1.0f, 0.95f, 0.2f, 1.0f);
                selectedEntity = entity;
                sceneRootSelected = false;
                log.AddLog("[SCENE] Added Empty Entity '%s'%s\n", entityName.c_str(), parent ? " as child" : "");
                return true;
        }

        bool deleteEntity(Scene& scene, Entity* entity) {
                if (entity == nullptr) {
                        return false;
                }
                std::function<bool(Entity*, Entity*)> containsEntity = [&](Entity* root, Entity* target) -> bool {
                        if (root == nullptr || target == nullptr) {
                                return false;
                        }
                        if (root == target) {
                                return true;
                        }
                        for (Entity* child : root->children) {
                                if (containsEntity(child, target)) {
                                        return true;
                                }
                        }
                        return false;
                };
                const std::string entityName = entity->name;
                const bool deletingSelected = containsEntity(entity, selectedEntity);
                if (scene.removeEntity(entity)) {
                        if (deletingSelected) {
                                selectedEntity = nullptr;
                                sceneRootSelected = true;
                        }
                        log.AddLog("[SCENE] Deleted entity '%s'\n", entityName.c_str());
                        return true;
                }
                return false;
        }
        static std::string firstExistingPath(const std::string& rawPath, const std::filesystem::path& baseDir, bool wantDirectory) {
                namespace fs = std::filesystem;
                std::vector<fs::path> candidates;
                candidates.push_back(fs::path(rawPath));
                if (!rawPath.empty() && rawPath[0] != '/' && !(rawPath.size() > 1 && rawPath[1] == ':')) {
                        candidates.push_back((baseDir / rawPath).lexically_normal());
                        candidates.push_back((fs::path("..") / rawPath).lexically_normal());
                }

                for (const fs::path& p : candidates) {
                        if (wantDirectory) {
                                if (fs::exists(p) && fs::is_directory(p)) {
                                        std::string resolved = p.generic_string();
                                        if (!resolved.empty() && resolved.back() != '/') {
                                                resolved.push_back('/');
                                        }
                                        return resolved;
                                }
                        } else {
                                if (fs::exists(p) && fs::is_regular_file(p)) {
                                        return p.generic_string();
                                }
                        }
                }

                return rawPath;
        }

        static std::filesystem::path detectProjectRootPath() {
                namespace fs = std::filesystem;
                if (fs::exists("res") && fs::is_directory("res") && fs::exists("src") && fs::is_directory("src")) {
                        return fs::path(".");
                }
                if (fs::exists("../res") && fs::is_directory("../res") && fs::exists("../src") && fs::is_directory("../src")) {
                        return fs::path("..");
                }
                return fs::path(".");
        }

        std::filesystem::path currentProjectPath() const {
                return (detectProjectRootPath() / "projects" / currentProjectName).lexically_normal();
        }

        std::filesystem::path currentProjectScenesPath() const {
                return (currentProjectPath() / "scenes").lexically_normal();
        }

        std::filesystem::path currentProjectAssetsPath() const {
                return (currentProjectPath() / "assets").lexically_normal();
        }

        void updateCurrentProjectFromScenePath(const std::string& path) {
                namespace fs = std::filesystem;
                fs::path normalized = fs::path(path).lexically_normal();
                std::vector<std::string> parts;
                for (const auto& part : normalized) {
                        parts.push_back(part.string());
                }
                for (size_t i = 0; i + 1 < parts.size(); ++i) {
                        if (parts[i] == "projects" && !parts[i + 1].empty()) {
                                currentProjectName = parts[i + 1];
                                return;
                        }
                }
        }

        static std::string trimWhitespace(const std::string& value) {
                size_t start = 0;
                while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
                        ++start;
                }
                size_t end = value.size();
                while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
                        --end;
                }
                return value.substr(start, end - start);
        }

        static std::vector<std::string> tokenizeCommand(const std::string& line) {
                std::vector<std::string> tokens;
                std::string current;
                bool inQuotes = false;
                for (char c : line) {
                        if (c == '"') {
                                inQuotes = !inQuotes;
                                continue;
                        }

                        if (!inQuotes && std::isspace(static_cast<unsigned char>(c)) != 0) {
                                if (!current.empty()) {
                                        tokens.push_back(current);
                                        current.clear();
                                }
                                continue;
                        }

                        current.push_back(c);
                }

                if (!current.empty()) {
                        tokens.push_back(current);
                }
                return tokens;
        }

        static std::string toLowerCopy(std::string value) {
                std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                });
                return value;
        }

        std::string expandSceneShortcut(const std::string& token, bool forWrite) const {
                namespace fs = std::filesystem;
                const std::string trimmed = trimWhitespace(token);
                if (trimmed.empty()) {
                        return sceneJsonPath;
                }

                fs::path p(trimmed);
                const bool hasSeparator = trimmed.find('/') != std::string::npos || trimmed.find('\\') != std::string::npos;
                if (p.is_absolute() || hasSeparator) {
                        return trimmed;
                }

                const fs::path projectRoot = detectProjectRootPath();
                const fs::path projectScenesDir = currentProjectScenesPath();
                if (!p.has_extension()) {
                        if (forWrite) {
                                return (projectScenesDir / (trimmed + ".scene.json")).lexically_normal().generic_string();
                        }

                        const std::vector<fs::path> candidates = {
                                (projectScenesDir / (trimmed + ".scene.json")).lexically_normal(),
                                (projectScenesDir / (trimmed + ".json")).lexically_normal(),
                                (projectRoot / "projects" / trimmed / "scenes/default.scene.json").lexically_normal(),
                                (projectRoot / "projects" / trimmed / (trimmed + ".scene.json")).lexically_normal(),
                                (projectRoot / "projects" / trimmed / (trimmed + ".json")).lexically_normal(),
                                (projectRoot / "res/scenes" / (trimmed + ".scene.json")).lexically_normal(),
                                (projectRoot / "res/scenes" / (trimmed + ".json")).lexically_normal()
                        };
                        for (const fs::path& candidate : candidates) {
                                if (fs::exists(candidate) && fs::is_regular_file(candidate)) {
                                        return candidate.generic_string();
                                }
                        }
                        return candidates[0].generic_string();
                }

                const fs::path inSceneDir = (projectScenesDir / p).lexically_normal();
                if (forWrite) {
                        return inSceneDir.generic_string();
                }
                if (fs::exists(inSceneDir) && fs::is_regular_file(inSceneDir)) {
                        return inSceneDir.generic_string();
                }
                const fs::path sharedSceneDir = (projectRoot / "res/scenes" / p).lexically_normal();
                if (fs::exists(sharedSceneDir) && fs::is_regular_file(sharedSceneDir)) {
                        return sharedSceneDir.generic_string();
                }
                return trimmed;
        }

        std::string resolvePathForRead(const std::string& rawPath) const {
                namespace fs = std::filesystem;
                const fs::path raw(rawPath);
                std::vector<fs::path> candidates;
                candidates.push_back(raw.lexically_normal());
                if (raw.is_absolute()) {
                        // absolute candidate already added
                } else {
                        const fs::path projectRoot = detectProjectRootPath();
                        candidates.push_back((projectRoot / raw).lexically_normal());
                }

                for (const fs::path& p : candidates) {
                        if (fs::exists(p) && fs::is_regular_file(p)) {
                                return p.generic_string();
                        }
                }
                if (!candidates.empty()) {
                        return candidates[0].generic_string();
                }
                return rawPath;
        }

        std::string resolvePathForWrite(const std::string& rawPath) const {
                namespace fs = std::filesystem;
                fs::path p(rawPath);
                if (p.empty()) {
                        p = fs::path("projects/default/scenes/default.scene.json");
                }
                if (p.extension().empty()) {
                        p += ".json";
                }
                if (p.is_absolute()) {
                        return p.lexically_normal().generic_string();
                }
                const std::string rawNormalized = p.lexically_normal().generic_string();
                if (rawNormalized.rfind("./", 0) == 0 || rawNormalized.rfind("../", 0) == 0) {
                        return rawNormalized;
                }
                if (rawNormalized.rfind("projects/", 0) == 0 || rawNormalized.rfind("res/", 0) == 0) {
                        return (detectProjectRootPath() / p).lexically_normal().generic_string();
                }
                return (currentProjectScenesPath() / p).lexically_normal().generic_string();
        }

        bool loadSceneAtPath(Scene& scene, const std::string& rawPath) {
                const std::string expandedPath = expandSceneShortcut(rawPath, false);
                const std::string resolvedPath = resolvePathForRead(expandedPath);
                std::string error;
                if (loadSceneJson(scene, resolvedPath, &error)) {
                        std::snprintf(sceneJsonPath, IM_ARRAYSIZE(sceneJsonPath), "%s", resolvedPath.c_str());
                        updateSceneDialogDirectoryFromPath(resolvedPath);
                        updateCurrentProjectFromScenePath(resolvedPath);
                        sceneRootSelected = true;
                        selectedEntity = nullptr;
                        log.AddLog("[SCENE] Loaded scene '%s'\n", resolvedPath.c_str());
                        return true;
                }
                log.AddLog("[ERROR] Failed to load scene '%s': %s\n", resolvedPath.c_str(), error.c_str());
                return false;
        }

        bool saveSceneAtPath(Scene& scene, const std::string& rawPath) {
                const std::string expandedPath = expandSceneShortcut(rawPath, true);
                const std::string resolvedPath = resolvePathForWrite(expandedPath);
                std::string error;
                if (saveSceneJson(scene, resolvedPath, &error)) {
                        std::snprintf(sceneJsonPath, IM_ARRAYSIZE(sceneJsonPath), "%s", resolvedPath.c_str());
                        updateSceneDialogDirectoryFromPath(resolvedPath);
                        updateCurrentProjectFromScenePath(resolvedPath);
                        log.AddLog("[SCENE] Saved scene '%s'\n", resolvedPath.c_str());
                        return true;
                }
                log.AddLog("[ERROR] Failed to save scene '%s': %s\n", resolvedPath.c_str(), error.c_str());
                return false;
        }

        void executeConsoleCommand(Scene& scene, const std::string& line) {
                const std::string trimmed = trimWhitespace(line);
                if (trimmed.empty()) {
                        return;
                }

                log.AddLog("[CMD] %s\n", trimmed.c_str());
                const std::vector<std::string> tokens = tokenizeCommand(trimmed);
                if (tokens.empty()) {
                        return;
                }

                const std::string command = toLowerCopy(tokens[0]);
                if (command == "save") {
                        saveSceneAtPath(scene, sceneJsonPath);
                        return;
                }
                if (command == "saveas") {
                        if (tokens.size() < 2) {
                                log.AddLog("[ERROR] saveas requires a target name or path.\n");
                                return;
                        }
                        saveSceneAtPath(scene, tokens[1]);
                        return;
                }
                if (command == "load") {
                        if (tokens.size() < 2) {
                                log.AddLog("[ERROR] load requires a scene name, project name, or path.\n");
                                return;
                        }
                        loadSceneAtPath(scene, tokens[1]);
                        requestedWorkspaceTabIndex = 0;
                        return;
                }
                if (command == "open") {
                        requestedWorkspaceTabIndex = 1;
                        log.AddLog("[INFO] Opened Scene Files tab.\n");
                        return;
                }
                if (command == "project") {
                        if (tokens.size() < 2) {
                                log.AddLog("[ERROR] project requires a project name.\n");
                                return;
                        }
                        currentProjectName = tokens[1];
                        const std::string projectScenePath = (currentProjectScenesPath() / "default.scene.json").lexically_normal().generic_string();
                        std::snprintf(sceneJsonPath, IM_ARRAYSIZE(sceneJsonPath), "%s", projectScenePath.c_str());
                        sceneDialogDirectory = currentProjectScenesPath().generic_string();
                        log.AddLog("[INFO] Active project set to '%s'\n", currentProjectName.c_str());
                        return;
                }
                if (command == "help") {
                        log.AddLog("[INFO] Commands: save | saveas \"name-or-path\" | load \"scene-or-project\" | project \"name\" | open\n");
                        return;
                }

                log.AddLog("[ERROR] Unknown command '%s'. Try 'help'.\n", tokens[0].c_str());
        }

        void updateSceneDialogDirectoryFromPath(const std::string& path) {
                namespace fs = std::filesystem;
                if (path.empty()) {
                        return;
                }

                fs::path p(path);
                fs::path dir;
                std::error_code ec;
                if (fs::exists(p, ec) && fs::is_directory(p, ec)) {
                        dir = p;
                } else if (p.has_parent_path()) {
                        dir = p.parent_path();
                } else {
                        return;
                }

                dir = dir.lexically_normal();
                if (!dir.empty()) {
                        sceneDialogDirectory = dir.generic_string();
                }
        }

        void drawSceneFileBrowser(char* pathBuffer, size_t pathBufferSize, const char* idSuffix) {
                namespace fs = std::filesystem;
                fs::path dir(sceneDialogDirectory);
                if (!fs::exists(dir) || !fs::is_directory(dir)) {
                        fs::path projectSceneDir = currentProjectScenesPath();
                        fs::path sharedSceneDir = (detectProjectRootPath() / "res/scenes").lexically_normal();
                        if (fs::exists(projectSceneDir) && fs::is_directory(projectSceneDir)) {
                                dir = projectSceneDir;
                        } else if (fs::exists(sharedSceneDir) && fs::is_directory(sharedSceneDir)) {
                                dir = sharedSceneDir;
                        } else {
                                dir = fs::path(".");
                        }
                        sceneDialogDirectory = dir.generic_string();
                }

                std::string upButtonId = std::string("Up##") + idSuffix;
                if (ImGui::Button(upButtonId.c_str())) {
                        if (dir.has_parent_path()) {
                                sceneDialogDirectory = dir.parent_path().generic_string();
                                dir = fs::path(sceneDialogDirectory);
                        }
                }
                ImGui::SameLine();
                ImGui::Text("Directory: %s", sceneDialogDirectory.c_str());

                std::vector<fs::directory_entry> directories;
                std::vector<fs::directory_entry> jsonFiles;
                std::error_code iterEc;
                for (const auto& entry : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, iterEc)) {
                        if (entry.is_directory()) {
                                directories.push_back(entry);
                        } else if (entry.is_regular_file()) {
                                std::string ext = entry.path().extension().string();
                                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                                if (ext == ".json") {
                                        jsonFiles.push_back(entry);
                                }
                        }
                }
                if (iterEc) {
                        ImGui::Text("Warning: cannot read '%s'", sceneDialogDirectory.c_str());
                }

                auto sortByName = [](const fs::directory_entry& a, const fs::directory_entry& b) {
                        return a.path().filename().string() < b.path().filename().string();
                };
                std::sort(directories.begin(), directories.end(), sortByName);
                std::sort(jsonFiles.begin(), jsonFiles.end(), sortByName);

                std::string browserId = std::string("SceneBrowser##") + idSuffix;
                if (ImGui::BeginChild(browserId.c_str(), ImVec2(620.0f, 220.0f), true)) {
                        for (const auto& d : directories) {
                                std::string label = "[DIR] " + d.path().filename().string();
                                if (ImGui::Selectable(label.c_str(), false)) {
                                        sceneDialogDirectory = d.path().generic_string();
                                }
                        }
                        for (const auto& f : jsonFiles) {
                                std::string label = f.path().filename().string();
                                if (ImGui::Selectable(label.c_str(), false)) {
                                        std::string selectedPath = f.path().generic_string();
                                        std::snprintf(pathBuffer, pathBufferSize, "%s", selectedPath.c_str());
                                }
                        }
                }
                ImGui::EndChild();
        }

        static SimpleJson::Value transformToJson(const Transform& transform) {
                SimpleJson::Value obj = SimpleJson::Value::makeObject();
                SimpleJson::Value position = SimpleJson::Value::makeArray();
                position.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.position.x));
                position.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.position.y));
                position.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.position.z));

                SimpleJson::Value rotation = SimpleJson::Value::makeArray();
                rotation.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.rotation.x));
                rotation.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.rotation.y));
                rotation.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.rotation.z));

                SimpleJson::Value scale = SimpleJson::Value::makeArray();
                scale.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.scale.x));
                scale.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.scale.y));
                scale.arrayValue.push_back(SimpleJson::Value::makeNumber(transform.scale.z));

                obj.objectValue["position"] = position;
                obj.objectValue["rotation"] = rotation;
                obj.objectValue["scale"] = scale;
                return obj;
        }

        static bool jsonToVec2(const SimpleJson::Value* value, glm::vec2& out) {
                if (value == nullptr || !value->isArray() || value->arrayValue.size() != 2) {
                        return false;
                }
                if (!value->arrayValue[0].isNumber() || !value->arrayValue[1].isNumber()) {
                        return false;
                }
                out.x = static_cast<float>(value->arrayValue[0].numberValue);
                out.y = static_cast<float>(value->arrayValue[1].numberValue);
                return true;
        }

        static bool jsonToVec3(const SimpleJson::Value* value, glm::vec3& out) {
                if (value == nullptr || !value->isArray() || value->arrayValue.size() != 3) {
                        return false;
                }
                if (!value->arrayValue[0].isNumber() || !value->arrayValue[1].isNumber() || !value->arrayValue[2].isNumber()) {
                        return false;
                }
                out.x = static_cast<float>(value->arrayValue[0].numberValue);
                out.y = static_cast<float>(value->arrayValue[1].numberValue);
                out.z = static_cast<float>(value->arrayValue[2].numberValue);
                return true;
        }

        static bool jsonToVec4(const SimpleJson::Value* value, glm::vec4& out) {
                if (value == nullptr || !value->isArray() || value->arrayValue.size() != 4) {
                        return false;
                }
                if (!value->arrayValue[0].isNumber() || !value->arrayValue[1].isNumber() ||
                        !value->arrayValue[2].isNumber() || !value->arrayValue[3].isNumber()) {
                        return false;
                }
                out.x = static_cast<float>(value->arrayValue[0].numberValue);
                out.y = static_cast<float>(value->arrayValue[1].numberValue);
                out.z = static_cast<float>(value->arrayValue[2].numberValue);
                out.w = static_cast<float>(value->arrayValue[3].numberValue);
                return true;
        }

        static SimpleJson::Value materialToJson(const MaterialComponent& material) {
                SimpleJson::Value materialObj = SimpleJson::Value::makeObject();
                SimpleJson::Value tint = SimpleJson::Value::makeArray();
                tint.arrayValue.push_back(SimpleJson::Value::makeNumber(material.tint.x));
                tint.arrayValue.push_back(SimpleJson::Value::makeNumber(material.tint.y));
                tint.arrayValue.push_back(SimpleJson::Value::makeNumber(material.tint.z));
                tint.arrayValue.push_back(SimpleJson::Value::makeNumber(material.tint.w));
                materialObj.objectValue["tint"] = tint;
                materialObj.objectValue["metallic"] = SimpleJson::Value::makeNumber(material.metallic);
                materialObj.objectValue["roughness"] = SimpleJson::Value::makeNumber(material.roughness);
                materialObj.objectValue["wireframe"] = SimpleJson::Value::makeBool(material.wireframe);
                return materialObj;
        }

        SimpleJson::Value entityToJson(const Entity* entity) const {
                SimpleJson::Value entityObj = SimpleJson::Value::makeObject();
                if (entity == nullptr) {
                        return entityObj;
                }

                entityObj.objectValue["name"] = SimpleJson::Value::makeString(entity->name);
                entityObj.objectValue["order"] = SimpleJson::Value::makeNumber(entity->order);
                entityObj.objectValue["transform"] = transformToJson(entity->transform);
                entityObj.objectValue["has_renderable"] = SimpleJson::Value::makeBool(entity->hasRenderable);
                entityObj.objectValue["show_origin_marker"] = SimpleJson::Value::makeBool(entity->showOriginMarker);
                entityObj.objectValue["material"] = materialToJson(entity->material);

                if (entity->hasRenderable && entity->renderable.model != nullptr) {
                        Model* model = entity->renderable.model;
                        entityObj.objectValue["type"] = SimpleJson::Value::makeString(model->sourceType);
                        if (model->sourceType == "obj") {
                                entityObj.objectValue["obj_path"] = SimpleJson::Value::makeString(model->sourceObjPath);
                                entityObj.objectValue["mtl_dir"] = SimpleJson::Value::makeString(model->sourceMtlDir);
                        } else {
                                entityObj.objectValue["texture"] = SimpleJson::Value::makeString(model->ownedTexturePath);
                                SimpleJson::Value size = SimpleJson::Value::makeArray();
                                size.arrayValue.push_back(SimpleJson::Value::makeNumber(model->sourcePlaneSize.x));
                                size.arrayValue.push_back(SimpleJson::Value::makeNumber(model->sourcePlaneSize.y));
                                entityObj.objectValue["size"] = size;
                        }
                } else {
                        entityObj.objectValue["type"] = SimpleJson::Value::makeString("empty");
                }

                SimpleJson::Value children = SimpleJson::Value::makeArray();
                for (Entity* child : entity->children) {
                        children.arrayValue.push_back(entityToJson(child));
                }
                entityObj.objectValue["children"] = children;
                return entityObj;
        }

        Entity* entityFromJson(const SimpleJson::Value& node,
                const std::filesystem::path& sceneFileDir,
                std::string* error) {
                if (!node.isObject()) {
                        return nullptr;
                }

                const SimpleJson::Value* modelName = node.get("name");
                const SimpleJson::Value* modelType = node.get("type");
                const SimpleJson::Value* modelOrder = node.get("order");
                const SimpleJson::Value* modelTransform = node.get("transform");
                if (modelName == nullptr || !modelName->isString() ||
                        modelType == nullptr || !modelType->isString() ||
                        modelOrder == nullptr || !modelOrder->isNumber() ||
                        modelTransform == nullptr || !modelTransform->isObject()) {
                        return nullptr;
                }

                Transform transform;
                if (!jsonToVec3(modelTransform->get("position"), transform.position) ||
                        !jsonToVec3(modelTransform->get("rotation"), transform.rotation) ||
                        !jsonToVec3(modelTransform->get("scale"), transform.scale)) {
                        return nullptr;
                }

                Entity* entity = new Entity();
                entity->name = modelName->stringValue;
                entity->order = static_cast<int>(modelOrder->numberValue);
                entity->transform = transform;
                entity->hasRenderable = (modelType->stringValue != "empty");
                const SimpleJson::Value* hasRenderable = node.get("has_renderable");
                if (hasRenderable != nullptr && hasRenderable->isBool()) {
                        entity->hasRenderable = hasRenderable->boolValue;
                }
                const SimpleJson::Value* showMarker = node.get("show_origin_marker");
                if (showMarker != nullptr && showMarker->isBool()) {
                        entity->showOriginMarker = showMarker->boolValue;
                } else {
                        entity->showOriginMarker = !entity->hasRenderable;
                }

                const SimpleJson::Value* materialObj = node.get("material");
                if (materialObj != nullptr && materialObj->isObject()) {
                        jsonToVec4(materialObj->get("tint"), entity->material.tint);
                        const SimpleJson::Value* metallic = materialObj->get("metallic");
                        if (metallic != nullptr && metallic->isNumber()) {
                                entity->material.metallic = static_cast<float>(metallic->numberValue);
                        }
                        const SimpleJson::Value* roughness = materialObj->get("roughness");
                        if (roughness != nullptr && roughness->isNumber()) {
                                entity->material.roughness = static_cast<float>(roughness->numberValue);
                        }
                        const SimpleJson::Value* wireframe = materialObj->get("wireframe");
                        if (wireframe != nullptr && wireframe->isBool()) {
                                entity->material.wireframe = wireframe->boolValue;
                        }
                }

                Model* model = nullptr;
                if (modelType->stringValue == "obj") {
                        const SimpleJson::Value* objPath = node.get("obj_path");
                        const SimpleJson::Value* mtlDir = node.get("mtl_dir");
                        if (objPath == nullptr || !objPath->isString() || mtlDir == nullptr || !mtlDir->isString()) {
                                delete entity;
                                return nullptr;
                        }
                        std::string resolvedObjPath = firstExistingPath(objPath->stringValue, sceneFileDir, false);
                        std::string resolvedMtlDir = firstExistingPath(mtlDir->stringValue, sceneFileDir, true);
                        model = new Model(entity->name.c_str(), transform);
                        if (!model->LoadOBJ(resolvedObjPath.c_str(), resolvedMtlDir.c_str())) {
                                delete model;
                                delete entity;
                                return nullptr;
                        }
                } else if (modelType->stringValue == "plane") {
                        const SimpleJson::Value* texture = node.get("texture");
                        const SimpleJson::Value* size = node.get("size");
                        glm::vec2 bounds;
                        if (texture == nullptr || !texture->isString() || !jsonToVec2(size, bounds)) {
                                delete entity;
                                return nullptr;
                        }
                        std::string resolvedTexturePath = firstExistingPath(texture->stringValue, sceneFileDir, false);
                        model = new Model(entity->name.c_str(), resolvedTexturePath.c_str(), bounds, transform);
                } else if (modelType->stringValue == "empty") {
                        model = nullptr;
                } else {
                        delete entity;
                        return nullptr;
                }

                if (entity->hasRenderable && model == nullptr) {
                        delete entity;
                        return nullptr;
                }
                if (model != nullptr) {
                        model->order = entity->order;
                        model->transform = entity->transform;
                        entity->renderable.model = model;
                        entity->hasRenderable = true;
                }

                const SimpleJson::Value* childrenValue = node.get("children");
                if (childrenValue != nullptr && childrenValue->isArray()) {
                        for (const SimpleJson::Value& childNode : childrenValue->arrayValue) {
                                Entity* child = entityFromJson(childNode, sceneFileDir, error);
                                if (child != nullptr) {
                                        child->parent = entity;
                                        entity->children.push_back(child);
                                }
                        }
                }

                return entity;
        }

        bool saveSceneJson(const Scene& scene, const std::string& path, std::string* error) {
                namespace fs = std::filesystem;
                fs::path outPath(path);
                if (outPath.has_parent_path()) {
                        std::error_code ec;
                        fs::create_directories(outPath.parent_path(), ec);
                }

                SimpleJson::Value root = SimpleJson::Value::makeObject();
                root.objectValue["version"] = SimpleJson::Value::makeNumber(1.0);

                SimpleJson::Value sceneObj = SimpleJson::Value::makeObject();
                sceneObj.objectValue["name"] = SimpleJson::Value::makeString(scene.name);

                SimpleJson::Value color = SimpleJson::Value::makeArray();
                color.arrayValue.push_back(SimpleJson::Value::makeNumber(scene.color.x));
                color.arrayValue.push_back(SimpleJson::Value::makeNumber(scene.color.y));
                color.arrayValue.push_back(SimpleJson::Value::makeNumber(scene.color.z));
                color.arrayValue.push_back(SimpleJson::Value::makeNumber(scene.color.w));
                sceneObj.objectValue["color"] = color;

                SimpleJson::Value models = SimpleJson::Value::makeArray();
                for (Entity* entity : scene.entities) {
                        models.arrayValue.push_back(entityToJson(entity));
                }

                sceneObj.objectValue["models"] = models;
                root.objectValue["scene"] = sceneObj;
                return SimpleJson::writeFile(path, root, error);
        }

        bool loadSceneJson(Scene& scene, const std::string& path, std::string* error) {
                SimpleJson::Value root;
                if (!SimpleJson::parseFile(path, root, error)) {
                        return false;
                }

                const SimpleJson::Value* sceneObj = root.get("scene");
                if (sceneObj == nullptr || !sceneObj->isObject()) {
                        if (error != nullptr) {
                                *error = "Missing 'scene' object.";
                        }
                        return false;
                }

                glm::vec4 loadedColor;
                if (!jsonToVec4(sceneObj->get("color"), loadedColor)) {
                        if (error != nullptr) {
                                *error = "Invalid scene color.";
                        }
                        return false;
                }

                const SimpleJson::Value* modelsValue = sceneObj->get("models");
                if (modelsValue == nullptr || !modelsValue->isArray()) {
                        if (error != nullptr) {
                                *error = "Missing models array.";
                        }
                        return false;
                }

                scene.clear();
                const SimpleJson::Value* nameValue = sceneObj->get("name");
                if (nameValue != nullptr && nameValue->isString()) {
                        scene.name = nameValue->stringValue;
                } else {
                        scene.name = "scene";
                }
                scene.color = loadedColor;
                std::filesystem::path sceneFileDir = std::filesystem::path(path).parent_path();

                for (const SimpleJson::Value& modelValue : modelsValue->arrayValue) {
                        Entity* entity = entityFromJson(modelValue, sceneFileDir, error);
                        if (entity != nullptr) {
                                scene.addEntity(entity);
                        }
                }

                return true;
        }

        void captureSceneFramebuffer() {
                GLint viewport[4] = { 0, 0, 0, 0 };
                glGetIntegerv(GL_VIEWPORT, viewport);
                const int width = std::max(1, viewport[2]);
                const int height = std::max(1, viewport[3]);
                ensureSceneTexture(width, height);

                glBindTexture(GL_TEXTURE_2D, sceneTextureID);
                glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
                glBindTexture(GL_TEXTURE_2D, 0);
        }

        void ensureSceneTexture(int width, int height) {
                if (sceneTextureID == 0) {
                        glGenTextures(1, &sceneTextureID);
                        glBindTexture(GL_TEXTURE_2D, sceneTextureID);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                        glBindTexture(GL_TEXTURE_2D, 0);
                }

                if (sceneTextureWidth != width || sceneTextureHeight != height) {
                        sceneTextureWidth = width;
                        sceneTextureHeight = height;
                        glBindTexture(GL_TEXTURE_2D, sceneTextureID);
                        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sceneTextureWidth, sceneTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                        glBindTexture(GL_TEXTURE_2D, 0);
                }
        }
};

class Console : public System {
public:
        Console(MessageBus* messageBus) : System({ ENGINE_EVENT, CONSOLE_EVENT }, messageBus) {}

        void update() override {

        }
private:
        void onNotify(Message message)
        {
                //timestamp();
                //std::cout << message.getEvent() << std::endl;
        }
        void timestamp() {
                std::time_t t = std::time(0);   // get time now
                std::tm* now = std::localtime(&t);
                std::cout << '[' << (now->tm_year + 1900) << '-'
                        << (now->tm_mon + 1) << '-'
                        << now->tm_mday << ' ' << (now->tm_hour) << ':' << (now->tm_min) << ':' << (now->tm_sec) << "] ";
        }
};

#endif /* GUI_H */
