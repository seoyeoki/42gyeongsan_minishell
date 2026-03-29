/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_redir.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seoykim <seoykim@student.42gyeongsan.      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/29 19:56:29 by seoykim           #+#    #+#             */
/*   Updated: 2026/03/29 19:56:30 by seoykim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

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
