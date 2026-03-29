/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: seoyeoki <seoyeoki@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/25 00:00:00 by seoyeoki          #+#    #+#             */
/*   Updated: 2026/03/25 00:00:00 by seoyeoki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parse_int.h"

t_token	*handle_lex_error(t_lex *lx, t_data *data)
{
	free_tokens(lx->head);
	if (lx->error == 1)
	{
		err_unclosed_quote();
		data->exit_status = 1;
	}
	else if (lx->error == 2)
	{
		err_syntax_token(lx->err_token);
		free(lx->err_token);
		data->exit_status = 2;
	}
	return (NULL);
}
