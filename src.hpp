#ifndef ESET
#define ESET

#include <utility>
#include <cstddef>
#include <stdexcept>
#include <functional>

template<class Key,class Compare = std::less<Key>>
class Eset {
private:
    struct Node {
        Key data;
        Node* left;
        Node* right;
        Node* parent;
        bool color; // 0 for red, 1 for black
        Node(const Key& val, bool c = 0, Node* p =nullptr, Node* l = nullptr, Node* r = nullptr)
            : data(val), color(c), parent(p), left(l), right(r) {}
    };
    Node* root;
    Node* header; // 用于实现begin和end
    Compare cmp;
    size_t size;

    class iterator {
    private:
        Node* node;
        const Eset* tree;
    public:
        iterator() : node(nullptr), tree(nullptr) {}
        iterator(Node* n, const Eset* t) : node(n), tree(t) {}
        Key& operator*() const { 
            if (*this == tree->end()) 
                throw "It's the end!";
            return node->data; 
        }
        iterator& operator++() {
            if (*this == tree->end()) 
                return *this;
            if (node->right) {
                node = node->right;
                while (node->left) node = node->left;
            } 
            else {
                Node* tmp_parent = node->parent;
                while (tmp_parent && node == tmp_parent->right) {
                    node = tmp_parent;
                    tmp_parent = tmp_parent->parent;
                }
                node = tmp_parent;
            }
            return *this;
        }
        iterator operator++(int) {
            iterator temp = *this;
            ++(*this);
            return temp;
        }
        iterator& operator--() {
            if (*this == tree->begin()) 
                return *this;
            if (node->left) {
                node = node->left;
                while (node->right) node = node->right;
            } 
            else {
                Node* tmp_parent = node->parent;
                while (tmp_parent && node == tmp_parent->left) {
                    node = tmp_parent;
                    tmp_parent = tmp_parent->parent;
                }
                node = tmp_parent;
            }
            return *this;
        }
        iterator operator--(int) {
            iterator temp = *this;
            --(*this);
            return temp;
        }
        bool operator==(const iterator& other) const { return node == other.node; }
        bool operator!=(const iterator& other) const { return node != other.node; }
    };

    iterator insert(const Key& key) {
        Node* new_node = new Node(key);
        if (!root) {
            root = new_node;
            header = new Node(Key(), 1, nullptr, root, nullptr); // header的左子树指向root
            size++;
            return iterator(new_node, this);
        }
        Node* parent = nullptr;
        Node* current = root;
        while (current) {
            parent = current;
            if (cmp(key, current->data)) {
                current = current->left;
            } 
            else {
                current = current->right;
            }
        }
        new_node->parent = parent;
        if (cmp(key, parent->data)) {
            parent->left = new_node;
        } 
        else {
            parent->right = new_node;
        }
        size++;
        // 插入后修复红黑树性质
        insert_fixup(new_node);
        return iterator(new_node, this);
    }

    void left_rotate(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        if (y->left) y->left->parent = x;
        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } 
        else {
            if (x == x->parent->left) {
                x->parent->left = y;
            } 
            else {
                x->parent->right = y;
            }
        }
        y->left = x;
        x->parent = y;
    }

    void right_rotate(Node* x) {
        Node* y = x->left;
        x->left = y->right;
        if (y->right) y->right->parent = x;
        y->parent = x->parent;
        if (!x->parent) {
            root = y;
        } 
        else {
            if (x == x->parent->right) {
                x->parent->right = y;
            } 
            else {
                x->parent->left = y;
            }
        }
        y->right = x;
        x->parent = y;
    }

    void insert_fixup(Node* z) {
        while (z->parent && z->parent->color == 0) {
            if (z->parent == z->parent->parent->left) {
                Node* y = z->parent->parent->right;
                if (y && y->color == 0) {
                    z->parent->color = 1;
                    y->color = 1;
                    z->parent->parent->color = 0;
                    z = z->parent->parent;
                } 
                else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        left_rotate(z);
                    }
                    z->parent->color = 1;
                    z->parent->parent->color = 0;
                    right_rotate(z->parent->parent);
                }
            } 
            else {
                Node* y = z->parent->parent->left;
                if (y && y->color == 0) {
                    z->parent->color = 1;
                    y->color = 1;
                    z->parent->parent->color = 0;
                    z = z->parent->parent;
                } 
                else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        right_rotate(z);
                    }
                    z->parent->color = 1;
                    z->parent->parent->color = 0;
                    left_rotate(z->parent->parent);
                }
            }
        }
        root->color = 1; // 根节点必须为黑色
    }

    void clear(Node* node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }
public:
    Eset() : root(nullptr), header(nullptr), size(0) {}
    ~Eset() { clear(root); }

    // 硬拷贝
    ESet(const ESet& other) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            this->emplace(*it);
        }
    }
    ESet& operator=(const ESet& other) {
        Eset tmp(other);
        std::swap(root, tmp.root);
        std::swap(header, tmp.header);
        std::swap(size, tmp.size);
        return *this;
    }
    
    template<class... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
         Key key(std::forward<Args>(args)...);
         auto it = find(key);
         if (it != end()) {
             return {it, false};
         }
         return {insert(key), true};
    }
    
    size_t erase(const Key& key);
    iterator find(const Key& key) const {
        Node* current = root;
        while (current) {
            if (cmp(key, current->data)) {
                current = current->left;
            } 
            else if (cmp(current->data, key)) {
                current = current->right;
            } 
            else {
                return iterator(current, this);
            }
        }
        return end();
    }

    size_t range(const Key& l, const Key& r) const;

    size_t size() const noexcept {
        return size;
    }

    iterator lower_bound(const Key& key) const;
    iterator upper_bound(const Key& key) const;

    // 要O(1)的begin和end，所以可能要有个Header节点
    iterator begin() const noexcept;
    iterator end() const noexcept;
};


#endif // ESET