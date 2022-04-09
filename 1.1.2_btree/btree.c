#include <stdlib.h>
#define M 3

typedef int KEY_TYPE;

struct btree_node {
    struct btree_node** children; // 1. 每个节点至多有M颗子树
    KEY_TYPE* keys;               // M * 2是为了方便树做分裂
    int num;                      // 节点的key的数量
    int leaf;                     // 是否为叶子节点
};

struct btree {
    struct btree_node* root;
    int t;
};

// 1 是叶子结点 0 不是
struct btree_node* btree_create_node(int t, int leaf)
{
    struct btree_node* node = (struct btree_node*)calloc(1, sizeof(struct btree_node));
    // 创建失败
    if(node == NULL)
	return NULL;

    node->num = 0;
    node->keys = (KEY_TYPE*)calloc(1, (2 * t - 1) * sizeof(KEY_TYPE));
    node->children = (struct btree_node**)calloc(1, 2 * t * sizeof(struct btree_node*));
    node->leaf = leaf;

    return node;
}

void btree_destroy_node(struct btree_node* node)
{
    if(node) {
	if(node->keys)
	    free(node->keys);
	if(node->children)
	    free(node->children);
	free(node);
    }
}

void btree_create(struct btree* T, int t)
{
    T->t = t;
    struct btree_node* x = btree_create_node(t, 1);
    T->root = x;
}

/**
 * @brief 结点分裂
 * @param T b树
 * @param x 结点
 * @param i 第几颗子树
 */
void btree_split_child(struct btree* T, struct btree_node* x, int i)
{
    struct btree_node* y = x->children[i]; // 第i个子树
    struct btree_node* z = btree_create_node(T->t, y->leaf); // 创建的新结点用作复制
   
    // Z结点的修改
    // 复制
    for (int j = 0; j < T->t - 1; j++) {
        z->keys[j] = y->keys[j + T->t];
    }
    
    // 如果有结点, 把结点下的子树一起复制过来
    if (y->leaf) {
        for (int j = 0; j < T->t; j++) {
            z->children[j] = z->children[j + T->t];
        }
    }
    
    // 中间结点分裂
    y->num = T->t - 1;

    // X 的修改
    // 选择分裂结点插入位置, 并把指针后移, 空个位置出来
    for (int j = x->num; j >= i + 1; j--) {
        x->children[j + 1] = x->children[j];
    }
    // 把复制出来的结点放到x结点上
    x->children[i + 1] = z;
    for (j = x->num - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }
    
    // X
    x->keys[i] = y->keys[T->t - 1];
    x->num += 1;
}

void btree_insert_not_full(struct btree* T, struct btree_node* x, KEY_TYPE key) {
    // 当前x结点最后一个key的索引
    int i = x->num - 1;
    if (x->leaf) {
        // 寻找插入位置, 如果不是最后一个后移指针
        while (i >= 0 && x->keys[i] > key) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }
        // 插入key
        x->keys[i + 1] = key;
        x->num += 1;
    } else {
        while (i >= 0 && x->keys[i] > key) i--;
        if (x->children[i + 1]->num == 2 * T->t - 1) {
            btree_split_child(T, x, i + 1);
            if (key > x->keys[i + 1]) i++;                  
        }

        btree_insert_not_full(T, x->children[i + 1], key);
    }
}

// insert
void btree_insert(struct btree* T, KEY_TYPE key)
{
    struct btree_node* root = T->root;
    // 根节点数量满了
    if (root->num == 2 * T->t - 1) {
       struct btree_node* node = btree_create_node(T->t, 0);
       T->root = node;
       node->children[0] = root;
       
       btree_split_child(T, node, 0);
    } else {

    }
}
