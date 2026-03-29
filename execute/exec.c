/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 00:00:00 by aylee             #+#    #+#             */
/*   Updated: 2026/03/29 17:37:41 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

static void	clean_child(t_data *data, t_cmd *head, t_pipes *pipeline)
{
	rl_clear_history();
	free_cmd_list(head);
	free_pipeline(pipeline);
	free_env_list(data->env);
}

static void	handle_exec_error(t_data *data, char *p, char **a, char **e)
{
	if (a && a[0])
		print_error(data, a[0], errno, 126);
	else
		print_error(data, "execve", errno, 126);
	free(p);
	free_split(a);
	free_split(e);
	exit(126);
}

int	set_fd_open(t_redir *redir)
{
	int	fd;

	fd = -1;
	if (redir->type == REDIR_IN)
		fd = open(redir->file, O_RDONLY);
	else if (redir->type == REDIR_OUT)
		fd = open(redir->file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (redir->type == REDIR_APPEND)
		fd = open(redir->file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	return (fd);
}

int	apply_redir(t_data *data, t_redir *redir)
{
	int	fd;

	while (redir)
	{
		fd = set_fd_open(redir);
		if (redir->type == REDIR_HEREDOC)
		{
			dup2(redir->fd, STDIN_FILENO);
			close(redir->fd);
			redir->fd = -1;
			redir = redir->next;
			continue ;
		}
		if (fd == -1)
			return (print_error(data, redir->file, errno, 1), -1);
		if (redir->type == REDIR_IN)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		close(fd);
		redir = redir->next;
	}
	return (0);
}

int	prepare_child(t_data *data, t_cmd *cmd, t_pipes *pipeline, int i)
{
	if (i > 0)
		dup2(pipeline->pipes[i - 1][0], STDIN_FILENO);
	if (i < pipeline->count - 1)
		dup2(pipeline->pipes[i][1], STDOUT_FILENO);
	close_all_pipes(pipeline->pipes, pipeline->count - 1);
	if (prepare_heredoc(data, cmd) == -1)
		return (-1);
	if (apply_redir(data, cmd->redir) == -1)
		return (-1);
	return (0);
}

void	exit_child(t_data *data, t_cmd *head, t_pipes *pipeline, int status)
{
	if (head)
		free_cmd_list(head);
	if (pipeline)
		free_pipeline(pipeline);
	if (data)
		free_env_list(data->env);
	exit(status);
}

void	exec_child(t_data *data, t_cmd *head, t_pipes *pipeline, int i)
{
	t_cmd	*cmd;
	char	**envp;
	char	**args;
	char	*path;
	int		idx;

	cmd = head;
	idx = i;
	while (idx-- > 0)
		cmd = cmd->next;
	signal(SIGQUIT, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	if (prepare_child(data, cmd, pipeline, i) == -1)
		exit_child(data, head, pipeline, 1);
	if (is_builtin(cmd->cmd))
		exit_child(data, head, pipeline, execute_builtin(data, cmd));
	path = find_command_path(cmd->cmd, data->env);
	if (!path)
		exit_child(data, head, pipeline, 127);
	envp = env_to_array(data->env);
	args = get_execve_args(cmd);
	clean_child(data, head, pipeline);
	execve(path, args, envp);
	handle_exec_error(data, path, args, envp);
}

int	execute_pipeline(t_data *data, t_cmd *cmd)
{
	t_pipes	pipeline;

	if (!cmd || !cmd->cmd)
		return (1);
	if (!cmd->next)
		return (no_pipe(data, cmd));
	init_pipes(data, cmd, &pipeline);
	if (!pipeline.pipes)
		return (1);
	if (get_pids(data, cmd, &pipeline))
	{
		free_pipeline(&pipeline);
		return (1);
	}
	wait_pids(data, &pipeline);
	free_pipeline(&pipeline);
	return (data->exit_status);
}
