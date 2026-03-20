#ifndef ESET
#define ESET

#include <utility>
#include <cstddef>
#include <stdexcept>

template<class Key,class Compare = std::less<Key>>
class ESet {
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
    size_t size_;
public:
    class iterator {
    private:
        Node* node;
        const ESet* tree;
    public:
        iterator() : node(nullptr), tree(nullptr) {}
        iterator(Node* n, const ESet* t) : node(n), tree(t) {}
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
                node = tmp_parent ? tmp_parent : tree->header;
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
            size_++;
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
        size_++;
        // 插入后修复红黑树性质
        insert_fixup(new_node);
        header->left = root;
        return iterator(new_node, this);
    }
private:
    void transplant(Node* u, Node* v) {
        if (!u->parent) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v) {
            v->parent = u->parent;
        }
    }

    void erase_fixup(Node* x, Node* x_parent) {
        while (x != root && (!x || x->color == 1)) {
            if (x == x_parent->left) {
                Node* w = x_parent->right;
                if (w && w->color == 0) { // 兄弟节点是红色
                    w->color = 1;
                    x_parent->color = 0;
                    left_rotate(x_parent);
                    w = x_parent->right;
                }
                if ((!w->left || w->left->color == 1) && (!w->right || w->right->color == 1)) {
                    if (w) w->color = 0;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (!w->right || w->right->color == 1) {
                        if (w->left) w->left->color = 1;
                        w->color = 0;
                        right_rotate(w);
                        w = x_parent->right;
                    }
                    w->color = x_parent->color;
                    x_parent->color = 1;
                    if (w->right) w->right->color = 1;
                    left_rotate(x_parent);
                    x = root;
                }
            } else { // 对称情况：x 是右孩子
                Node* w = x_parent->left;
                if (w && w->color == 0) {
                    w->color = 1;
                    x_parent->color = 0;
                    right_rotate(x_parent);
                    w = x_parent->left;
                }
                if ((!w->right || w->right->color == 1) && (!w->left || w->left->color == 1)) {
                    if (w) w->color = 0;
                    x = x_parent;
                    x_parent = x->parent;
                } else {
                    if (!w->left || w->left->color == 1) {
                        if (w->right) w->right->color = 1;
                        w->color = 0;
                        left_rotate(w);
                        w = x_parent->left;
                    }
                    w->color = x_parent->color;
                    x_parent->color = 1;
                    if (w->left) w->left->color = 1;
                    right_rotate(x_parent);
                    x = root;
                }
            }
        }
        if (x) x->color = 1;
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
    ESet() : root(nullptr), header(nullptr), size_(0) {}
    ~ESet() { clear(root); }

    // 硬拷贝
    ESet(const ESet& other) {
        for (auto it = other.begin(); it != other.end(); ++it) {
            this->emplace(*it);
        }
    }
    ESet& operator=(const ESet& other) {
        ESet tmp(other);
        std::swap(root, tmp.root);
        std::swap(header, tmp.header);
        std::swap(size_, tmp.size_);
        return *this;
    }
    
    ESet(ESet&& other) noexcept : root(other.root), header(other.header), size_(other.size_) {
        other.root = nullptr;
        other.header = nullptr;
        other.size_ = 0;
    }
    ESet& operator=(ESet&& other) noexcept {
        if (this != &other) {
            clear(root);
            root = other.root;
            header = other.header;
            size_ = other.size_;
            other.root = nullptr;
            other.header = nullptr;
            other.size_ = 0;
        }
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
    
    size_t erase(const Key& key) {
        Node* z = root;
        // 1. 查找要删除的节点
        while (z) {
            if (cmp(key, z->data)) z = z->left;
            else if (cmp(z->data, key)) z = z->right;
            else break; // 找到了
        }
        if (!z) return 0; // 没找到

        // 2. 红黑树的删除逻辑
        Node* y = z;
        bool y_original_color = y->color;
        Node* x = nullptr;
        Node* x_parent = nullptr; // 因为 x 可能为空，必须单独记录它的父节点以供 fixup 使用

        if (!z->left) {
            x = z->right;
            x_parent = z->parent;
            transplant(z, z->right);
        } else if (!z->right) {
            x = z->left;
            x_parent = z->parent;
            transplant(z, z->left);
        } else {
            // 找后继节点
            y = z->right;
            while (y->left) y = y->left;
            y_original_color = y->color;
            x = y->right;
            
            if (y->parent == z) {
                x_parent = y;
            } else {
                x_parent = y->parent;
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color; // y 继承 z 的颜色
        }
        
        delete z;
        size_--;

        // 3. 如果删掉的物理节点 y 是黑色，可能会破坏红黑树性质，需要修复
        if (y_original_color == 1) { // 1 代表黑色
            erase_fixup(x, x_parent);
        }
        if (header) {
            header->left = root;
        }
        
        // 特殊处理：如果树被删空了，清理 header
        if (size_ == 0 && header) {
            delete header;
            header = nullptr;
        }

        return 1;
    }
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

    size_t range(const Key& l, const Key& r) const {
        size_t count = 0;
        // 从 >= l 的第一个元素开始，遍历直到元素 > r
        for (auto it = lower_bound(l); it != end() && !cmp(r, *it); ++it) {
            count++;
        }
        return count;
    }

    size_t size() const noexcept {
        return size_;
    }

    iterator lower_bound(const Key& key) const {
        Node* current = root;
        Node* result = header; // 默认返回 end()
        while (current) {
            if (!cmp(current->data, key)) { // current->data >= key
                result = current;
                current = current->left;    // 继续往左找更贴近的
            } else {
                current = current->right;   // current->data < key
            }
        }
        return iterator(result, this);
    }
    iterator upper_bound(const Key& key) const {
        Node* current = root;
        Node* result = header; 
        while (current) {
            if (cmp(key, current->data)) {  // current->data > key
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return iterator(result, this);
    }

    // 要O(1)的begin和end，所以可能要有个Header节点
    iterator begin() const noexcept {
        if (!root) return end();
        // 你的 insert 并没有动态维护 leftmost，因此这里用 O(log N) 的方式找最小值。
        // 若要实现严格的 O(1) begin，需在 insert/erase 中维护 header->left 始终指向最小节点。
        Node* current = root;
        while (current->left) {
            current = current->left;
        }
        return iterator(current, this);
    }
    iterator end() const noexcept {
        return iterator(header, this);
    }
};


#endif // ESET