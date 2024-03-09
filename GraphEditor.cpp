#include "GraphEditor.hpp"

GraphEditorImpl::GraphEditorImpl() {

}

GraphEditorImpl::~GraphEditorImpl() {
  
}

void GraphEditorImpl::addNode(const char *name, GraphEditor::TemplateIndex templateIndex, float x, float y) {
  // copy name 

  m_nodes.push_back({ name, templateIndex, x, y, false });
}

void GraphEditorImpl::addLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) {
  m_links.push_back({ inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex });
}

void GraphEditorImpl::delLink(GraphEditor::LinkIndex linkIndex) {
  m_links.erase(m_links.begin() + linkIndex);
}

void GraphEditorImpl::selectNode(GraphEditor::NodeIndex nodeIndex, bool selected) {
  m_nodes[nodeIndex].mSelected = selected;
}

bool GraphEditorImpl::AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) {
  return true;
}

void GraphEditorImpl::SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) {
  selectNode(nodeIndex, selected);
}

void GraphEditorImpl::MoveSelectedNodes(const ImVec2 delta) {
  for (auto &node : m_nodes) {
    if (node.mSelected) {
      node.x += delta.x;
      node.y += delta.y;
    }
  }
}

void GraphEditorImpl::RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) {
  this->isContextMenuOpen = !this->isContextMenuOpen;
}

void GraphEditorImpl::AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) {
  addLink(inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex);
}

void GraphEditorImpl::DelLink(GraphEditor::LinkIndex linkIndex) {
  delLink(linkIndex);
}

void GraphEditorImpl::CustomDraw(ImDrawList *drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) {

}

const size_t GraphEditorImpl::GetTemplateCount() {
  return sizeof(mTemplates) / sizeof(GraphEditor::Template);
}

const GraphEditor::Template GraphEditorImpl::GetTemplate(GraphEditor::TemplateIndex index) {
  return mTemplates[index];
}

const size_t GraphEditorImpl::GetNodeCount() {
  return m_nodes.size();
}

const GraphEditor::Node GraphEditorImpl::GetNode(GraphEditor::NodeIndex index) {
  const auto& myNode = this->m_nodes[index];
  return GraphEditor::Node {
    myNode.name,
    myNode.templateIndex,
    ImRect(ImVec2(myNode.x, myNode.y), ImVec2(myNode.x + 200, myNode.y + 200)),
    myNode.mSelected
  }; 
}

const size_t GraphEditorImpl::GetLinkCount() {
  return m_links.size();
}

const GraphEditor::Link GraphEditorImpl::GetLink(GraphEditor::LinkIndex index) {
  return m_links[index];
}