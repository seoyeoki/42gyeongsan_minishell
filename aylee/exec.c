/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/21 00:00:00 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 13:53:41 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

char	*find_command_path(char *cmd, t_env *env)
{
	t_env	*path_node;
	char	**dirs;
	char	*full_path;

	full_path = NULL;
	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, X_OK) == 0)
			return (ft_strdup(cmd));
		return (NULL);
	}
	path_node = find_env_node(env, "PATH");
	if (!path_node || !path_node->value)
		return (NULL);
	dirs = ft_split(path_node->value, ':');
	if (!dirs)
		return (NULL);
	if (make_right_path(cmd, dirs, &full_path))
		return (free_split(dirs), full_path);
	return (free_split(dirs), NULL);
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
			redir = redir->next;
			continue ;
		}
		if (fd == -1)
		{
			print_error(data, redir->file, errno, 1);
			return (-1);
		}
		if (redir->type == REDIR_IN)
			dup2(fd, STDIN_FILENO);
		else
			dup2(fd, STDOUT_FILENO);
		close(fd);
		redir = redir->next;
	}
	return (0);
}

void	close_all_pipes(int **pipes, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		close(pipes[i][0]);
		close(pipes[i][1]);
		i++;
	}
}

int	count_cmd(t_cmd *cmd)
{
	int	count;

	count = 0;
	while (cmd)
	{
		count++;
		cmd = cmd->next;
	}
	return (count);
}

char	**get_execve_args(t_cmd *cmd) //합치는 코드
{
	char	**args;
	int		argc;
	int		i;

	argc = 0;
	i = 0;
	while (cmd->argv && cmd->argv[argc])
		argc++;
	args = malloc(sizeof(char *) * (argc + 2));
	if (!args)
		return (NULL);
	args[0] = ft_strdup(cmd->cmd);
	if (!args[0])
		return (free(args), NULL);
	while (i < argc)
	{
		args[i + 1] = ft_strdup(cmd->argv[i]);
		if (!args[i + 1])
			return (free_split(args), NULL);
		i++;
	}
	args[argc + 1] = NULL;
	return (args);
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

void	exec_child(t_data *data, t_cmd *cmd, t_pipes *pipeline, int i)
{
	char	*cmd_path;
	char	**envp;
	char	**args;

	if (prepare_child(data, cmd, pipeline, i) == -1)
		exit(1);
	if (is_builtin(cmd->cmd))
		exit(execute_builtin(data, cmd));
	cmd_path = find_command_path(cmd->cmd, data->env);
	if (!cmd_path)
	{
		print_error_msg(data, cmd->cmd, "command not found", 127);
		exit(127);
	}
	envp = env_to_array(data->env);
	args = get_execve_args(cmd);
	execve(cmd_path, args, envp);
	print_error(data, cmd->cmd, errno, 126);
	free(cmd_path);
	free_split(args);
	free_split(envp);
	exit(126);
}

void	init_pipes(t_data *data, t_cmd *cmd, t_pipes *pipeline)
{
	int	i;

	pipeline->count = count_cmd(cmd);
	pipeline->pids = malloc(sizeof(pid_t) * pipeline->count);
	pipeline->pipes = malloc(sizeof(int *) * (pipeline->count - 1));
	if (!pipeline->pipes || !pipeline->pids)
	{
		print_error(data, "malloc", errno, 1);
		pipeline->pipes = NULL;
		return ;
	}
	i = 0;
	while (i < pipeline->count - 1)
	{
		pipeline->pipes[i] = malloc(sizeof(int) * 2);
		if (!pipeline->pipes[i] || pipe(pipeline->pipes[i]) == -1)
		{
			print_error(data, "pipe", errno, 1);
			close_all_pipes(pipeline->pipes, i);
			free(pipeline->pipes);
			pipeline->pipes = NULL;
			return ;
		}
		i++;
	}
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
