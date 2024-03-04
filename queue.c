#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */


/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *new = malloc(sizeof(struct list_head));
    if (!new) {
        free(new);
        return NULL;
    }
    INIT_LIST_HEAD(new);
    return new;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head) {
        return;
    }
    element_t *entry, *safe;
    list_for_each_entry_safe (entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    element_t *node = malloc(sizeof(element_t));
    if (!node) {
        return false;
    }
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head) {
        return false;
    }
    return q_insert_head(head->prev, s);
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *first = list_first_entry(head, element_t, list);
    list_del(&first->list);
    if (sp) {
        for (char *i = first->value; bufsize > 1 && *i; sp++, i++, bufsize--)
            *sp = *i;
        *sp = '\0';
    }
    return first;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    return q_remove_head(head->prev->prev, sp, bufsize);
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;

    int len = 0;
    struct list_head *li;

    list_for_each (li, head)
        len++;
    return len;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return false;
    }
    struct list_head **indir = &head;
    int mid = q_size(head) % 2 == 0 ? q_size(head) / 2 : q_size(head) / 2 + 1;
    for (int i = 0; i < mid; i++) {
        indir = &(*indir)->next;
    }
    element_t *del = list_entry(*indir, element_t, list);
    list_del(*indir);
    q_release_element(del);
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head))
        return;
    struct list_head *node;
    list_for_each (node, head) {
        if (node->next == head) {
            break;
        }
        list_move(node, node->next);
    }
    // https://leetcode.com/problems/swap-nodes-in-pairs/
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head || list_empty(head)) {
        return;
    }
    struct list_head *node, *safe;
    list_for_each_safe (node, safe, head)
        list_move(node, head);
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    // https://leetcode.com/problems/reverse-nodes-in-k-group/
}

void q_merge_two(struct list_head *list1, struct list_head *list2)
{
    if (!list1 || !list2)
        return;
    struct list_head tmp_head;
    INIT_LIST_HEAD(&tmp_head);
    while (!list_empty(list1) && !list_empty(list2)) {
        element_t *list1_element = list_first_entry(list1, element_t, list);
        element_t *list2_element = list_first_entry(list2, element_t, list);
        char *list1_str = list1_element->value,
             *list2_str = list2_element->value;
        element_t *mini =
            (strcmp(list1_str, list2_str) < 0) ? list1_element : list2_element;
        list_move_tail(&mini->list, &tmp_head);
    }
    list_splice_tail_init(list1, &tmp_head);
    list_splice_tail_init(list2, &tmp_head);
    list_splice(&tmp_head, list1);
}

void merge_sort(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *slow = head, *fast = head->next;
    for (; fast != head && fast->next != head; fast = fast->next->next)
        slow = slow->next;
    struct list_head left;
    list_cut_position(&left, head, slow);
    merge_sort(&left);
    merge_sort(head);
    q_merge_two(head, &left);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    merge_sort(head);
    if (descend) {
        q_reverse(head);
    }
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    struct list_head *tmp = head->prev;
    while (tmp->prev != head) {
        element_t *prev_ele = container_of(tmp->prev, element_t, list);
        element_t *tmp_ele = container_of(tmp, element_t, list);
        if (strcmp(tmp_ele->value, prev_ele->value) < 0) {
            list_del(&prev_ele->list);
            q_release_element(prev_ele);
        } else
            tmp = tmp->prev;
    }
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    struct list_head *tmp = head->prev;
    while (tmp->prev != head) {
        element_t *prev_ele = container_of(tmp->prev, element_t, list);
        element_t *tmp_ele = container_of(tmp, element_t, list);
        if (strcmp(tmp_ele->value, prev_ele->value) > 0) {  // tmp > prev
            list_del(&prev_ele->list);
            q_release_element(prev_ele);
        } else
            tmp = tmp->prev;
    }
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    // https://leetcode.com/problems/merge-k-sorted-lists/
    return 0;
}