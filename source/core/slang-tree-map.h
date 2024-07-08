#ifndef SLANG_CORE_TREE_MAP_H
#define SLANG_CORE_TREE_MAP_H

#include "slang-list.h"
#include "slang-common.h"
#include <initializer_list>

namespace Slang
{

    // Array backed red-black tree map
    template<typename T>
    class TreeMap
    {
        static constexpr Index kInvalidIndex = -1;

        enum class NodeColor : uint8_t
        {
            Black = 1,
            Red = 2,
        };

        class TreeMapNode
        {
            // Context
            TreeMap<T>* m_treeMap = nullptr;
            // Index of the data this node links to inside m_treeMap->m_data
            Index m_dataIndex = kInvalidIndex;
            // Index of nodes this node has a relationship. Indices relate to m_treeMap->m_nodes
            Index m_parentNodeIndex = kInvalidIndex;
            Index m_leftNodeIndex = kInvalidIndex;
            Index m_rightNodeIndex = kInvalidIndex;
            // Red-black tree node color
            NodeColor m_nodeColor = NodeColor::Black;

        public:
            TreeMapNode()
            {
            }

            TreeMapNode(TreeMap<T>* treeMap, Index dataIndex, Index parentNodeIndex,
                Index leftNodeIndex, Index rightNodeIndex, NodeColor nodeColor)
                : m_treeMap(treeMap), m_dataIndex(dataIndex), m_parentNodeIndex(parentNodeIndex),
                m_leftNodeIndex(leftNodeIndex), m_rightNodeIndex(rightNodeIndex), m_nodeColor(nodeColor)
            {
            }

            static inline TreeMapNode makeInvalidNode()
            {
                return {};
            }

            TreeMapNode& getNextLargestParentNode()
            {
                // We have do not have a 'parent', we are the largest node.
                auto& parent = getParentNodeRef();
                if (!parent)
                    return parent;
                // If we are a 'leftNode' of 'parent', we are smaller.
                else if (parent.getLeftNodeRef() == *this)
                    return parent;
                // If we are a 'rightNode' of parent, 'parent.parent' is next largest
                // as long as 'parent.parent.left' is equal to 'parent' 
                else
                    return parent.getNextLargestParentNode();
            }

            NodeColor getNodeColor()
            {
                return m_nodeColor;
            }

            void setNodeColor(NodeColor color)
            {
                m_nodeColor = color;
            }

            TreeMapNode& getSiblingNodeRef(bool& isThisNodeALeftChild)
            {
                auto parent = getParentNodeRef();
                if (*this == parent.getLeftNodeRef())
                {
                    isThisNodeALeftChild = true;
                    return parent.getRightNodeRef();
                }
                isThisNodeALeftChild = false;
                return parent.getLeftNodeRef();
            }

            Index getParentNodeIndex()
            {
                return m_parentNodeIndex;
            }

            TreeMapNode& getParentNodeRef()
            {
                return m_treeMap->getNodeRef(m_parentNodeIndex);
            }

            TreeMapNode getParentNode()
            {
                return m_treeMap->getNodeRef(m_parentNodeIndex);
            }

            void setParentNode(Index nodeIndex)
            {
                m_parentNodeIndex = nodeIndex;
            }

            Index getLeftNodeIndex()
            {
                return m_leftNodeIndex;
            }

            TreeMapNode getLeftNode()
            {
                return m_treeMap->getNodeRef(m_leftNodeIndex);
            }

            TreeMapNode& getLeftNodeRef()
            {
                return m_treeMap->getNodeRef(m_leftNodeIndex);
            }

            void setLeftNode(Index nodeIndex)
            {
                m_leftNodeIndex = nodeIndex;
            }

            Index getRightNodeIndex()
            {
                return m_rightNodeIndex;
            }

            TreeMapNode getRightNode()
            {
                return m_treeMap->getNodeRef(m_rightNodeIndex);
            }

            TreeMapNode& getRightNodeRef()
            {
                return m_treeMap->getNodeRef(m_rightNodeIndex);
            }

            void setRightNode(Index nodeIndex)
            {
                m_rightNodeIndex = nodeIndex;
            }

            Index getDataIndex()
            {
                return m_dataIndex;
            }

            TreeMapNode& operator++()
            {
                // Next largest node if available
                if (auto rightNode = getRightNodeRef())
                {
                    *this = rightNode;
                    return *this;
                }

                // If we don't have a rightNode, parent may be the next largest.
                *this = getNextLargestParentNode();
                return *this;
            }

            bool operator==(const TreeMapNode& other) const
            {
                return other.m_treeMap == this->m_treeMap
                    && other.m_dataIndex == this->m_dataIndex;
            }

            bool operator!=(const TreeMapNode& other) const
            {
                return !(*this == other);
            }

            explicit operator bool() const
            {
                return *this != TreeMap<T>::getInvalidNode();
            }

            T& operator*()
            {
                return m_treeMap->getData(m_dataIndex);
            }
        };

