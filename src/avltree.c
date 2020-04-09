// C program to insert a node in AVL tree 
#include<stdio.h> 
#include<stdlib.h> 
#include <stdint.h>
#include "../ZTR/avltree.h"
  
// An AVL tree node 

// A utility function to get maximum of two integers 
unsigned long long max(unsigned long long a, unsigned long long b); 
  
// A utility function to get the height of the tree 
int height(struct AVLNode *N) 
{ 
    if (N == NULL) 
        return 0; 
    return N->height; 
} 
  
// A utility function to get maximum of two integers 
unsigned long long max(unsigned long long a, unsigned long long b) 
{ 
    return (a > b)? a : b; 
} 
  
/* Helper function that allocates a new node with the given key and 
    NULL left and right pointers. */
struct AVLNode* AVLNewNode(unsigned long long key,void* content) 
{ 
    struct AVLNode* node = (struct AVLNode*) 
                        malloc(sizeof(struct AVLNode)); 
    node->key   = key; 
    node->left   = NULL; 
    node->right  = NULL; 
    node->content = content;
    node->height = 1;  // new node is initially added at leaf 
    return(node); 
} 
  
// A utility function to right rotate subtree rooted with y 
// See the diagram given above. 
struct AVLNode *rightRotate(struct AVLNode *y) 
{ 
    struct AVLNode *x = y->left; 
    struct AVLNode *T2 = x->right; 
  
    // Perform rotation 
    x->right = y; 
    y->left = T2; 
  
    // Update heights 
    y->height = max(height(y->left), height(y->right))+1; 
    x->height = max(height(x->left), height(x->right))+1; 
  
    // Return new root 
    return x; 
} 
  
// A utility function to left rotate subtree rooted with x 
// See the diagram given above. 
struct AVLNode *leftRotate(struct AVLNode *x) 
{ 
    struct AVLNode *y = x->right; 
    struct AVLNode *T2 = y->left; 
  
    // Perform rotation 
    y->left = x; 
    x->right = T2; 
  
    //  Update heights 
    x->height = max(height(x->left), height(x->right))+1; 
    y->height = max(height(y->left), height(y->right))+1; 
  
    // Return new root 
    return y; 
} 
  
// Get Balance factor of node N 
int getBalance(struct AVLNode *N) 
{ 
    if (N == NULL) 
        return 0; 
    return height(N->left) - height(N->right); 
} 

void AVLInsertInPlace(struct AVLNode** node, unsigned long long int key, void* content){
    *node = AVLInsert(*node,key,content);
}
  
// Recursive function to insert a key in the subtree rooted 
// with node and returns the new root of the subtree. 
struct AVLNode* AVLInsert(struct AVLNode* node, unsigned long long int key, void* content) 
{ 
    /* 1.  Perform the normal BST insertion */
    if (node == NULL) 
        return(AVLNewNode(key,content)); 
  
    if (key < node->key) 
        node->left  = AVLInsert(node->left, key,content); 
    else if (key > node->key) 
        node->right = AVLInsert(node->right, key,content); 
    else // Equal keys are not allowed in BST 
        return node; 
  
    /* 2. Update height of this ancestor node */
    node->height = 1 + max(height(node->left), 
                           height(node->right)); 
  
    /* 3. Get the balance factor of this ancestor 
          node to check whether this node became 
          unbalanced */
    int balance = getBalance(node); 
  
    // If this node becomes unbalanced, then 
    // there are 4 cases 
  
    // Left Left Case 
    if (balance > 1 && key < node->left->key) 
        return rightRotate(node); 
  
    // Right Right Case 
    if (balance < -1 && key > node->right->key) 
        return leftRotate(node); 
  
    // Left Right Case 
    if (balance > 1 && key > node->left->key) 
    { 
        node->left =  leftRotate(node->left); 
        return rightRotate(node); 
    } 
  
    // Right Left Case 
    if (balance < -1 && key < node->right->key) 
    { 
        node->right = rightRotate(node->right); 
        return leftRotate(node); 
    } 
  
    /* return the (unchanged) node pointer */
    return node; 
} 

void* AVLFind(AVLNode* root, unsigned long long int key){
    if(root == NULL){
        return NULL;
    }
    if(root->key == key){
        return root->content;
    }else {
        if(key < root->key && root->left){
            return AVLFind(root->left,key);
        }else if(root->right){
            return AVLFind(root->left,key);
        }else{
            return NULL;
        }
    }
}

void AVLFindClosestUtil(AVLNode *ptr, uint64_t k, uint64_t *min_diff, 
                                      AVLNode** bestCand) { 
    if (ptr == NULL) 
        return ; 
  
    // If k itself is present 
    if (ptr->key == k) 
    { 
        *bestCand = ptr;
        return; 
    } 
  
    // update min_diff and min_diff_key by checking 
    // current node value 
    if (*min_diff > abs(ptr->key - k)) 
    { 
        *min_diff = abs(ptr->key - k); 
        *bestCand = ptr; 
    } 
  
    // if k is less than ptr->key then move in 
    // left subtree else in right subtree 
    if (k < ptr->key) 
        AVLFindClosestUtil(ptr->left, k, min_diff, bestCand); 
    else
        AVLFindClosestUtil(ptr->right, k, min_diff, bestCand); 
} 

void* AVLFindClosest(AVLNode* root, unsigned long long int key){
    uint64_t min_diff = UINT64_MAX;
    AVLNode* cBest = NULL;
  
    // Find value of min_diff_key (Closest key 
    // in tree with k) 
    AVLFindClosestUtil(root, key, &min_diff, &cBest); 
  
    return cBest->content; 
}

void AVLFree(AVLNode* root,bool freeResource){
    if(root->left != NULL){
        AVLFree(root->left,freeResource);
    }
    if(root->right != NULL){
        AVLFree(root->right,freeResource);
    }
    if(freeResource){
        free(root->content);
    }
    free(root);
}


  
// A utility function to print preorder traversal 
// of the tree. 
// The function also prints height of every node 
/*void preOrder(struct AVLNode *root) 
{ 
    if(root != NULL) 
    { 
        printf("%d ", root->key); 
        preOrder(root->left); 
        preOrder(root->right); 
    } 
} */
  
