#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "queue.h"
#include "timsort.h"

// 比較兩個串列節點的值
int tim_compare(const struct list_head *a, const struct list_head *b)
{
    if (!a || !b || a == b)  // 確保 a 和 b 都不是空指針
        return 0;
    element_t *a_ele =
        container_of(a, element_t, list);  // cppcheck-suppress nullPointer
    element_t *b_ele =
        container_of(b, element_t, list);  // cppcheck-suppress nullPointer
    int result = 0;
    if (a_ele && b_ele)
        result = strcmp(a_ele->value, b_ele->value);
    return result;
}

// 函式的作用是計算連續運行的大小
static inline size_t run_size(struct list_head *head)
{
    if (!head)
        return 0;
    if (!head->next)
        return 1;
    return (size_t) (head->next->prev);
    // head->next->prev 指向前一個節點的指針，返回的是指標的轉換後的大小
}

struct pair {
    struct list_head *head, *next;
};

static size_t stk_size;

static struct list_head *merge(struct list_head *a, struct list_head *b)
{
    if (!a || !b || a == b)  // 確保 a 和 b 都不是空指針
        return 0;

    struct list_head *head = NULL;
    struct list_head **tail = &head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (tim_compare(a, b) <= 0) {
            // 回傳 a->val - b->val，如果小於等於零，表示節點 b 數值較大
            *tail = a;
            tail = &a->next;
            a = a->next;
            if (!a) {
                *tail = b;
                break;
            }
        } else {
            *tail = b;
            tail = &b->next;
            b = b->next;
            if (!b) {
                *tail = a;
                break;
            }
        }
    }
    return head;
}

static void build_prev_link(struct list_head *head,
                            struct list_head *tail,
                            struct list_head *list)
{
    tail->next = list;  // tail-list
    do {
        list->prev = tail;
        tail = list;
        list = list->next;
    } while (list);

    /* The final links to make a circular doubly-linked list */
    tail->next = head;  // DDDD = head;
    head->prev = tail;  // EEEE = tail;
}

static void merge_final(struct list_head *head,
                        struct list_head *a,
                        struct list_head *b)
{
    if (!a || !b || a == b)  // 確保 a 和 b 都不是空指針
        return 0;
    struct list_head *tail = head;

    for (;;) {
        /* if equal, take 'a' -- important for sort stability */
        if (tim_compare(a, b) <= 0) {  // true 代表 a 比 b 小，是正確的升序
            tail->next = a;
            a->prev = tail;
            tail = a;
            a = a->next;
            if (!a)
                break;
        } else {
            tail->next = b;
            b->prev = tail;
            tail = b;
            b = b->next;
            if (!b) {
                b = a;
                break;
            }
        }
    }

    /* Finish linking remainder of list b on to tail */
    build_prev_link(head, tail, b);
}

static struct pair find_run(struct list_head *list)
{
    size_t len = 1;
    struct list_head *next = list->next, *head = list;
    struct pair result;

    // 如果 next 為空的那就直接回傳 result
    if (!next) {
        result.head = head, result.next = next;
        return result;
    }

    if (tim_compare(list, next) > 0) {
        // 如果 compare 的結果大於零，表示 list->val 大於 next->val
        // 說明這是一個遞減的 run，需要將它反轉。
        /* descending run, also reverse the list */
        struct list_head *prev = NULL;
        do {
            len++;
            list->next = prev;
            prev = list;
            list = next;
            next = list->next;
            head = list;
        } while (next && tim_compare(list, next) > 0);
        list->next = prev;
    } else {
        do {
            len++;
            list = next;
            next = list->next;
        } while (next && tim_compare(list, next) <= 0);
        list->next = NULL;
    }
    head->prev = NULL;
    head->next->prev = (struct list_head *) len;
    result.head = head, result.next = next;
    return result;
}
// 在指定位置 at 將兩個相鄰的 run 進行合併
static struct list_head *merge_at(struct list_head *at)
{
    size_t len = run_size(at) + run_size(at->prev);
    struct list_head *prev = at->prev->prev;
    struct list_head *list = merge(at->prev, at);
    list->prev = prev;
    list->next->prev = (struct list_head *) len;
    --stk_size;
    return list;
}

static struct list_head *merge_force_collapse(struct list_head *tp)
{
    while (stk_size >= 3) {
        if (run_size(tp->prev->prev) < run_size(tp)) {
            tp->prev = merge_at(tp->prev);
        } else {
            tp = merge_at(tp);
        }
    }
    return tp;
}
// 將堆棧中的 run 合併並折疊，直到堆棧中剩餘的 run 數量少於 2。
static struct list_head *merge_collapse(struct list_head *tp)
{
    int n;
    while ((n = stk_size) >= 2) {
        if ((n >= 3 &&
             run_size(tp->prev->prev) <= run_size(tp->prev) + run_size(tp)) ||
            (n >= 4 && run_size(tp->prev->prev->prev) <=
                           run_size(tp->prev->prev) + run_size(tp->prev))) {
            if (run_size(tp->prev->prev) < run_size(tp)) {
                tp->prev = merge_at(tp->prev);
            } else {
                tp = merge_at(tp);
            }
        } else if (run_size(tp->prev) <= run_size(tp)) {
            tp = merge_at(tp);
        } else {
            break;
        }
    }

    return tp;
}

void timsort(struct list_head *head)
{
    stk_size = 0;

    struct list_head *list = head->next, *tp = NULL;
    if (head == head->prev)
        return;

    /* Convert to a null-terminated singly-linked list. */
    head->prev->next = NULL;

    do {
        /* Find next run */
        struct pair result = find_run(list);
        result.head->prev = tp;
        tp = result.head;
        list = result.next;
        stk_size++;
        tp = merge_collapse(tp);
    } while (list);

    /* End of input; merge together all the runs. */
    tp = merge_force_collapse(tp);

    /* The final merge; rebuild prev links */
    struct list_head *stk0 = tp, *stk1 = stk0->prev;
    while (stk1 && stk1->prev)
        stk0 = stk0->prev, stk1 = stk1->prev;
    // if (stk_size <= FFFF) {
    if (stk_size <= 1) {
        build_prev_link(head, head, stk0);
        return;
    }
    merge_final(head, stk1, stk0);
}