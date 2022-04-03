typedef int KEY_TYPE;

int key_compare(KEY_TYPE a, KEY_TYPE b){
	return 0;
}

typedef struct _rbtree_node {
    unsigned char color;
    struct _rbtree_node *left;
    struct _rbtree_node *right;
    struct _rbtree_node *parent;

    KEY_TYPE key;
    void* value;
} rbtree_node;

typedef struct _rbtree {
    rbtree_node* root; // 根结点
    rbtree_node* nil; // 空结点, 所有空结点都指向这个
} rbtree;

void _left_rotate(rbtree* T, rbtree_node* x){
	rbtree_node *y = x->right;
	
	x->right = y->left; // x的右子树指向y的左子树
	if (y->left != T->nil) {
		y->left->parent = x; // x的右子树指向y的左子树
	}

	y->parent = x->parent; // y的父结点指向x的父结点
	if (x->parent == T->nil) { // 判断是否为根结点
		T->root = y;
	} else if (x == x->parent->left) { // 判断y要接到原来的左子树还是右子树
		x->parent->left = y;
	} else {
		x->parent->right = y; // 判断y要接到原来的左子树还是右子树
	}

	y->left = x; // y的左子树指向x
	x->parent = y;  // y的左子树指向x
}

void _right_rotate(rbtree* T, rbtree_node* y){
	rbtree_node* x = y->left;
	
	y->left = x->right; // y的左子树指向x的右子树
	if (x->right != T->nil) {
		x->right->parent = y; // y的左子树指向x的右子树
	}

	x->parent = y->parent; // x的父结点指向y的父结点
	if (y->parent == T->nil) { // 判断是否为根结点
		T->root = x;
	} else if (y == y->parent->right) { // 判断y要接到原来的左子树还是右子树
		y->parent->right = x;
	} else {
		y->parent->left = x; // 判断y要接到原来的左子树还是右子树
	}

	x->right = y; // x的右子树指向y
	y->parent = x; // x的右子树指向y
}

void rbtree_insert(rbtree* T, struct rbtree_node* z)
{
	rbtree_node* x = T->root; // dameon, 供遍历使用

	while (x != T->nil) {
		
		if (z->key < x->key) {
			x = x->left;
		} else if (z->key > x->key) {
			x = x->right;
		} else {
			// 如果有相同的这里有两个处理方式
			// 可以直接返回不做插入
			// 也可以覆盖原本的结点
			// 取决于应用场景
#if 1
			return;
#else
			x->key = z->key;
			return;
#endif
		}
	}

}
