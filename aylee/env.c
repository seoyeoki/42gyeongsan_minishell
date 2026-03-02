/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 14:46:53 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 15:33:39 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

t_env	*create_env_node(const char *key, const char *value)
{
	t_env	*new_node;

	new_node = (t_env *)malloc(sizeof(t_env));
	if (!new_node)
		return (NULL);
	new_node->key = ft_strdup(key);
	if (value != NULL)
		new_node->value = ft_strdup(value);
	else
		new_node->value = NULL;
	new_node->next = NULL;
	return (new_node);
}

void	free_env_node(t_env *node)
{
	if (node)
	{
		free(node->key);
		if (node->value)
			free(node->value);
		free(node);
	}
}

t_env	*find_env_node(t_env *head, const char *key)
{
	t_env	*current;

	current = head;
	while (current)
	{
		if (ft_strncmp(current->key, key, ft_strlen(key) + 1) == 0)
			return (current);
		current = current->next;
	}
	return (NULL);
}

t_env	*add_env_node(t_env **head, const char *key, const char *value)
{
	t_env	*new_node;
	t_env	*current;

	new_node = create_env_node(key, value);
	if (!new_node)
		return (NULL);
	if (*head == NULL)
	{
		*head = new_node;
		return (new_node);
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_node;
	return (new_node);
}

int	sub_parse_env(char **envp, char *equal_sign, t_env **env_list, int i)
{
	char	*key;
	char	*value;
	int		key_len;

	key_len = equal_sign - envp[i];
	key = (char *)malloc(key_len + 1);
	if (!key)
		return (1);
	ft_strlcpy(key, envp[i], key_len + 1);
	value = ft_strdup(equal_sign + 1);
	add_env_node(env_list, key, value);
	free(key);
	free(value);
	return (0);
}

t_env	*parse_env(char **envp)
{
	t_env	*env_list;
	int		i;
	char	*equal_sign;

	i = 0;
	env_list = NULL;
	while (envp[i])
	{
		equal_sign = ft_strchr(envp[i], '=');
		if (equal_sign)
		{
			if (sub_parse_env(envp, equal_sign, &env_list, i))
				return (env_list);
		}
		i++;
	}
	return (env_list);
}

void	free_env_list(t_env *head)
{
	t_env	*current;
	t_env	*next_node;

	current = head;
	while (current)
	{
		next_node = current->next;
		free_env_node(current);
		current = next_node;
	}
}

void	delete_env(t_env **head, char *key)
{
	t_env	*current;
	t_env	*prev;

	current = *head;
	prev = NULL;
	while (current)
	{
		if (ft_strncmp(current->key, key, ft_strlen(key) + 1) == 0)
		{
			if (prev)
				prev->next = current->next;
			else
				*head = current->next;
			free_env_node(current);
			return ;
		}
		prev = current;
		current = current->next;
	}
}

void	print_env_list(t_env *head)
{
	t_env	*current;

	current = head;
	while (current)
	{
		if (current->value)
		{
			ft_putstr_fd(current->key, 1);
			ft_putchar_fd('=', 1);
			ft_putendl_fd(current->value, 1);
		}
		current = current->next;
	}
}
