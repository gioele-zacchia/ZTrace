#ifndef ALVTREE_H
#define AVLREE_H

#include <stdbool.h>

typedef struct AVLNode 
{ 
    unsigned long long key;
    void* content; 
    struct AVLNode *left; 
    struct AVLNode *right; 
    int height; 
}AVLNode; 
  
struct AVLNode* AVLNewNode(unsigned long long key,void* content);
struct AVLNode* AVLInsert(struct AVLNode* node, unsigned long long int key, void* content);
void AVLInsertInPlace(struct AVLNode** node, unsigned long long int key, void* content);
void* AVLFind(AVLNode* root, unsigned long long int key);
void* AVLFindClosest(AVLNode* root, unsigned long long int key);
void AVLFree(AVLNode* root,bool freeResource);

#endif