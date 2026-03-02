/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec2.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 18:50:42 by aylee             #+#    #+#             */
/*   Updated: 2026/03/02 15:47:00 by aylee            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

int	make_right_path(const char *cmd, char **path_dirs, char **full_path)
{
	char	*tmp;
	int		i;

	i = 0;
	while (path_dirs[i])
	{
		tmp = ft_strjoin(path_dirs[i], "/");
		*full_path = ft_strjoin(tmp, cmd);
		free(tmp);
		if (access(*full_path, X_OK) == 0)
			return (1);
		free(*full_path);
		*full_path = NULL;
		i++;
	}
	return (0);
}

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

void	sigint_handler(int signum) //////////임시 메모리 빵꾸남.
{
	(void)signum;
	write(STDOUT_FILENO, "\n", 1);
}

void	signal_in_message(int line_count, char *delim)
{
	ft_putstr_fd("minishell: warning: here-document at line ", STDOUT_FILENO);
	ft_putnbr_fd(line_count, STDOUT_FILENO);
	ft_putstr_fd(" delimited by end-of-file (wanted '", STDOUT_FILENO);
	ft_putstr_fd(delim, STDOUT_FILENO);
	ft_putstr_fd("')\n", STDOUT_FILENO);
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

int	collect_heredoc_fork(t_redir *redir, char *delim)
{
	pid_t	pid;
	int		status;
	int		fd[2];

	if (pipe(fd) == -1)
		return (-1);
	pid = fork();
	if (pid == -1)
		return (close(fd[0]), close(fd[1]), -1);
	else if (pid == 0)
	{
		close(fd[0]);
		heredoc_child(fd[1], delim);
	}
	close(fd[1]);
	signal(SIGINT, SIG_IGN);
	waitpid(pid, &status, 0);
	signal(SIGINT, sigint_handler); //따로 양식 맞춰서 넣어야함.
	if (WIFSIGNALED(status))
		return (close(fd[0]), -1);
	redir->fd = fd[0];
	return (0);
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

int	prepare_heredoc(t_data *data, t_cmd *cmd)
{
	t_redir	*redir;
	t_cmd	*cur;

	if (count_heredocs(cmd) > 16)
	{
		print_error_msg(data, "heredoc",
			"maximum here-document count exceeded", 1);
		return (-1);
	}
	cur = cmd;
	while (cur)
	{
		redir = cur->redir;
		while (redir)
		{
			if (redir->type == REDIR_HEREDOC)
			{
				if (collect_heredoc(redir) == -1)
					return (-1);
			}
			redir = redir->next;
		}
		cur = cur->next;
	}
	return (0);
}
