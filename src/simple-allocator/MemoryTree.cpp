// Simple Allocator 2024
#include "MemoryTree.h"

#include "MemoryBlock.h"

#include <cassert>
#include <new>
#include <utility>

class MemoryTree::TreeNode {
public:
  TreeNode *left{nullptr};
  TreeNode *right{nullptr};
  TreeNode *parent{nullptr};
  enum { RED, BLACK } color{RED};
  size_t block_size{0};
  TreeNode *same_size_nodes{nullptr};

  explicit TreeNode(size_t size) noexcept
    : block_size(size) {}

  TreeNode *Uncle() const noexcept {
    if (!parent || !parent->parent) {
      return nullptr;
    }
    auto *grandpa = parent->parent;
    return parent->IsLeft() ? grandpa->right : grandpa->left;
  }

  bool IsLeft() const noexcept {
    return this == parent->left;
  }

  void ReplaceSelfOnParent(TreeNode *replacer) noexcept {
    if (IsLeft()) {
      parent->left = replacer;
    } else {
      parent->right = replacer;
    }
  }

  TreeNode *Sibling() const noexcept {
    if (!parent) {
      return nullptr;
    }
    return IsLeft() ? parent->right : parent->left;
  }

  void MoveDown(TreeNode *new_parent) noexcept {
    if (parent) {
      ReplaceSelfOnParent(new_parent);
    }
    new_parent->parent = parent;
    parent = new_parent;
  }

  bool HasRedChild() const noexcept {
    return (left && left->color == RED) || (right && right->color == RED);
  }
};

void MemoryTree::InsertBlock(MemoryBlock *memory_block) noexcept {
  const size_t size = memory_block->GetBlockSize();
  assert(sizeof(TreeNode) <= size);
  TreeNode *new_node = new (memory_block->UserMemoryBegin()) TreeNode{size};
  if (!root_) {
    new_node->color = TreeNode::BLACK;
    root_ = new_node;
    return;
  }
  TreeNode *parent = LookupNode(size, false);
  if (parent->block_size == size) {
    new_node->same_size_nodes = parent->same_size_nodes;
    parent->same_size_nodes = new_node;
    return;
  }

  new_node->parent = parent;
  if (size < parent->block_size) {
    parent->left = new_node;
  } else {
    parent->right = new_node;
  }

  FixRedRed(new_node);
}

MemoryBlock *MemoryTree::RetrieveBlock(size_t size) noexcept {
  if (TreeNode *v = LookupNode(size, true)) {
    if (v->same_size_nodes) {
      TreeNode *same_size_node = v->same_size_nodes;
      v->same_size_nodes = same_size_node->same_size_nodes;
      return MemoryBlock::FromUserMemory(same_size_node);
    }
    DetachNode(v);
    return MemoryBlock::FromUserMemory(v);
  }
  return nullptr;
}

MemoryTree::TreeNode *MemoryTree::LookupNode(size_t size, bool lower_bound) const noexcept {
  TreeNode *node = root_;
  TreeNode *lower_bound_node = nullptr;
  while (node && size != node->block_size) {
    if (size < node->block_size) {
      lower_bound_node = node;
      if (!node->left) {
        break;
      }
      node = node->left;
    } else {
      if (!node->right) {
        break;
      }
      node = node->right;
    }
  }
  if (node && node->block_size == size) {
    lower_bound_node = node;
  }
  return lower_bound ? lower_bound_node : node;
}

MemoryTree::TreeNode *MemoryTree::FindReplacer(TreeNode *node) noexcept {
  if (node->left && node->right) {
    TreeNode *replacer = node->right;
    while (replacer->left) {
      replacer = replacer->left;
    }
    return replacer;
  }
  return node->left ? node->left : node->right;
}

void MemoryTree::DetachLeaf(TreeNode *detaching_node) noexcept {
  if (detaching_node == root_) {
    root_ = nullptr;
    return;
  }

  if (detaching_node->color == TreeNode::BLACK) {
    FixDoubleBlack(detaching_node);
  } else if (TreeNode *sibling = detaching_node->Sibling()) {
    sibling->color = TreeNode::RED;
  }

  detaching_node->ReplaceSelfOnParent(nullptr);
}

void MemoryTree::DetachNodeWithOneChild(TreeNode *detaching_node, TreeNode *replacer) noexcept {
  if (detaching_node == root_) {
    assert(!replacer->left && !replacer->right);
    assert(replacer->parent == detaching_node);
    replacer->parent = nullptr;
    replacer->color = detaching_node->color;
    root_ = replacer;
    return;
  }

  detaching_node->ReplaceSelfOnParent(replacer);
  replacer->parent = detaching_node->parent;
  if (replacer->color == TreeNode::BLACK && detaching_node->color == TreeNode::BLACK) {
    FixDoubleBlack(replacer);
  } else {
    replacer->color = TreeNode::BLACK;
  }
}

