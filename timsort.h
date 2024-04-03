/* Forward declaration of the struct list_head */
struct list_head;

/* Definition of the function pointer type for comparing list elements */
// 定義一個名為 list_cmp_func_t
// 的函式指標類型，該函式指標類型可以指向一個具有特定格式的函式。
typedef int (*list_cmp_func_t)(void *,
                               const struct list_head *,
                               const struct list_head *);

/*
 * timsort() - Perform TimSort on a linked list
 * @head: Pointer to the head of the linked list
 *
 * This function performs TimSort on the linked list specified by @head. TimSort
 * is a sorting algorithm derived from merge sort and insertion sort, designed
 * to perform well on real-world data and partially sorted arrays. It operates
 * by dividing the list into small sublists, sorting them individually using
 * insertion sort, and then merging them using merge sort.
 *
 * The comparison function @cmp is called to determine the relative order of
 * elements in the list. It should return a negative value if the first element
 * is less than the second, a positive value if the first element is greater
 * than the second, and zero if the elements are equal. The function pointer
 * may also take an additional parameter @priv, which can be used to pass
 * additional context or data to the comparison function.
 */
void timsort(struct list_head *head);

void tra_dblist(struct list_head *head, char *remark);