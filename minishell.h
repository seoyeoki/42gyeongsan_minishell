/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aylee <aylee@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/12 00:00:00 by aylee             #+#    #+#             */
/*   Updated: 2026/03/29 18:42:05 by seoykim          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <fcntl.h>
# include <sys/wait.h>
# include <errno.h>
# include <signal.h>
# include <readline/readline.h>
# include <readline/history.h>
# include "./libft/libft.h"

extern volatile sig_atomic_t	g_signal;

typedef struct s_env
{
	char			*key;
	char			*value;
	struct s_env	*next;
}	t_env;

typedef struct s_data
{
	t_env	*env;
	int		exit_status;
}	t_data;

typedef enum e_redir_type
{
	REDIR_IN,
	REDIR_OUT,
	REDIR_APPEND,
	REDIR_HEREDOC,
	REDIR_PIPE
}	t_redir_type;

typedef struct s_redir
{
	t_redir_type	type;
	char			*file;
	int				fd;
	int				quoted;
	struct s_redir	*next;
}	t_redir;

typedef struct s_cmd
{
	char			*cmd;
	char			**argv;
	t_redir			*redir;
	struct s_cmd	*next;
}	t_cmd;

typedef struct s_pipes
{
	int		**pipes;
	int		count;
	pid_t	*pids;
}	t_pipes;

// env.c
t_env	*parse_env(char **envp);
void	free_env_list(t_env *head);
void	print_env_list(t_env *head);

// make_node.c
t_env	*create_env_node(const char *key, const char *value);
t_env	*find_env_node(t_env *head, const char *key);
t_env	*add_env_node(t_env **head, const char *key, const char *value);
void	free_env_node(t_env *node);
void	delete_env(t_env **head, char *key);

// built_in.c
int		builtin_pwd(t_data *data);
int		builtin_unset(t_data *data, char **args);
int		builtin_env(t_data *data, char **args);

// sub_built.c
int		builtin_echo(t_data *data, char **args);
int		builtin_cd(t_data *data, char **args);
int		execute_builtin(t_data *data, t_cmd *cmd);

// exit.c
int		builtin_exit(t_data *data, t_cmd *cmd, char **args);

// export.c
int		builtin_export(t_data *data, char **args);
int		set_env_var(t_data *data, const char *key, const char *value);
void	make_env_new(t_data *data, char **args, char *equal_sign, int i);

// utils.c
char	**env_to_array(t_env *env);
void	clean_up(t_data *data, t_cmd *cmd);
void	free_split(char **split);

// stderr.c
void	print_error(t_data *data, char *cmd, int err_num, int exit_code);
void	print_error_msg(t_data *data, char *cmd, char *msg, int exit_code);

// main.c
int		is_builtin(char *cmd);

// main_init.c
int		count_cmd(t_cmd *cmd);
void	init_data(t_data *data, char **envp);
void	close_all_pipes(int **pipes, int count);
void	init_pipes(t_data *data, t_cmd *cmd, t_pipes *pipeline);

// command.c
int		wait_child(t_data *data, pid_t pid);
int		execute_command(t_data *data, t_cmd *cmd);
void	update_exit_status(t_data *data, int status);

// exec.c
int		execute_pipeline(t_data *data, t_cmd *cmd);
void	exec_child(t_data *data, t_cmd *head, t_pipes *pipeline, int i);
void	exit_child(t_data *data, t_cmd *head, t_pipes *pipeline, int status);

// exec2.c
int		make_right_path(const char *cmd, char **path_dirs, char **full_path);
int		prepare_heredoc(t_data *data, t_cmd *cmd);
int		collect_heredoc_fork(t_redir *redir, t_data *data, t_cmd *head);
void	signal_in_message(int line_count, char *delim);

// exec_redir.c
int		set_fd_open(t_redir *redir);
int		apply_redir(t_data *data, t_redir *redir);
int		prepare_child(t_data *data, t_cmd *cmd, t_pipes *pipeline, int i);

// cmd_path.c
char	*find_command_path(char *cmd, t_env *env);
char	**get_execve_args(t_cmd *cmd);

// image.c
void	shell_init(void);

// heredoc.c
int		count_heredocs(t_cmd *cmd);
int		collect_heredoc(t_redir *redir, t_data *data, t_cmd *head);
void	heredoc_child(int write_fd, t_data *data, t_redir *redir, t_cmd *head);

// no_pipe.c
int		no_pipe(t_data *data, t_cmd *cmd);

// pipe.c
int		get_pids(t_data *data, t_cmd *cmd, t_pipes *pipeline);
int		wait_pids(t_data *data, t_pipes *pipeline);
void	free_pipeline(t_pipes *pipeline);

// parse/
typedef enum e_tok_type
{
	TOK_WORD,
	TOK_PIPE,
	TOK_REDIR_IN,
	TOK_REDIR_OUT,
	TOK_REDIR_APPEND,
	TOK_HEREDOC,
}	t_tok_type;

typedef struct s_token
{
	t_tok_type		type;
	char			*str;
	int				quoted;
	struct s_token	*next;
}	t_token;

// parse.c
t_cmd	*parse_pipeline(char *input, t_data *data);

// parse_utils.c
char	*str_append(char *s, char *add);
void	free_cmd_list(t_cmd *cmd);
void	free_redir_list(t_redir *redir);

// lexer.c
t_token	*lexer(char *input, t_data *data);

// lexer_utils.c
t_token	*new_token(t_tok_type type, char *str);
void	free_tokens(t_token *head);

// expand.c
char	*expand_dollar(char *input, int *i, t_data *data);
char	*expand_tilde(t_data *data);
char	*expand_line(char *line, t_data *data);

// signals.c
void	signal_interactive(void);

#endif
