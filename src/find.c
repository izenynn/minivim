#include <minivim.h>

/* editor find callback */
void editor_find_callback(char *query, int key) {
	static int last_match = -1;
	static int dir = 1;

	/* hl match saves to know which lines needs to be restored */
	static int saved_hl_line;
	static char *saved_hl = NULL;

	/* restore if it is something */
	if (saved_hl) {
		memcpy(g_e.row[saved_hl_line].hl, saved_hl, g_e.row[saved_hl_line].r_sz);
		free(saved_hl);
		saved_hl = NULL;
	}

	/* return if is a special action key */
	if (key == '\r' || key == '\x1b') {
		last_match = -1;
		dir = 1;
		return ;
	/* check direction */
	} else if (key == K_ARROW_RIGHT || key == K_ARROW_DOWN) {
		dir = 1;
	} else if (key == K_ARROW_LEFT || key == K_ARROW_UP) {
		dir = -1;
	} else {
		last_match = -1;
		dir = 1;
	}

	/* check last_match */
	if (last_match == -1) dir = 1;
	int cur = last_match;
	/* loop rows and search for next match */
	int i;
	for (i = 0; i < g_e.n_rows; i++) {
		/* move cur one row (loop) */
		cur += dir;
		/* handle special cases */
		if (cur == -1) cur = g_e.n_rows -1;
		else if (cur == g_e.n_rows) cur = 0;

		/* get row */
		e_row *row = &g_e.row[cur];
		/* check for match into row */
		char *match = strstr(row->rend, query);
		if (match) {
			/* update last match */
			last_match = cur;
			/* positionate cursor y on match */
			g_e.cy = cur;
			/* positionate cursor x on start of the match */
			g_e.cx = editor_row_rx_to_cx(row, match - row->rend);
			/* positionate match line in top of screen */
			g_e.y_off = g_e.n_rows;

			/* get saved hl (so we can restore it later) */
			saved_hl_line = cur;
			saved_hl = (char *)malloc(row->r_sz);
			memcpy(saved_hl, row->hl, row->r_sz);
			/* set hl color to match */
			memset(&row->hl[match - row->rend], HL_MATCH, strlen(query));

			break;
		}
	}
}

/* find string in editor */
void editor_find() {
	/* save cursor pos */
	int saved_cx = g_e.cx;
	int saved_cy = g_e.cy;
	int saved_x_off = g_e.x_off;
	int saved_y_off = g_e.y_off;

	/* ask for keyword and search in callback */
	char *query = editor_prompt("Search: %s", editor_find_callback);

	/* free */
	if (query) {
		free(query);
	/* if query is null is becouse we pressed ESC */
	} else {
		g_e.cx = saved_cx;
		g_e.cy = saved_cy;
		g_e.x_off = saved_x_off;
		g_e.y_off = saved_y_off;
	}
}
