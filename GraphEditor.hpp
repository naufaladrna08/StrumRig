#ifndef GRAPHEDITOR_HPP
#define GRAPHEDITOR_HPP

#include <GraphEditor.h>
#include "Array.hpp"
#include <printf.h>
#define LOG(...) printf(__VA_ARGS__)

class GraphEditorImpl : public GraphEditor::Delegate {
  public:
    GraphEditorImpl();
    ~GraphEditorImpl();

    void addNode(const char *name, GraphEditor::TemplateIndex templateIndex, float x, float y);
    void addLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex);
    void delLink(GraphEditor::LinkIndex linkIndex);
    void selectNode(GraphEditor::NodeIndex nodeIndex, bool selected);

    /* Override */
    bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override;
    void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override;
    void MoveSelectedNodes(const ImVec2 delta) override;
    void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override;
    void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override;
    void DelLink(GraphEditor::LinkIndex linkIndex) override;
    void CustomDraw(ImDrawList *drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override;
    const size_t GetTemplateCount() override;
    const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override;
    const size_t GetNodeCount() override;
    const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override;
    const size_t GetLinkCount() override;
    const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override;

    struct Node {
      const char *name;
      GraphEditor::TemplateIndex templateIndex;
      float x, y;
      bool mSelected;
    };

    static bool isContextMenuOpen;
  private:
    static const inline GraphEditor::Template mTemplates[] = {
      // Only takes output
      {
        IM_COL32(64, 64, 64, 255),
        IM_COL32(24, 24, 24, 239),
        IM_COL32(32, 32, 32, 255),
        0,
        Array{""},
        nullptr,
        1,
        Array{"Guitar"},
        nullptr
      },

      // Only takes input
      {
        IM_COL32(64, 64, 64, 255),
        IM_COL32(24, 24, 24, 239),
        IM_COL32(32, 32, 32, 255),
        1,
        nullptr,
        Array {
          IM_COL32(200, 100, 100, 255)
        }, 0,
        nullptr,
        nullptr
      },

      // One to one
      {
        IM_COL32(64, 64, 64, 255),
        IM_COL32(24, 24, 24, 239),
        IM_COL32(32, 32, 32, 255),
        1,
        Array{""},
        Array {
          IM_COL32(100, 200, 100, 255)
        }, 1,
        Array{""},
        nullptr
      }
    };

    std::vector<Node> m_nodes;
    std::vector<GraphEditor::Link> m_links;
};

#endif