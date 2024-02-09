// Simple Allocator 2024
#ifndef MEMORYTREE_H
#define MEMORYTREE_H
#include <cstddef>

struct MemoryBlock;

class MemoryTree {
public:
  void InsertBlock(MemoryBlock *memory_block) noexcept;
  MemoryBlock *RetrieveBlock(size_t size) noexcept;

private:
  class TreeNode;

  TreeNode *LookupNode(size_t size, bool lower_bound) const noexcept;

  TreeNode *FindReplacer(TreeNode *node) noexcept;
  void DetachLeaf(TreeNode *detaching_node) noexcept;
  void DetachNodeWithOneChild(TreeNode *detaching_node, TreeNode *replacer) noexcept;
  void SwapDetachingNodeWithReplacer(TreeNode *detaching_node, TreeNode *replacer) noexcept;
  void DetachNode(TreeNode *detaching_node) noexcept;
  void FixRedRed(TreeNode *node) noexcept;
  void FixDoubleBlack(TreeNode *node) noexcept;
  void RightRotate(TreeNode *node) noexcept;
  void LeftRotate(TreeNode *node) noexcept;

  TreeNode *root_{nullptr};
};

#endif // MEMORYTREE_H
