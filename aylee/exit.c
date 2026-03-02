/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exit.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 19:01:21 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 19:02:42 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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