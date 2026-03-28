NAME	= minishell
CC		= cc

UNAME	= $(shell uname)
ifeq ($(UNAME), Darwin)
	LDFLAGS	= -lreadline -L/opt/homebrew/opt/readline/lib
else
	LDFLAGS	= -lreadline
endif

LIBFT	= libft/libft.a
LIBPARSE= parse/libparse.a
LIBEXEC	= execute/libexec.a

all: $(NAME)

$(NAME): $(LIBFT) $(LIBPARSE) $(LIBEXEC)
	@$(CC) $(LIBEXEC) $(LIBPARSE) $(LIBFT) $(LDFLAGS) -o $(NAME)
	@echo " [ ok ] | $(NAME) is linked! "

$(LIBFT):
	@make -s -C libft bonus
	@echo " [ ok ] | all libft is ready! "

$(LIBPARSE):
	@make -s -C parse
	@echo " [ ok ] | all parse is ready! "

$(LIBEXEC):
	@make -s -C execute
	@echo " [ ok ] | all execute is ready! "

clean:
	@make -s -C parse clean
	@make -s -C execute clean
	@make -s -C libft clean
	@echo " [ ok ] | all files are clean! "

fclean:
	@make -s -C libft fclean
	@make -s -C parse fclean
	@make -s -C execute fclean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
