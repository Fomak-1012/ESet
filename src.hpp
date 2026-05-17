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
        size_t subtree_size;
        Node* subtree_min;
        Node* subtree_max;
        Node(const Key& val, bool c = 0, Node* p =nullptr, Node* l = nullptr, Node* r = nullptr)
            : data(val), left(l), right(r), parent(p), color(c), subtree_size(1), subtree_min(this), subtree_max(this) {}
    };

    struct HeaderNode {
        Node* parent;
        Node* left;
        Node* right;
    };

    Node* root;
    HeaderNode header; // parent=root, left=min, right=max
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
            if (*this == tree->end()) {
                if (!tree->header.right) return *this;
                node = tree->header.right;
                return *this;
            }
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

private:
    static size_t node_size(Node* node) {
        return node ? node->subtree_size : 0;
    }

    static void pull(Node* node) {
        if (node) {
            node->subtree_size = node_size(node->left) + node_size(node->right) + 1;
            node->subtree_min = node->left ? node->left->subtree_min : node;
            node->subtree_max = node->right ? node->right->subtree_max : node;
        }
    }

    void pull_up(Node* node) {
        while (node) {
            pull(node);
            node = node->parent;
        }
    }

    Node* minimum(Node* node) const {
        if (!node) return nullptr;
        while (node->left) node = node->left;
        return node;
    }

    Node* maximum(Node* node) const {
        if (!node) return nullptr;
        while (node->right) node = node->right;
        return node;
    }

    void refresh_header() {
        header.parent = root;
        header.left = root ? root->subtree_min : nullptr;
        header.right = root ? root->subtree_max : nullptr;
    }

    size_t count_less(const Key& key) const {
        size_t count = 0;
        Node* current = root;
        while (current) {
            if (cmp(current->data, key)) {
                count += node_size(current->left) + 1;
                current = current->right;
            } else {
                current = current->left;
            }
        }
        return count;
    }

    size_t count_less_equal(const Key& key) const {
        size_t count = 0;
        Node* current = root;
        while (current) {
            if (!cmp(key, current->data)) {
                count += node_size(current->left) + 1;
                current = current->right;
            } else {
                current = current->left;
            }
        }
        return count;
    }

