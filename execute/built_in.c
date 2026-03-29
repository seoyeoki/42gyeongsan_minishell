/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   built_in.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/06 14:46:49 by aylee             #+#    #+#             */
/*   Updated: 2026/03/29 17:37:39 by aylee            ###   ########.fr       */
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
	while (args && args[i] && is_n_flag(args[i]))
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

int	builtin_env(t_data *data, char **args)
{
	if (args && args[0] != NULL)
	{
		print_error_msg(data, "env", "too many arguments", 1);
		return (1);
	}
	print_env_list(data->env);
	return (0);
}