        // Node storage
        List<TreeMapNode> m_nodes;

        // Storage of data TreeMapNode point to
        List<T> m_data;
        
        // These indices act as 'pointers' into m_nodes.
        Index m_rootNode = kInvalidIndex;
        Index m_smallestNode = kInvalidIndex;
        Index m_largestNode = kInvalidIndex;

        // Cached invalid node
        static inline TreeMapNode invalidNode = TreeMapNode::makeInvalidNode();
        static inline TreeMapNode& getInvalidNode()
        {
            return invalidNode;
        }

        T& getData(Index dataIndex)
        {
            SLANG_ASSERT(dataIndex > -1 && dataIndex < m_data.getCount());
            return m_data[dataIndex];
        }

        TreeMapNode& getNodeRef(Index nodeIndex)
        {
            if (nodeIndex == kInvalidIndex)
            {
                SLANG_ASSERT(TreeMap<T>::getInvalidNode() == TreeMapNode::makeInvalidNode());
                return TreeMap<T>::getInvalidNode();
            }
            SLANG_ASSERT(nodeIndex > -1 && nodeIndex < m_nodes.getCount());
            return m_nodes[nodeIndex];
        }

        Index makeNewNode(T&& obj, Index parentNodeIndex, NodeColor nodeColor)
        {
            Index newDataIndex = m_data.getCount();
            m_data.add(obj);
            TreeMapNode newNode = TreeMapNode( this, newDataIndex, parentNodeIndex, kInvalidIndex, kInvalidIndex, nodeColor);
            Index nodeIndex = m_nodes.getCount();
            m_nodes.add(newNode);
            return nodeIndex;
        }

        void leftRotation(Index currentNodeIndex)
        {
            if (currentNodeIndex == kInvalidIndex)
                return;
            
            auto currentNode = getNodeRef(currentNodeIndex);
            auto rightNodeIndex = currentNode.getRightNodeIndex();
            auto rightNode = currentNode.getRightNodeRef();
            currentNode.setRightNode(rightNode.getLeftNodeIndex());
            if (auto leftNode = rightNode.getLeftNode())
                leftNode.getLeftNodeRef().setParentNode(currentNodeIndex);
            auto parentNodeIndex = currentNode.getParentNodeIndex();
            auto parentNode = currentNode.getParentNode();
            rightNode.setParentNode(parentNodeIndex);
            if (parentNodeIndex == kInvalidIndex)
                m_rootNode = parentNodeIndex;
            else if (currentNodeIndex == parentNode.getLeftNodeIndex())
                parentNode.setLeftNode(rightNodeIndex);
            else
                parentNode.setRightNode(rightNodeIndex);
            rightNode.setLeftNode(currentNodeIndex);
            currentNode.setParentNode(rightNodeIndex);
        }

        void rightRotation(Index currentNodeIndex)
        {
            if (currentNodeIndex == kInvalidIndex)
                return;

            auto currentNode = getNodeRef(currentNodeIndex);
            auto leftNodeIndex = currentNode.getLeftNodeIndex();
            auto leftNode = currentNode.getLeftNodeRef();
            currentNode.setLeftNode(leftNode.getRightNodeIndex());
            if (auto rightNode = leftNode.getRightNode())
                rightNode.getRightNodeRef().setParentNode(currentNodeIndex);
            auto parentNodeIndex = currentNode.getParentNodeIndex();
            auto parentNode = currentNode.getParentNode();
            leftNode.setParentNode(parentNodeIndex);
            if (parentNodeIndex == kInvalidIndex)
                m_rootNode = parentNodeIndex;
            else if (currentNodeIndex == parentNode.getRightNodeIndex())
                parentNode.setRightNode(leftNodeIndex);
            else
                parentNode.setLeftNode(leftNodeIndex);
            leftNode.setRightNode(currentNodeIndex);
            currentNode.setParentNode(leftNodeIndex);
        }

