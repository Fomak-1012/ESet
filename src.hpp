#ifndef ESET
#define ESET

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
    Compare cmp;
public:
    Eset() : root(nullptr) {}
    ~Eset() { clear(root); }

    // 硬拷贝
    ESet(const ESet& other);
    ESet& operator=(const ESet& other);
    
    template< class... Args >
    std::pair<iterator, bool> emplace( Args&&... args ); 
    
    size_t erase(const Key& key);
    iterator find(const Key& key) const;

    size_t range(const Key& l, const Key& r) const;

    size_t size() const noexcept;

    iterator lower_bound(const Key& key) const;
    iterator upper_bound(const Key& key) const;

    // 要O(1)的begin和end，所以可能要有个Header节点
    iterator begin() const noexcept;
    iterator end() const noexcept;
};


#endif // ESET