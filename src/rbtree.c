#include "rbtree.h"
#include <stdlib.h>

#include <stdbool.h>
#include <malloc.h>

node_t *idea; // 더미노드
// key_t m_cnt;
bool isLeaf(const node_t *);                                                      // 리프 노드인지 확인하기 위해
void bindNode(node_t *delp);                                                      // 부모노드의 red를 자식 노드들로 바꿔줌 , btree에서 보면 3노드를 4노드로
bool borrowKey(rbtree *t, node_t *delgp, node_t *delp, node_t *del, node_t *sib); // 순회중에 해당 노드의 형제 노드가 black이고 자식 노드 중 red가 있다면 rotate 하는 함수
key_t swapKey(node_t *del);                                                       // 리프 노드가 아닌 내부 노드를 삭제 할 때, 지워야 할 값과 가장 가까운 값을 찾음
bool redAsP(rbtree *t, node_t *delgp, node_t *delp, node_t *sib);                 // del 의 형제가 red일때 형제를 del의 위쪽으로 올리기 위해 rotate 시켜줌
bool delLeaf(rbtree *t, const key_t key, node_t *delp, node_t *del);              // 리프노드인 상태에서 각각의 상황별로 지워 주는 함수
bool is2node(const node_t *p);                                                    // 2노드인가
bool isLeaf(const node_t *p);                                                     // 리프 노드 인가
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n);                 //트리를 배열로 바꿔주는 함수
int add_to_array(node_t *node, key_t *arr, int i);

rbtree *new_rbtree(void)
{
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  return p;
}

void deleteAll(node_t *p)
{
  if (p != NULL)
  {
    deleteAll(p->left);
    deleteAll(p->right);
    free(p);
  }
}
//트리의 모든 노드를 삭제 시킴
void delete_rbtree(rbtree *t)
{
  // TODO: reclaim the tree nodes's memory
  deleteAll(idea->left);
  free(idea);
  free(t);
}

//트리 노드들을 회전하는 함수
node_t *rbtree_rotate(rbtree *t, node_t *pivot, const key_t key)
{
  node_t *child, *gchild;

  if ((key > pivot->key || key == pivot->key) && pivot != idea)
    child = (node_t *)pivot->right;
  else
    child = (node_t *)pivot->left;

  //rotate left
  if (key > child->key || key == child->key)
  {
    gchild = (node_t *)child->right;
    child->right = gchild->left;
    gchild->left = (node_t *)child;
  }
  //rotate right
  else
  {
    gchild = (node_t *)child->left;
    child->left = gchild->right;
    gchild->right = (node_t *)child;
  }

  if ((key > pivot->key || key == pivot->key) && pivot != idea)
    pivot->right = gchild;
  else
    pivot->left = gchild;
  return gchild;
}

