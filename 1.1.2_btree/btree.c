#include <stdio.h>
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
    for (int j = x->num - 1; j >= i; j--) {
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
        // 判断是插入在哪个子树上
        while (i >= 0 && x->keys[i] > key) i--;
        // 对比子树
        // 如果满了就进行分裂
        if (x->children[i + 1]->num == 2 * T->t - 1) {
            btree_split_child(T, x, i + 1);
            // 这个真的非常绝, 就知道是插入在分裂之后的哪个子树上
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
        btree_insert_not_full(T, root, key);
    }
}

//{child[idx], key[idx], child[idx+1]}
void btree_merge(struct btree *T, struct btree_node *node, int idx) {
    struct btree_node *left = node->children[idx];
	struct btree_node *right = node->children[idx+1];

	int i = 0;

	//data merge
	left->keys[T->t-1] = node->keys[idx];
	for (i = 0;i < T->t-1;i ++) {
		left->keys[T->t+i] = right->keys[i];
	}
	if (!left->leaf) {
		for (i = 0;i < T->t;i ++) {
			left->children[T->t+i] = right->children[i];
		}
	}
	left->num += T->t;

	//destroy right
	btree_destroy_node(right);

	//node
	for (i = idx+1;i < node->num;i ++) {
		node->keys[i-1] = node->keys[i];
		node->children[i] = node->children[i+1];
	}
	node->children[i+1] = NULL;
	node->num -= 1;

	if (node->num == 0) {
		T->root = left;
		btree_destroy_node(node);
	}
}

void btree_delete_key(struct btree* T, struct btree_node* node, KEY_TYPE key) {
    if (node == NULL) return;

    int idx = 0, i;
    while (idx < node->num && key > node->keys[idx]) {
        idx++;
    }

    if (idx < node->num && key == node->keys[idx]) {
        if (node->leaf) {
            for (i = idx; i < node->num - 1; i++) {
                node->keys[i] = node->keys[i + 1];
            }

            node->keys[node->num - 1] = 0;
            node->num--;

            if (node->num == 0) {
                // root
                free(node);
                T->root = NULL;
            }

            return;
        } else if (node->children[idx]->num >= T->t) {
            struct btree_node* right = node->children[idx + 1];
            node->keys[idx] = right->keys[0];

            btree_delete_key(T, right, right->keys[0]);
        } else {
            btree_merge(T, node, idx);
            btree_delete_key(T, node->children[idx], key);
        }
    } else {
        struct btree_node* child = node->children[idx];
        if (child == NULL) {
            printf("Cannot del key = %d\n", key);
            return;
        }

        if (child->num == T->t - 1) {
            struct btree_node* left = NULL;
            struct btree_node* right = NULL;
            if (idx - 1 >= 0) {
                left = node->children[idx - 1];
            }
            if (idx + 1 <= node->num) {
                right = node->children[idx + 1];
            }
            if ((left && left->num >= T->t) ||
                    (right && right->num >= T->t)) {
                int richR = 0;
                if (right) richR = 1;
                if (left && right) richR = (right->num > left->num) ? 1 : 0;
                
                if (right && right->num >= T->t && richR) { //borrow from next
					child->keys[child->num] = node->keys[idx];
					child->children[child->num+1] = right->children[0];
					child->num ++;

					node->keys[idx] = right->keys[0];
					for (i = 0;i < right->num - 1;i ++) {
						right->keys[i] = right->keys[i+1];
						right->children[i] = right->children[i+1];
					}

					right->keys[right->num-1] = 0;
					right->children[right->num-1] = right->children[right->num];
					right->children[right->num] = NULL;
					right->num --;

				} else { //borrow from prev

					for (i = child->num;i > 0;i --) {
						child->keys[i] = child->keys[i-1];
						child->children[i+1] = child->children[i];
					}

					child->children[1] = child->children[0];
					child->children[0] = left->children[left->num];
					child->keys[0] = node->keys[idx-1];

					child->num ++;

					node->keys[idx-1] = left->keys[left->num-1];
					left->keys[left->num-1] = 0;
					left->children[left->num] = NULL;
					left->num --;
				}
            
            } else if (!left || (left->num == T->t - 1) &&
                    (!right || (right->num == T->t - 1))) {
                if (left && left->num == T->t - 1) {
					btree_merge(T, node, idx-1);					
					child = left;
				} else if (right && right->num == T->t - 1) {
					btree_merge(T, node, idx);
				}
            }
        }
        btree_delete_key(T, child, key);
    }
}

int btree_delete(struct btree* T, KEY_TYPE key) {
    if (!T->root) return -1;

    btree_delete_key(T, T->root, key);
    return 0;
}

void btree_print(struct btree *T, struct btree_node *node, int layer)
{
        struct btree_node* p = node;
            int i;
            if(p){
                        printf("\nlayer = %d keynum = %d is_leaf = %d\n", layer, p->num, p->leaf);
                                for(i = 0; i < node->num; i++)
                                                printf("%c ", p->keys[i]);
                                        printf("\n");
#if 0
                                                printf("%p\n", p);
                                                        for(i = 0; i <= 2 * T->t; i++)
                                                                        printf("%p ", p->childrens[i]);
                                                                printf("\n");
#endif
                                                                        layer++;
                                                                                for(i = 0; i <= p->num; i++)
                                                                                                if(p->children[i])
                                                                                                                    btree_print(T, p->children[i], layer);
                                                                                    
            }
                else printf("the tree is empty\n");
                
}


int main() {
    struct btree T = {0};

	btree_create(&T, 3);
	srand(48);

	int i = 0;
	char key[26] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	for (i = 0;i < 26;i ++) {
		//key[i] = rand() % 1000;
		printf("%c ", key[i]);
		btree_insert(&T, key[i]);
	}

	btree_print(&T, T.root, 0);

	for (i = 0;i < 26;i ++) {
		printf("\n---------------------------------\n");
		btree_delete(&T, key[25-i]);
		//btree_traverse(T.root);
		btree_print(&T, T.root, 0);
	}
    return 0;
}
