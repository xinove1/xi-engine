#ifndef XI_LIST_H_
# define XI_LIST_H_
# include <stdio.h>
# include "arenas.h"

// typedef struct List
// {
// 	void		*content;
// 	struct List	*next;
// } List;

// List
List	*lstnew(Arena *arena, void *content);
void	lstadd_front(List **lst, List *new_list);
int		lstsize(List *lst);
List	*lstlast(List *lst);
void	lstadd_back(List **list, List *new_list);
void	lstdelone(List *lst, void (*del)(void*));
void	lstclear(List **lst, void (*del)(void*));
void	lstiter(List *lst, void (*f)(void *));
List	*lstmap(Arena *arena, List *lst, void *(*f)(void *), void (*del)(void *));

#endif

#ifdef XI_LIST_IMPLEMENTATION
// NOLINTBEGIN(misc-definitions-in-headers)

List	*lstnew(Arena *arena, void *content)
{
	List	*new;

	if (!arena) {
		new = calloc(1, sizeof(List));
	} else if (arena->chunk_sz) {
		new = pool_alloc(arena);
	} else {
		new = linear_alloc(arena, sizeof(List));
	}
	if (!new) {
		return (NULL);
	}
	new->content = content;
	new->next = NULL;
	return (new);
}

void	lstadd_back(List **list, List *new)
{
	List	*i;

	if (*list == NULL) {
		*list = new;
	} else {
		assert(*list);
		i = lstlast(*list);
		i->next = new;
	}
}

void	lstadd_front(List **lst, List *new)
{
	new->next = *lst;
	*lst = new;
}

void	lstiter(List *lst, void (*f)(void *))
{
	List	*i;

	i = lst;
	while (i) {
		f(i->content);
		i = i->next;
	}
}

List	*lstlast(List *lst)
{
	List	*i;

	if (!lst) {
		return (NULL);
	}
	i = lst;
	while (i->next) {
		i = i->next;
	}
	return (i);
}

int	lstsize(List *lst)
{
	int		i;
	List	*j;

	i = 0;
	j = lst;
	while (j) {
		j = j->next;
		i++;
	}
	return (i);
}

void	lstclear(List **lst, void (*del)(void*))
{
	List	*tmp;

	if (!lst) {
		return ;
	}
	while (*lst) {
		tmp = (*lst)->next;
		if (del != NULL) // NOTE modification from original
			lstdelone(*lst, del);
		*lst = tmp;
	}
	lst = NULL;
}

void	lstdelone(List *lst, void (*del)(void*))
{
	del(lst->content);
	free(lst);
}

List	*lstmap(Arena *arena, List *lst, void *(*f)(void *), void (*del)(void *))
{
	List	*i;
	List	*newlst;
	List	*tmp;

	if (!lst) {
		return (NULL);
	}
	newlst = lstnew(arena, f(lst->content));
	i = lst->next;
	while (i) {
		tmp = lstnew(arena, f(i->content));
		if (!tmp)
		{
			lstclear(&newlst, del);
			return (NULL);
		}
		lstadd_back(&newlst, tmp);
		i = i->next;
	}
	return (newlst);
}
// NOLINTEND(misc-definitions-in-headers)
#endif