node_t *rbtree_insert(rbtree *t, const key_t key)
{
  // TODO: implement insert
  node_t *target, *p, *gp, *ggp;
  //처음 노드 값이 없다면 만들고 리턴 시켜줌
  if (t->root == NULL)
  {
    node_t *temp;
    temp = malloc(sizeof(node_t));
    t->root = temp;
    t->root->key = key;
    t->root->color = RBTREE_BLACK;
    t->root->left = t->root->right = t->root->parent = NULL;

    // 더미 노드를 만들어서 기존 루트의 위에 위치시킴
    idea = (node_t *)malloc(sizeof(node_t));
    idea->color = RBTREE_BLACK;
    idea->left = t->root; //루트는 더미노드의 왼편에서 시작함
    idea->right = 0;

    return t->root;
  }

  ggp = gp = p = (node_t *)idea;
  target = (node_t *)idea->left;
  //타겟이 우리가 순회하면서 리프노드 까지 내려감
  while (target)
  {
    if (key == target->key)
      return NULL;
    if (target->left && target->right && target->left->color == RBTREE_RED && target->right->color == RBTREE_RED)
    { //color promotion부분으로 자식들이 모두 red라면 부모로 색을 올려주고 자식은 black이 됨
      target->color = RBTREE_RED;
      target->left->color = target->right->color = RBTREE_BLACK;
      //올린 후 만약 그 부모의 색이 red라면 회전을 해야됨
      if (p->color == RBTREE_RED) //
      {
        gp->color = RBTREE_RED;
        if ((key > gp->key) != (key > p->key)) // 현재 키값의 위치에 따라 두번 회전할지 한번 할지를 정해줌
          p = rbtree_rotate(t, gp, key);
        target = rbtree_rotate(t, ggp, key);
        target->color = RBTREE_BLACK;
      }
      idea->left->color = RBTREE_BLACK;
    }

    //밑으로 내려옴
    ggp = gp;
    gp = p;
    p = target;
    //찾을 값에 따라 타겟을 움직여줌
    if (key > target->key)
      target = target->right;
    else
      target = target->left;
  }
  //타겟이 리프로 도달해 while문을 탈출하면 값을 새로 넣어준다
  target = (node_t *)calloc(sizeof(node_t), 1);
  target->key = key;
  target->left = target->right = 0;
  target->color = RBTREE_RED;
  if (key > p->key && p != idea)
    p->right = target;
  else
    p->left = target;

  t->m_cnt++;

  //그 부모가 red이면 회전 시켜줘야 한다.
  if (p->color == RBTREE_RED)
  {
    gp->color = RBTREE_RED;
    if ((key > gp->key) != (key > p->key))
      p = rbtree_rotate(t, gp, key);
    target = rbtree_rotate(t, ggp, key);
    target->color = RBTREE_RED;
  }

  //최상단 노드는 검은색이 되야됨
  idea->left->color = RBTREE_BLACK;

  t->root = idea->left;
  return target;
}
// 값을 찾는 함수
node_t *rbtree_find(const rbtree *t, const key_t key)
{
  // TODO: implemoent find
  node_t *s;
  s = t->root;
  while (s && !(key == s->key))
  {
    if (key > s->key)
      s = s->right;
    else
      s = s->left;
  }

  if (s == 0)
    return NULL;
  return s;
}
//트리에서 min값찾기
node_t *rbtree_min(const rbtree *t)
{
  // TODO: implement find
  node_t *cur;
  cur = idea->left;
  while (cur && cur->left != NULL)
  {
    cur = cur->left;
  }

  return cur;
}
//트리에서 max값 찾기
node_t *rbtree_max(const rbtree *t)
{
  // TODO: implement find
  node_t *cur;
  cur = idea->left;
  while (cur && cur->right != NULL)
  {
    cur = cur->right;
  }

  return cur;
}
//리프노드 인가? delete할때 필요함
bool isLeaf(const node_t *p)
{
  if (p == 0)
    return false;
  //왼쪽 자식 없거나 있으면 왼쪽 자식의 색이 빨강 그리고 자식들 없어야됨 오른쪽도 마찬가지
  if ((p->left == 0 || (p->left && p->left->color == RBTREE_RED && !p->left->left && !p->left->right)) && ((p->right == 0 || (p->right && p->right->color == RBTREE_BLACK && p->right->right && p->right->right))))
    return true;
  else
    return false;
}
//2node 인가 해당 노드의 자식이 모두  black인 상태
bool is2node(const node_t *p)
{
  if (p == 0)
    return false;
  if (p->color == RBTREE_RED)
    return false;
  if ((p->left == 0 && p->right == 0) || (p->left && p->right && p->left->color == RBTREE_BLACK && p->right->color == RBTREE_BLACK))
    return true;
  else
    return false;
}
//리프 노드 일때 지우는 함수
bool delLeaf(rbtree *t, const key_t key, node_t *delp, node_t *del)
{
  //첫 번째 경우 del 값과 지우는 값이 같고 지우는 값이 리프노드일때
  if (key == del->key && !del->left && !del->right)
  {
    free(del);
    if ((key > delp->key || key == delp->key) && delp != idea)
      delp->right = 0;
    else
      delp->left = 0;

    return true;
  }
  // 지우는 값이 리프 노드의 부모 일때
  else if (key == del->key)
  {
    node_t *ret;
    if (del->left)
    {
      //올릴값이 왼쪽 값인 경우임, 왼쪽값의 오른값에 부모의 오른쪽값을 가리키게함
      del->left->right = del->right;
      ret = del->left;
      ret->color = RBTREE_BLACK;

      free(del);
    }
    //위와 반대
    else if (del->right)
    {
      del->right->left = del->left;
      ret = del->right;
      ret->color = RBTREE_BLACK;

      free(del);
    }
    if ((ret->key > delp->key || ret->key == delp->key) && delp != idea)
      delp->right = ret;
    else
      delp->left = ret;

    return true;
  }
  // 현재 순회중인 위치 del은 부모이고 지우는 값이 자식일때
  else if (del->left && key == del->left->key)
  {
    free(del->left);
    del->left = 0;

    return true;
  }
  else if (del->right && key == del->right->key)
  {
    free(del->right);
    del->right = 0;

    return true;
  }
  else

    return false;
}

bool redAsP(rbtree *t, node_t *delgp, node_t *delp, node_t *sib)
{
  //형제색이 red일때 회전하는 경우기 때문에 아니면 그냥 나감
  if (sib == 0 || sib->color == RBTREE_BLACK)
    return false;
  rbtree_rotate(t, delgp, sib->key);
  sib->color = RBTREE_BLACK;
  delp->color = RBTREE_RED;
  return true;
}
key_t swapKey(node_t *del)
{
  node_t *target;
  target = del->right;
  while (target->left)
    target = target->left;
  del->key = target->key;
  return target->key;
}

