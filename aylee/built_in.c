/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   built_in.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 14:46:49 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 11:48:47 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	builtin_echo(t_data *data, char **args)
{
	int	i;
	int	newline;

	(void)data;
	newline = 1;
	i = 0;
	if (args && ft_strncmp(args[0], "-n", 3) == 0)
	{
		newline = 0;
		i++;
	}
	while (args && args[i])
	{
		if (ft_strncmp(args[i], "$?", 3) == 0)
			printf("%d", data->exit_status);
		else
			printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
		i++;
	}
	if (newline)
		printf("\n");
	return (0);
}

int	builtin_cd(t_data *data, char **args)
{
	const char	*path;
	t_env		*home_node;

	if (!args || args[0] == NULL)
	{
		home_node = find_env_node(data->env, "HOME");
		if (home_node)
			path = home_node->value;
		else
		{
			print_error_msg(data, "cd", "HOME not set", 1);
			return (1);
		}
	}
	else
		path = args[0];
	if (chdir(path) != 0)
	{
		print_error_msg(data, "cd", strerror(errno), 1);
		return (1);
	}
	return (0);
}

int	builtin_pwd(t_data *data)
{
	char	cwd[1024];

	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		printf("%s\n", cwd);
		return (0);
	}
	print_error_msg(data, "pwd", strerror(errno), 1);
	return (1);
}

static int	is_valid_identifier(const char *str)
{
	if (!str || !*str)
		return (0);
	if (!ft_isalpha(str[0]) && str[0] != '_')
		return (0);
	while (*str && *str != '=')
	{
		if (!ft_isalnum(*str) && *str != '_')
			return (0);
		str++;
	}
	return (1);
}

void	default_export(t_data *data)
{
	t_env	*current;

	current = data->env;
	while (current)
	{
		if (current->value)
			printf("declare -x %s=\"%s\"\n", current->key, current->value);
		else
			printf("declare -x %s\n", current->key);
		current = current->next;
	}
}

void	make_env_new(t_data *data, char **args, char *equal_sign, int i)
{
	int		key_len;
	char	*key;
	char	*value;

	key_len = equal_sign - args[i];
	key = (char *)malloc(key_len + 1);
	if (!key)
		return ;
	ft_strlcpy(key, args[i], key_len + 1);
	value = equal_sign + 1;
	set_env_var(data, key, value);
	free(key);
}

int	builtin_export(t_data *data, char **args)
{
	char	*equal_sign;
	int		i;

	i = 0;
	if (!args || args[0] == NULL)
		return (default_export(data), 0);
	while (args[i])
	{
		if (!is_valid_identifier(args[i]))
		{
			printf("minishell: export: `%s'", args[i++]);
			printf(": not a valid identifier\n");
			data->exit_status = 1;
			continue ;
		}
		equal_sign = ft_strchr(args[i], '=');
		if (equal_sign)
			make_env_new(data, args, equal_sign, i);
		else if (!find_env_node(data->env, args[i]))
			add_env_node(&data->env, args[i], NULL);
		i++;
	}
	return (0);
}

int	set_env_var(t_data *data, const char *key, const char *value)
{
	t_env	*node;

	node = find_env_node(data->env, key);
	if (node)
	{
		if (node->value)
			free(node->value);
		node->value = ft_strdup(value);
	}
	else
		add_env_node(&data->env, key, value);
	return (0);
}

int	builtin_unset(t_data *data, char **args)
{
	int	i;

	if (!args || args[0] == NULL)
		return (0);
	i = 0;
	while (args[i])
	{
		delete_env(&data->env, args[i]);
		i++;
	}
	return (0);
}

static int	is_valid_num(const char *str)
{
	int	i;

	if (!str || !*str)
		return (0);
	i = 0;
	if (str[0] == '-' || str[0] == '+')
		i++;
	while (str[i])
	{
		if (!ft_isdigit(str[i]))
			return (0);
		i++;
	}
	return (1);
}

int	builtin_exit(t_data *data, char **args)
{
	int	status;

	printf("exit\n");
	if (!args || !args[0])
	{
		status = data->exit_status;
		clean_up(data);
		exit(status);
	}
	if (!is_valid_num(args[0]))
	{
		print_error_msg(data, "exit", "numeric argument required", 2);
		clean_up(data);
		exit(2);
	}
	if (args[1])
	{
		print_error_msg(data, "exit", "too many arguments", 1);
		return (1);
	}
	status = ft_atoi(args[0]);
	clean_up(data);
	exit(status);
}

int	builtin_env(t_data *data)
{
	print_env_list(data->env);
	return (0);
}

int	execute_builtin(t_data *data, t_cmd *cmd)
{
	if (ft_strncmp(cmd->cmd, "echo", 5) == 0)
		return (builtin_echo(data, cmd->argv));
	else if (ft_strncmp(cmd->cmd, "cd", 3) == 0)
		return (builtin_cd(data, cmd->argv));
	else if (ft_strncmp(cmd->cmd, "pwd", 4) == 0)
		return (builtin_pwd(data));
	else if (ft_strncmp(cmd->cmd, "export", 7) == 0)
		return (builtin_export(data, cmd->argv));
	else if (ft_strncmp(cmd->cmd, "unset", 6) == 0)
		return (builtin_unset(data, cmd->argv));
	else if (ft_strncmp(cmd->cmd, "exit", 5) == 0)
		return (builtin_exit(data, cmd->argv));
	else if (ft_strncmp(cmd->cmd, "env", 4) == 0)
		return (builtin_env(data));
	return (-1);
}
