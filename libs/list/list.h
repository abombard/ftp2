#ifndef LIST_H
# define LIST_H

# include <stddef.h>

/*
** Define the offset of the member m of type t.
** @param t: type		the type of the structure
** @param m: member		the member to get the offset
*/

# undef OFST_OF
# define OFST_OF(t, m) ((size_t) &((t *)0)->m)

/*
** Cast a member of a structure of type t pointed by ptr and named m out to the
** containing structure.
** @param p: ptr		the pointer to the member
** @param t: type		the type of the container structure
** @param m: member		the name of the member
*/

# undef CONTAINER_OF
# undef TYPE_M
# define TYPE_M(t,m) const typeof(((t*)0)->m)
# define CONTAINER_OF(p,t,m)({TYPE_M(t,m)*mp=(p);(t*)((char*)mp-OFST_OF(t,m));})

/*
** Double linked list standard implementation.
** TODO rename the t_list_* functions
*/

typedef struct s_list	t_list;

struct		s_list
{
	t_list	*prev;
	t_list	*next;
};

void		t_list_add(t_list *new, t_list *prev, t_list *next);

void		t_list_del(t_list *prev, t_list *next);
void		t_list_del_init(t_list *entry);

void		t_list_splice(t_list *list, t_list *head);

/*
** Initialize list.
** TODO ask comments and use-cases
*/

# define LIST_HEAD_INIT(name) { &(name), &(name) }

# define LIST_HEAD(name) t_list	name = LIST_HEAD_INIT(name)

# define INIT_LIST_HEAD(ptr) (ptr)->next = (ptr); (ptr)->prev = (ptr);

/*
** Add the node new at the begining of the list pointed by head.
*/
void		list_add(t_list *new, t_list *head);

/*
** Add the node new at the end of the list pointed by head.
*/
void		list_add_tail(t_list *new, t_list *head);

/*
** Delete the list pointed by entry.
*/
void		list_del(t_list *entry);

/*
** Move the element pointed by list to head.
*/

void		list_move(t_list *list, t_list *head);
void		list_move_tail(t_list *list, t_list *head);

/*
** Check if the list pointed by head is empty.
*/
int			list_is_empty(t_list *head);

/*
** Get the length of the list pointed by head.
*/
size_t		list_size(t_list *head);

/*
** Concatenate the list pointed by list at the begining of the list head.
*/
void		list_splice(t_list *list, t_list *head);

/*
** Get the nth node of the list pointed by head.
*/
t_list		*list_nth(const t_list *head, const int index);
t_list		*list_nth_safe(const t_list *head, const int index);

/*
** list slice: get a part of a list
*/
t_list		*list_slice(t_list *new, t_list *head, const int index, int len);

/*
** list insert
*/
void		list_insert(t_list *new, t_list *head, unsigned int index);

/*
** list foreach: iterate through element of a list
*/
# define LIST_FOREACH(h, p) p=(t_list*)h;while((p=p->next)&&p!=h)

# define LIST_FOREACH_PREV(head,pos) pos=head;while((pos=pos->prev)&&pos!=head)

#endif