//형제에게서 빨간색을 빌려옴// del 검정, 형제 자식이 빨간색이 있어야됨
bool borrowKey(rbtree *t, node_t *delgp, node_t *delp, node_t *del, node_t *sib)
{
  node_t *sibrc;
  //2 노드라면 자식이 검정색이니 함수를 나간다
  if (is2node(sib) == true)
    return false;
  //del값이 형제 보다 클때 형제는 왼편에 위치 하게됨
  if (del->key > sib->key)
  { //위의 조건에 따라 형제의 왼쪽 자식이 존재 하면 형제의 자식을 왼편으로
    if (sib->left && sib->left->color == RBTREE_RED)
      sibrc = sib->left;
    else
      sibrc = sib->right;
  }
  //위와 반대
  else
  {
    if (sib->right && sib->right->color == RBTREE_RED)
      sibrc = sib->right;
    else
      sibrc = sib->left;
  }
  //두번 회전 조건을 보는 경우임 각 경우를 볼때 다르게 된다면 노드가 한번 꺽여서 방향이 나오게 되어있다.
  if ((delp->key > sib->key) != (sib->key > sibrc->key))
  {
    rbtree_rotate(t, delp, sibrc->key);
    rbtree_rotate(t, delgp, sibrc->key);
    sib->color = RBTREE_BLACK;
    sibrc->color = RBTREE_RED;
  }
  //한번 회전
  else
  {
    rbtree_rotate(t, delp, sibrc->key);
    sib->color = RBTREE_RED;
    sibrc->color = RBTREE_BLACK;
  }
  //회전 후 black 노드 깊이를 유지하기 위해 색변환
  delp->color = RBTREE_BLACK;
  del->color = RBTREE_RED;

  if (idea->color == RBTREE_RED)
    idea->color = RBTREE_BLACK;

  return true;
}
// 삭제시 색을 밑으로 내려 자식들이 red가 됨
void bindNode(node_t *delp)
{
  delp->color = RBTREE_BLACK;
  delp->left->color = RBTREE_RED;
  delp->right->color = RBTREE_RED;
}

//노드를 삭제하는 과정
int rbtree_erase(rbtree *t, node_t *p)
{
  // TODO: implement erase
  node_t *delgp, *delp, *del, *sib;
  key_t value = p->key;
  //처음 부모들은 더미를 가리키고 있다
  delgp = delp = (node_t *)idea;
  //왼편 부터 순회 시작
  del = idea->left;
  sib = 0;

  //리프 노드라면 반복문 들어갈 필요 없음
  //아니라면 게속 순회함
  while (isLeaf(del) == false)
  {
    if (del->color != RBTREE_RED) //지우고자 하는 색상이 red가 아니라면
    {                             //위로 red 가져옴
      if (redAsP(t, delgp, delp, sib) == true)
      {
        //기존의 gp 자리에 형제가 올라 오게됨
        delgp = sib;
        //위치에 따라 형제도 정해짐
        if (del->key > delp->key || del->key == delp->key)
          sib = delp->left;
        else
          sib = delp->right;
      }
    }

    //트리의 처음이 아니고, 2노드 일때
    if (del != idea->left && is2node(del) == true)
    { //각 조건을 확인해 본다 형제의 자식이 조건에 따라 회전 가능하면 실행하고 그게 아니면 색을 내려줌
      if (borrowKey(t, delgp, delp, del, sib) == false)
        bindNode(delp);
    }
    // 내부 노드중 값을 찾는다면 그 값보다 큰 값중 작은 값을 찾아 바꿔줌
    if (value == del->key)
      value = swapKey(del);

    //내려옴
    delgp = delp;
    delp = del;
    //자식과 del도 키값에 따라 정해줌
    if (value > del->key || value == del->key)
    {
      sib = del->left;
      del = del->right;
    }
    else
    {
      sib = del->right;
      del = del->left;
    }
  }
  //del이 리프 노드 인것을 찾고 반복문을 나와 한번더 확인 절차를 거친다
  if (del->color != RBTREE_RED)
  {
    if (redAsP(t, delgp, delp, sib) == true)
    {
      delgp = sib;
      if (del->key > delp->key || del->key == delp->key)
        sib = delp->left;
      else
        sib = delp->right;
    }
  }
  if (del != idea->left && is2node(del) == true)
  {
    if (borrowKey(t, delgp, delp, del, sib) == false)
      bindNode(delp);
  }
  //리프라면 조건에 맞게 처리해줌
  if (delLeaf(t, value, delp, del) == true)
  {
    //위와 같이 현재 루트 주소를 더미 왼편 주소를 가리키게함
    t->root = idea->left;
    t->m_cnt--;
    return true;
  }
  else
  {
    t->root = idea->left;
    return false;
  }
}
//비교함수
static int comp(const void *p1, const void *p2)
{
  const key_t *e1 = (const key_t *)p1;
  const key_t *e2 = (const key_t *)p2;
  if (*e1 < *e2)
  {
    return -1;
  }
  else if (*e1 > *e2)
  {
    return 1;
  }
  else
  {
    return 0;
  }
};

//트리를 배열로 변환 하는 함수
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n)
{
  // TODO: implement to_array

  int i = 0;
  add_to_array(idea->left, arr, i);

  qsort((void *)arr, n, sizeof(key_t), comp);
  return 0;
}
//재귀 적으로 순회하며 배열에 값을 넣어줌
int add_to_array(node_t *node, key_t *arr, int i)
{
  if (node == NULL)
    return i;
  arr[i] = node->key;
  i++;
  if (node->left != NULL)
    i = add_to_array(node->left, arr, i);
  if (node->right != NULL)
    i = add_to_array(node->right, arr, i);

  return i;
}

