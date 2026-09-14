/*
MIT License

Copyright (c) 2019 Pierre Vigier

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Derived from Pierre Vigier's Quadtree (https://github.com/pvigier/Quadtree).
// Adopted as engine code rather than kept as a vendored dependency, because it
// no longer matches upstream:
//
//   2024-10-18 (FJBC)  use Vector2D<T> instead of the original Vector2
//   2026-09-11         moved into namespace de, dropped the <iostream>
//                      dependency, and made add()/remove() report failure
//                      instead of relying on asserts that vanish under NDEBUG
//   2026-09-11         added removeAt(), which locates a value by a supplied
//                      box rather than its current bounds, so values that
//                      have moved can be re-filed

#ifndef DE_SPATIAL_QUADTREE_HPP
#define DE_SPATIAL_QUADTREE_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <engine/core/vector_2d.hpp>
#include <memory>
#include <type_traits>
#include <vector>

namespace de
{
/// Axis-aligned bounding box: top-left corner (`x`, `y`) plus extents.
///
/// An aggregate, like Vector2D. Construct it as
/// `Box<float>(left, top, width, height)`.
template <typename T>
struct Box
{
    T x{};
    T y{};
    T width{};
    T height{};

    constexpr T left() const noexcept { return x; }
    constexpr T top() const noexcept { return y; }
    constexpr T right() const noexcept { return x + width; }
    constexpr T bottom() const noexcept { return y + height; }

    constexpr bool contains(const Box<T>& box) const noexcept
    {
        return x <= box.x && box.right() <= right() && y <= box.y &&
               box.bottom() <= bottom();
    }

    constexpr bool operator==(const Box<T>& box) const noexcept
    {
        return x == box.x && y == box.y && width == box.width &&
               height == box.height;
    }

    constexpr bool intersects(const Box<T>& box) const noexcept
    {
        return !(x >= box.right() || right() <= box.x || y >= box.bottom() ||
                 bottom() <= box.y);
    }
};

/// Loose quadtree over values whose bounds are produced by `GetBox`.
///
/// add() and remove() return false instead of asserting when a value lies
/// outside the tree bounds or is not present: the callers are game systems
/// running in release builds, where an assert would be compiled out and leave
/// the tree silently corrupt.
template <typename T, typename GetBox, typename Equal = std::equal_to<T>,
          typename Float = float>
class Quadtree
{
    static_assert(
        std::is_convertible_v<std::invoke_result_t<GetBox, const T&>,
                              Box<Float>>,
        "GetBox must be a callable of signature Box<Float>(const T&)");
    static_assert(
        std::is_convertible_v<std::invoke_result_t<Equal, const T&, const T&>,
                              bool>,
        "Equal must be a callable of signature bool(const T&, const T&)");
    static_assert(std::is_arithmetic_v<Float>);

public:
    Quadtree(const Box<Float>& box, const GetBox& getBox = GetBox(),
             const Equal& equal = Equal())
        : m_box(box), m_root(std::make_unique<Node>()), m_getBox(getBox),
          m_equal(equal)
    {
    }

    /// Inserts `value`. Returns false (without inserting) when its box is not
    /// fully contained in the tree bounds.
    bool add(const T& value)
    {
        if (!m_box.contains(m_getBox(value)))
            return false;
        add(m_root.get(), 0, m_box, value);
        return true;
    }

    /// Removes `value`, located by its current bounds.
    bool remove(const T& value) { return removeAt(value, m_getBox(value)); }

    /// Removes `value`, navigating with `valueBox` rather than the bounds
    /// GetBox reports now.
    ///
    /// This is what makes moving values possible. The tree filed the value
    /// under the box it had when it was added; once it moves, its current box
    /// leads down a different branch and the value is never found. Callers
    /// that move things must remove with the old box and add with the new one.
    bool removeAt(const T& value, const Box<Float>& valueBox)
    {
        if (!m_box.contains(valueBox))
            return false;
        bool removed = false;
        remove(m_root.get(), m_box, valueBox, value, removed);
        return removed;
    }

    std::vector<T> query(const Box<Float>& box) const
    {
        auto values = std::vector<T>();
        if (!box.intersects(m_box))
            return values;
        query(m_root.get(), m_box, box, values);
        return values;
    }

    std::vector<std::pair<T, T>> findAllIntersections() const
    {
        auto intersections = std::vector<std::pair<T, T>>();
        findAllIntersections(m_root.get(), intersections);
        return intersections;
    }

    Box<Float> getBox() const { return m_box; }

private:
    static constexpr auto Threshold = std::size_t(16);
    static constexpr auto MaxDepth = std::size_t(8);

    struct Node
    {
        std::array<std::unique_ptr<Node>, 4> children;
        std::vector<T> values;
    };

    Box<Float> m_box;
    std::unique_ptr<Node> m_root;
    GetBox m_getBox;
    Equal m_equal;

    bool isLeaf(const Node* node) const
    {
        return !static_cast<bool>(node->children[0]);
    }

    Box<Float> computeBox(const Box<Float>& box, int i) const
    {
        const Float halfW = box.width / static_cast<Float>(2);
        const Float halfH = box.height / static_cast<Float>(2);
        switch (i)
        {
            // North West
            case 0:
                return Box<Float>(box.x, box.y, halfW, halfH);
            // North East
            case 1:
                return Box<Float>(box.x + halfW, box.y, halfW, halfH);
            // South West
            case 2:
                return Box<Float>(box.x, box.y + halfH, halfW, halfH);
            // South East
            case 3:
                return Box<Float>(box.x + halfW, box.y + halfH, halfW, halfH);
            default:
                assert(false && "Invalid child index");
                return Box<Float>();
        }
    }

    int getQuadrant(const Box<Float>& nodeBox, const Box<Float>& valueBox) const
    {
        const Vector2D<Float> center(nodeBox.x + nodeBox.width / 2,
                                     nodeBox.y + nodeBox.height / 2);
        // West
        if (valueBox.right() < center.x)
        {
            // North West
            if (valueBox.bottom() < center.y)
                return 0;
            // South West
            else if (valueBox.top() >= center.y)
                return 2;
            // Not contained in any quadrant
            else
                return -1;
        }
        // East
        else if (valueBox.left() >= center.x)
        {
            // North East
            if (valueBox.bottom() < center.y)
                return 1;
            // South East
            else if (valueBox.top() >= center.y)
                return 3;
            // Not contained in any quadrant
            else
                return -1;
        }
        // Not contained in any quadrant
        else
            return -1;
    }

    void add(Node* node, std::size_t depth, const Box<Float>& box,
             const T& value)
    {
        assert(node != nullptr);
        if (isLeaf(node))
        {
            // Insert the value in this node if possible
            if (depth >= MaxDepth || node->values.size() < Threshold)
                node->values.push_back(value);
            // Otherwise, we split and we try again
            else
            {
                split(node, box);
                add(node, depth, box, value);
            }
        }
        else
        {
            auto i = getQuadrant(box, m_getBox(value));
            // Add the value in a child if the value is entirely contained in it
            if (i != -1)
                add(node->children[static_cast<std::size_t>(i)].get(),
                    depth + 1, computeBox(box, i), value);
            // Otherwise, we add the value in the current node
            else
                node->values.push_back(value);
        }
    }

    void split(Node* node, const Box<Float>& box)
    {
        assert(node != nullptr);
        assert(isLeaf(node) && "Only leaves can be split");
        // Create children
        for (auto& child : node->children)
            child = std::make_unique<Node>();
        // Assign values to children
        auto newValues = std::vector<T>(); // New values for this node
        for (const auto& value : node->values)
        {
            auto i = getQuadrant(box, m_getBox(value));
            if (i != -1)
                node->children[static_cast<std::size_t>(i)]->values.push_back(
                    value);
            else
                newValues.push_back(value);
        }
        node->values = std::move(newValues);
    }

    /// Returns whether the caller should try to merge `node`; `removed` reports
    /// whether the value was actually found.
    bool remove(Node* node, const Box<Float>& box, const Box<Float>& valueBox,
                const T& value, bool& removed)
    {
        assert(node != nullptr);
        if (isLeaf(node))
        {
            removed = removeValue(node, value);
            return true;
        }
        auto i = getQuadrant(box, valueBox);
        if (i != -1)
        {
            if (remove(node->children[static_cast<std::size_t>(i)].get(),
                       computeBox(box, i), valueBox, value, removed))
                return tryMerge(node);
        }
        else
        {
            removed = removeValue(node, value);
        }
        return false;
    }

    bool removeValue(Node* node, const T& value)
    {
        auto it = std::find_if(std::begin(node->values), std::end(node->values),
                               [this, &value](const auto& rhs)
                               { return m_equal(value, rhs); });
        if (it == std::end(node->values))
            return false;
        // Swap with the last element and pop back
        *it = std::move(node->values.back());
        node->values.pop_back();
        return true;
    }

    bool tryMerge(Node* node)
    {
        assert(node != nullptr);
        assert(!isLeaf(node) && "Only interior nodes can be merged");
        auto nbValues = node->values.size();
        for (const auto& child : node->children)
        {
            if (!isLeaf(child.get()))
                return false;
            nbValues += child->values.size();
        }
        if (nbValues <= Threshold)
        {
            node->values.reserve(nbValues);
            // Merge the values of all the children
            for (const auto& child : node->children)
            {
                for (const auto& value : child->values)
                    node->values.push_back(value);
            }
            // Remove the children
            for (auto& child : node->children)
                child.reset();
            return true;
        }
        return false;
    }

    void query(Node* node, const Box<Float>& box, const Box<Float>& queryBox,
               std::vector<T>& values) const
    {
        assert(node != nullptr);
        assert(queryBox.intersects(box));
        for (const auto& value : node->values)
        {
            if (queryBox.intersects(m_getBox(value)))
                values.push_back(value);
        }
        if (!isLeaf(node))
        {
            for (auto i = std::size_t(0); i < node->children.size(); ++i)
            {
                auto childBox = computeBox(box, static_cast<int>(i));
                if (queryBox.intersects(childBox))
                    query(node->children[i].get(), childBox, queryBox, values);
            }
        }
    }

    void findAllIntersections(Node* node,
                              std::vector<std::pair<T, T>>& intersections) const
    {
        // Find intersections between values stored in this node
        // Make sure to not report the same intersection twice
        for (auto i = std::size_t(0); i < node->values.size(); ++i)
        {
            for (auto j = std::size_t(0); j < i; ++j)
            {
                if (m_getBox(node->values[i])
                        .intersects(m_getBox(node->values[j])))
                    intersections.emplace_back(node->values[i],
                                               node->values[j]);
            }
        }
        if (!isLeaf(node))
        {
            // Values in this node can intersect values in descendants
            for (const auto& child : node->children)
            {
                for (const auto& value : node->values)
                    findIntersectionsInDescendants(child.get(), value,
                                                   intersections);
            }
            // Find intersections in children
            for (const auto& child : node->children)
                findAllIntersections(child.get(), intersections);
        }
    }

    void findIntersectionsInDescendants(
        Node* node, const T& value,
        std::vector<std::pair<T, T>>& intersections) const
    {
        // Test against the values stored in this node
        for (const auto& other : node->values)
        {
            if (m_getBox(value).intersects(m_getBox(other)))
                intersections.emplace_back(value, other);
        }
        // Test against values stored into descendants of this node
        if (!isLeaf(node))
        {
            for (const auto& child : node->children)
                findIntersectionsInDescendants(child.get(), value,
                                               intersections);
        }
    }
};

} // namespace de

#endif // DE_SPATIAL_QUADTREE_HPP
