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
    struct btree_node* y = x->children[i];
    struct btree_node* z = btree_create_node(T->t, y->leaf);

    int j = 0;
    for(j = 0; j < T->t - 1; j++) {
	z->keys[j] = y->keys[j + T->t];
    }

    if(y->leaf == 0) {
	for(j = 0; j < T->t; j++) {
	    i z->children[j] = z->children[j + T->t];
	}
    }

    y->num = T->t - 1;
    for (j = x->
}

// insert
void btree_insert(struct btree* T, KEY_TYPE key)
{
    struct btree_node* root = T->root;
    if(root->num) {
	
    }
}