void MemoryTree::SwapDetachingNodeWithReplacer(TreeNode *detaching_node, TreeNode *replacer) noexcept {
  if (detaching_node->parent) {
    detaching_node->ReplaceSelfOnParent(replacer);
  } else {
    root_ = replacer;
  }

  if (detaching_node->left && detaching_node->left != replacer) {
    detaching_node->left->parent = replacer;
  }
  if (detaching_node->right && detaching_node->right != replacer) {
    detaching_node->right->parent = replacer;
  }

  if (replacer->left) {
    replacer->left->parent = detaching_node;
  }
  if (replacer->right) {
    replacer->right->parent = detaching_node;
  }

  if (replacer->parent == detaching_node) {
    replacer->parent = detaching_node->parent;
    detaching_node->parent = replacer;
  } else {
    assert(replacer->parent);
    replacer->ReplaceSelfOnParent(detaching_node);
    std::swap(replacer->parent, detaching_node->parent);
  }
  std::swap(replacer->left, detaching_node->left);
  std::swap(replacer->right, detaching_node->right);
  std::swap(replacer->color, detaching_node->color);
}

void MemoryTree::DetachNode(TreeNode *detaching_node) noexcept {
  TreeNode *replacer = FindReplacer(detaching_node);
  if (!replacer) {
    DetachLeaf(detaching_node);
    return;
  }

  if (!detaching_node->left || !detaching_node->right) {
    DetachNodeWithOneChild(detaching_node, replacer);
    return;
  }

  SwapDetachingNodeWithReplacer(detaching_node, replacer);
  DetachNode(detaching_node);
}

void MemoryTree::FixRedRed(TreeNode *node) noexcept {
  if (node == root_) {
    node->color = TreeNode::BLACK;
    return;
  }

  TreeNode *parent = node->parent;
  TreeNode *grandparent = parent->parent;
  TreeNode *uncle = node->Uncle();

  if (parent->color != TreeNode::BLACK) {
    if (uncle && uncle->color == TreeNode::RED) {
      parent->color = TreeNode::BLACK;
      uncle->color = TreeNode::BLACK;
      grandparent->color = TreeNode::RED;
      FixRedRed(grandparent);
    } else {
      if (parent->IsLeft()) {
        if (node->IsLeft()) {
          std::swap(parent->color, grandparent->color);
        } else {
          LeftRotate(parent);
          std::swap(node->color, grandparent->color);
        }
        RightRotate(grandparent);
      } else {
        if (node->IsLeft()) {
          RightRotate(parent);
          std::swap(node->color, grandparent->color);
        } else {
          std::swap(parent->color, grandparent->color);
        }
        LeftRotate(grandparent);
      }
    }
  }
}

void MemoryTree::FixDoubleBlack(TreeNode *node) noexcept {
  if (node == root_) {
    return;
  }

  TreeNode *parent = node->parent;
  if (TreeNode *sibling = node->Sibling()) {
    if (sibling->color == TreeNode::RED) {
      parent->color = TreeNode::RED;
      sibling->color = TreeNode::BLACK;
      if (sibling->IsLeft()) {
        RightRotate(parent);
      } else {
        LeftRotate(parent);
      }
      FixDoubleBlack(node);
      return;
    }

    if (sibling->HasRedChild()) {
      if (sibling->left && sibling->left->color == TreeNode::RED) {
        if (sibling->IsLeft()) {
          sibling->left->color = sibling->color;
          sibling->color = parent->color;
          RightRotate(parent);
        } else {
          sibling->left->color = parent->color;
          RightRotate(sibling);
          LeftRotate(parent);
        }
      } else {
        if (sibling->IsLeft()) {
          sibling->right->color = parent->color;
          LeftRotate(sibling);
          RightRotate(parent);
        } else {
          sibling->right->color = sibling->color;
          sibling->color = parent->color;
          LeftRotate(parent);
        }
      }
      parent->color = TreeNode::BLACK;
      return;
    }

    sibling->color = TreeNode::RED;
    if (parent->color == TreeNode::BLACK) {
      FixDoubleBlack(parent);
    } else {
      parent->color = TreeNode::BLACK;
    }
    return;
  }

  FixDoubleBlack(parent);
}

void MemoryTree::RightRotate(TreeNode *node) noexcept {
  TreeNode *new_parent = node->left;
  if (node == root_) {
    root_ = new_parent;
  }

  node->MoveDown(new_parent);
  node->left = new_parent->right;
  if (new_parent->right) {
    new_parent->right->parent = node;
  }

  new_parent->right = node;
}

void MemoryTree::LeftRotate(TreeNode *node) noexcept {
  TreeNode *new_parent = node->right;
  if (node == root_) {
    root_ = new_parent;
  }

  node->MoveDown(new_parent);
  node->right = new_parent->left;
  if (new_parent->left) {
    new_parent->left->parent = node;
  }

  new_parent->left = node;
}
