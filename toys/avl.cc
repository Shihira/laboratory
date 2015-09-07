#include <iostream>
#include <string>

template <
    typename DataType,
    typename LessThanType = std::less<DataType>,
    typename GreaterThanType = std::greater<DataType>
>
class avl_tree {
public:
    typedef DataType data_t;
    typedef LessThanType less_than_t;
    typedef GreaterThanType greater_than_t;

protected:
    struct avl_node {
        typedef DataType data_t;
        typedef avl_node self_t;

        self_t* left = nullptr;
        self_t* right = nullptr;
        self_t* parent = nullptr;

        data_t data;
        int height = 1;

        avl_node(const data_t& d, self_t* p = nullptr)
                : parent(p), data(d) { }

        void rotation_l() {
            node_t* p = parent;
            p->left = this->right;
            this->right = p;
            this->parent = p->parent;
            p->parent = this;
        }

        void rotation_r() {
            node_t* p = parent;
            p->right = this->left;
            this->left = p;
            this->parent = p->parent;
            p->parent = this;
        }

        int balance_factor() { return right - left; }

        self_t* insert(const data_t& d) {
            if(less_than_t()(d, this->data)) {
                if(left) left->insert(d);
                else left = new node_t(d, this);
            } else if(greater_than_t()(d, this->data)) {
                if(right) right->insert(d);
                else right = new node_t(d, this);
            }

            return this;
        }

        void print(int indent = 0, int ind_width = 4) {
            for(int i = 0; i < indent; i++) {
                std::cout << '|'
                    << std::string(ind_width - 1,
                    (i == indent - 1) ? '-' : ' ');
            }

            std::cout << data << std::endl;

            if(left) { left->print(indent + 1); }
            if(right) { right->print(indent + 1); }
        }

    };

protected:
        typedef avl_node node_t;

public:
    node_t* root_ = nullptr;

    void insert(const data_t& data) {
        if(root_) root_ = root_->insert(data);
        else root_ = new node_t(data);
    }
    void print() { if(root_) root_->print(); }

    avl_tree() = default;
    avl_tree(const node_t& n) { root_ = new node_t(n); }
};

int main()
{
    avl_tree<int> root;

    root.insert(10);
    root.insert(9);
    root.insert(8);
    root.insert(7);
    root.insert(6);
    root.root_->left->left->rotation_l();

    root.print();
}
