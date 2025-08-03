#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <string>
#include "Graph.h"

struct VisualNode {
    int id;
    ImVec2 position;
};

GLFWwindow* window;
const char* glsl_version = "#version 130";

Graph graph;
std::vector<VisualNode> nodes;
std::vector<std::pair<int, int>> edges;

int nextNodeId = 0;
int selectedEdgeStart = -1;
float edgeWeightInput = 1.0f;

int selectedPathStart = -1;
int selectedPathEnd = -1;
std::vector<int> shortestPath;

int dfsStart = -1;                     
int dfsEnd = -1;                      
std::vector<int> dfsPathResult;       

float distance(ImVec2 a, ImVec2 b) {
    return std::sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

int getNodeAtPosition(ImVec2 mousePos, float radius = 15.0f) {
    for (const auto& node : nodes) {
        if (distance(mousePos, node.position) <= radius)
            return node.id;
    }
    return -1;
}

void setupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* bigFont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 20.0f);
  

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void cleanupImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void renderGraph(ImDrawList* drawList) {
    for (const auto& edge : edges) {
        ImVec2 pos1, pos2;
        for (const auto& node : nodes) {
            if (node.id == edge.first) pos1 = node.position;
            if (node.id == edge.second) pos2 = node.position;
        }

        ImU32 edgeColor = IM_COL32(200, 200, 200, 255); 

        for (size_t i = 1; i < shortestPath.size(); ++i) {
            if ((shortestPath[i - 1] == edge.first && shortestPath[i] == edge.second) ||
                (shortestPath[i - 1] == edge.second && shortestPath[i] == edge.first)) {
                edgeColor = IM_COL32(255, 0, 0, 255); 
                break;
            }
        }

        for (size_t i = 1; i < dfsPathResult.size(); ++i) {
            if ((dfsPathResult[i - 1] == edge.first && dfsPathResult[i] == edge.second) ||
                (dfsPathResult[i - 1] == edge.second && dfsPathResult[i] == edge.first)) {
                edgeColor = IM_COL32(0, 255, 0, 255);
                break;
            }
        }

        drawList->AddLine(pos1, pos2, edgeColor, 2.0f);

        ImVec2 mid((pos1.x + pos2.x) / 2, (pos1.y + pos2.y) / 2);
        float weight = graph.getWeight(edge.first, edge.second);
        drawList->AddText(mid, IM_COL32(255, 255, 255, 255), std::to_string(weight).c_str());
    }

    for (const auto& node : nodes) {
        bool isStart = (node.id == selectedPathStart || node.id == dfsStart);
        bool isEnd = (node.id == selectedPathEnd || node.id == dfsEnd);
        ImU32 color = isStart ? IM_COL32(0, 255, 0, 255) :
            isEnd ? IM_COL32(0, 0, 255, 255) :
            IM_COL32(255, 255, 0, 255);
        drawList->AddCircleFilled(node.position, 15.0f, color);
        drawList->AddText(ImVec2(node.position.x - 4, node.position.y - 7), IM_COL32(0, 0, 0, 255), std::to_string(node.id).c_str());
    }
}

void renderUI() {
    ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 450, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(450, 0), ImGuiCond_Always);
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);

    ImGui::Text("Instructions:");
    ImGui::BulletText("Left-click: Add a node");
    ImGui::BulletText("Right-click 2 nodes: Add edge");
    ImGui::BulletText("Shift + Right-click on a node: Delete node");
    ImGui::BulletText("Middle-click 2 nodes: Dijkstra path");
    ImGui::BulletText("Ctrl + Right-click 2 nodes: DFS check");

    ImGui::SliderFloat("Edge Weight", &edgeWeightInput, 0.1f, 20.0f, "%.1f");

    if (!shortestPath.empty()) {
        ImGui::Text("Shortest Path:");
        for (int node : shortestPath) {
            ImGui::SameLine();
            ImGui::Text("%d", node);
        }
    }

    if (!dfsPathResult.empty()) {
        ImGui::Separator();
        ImGui::Text("DFS Path:");
        for (int node : dfsPathResult) {
            ImGui::SameLine();
            ImGui::Text("%d", node);
        }
    }

    if (ImGui::Button("Reset")) {
        selectedEdgeStart = -1;
        selectedPathStart = selectedPathEnd = -1;
        dfsStart = dfsEnd = -1;
        shortestPath.clear();
        dfsPathResult.clear();
    }

    ImGui::End();

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();
    renderGraph(drawList);
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(800, 600, "Interactive Graph - Dijkstra & DFS", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    setupImGui(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            VisualNode node;
            node.id = nextNodeId++;
            node.position = io.MousePos;
            nodes.push_back(node);
            graph.addNode(node.id);
        }
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            int clicked = getNodeAtPosition(io.MousePos);
            if (clicked != -1) {
                if (io.KeyShift) {
                    nodes.erase(std::remove_if(nodes.begin(), nodes.end(),
                        [clicked](const VisualNode& n) { return n.id == clicked; }), nodes.end());
                    edges.erase(std::remove_if(edges.begin(), edges.end(),
                        [clicked](const auto& e) { return e.first == clicked || e.second == clicked; }), edges.end());
                    graph.removeNode(clicked);
                }
                else if (io.KeyCtrl) { 
                    if (dfsStart == -1) {
                        dfsStart = clicked;
                    }
                    else {
                        dfsEnd = clicked;
                        dfsPathResult = graph.dfsPath(dfsStart, dfsEnd);
                        dfsStart = dfsEnd = -1;
                    }
                }
                else {
                    if (selectedEdgeStart == -1) {
                        selectedEdgeStart = clicked;
                    }
                    else {
                        graph.addEdge(selectedEdgeStart, clicked, edgeWeightInput);
                        edges.push_back({ selectedEdgeStart, clicked });
                        selectedEdgeStart = -1;
                    }
                }
            }
        }
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
            int clicked = getNodeAtPosition(io.MousePos);
            if (clicked != -1) {
                if (selectedPathStart == -1) {
                    selectedPathStart = clicked;
                }
                else {
                    selectedPathEnd = clicked;
                    shortestPath = graph.dijkstra(selectedPathStart, selectedPathEnd);
                    selectedPathStart = selectedPathEnd = -1;
                }
            }
        }

        renderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    cleanupImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
