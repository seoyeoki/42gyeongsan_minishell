/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 13:27:25 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 15:21:58 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	wait_child(t_data *data, pid_t pid)
{
	int	status;

	waitpid(pid, &status, 0);
	if (WIFEXITED(status))
		data->exit_status = WEXITSTATUS(status);
	else if (WIFSIGNALED(status))
		data->exit_status = 128 + WTERMSIG(status);
	return (data->exit_status);
}

int	fork_and_exec(t_data *data, t_cmd *cmd, char *cmd_path)
{
	char	**envp;
	char	**args;
	pid_t	pid;

	envp = env_to_array(data->env);
	args = get_execve_args(cmd);
	pid = fork();
	if (pid == -1)
	{
		print_error(data, "fork", errno, 1);
		free_split(envp);
		free_split(args);
		return (free(cmd_path), 1);
	}
	if (pid == 0)
	{
		execve(cmd_path, args, envp);
		print_error(data, cmd->cmd, errno, 126);
		free(cmd_path);
		exit(126);
	}
	free(cmd_path);
	free_split(envp);
	free_split(args);
	return (wait_child(data, pid));
}

int	execute_command(t_data *data, t_cmd *cmd)
{
	char	*cmd_path;

	if (!cmd || !cmd->cmd)
		return (0);
	if (is_builtin(cmd->cmd))
		return (execute_builtin(data, cmd));
	cmd_path = find_command_path(cmd->cmd, data->env);
	if (!cmd_path)
	{
		print_error_msg(data, cmd->cmd, "command not found", 127);
		return (127);
	}
	return (fork_and_exec(data, cmd, cmd_path));
}