public:

    iterator insert(const Key& key) {
        Node* new_node = new Node(key);
        if (!root) {              // 建树
            root = new_node;
            root->color = 1;       //根必黑
            size_++;
            refresh_header();
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
        insert_fixup(new_node);
        pull_up(new_node);
        refresh_header();
        return iterator(new_node, this);
    }
private:
    void transplant(Node* u, Node* v) {
        if (!u->parent) {
            root = v;
        } 
        else {
            if (u == u->parent->left) {
                u->parent->left = v;
            } 
            else {
                u->parent->right = v;
            }
        }
        if (v) {
            v->parent = u->parent;
        }
    }

    void erase_fixup(Node* cur, Node* father) {
        while (cur != root && (!cur || cur->color == 1)) {
            if (!father) break;
            if (cur == father->left) {
                #define brother (father->right)

                if (brother && brother->color == 0) { // 状况1：兄弟是红色
                    brother->color = 1;
                    father->color = 0;
                    left_rotate(father);
                }
                if (!brother) {
                    cur = father;
                    father = father->parent;
                    continue;
                }
                if ((!brother->left || brother->left->color == 1) && 
                    (!brother->right || brother->right->color == 1)) { // 状况2：两个侄子均为黑
                    brother->color = 0;
                    cur = father;
                    father = cur->parent;
                } 
                else {
                    if (!brother->right || brother->right->color == 1) { // 状况3：左红右黑
                        if (brother->left) brother->left->color = 1;
                        brother->color = 0;
                        right_rotate(brother);
                    }
                    // 状况4：右侄子为红
                    brother->color = father->color;
                    father->color = 1;
                    if (brother->right) brother->right->color = 1;
                    left_rotate(father);
                    cur = root;
                }
                #undef brother
            } 
            else { // 完全对称
                #define brother (father->left)
                if (brother && brother->color == 0) {
                    brother->color = 1;
                    father->color = 0;
                    right_rotate(father);
                }
                if (!brother) {
                    cur = father;
                    father = father->parent;
                    continue;
                }
                if ((!brother->right || brother->right->color == 1) && 
                    (!brother->left || brother->left->color == 1)) {
                    brother->color = 0;
                    cur = father;
                    father = cur->parent;
                } 
                else {
                    if (!brother->left || brother->left->color == 1) {
                        if (brother->right) brother->right->color = 1;
                        brother->color = 0;
                        left_rotate(brother);
                    }
                    brother->color = father->color;
                    father->color = 1;
                    if (brother->left) brother->left->color = 1;
                    right_rotate(father);
                    cur = root;
                }
                #undef brother
            }
        }
        if (cur) cur->color = 1; 
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
        pull(x);
        pull(y);
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
        pull(x);
        pull(y);
    }

    void insert_fixup(Node* cur) {
        #define father (cur->parent)
        #define grandpa (cur->parent->parent)
        while (father && grandpa && father->color == 0) {       //和父节点都是红的
            if (father == grandpa->left) {
                #define uncle  (cur->parent->parent->right)
                if (uncle && uncle->color == 0) {
                    father->color = 1;
                    uncle->color = 1;
                    grandpa->color = 0;
                    cur = grandpa;
                } 
                else {
                    if (cur == father->right) {
                        cur = father;
                        left_rotate(cur);
                    }
                    father->color = 1;
                    grandpa->color = 0;
                    right_rotate(grandpa);
                }
                #undef uncle
            } 
            else {
                #define uncle (cur->parent->parent->left)
                if (uncle && uncle->color == 0) {
                    father->color = 1;
                    uncle->color = 1;
                    grandpa->color = 0;
                    cur = grandpa;
                } 
                else {
                    if (cur == father->left) {
                        cur = father;
                        right_rotate(cur);
                    }
                    father->color = 1;
                    grandpa->color = 0;
                    left_rotate(grandpa);
                }
                #undef uncle
            }
        }
        root->color = 1; // 根节点必须为黑色
        #undef father
        #undef grandpa
    }

    void clear(Node* node) {
        if (node) {
            clear(node->left);
            clear(node->right);
            delete node;
        }
    }
public:
    ESet() : root(nullptr), header{nullptr, nullptr, nullptr}, size_(0) {}
    ~ESet() {
        clear(root);
    }

    ESet(const ESet& other) : root(nullptr), header{nullptr, nullptr, nullptr}, size_(0) {
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
        other.header = {nullptr, nullptr, nullptr};
        other.size_ = 0;
    }
    ESet& operator=(ESet&& other) noexcept {
        if (this != &other) {
            clear(root);
            root = other.root;
            header = other.header;
            size_ = other.size_;
            other.root = nullptr;
            other.header = {nullptr, nullptr, nullptr};
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
        Node* cur = root;
        while (cur) {
            if (cmp(key, cur->data)) cur = cur->left;
            else if (cmp(cur->data, key)) cur = cur->right;
            else break; 
        }
        if (!cur) return 0;

        Node* toerase = cur;                
        bool toerase_color = toerase->color;      
        Node* standin = nullptr;            
        Node* standin_father = nullptr;     

        if (!cur->left) {           
            standin = cur->right;
            standin_father = cur->parent;
            transplant(cur, standin);
        } 
        else if (!cur->right) {      
            standin = cur->left;
            standin_father = cur->parent;
            transplant(cur, standin);
        } 
        else {                       
            toerase = cur->right;
            while (toerase->left) toerase = toerase->left;
            toerase_color = toerase->color;      
            standin = toerase->right;             
            
            if (toerase->parent == cur) {
                standin_father = toerase;        
            } else {
                standin_father = toerase->parent; 
                transplant(toerase, standin);    
                toerase->right = cur->right; 
                toerase->right->parent = toerase;
            }
            transplant(cur, toerase);      
            toerase->left = cur->left;     
            toerase->left->parent = toerase;
            toerase->color = cur->color;   
        }
        
        delete cur;
        size_--;

        pull_up(standin_father ? standin_father : standin);

        if (toerase_color == 1) { 
            erase_fixup(standin, standin_father);
        }

        refresh_header();
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
        if (cmp(r, l)) return 0;
        return count_less_equal(r) - count_less(l);
    }

    size_t size() const noexcept {
        return size_;
    }

    iterator lower_bound(const Key& key) const {
        Node* current = root;
        Node* result = nullptr; 
        while (current) {
            if (!cmp(current->data, key)) {
                result = current;
                current = current->left;    
            } else {
                current = current->right;   
            }
        }
        return iterator(result, this);
    }
    iterator upper_bound(const Key& key) const {
        Node* current = root;
        Node* result = nullptr; 
        while (current) {
            if (cmp(key, current->data)) { 
                result = current;
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return iterator(result, this);
    }

    iterator begin() const noexcept {
        return iterator(header.left, this);
    }
    iterator end() const noexcept {
        return iterator(nullptr, this);
    }
};


#endif // ESET