/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   heredoc.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/02 19:17:25 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 19:19:11 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	count_heredocs(t_cmd *cmd)
{
	int		count;
	t_redir	*redir;

	count = 0;
	while (cmd)
	{
		redir = cmd->redir;
		while (redir)
		{
			if (redir->type == REDIR_HEREDOC)
				count++;
			redir = redir->next;
		}
		cmd = cmd->next;
	}
	return (count);
}

void	heredoc_child(int write_fd, char *delim)
{
	char	*line;
	int		line_count;

	signal(SIGINT, SIG_DFL);
	line_count = 0;
	while (1)
	{
		line = readline("> ");
		if (!line)
		{
			signal_in_message(line_count, delim);
			break ;
		}
		if (ft_strncmp(line, delim, ft_strlen(delim) + 1) == 0)
		{
			free(line);
			break ;
		}
		write(write_fd, line, ft_strlen(line));
		write(write_fd, "\n", 1);
		free(line);
		line_count++;
	}
	close(write_fd);
	exit(0);
}

int	collect_heredoc(t_redir *redir)
{
	while (redir)
	{
		if (redir->type == REDIR_HEREDOC)
		{
			if (collect_heredoc_fork(redir, redir->file) == -1)
				return (-1);
		}
		redir = redir->next;
	}
	return (0);
}
