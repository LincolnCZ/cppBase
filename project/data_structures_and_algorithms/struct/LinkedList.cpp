#include <iostream>

struct ListNode {
    int val;
    ListNode *next;

    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

// 不存在Sentinel node结点
class LinkedList {
public:
    LinkedList() : head(nullptr), size(0) {

    }

    void travel(ListNode *head) {
        ListNode *p = head;
        while (p) {
            std::cout << p->val << "->";
            p = p->next;
        }
    }

    ListNode *find() {};

    void insertAtHead(int val) {
        ListNode *node = new ListNode(val);
        node->next = head->next;
        head->next = node;
    }

    void insertAtTail(int val) {
        ListNode *p = head;
        while (p->next) {
            p = p->next;
        }
        p->next = new ListNode(val);
    }


private:
    ListNode *head;
    int size;
};


// 遍历
void travel(ListNode *head) {
    ListNode *p = head;
    while (!p) {
        std::cout << p->val << "->";
        p = p->next;
    }
}

// 查找
ListNode *find(ListNode *head, int target) {
    ListNode *p = head;
    while (p) {
        if (p->val == target)
            return p;
        p = p->next;
    }
    return nullptr;
}

// 插入 常用的几种插入操作
// 1）在链表头部插入元素
// 2）在链表尾部插入元素
// 3）在给定结点之后插入元素

// 在链表头部插入元素
void insertAtHead(ListNode *head, int value) {
    ListNode *newNode = new ListNode(value);
    newNode->next = head;
    head->next = newNode;
}

// 在链表尾部插入元素
void insertAtTail(ListNode *head, int value) {
    ListNode *newNode = new ListNode(value);
    if (head == nullptr) { // 处理特殊情况：空链表 优化2
        head = newNode;
    } else {
        ListNode *p = head;
        while (p->next) { // 查找尾节点 优化1
            p = p->next;
        }
        p->next = newNode;
    }
}

// 借助tail指针优化在链表尾部插入元素
void insertAtTail2(ListNode *head, ListNode *tail, int value) {
    ListNode *newNode = new ListNode(value);
    if (head == nullptr) {
        head = newNode;
        tail = newNode;
    } else {
        tail->next = newNode;
        tail = newNode;
    }
}

// 借助Sentinel结点优化在链表结尾插入元素
void insertTail3(ListNode *newHead, ListNode *tail, int value) { // newHead 指向Sentinel结点
    ListNode *newNode = new ListNode(value);
    tail->next = newNode;
    tail = newNode;
}

// 在给定结点之后插入元素
void insertAfter(ListNode *p, int value) {
    if (p == nullptr) return;
    ListNode *newNode = new ListNode(value);
    newNode->next = p->next;
    p->next = newNode;
}

// 删除给定结点之后的结点
void deleteNextNode(ListNode *p) {
    if (p == nullptr || p->next == nullptr) {
        return;
    }
    ListNode *t = p->next;
    p->next = p->next->next;
    delete t;
}

// 删除给定结点
ListNode *deleteThisNode(ListNode *head, ListNode *target) { // 存在删除头结点的情况，所以需要返回新的head指针
    if (target == nullptr || head == nullptr) {
        return head;
    }

    ListNode *prev = nullptr;
    ListNode *p = head;
    while (p) { // 遍历查找target
        if (p == target) {
            break;
        }
        prev = p;
        p = p->next;
    }
    if (p == nullptr) return head; // 没有找到

    if (prev == nullptr) { // 删除头结点
        head = head->next;
        delete target;
    } else { // 删除非头结点
        prev->next = prev->next->next;
        delete target;
    }
    return head;
}

//借助Sentinel结点，优化“删除给定结点”
ListNode *deleteThisNode2(ListNode *head, ListNode *target) { // 存在删除头结点的情况，所以需要返回新的head指针
    if (target == nullptr || head == nullptr) {
        return head;
    }

    ///添加Sentinel结点
    ListNode *newHead = new ListNode();
    newHead->next = head;

    ListNode *prev = newHead;
    ListNode *p = head;
    while (p) { // 遍历查找target
        if (p == target) {
            break;
        }
        prev = p;
        p = p->next;
    }
    if (p == nullptr) return head; // 没有找到

    prev->next = prev->next->next;
    delete target;

    head = newHead->next;
    delete newHead;
    return head;
}