        // Regular red-black tree rebalancing
        // Note: Rebalancing can be disabled if this function is removed
        void validateAndFixInsert(Index currentNodeIndex)
        {
            auto currentNode = getNodeRef(currentNodeIndex);
            auto parentNodeIndex = currentNode.getParentNodeIndex();
            auto parentNode = currentNode.getParentNodeRef();
            auto grandParentNode = parentNode.getParentNodeRef();
            while (currentNodeIndex != m_rootNode && parentNode.getNodeColor() == NodeColor::Red)
            {
                bool isParentNodeALeftChild = false;
                auto uncleNode = parentNode.getSiblingNodeRef(isParentNodeALeftChild);
                // Case 1
                if (uncleNode.getNodeColor() == NodeColor::Red)
                {
                    parentNode.setNodeColor(NodeColor::Black);
                    uncleNode.setNodeColor(NodeColor::Black);
                    grandParentNode.setNodeColor(NodeColor::Red);
                    currentNode = grandParentNode;
                }
                // Case 2
                else
                {
                    bool isCurrentNodeALeftChild = currentNodeIndex == parentNode.getRightNodeIndex();
                    if (isCurrentNodeALeftChild == isParentNodeALeftChild)
                    {
                        currentNodeIndex = parentNodeIndex;
                        currentNode = getNodeRef(currentNodeIndex);

                        parentNodeIndex = currentNode.getParentNodeIndex();
                        parentNode = currentNode.getParentNodeRef();

                        grandParentNode = parentNode.getParentNodeRef();

                        if (isParentNodeALeftChild)
                            leftRotation(currentNodeIndex);
                        else
                            rightRotation(currentNodeIndex);
                    }
                    parentNode.setNodeColor(NodeColor::Black);
                    grandParentNode.setNodeColor(NodeColor::Red);
                    if (isParentNodeALeftChild)
                        rightRotation(currentNodeIndex);
                    else
                        leftRotation(currentNodeIndex);
                }
            }
            getNodeRef(m_rootNode).setNodeColor(NodeColor::Black);
        }

        // To figure out m_largestNode and m_smallestNode we have 1 case to take care of:
        // 1. if smallest we must only move 'left' down the node tree and eventually make a new node at the bottom left
        // 2. if largest we must only move 'right' down the node tree and eventually make a new node at the bottom right
        // We can track track movements required to add a new node to figure out if we are adding a new largest/smallest node.
        enum class AddNewNodeInfo : int
        {
            Head       = 0b00,
            MovedLeft  = 0b01,
            MovedRight = 0b10,
        };

        // Adding a new node requires to work off of 'currentNodeIndex' and not the actual
        // currentNode pointer since when we add new nodes to our m_nodes list we may invalidate
        // all existing pointers by resizing the backing-list.
        void _add(T&& obj, Index currentNodeIndex, int addNewNodeInfo)
        {
            TreeMapNode& currentNode = getNodeRef(currentNodeIndex);
            // Don't duplicate an already existing node
            if (obj == *currentNode)
                return;
            else if (obj > *currentNode)
            {
                addNewNodeInfo |= (int)AddNewNodeInfo::MovedRight;
                if (currentNode.getRightNodeRef())
                {
                    _add(std::move(obj), currentNode.getRightNodeIndex(), addNewNodeInfo);
                }
                else
                {
                    Index nodeIndex = makeNewNode(std::move(obj), currentNodeIndex, NodeColor::Red);
                    // Re-fetch node in-case we resize the backing list
                    getNodeRef(currentNodeIndex).setRightNode(nodeIndex);
                    // Only moved 'right', we added a new largest node
                    if (addNewNodeInfo == (int)AddNewNodeInfo::MovedRight)
                        m_largestNode = nodeIndex;
                    
                    // Tree re-balancing
                    validateAndFixInsert(nodeIndex);
                }
            }
            else if (obj < *currentNode)
            {
                addNewNodeInfo |= (int)AddNewNodeInfo::MovedLeft;
                if (currentNode.getLeftNodeRef())
                {
                    _add(std::move(obj), currentNode.getLeftNodeIndex(), addNewNodeInfo);
                }
                else
                {
                    Index nodeIndex = makeNewNode(std::move(obj), currentNodeIndex, NodeColor::Red);
                    // Re-fetch node in-case we resize the backing list
                    getNodeRef(currentNodeIndex).setLeftNode(nodeIndex);
                    // Only moved 'Left', we added a new largest node
                    if (addNewNodeInfo == (int)AddNewNodeInfo::MovedLeft)
                        m_smallestNode = nodeIndex;

                    // Tree re-balancing
                    validateAndFixInsert(nodeIndex);
                }
            }
        }

        // BST element search implementation
        bool _contains(const T& obj, TreeMapNode& currentNode)
        {
            if (!currentNode)
                return false;
            else if (*currentNode == obj)
                return true;
            else if (*currentNode < obj)
                return _contains(obj, currentNode.getRightNodeRef());
            else // (*currentNode > obj)
                return _contains(obj, currentNode.getLeftNodeRef());
        }

    public:
        void add(T&& obj)
        {
            if (m_rootNode == kInvalidIndex)
            {
                Index nodeIndex = makeNewNode(std::move(obj), kInvalidIndex, NodeColor::Black);
                m_rootNode = nodeIndex;
                m_smallestNode = nodeIndex;
                m_largestNode = nodeIndex;
                return;
            }

            _add(std::move(obj), m_rootNode, (int)AddNewNodeInfo::Head);
        }

        void add(const T& obj)
        {
            add(std::move(obj));
        }

        // BST element search
        bool contains(const T& obj)
        {
            return _contains(obj, getNodeRef(m_rootNode));
        }
        
        // Iterator is not stable if elements are added
        TreeMapNode begin()
        {
            return getNodeRef(m_smallestNode);
        }

        TreeMapNode end()
        {
            return getInvalidNode();
        }
    };

}

#endif
