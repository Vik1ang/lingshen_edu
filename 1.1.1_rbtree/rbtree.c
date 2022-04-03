#define RED 1
#define BLACK 2

typedef int KEY_TYPE;

int key_compare(KEY_TYPE a, KEY_TYPE b)
{
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

void _left_rotate(rbtree* T, rbtree_node* x)
{
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

void _right_rotate(rbtree* T, rbtree_node* y)
{
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

void rbtree_insert_fixup(rbtree* T, rbtree_node* z)
{
	// 父节点是红色的需要调整
	while (z->parent->color == RED) {
		// 父节点是祖父节点的左子树还是右子树
		// 左子树
		if (z->parent == z->parent->parent->left) {
			rbtree_node* y = z->parent->right; // 叔父结点
			// 1. 叔父结点是红色
			if (y->color == RED) {
				// 把祖父结点变成红色, 把父节点和叔父结点变成黑色
				// 父节点变黑
				z->parent->color = BLACK;
				// 叔父结点变黑
				y->color = BLACK;
				// 祖父结点变红
				z->parent->parent->color = RED;

				// 将当前结点变成祖父结点, 继续向上检查红黑树是否需要调整
				z = z->parent->parent;
			} else {
				// 3. 叔父结点是黑色, 当前结点是右子树
				if (z == z->parent->right) {
					// 左旋完之后, 会把父节点旋转成为子节点
					z = z->parent;
					_left_rotate(T, z);
				}

				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				// 右旋祖父结点
				_right_rotate(T, z->parent->parent);
			}
		} else {
			rbtree_node* y = z->parent->parent->left;
			if (y -> color == RED) {
				z->parent->color = BLACK;
				y->color = BLACK;
				z->parent->parent->color = RED;
				z = z->parent->parent;
			} else {
				// 2. 叔父结点是黑色, 当前结点是左子树 这种情况会直接进行右旋平衡
				if (z == z->parent->left) {
					z = z->parent;
					_right_rotate(T, z);
				}
				z->parent->color = BLACK;
				z->parent->parent->color = RED;
				_left_rotate(T, z->parent->parent);
			}
		}
	}

	// 把根节点的颜色置为黑色
	T->root->color = BLACK;
}

void rbtree_insert(rbtree* T, rbtree_node* z)
{
	rbtree_node* x = T->root; // dameon节点, 供遍历使用
	rbtree_node* y = T->nil; // 这个的用处是在退出循环的时候, y一定会是x的父节点
	while (x != T->nil) {
		y = x; // 保证y是x的父结点
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

	z->parent = y;
	if (y == T->nil) {
		// 插入的第一个结点, 也就是说是根结点
		T->root = z;
	} else if (z->key < y->key) {
		y->left = z;
	} else {
		y->right = z;
	}

	// 新插入的叶子结点都置空
	z->left = T->nil;
	z->right = T->nil;
	z->color = RED; // 结点默认为红色, 因为比较好判断和操作


	rbtree_insert_fixup(T, z);
}

rbtree_node* rbtree_delete(rbtree* T, rbtree_node* z)
{
	// TODO: 还没完成
	return T->nil;
}
