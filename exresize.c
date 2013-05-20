#define EXPAND_LEFT  (1 << 0)
#define EXPAND_RIGHT (1 << 2)
#define EXPAND_UP    (1 << 4)
#define EXPAND_DOWN  (1 << 6)

#define IS_SET(q, w) ((q & w) != 0)
#define IS_FORCED(q, w) IS_SET((q << 1), w)

#define EXPANDALL       (EXPAND_LEFT | EXPAND_RIGHT | EXPAND_UP | EXPAND_DOWN)
#define UNEXPAND        (EXPANDALL << 1) // Force all directions to 0
#define FORCE_EXPANDALL ~0 // Force expand in all directions

enum { EX_NW, EX_N, EX_NE, EX_W, EX_C, EX_E, EX_SW, EX_S, EX_SE };
enum { EX_UP=1, EX_RIGHT=2, EX_DOWN=4, EX_LEFT=8 };

void expand(unsigned char mask);

void togglemaximize(const Arg *arg);
void toggleverticalexpand(const Arg *arg);
void togglehorizontalexpand(const Arg *arg);
void exresize(const Arg *arg);
void explace(const Arg *arg);
void exfocus(const Arg *arg);

Client *dirtoclient(char dir) {
    Client *c = selmon->sel, *r = NULL, *i;
    int cx, cy, dist = INT_MAX, adist = INT_MAX;
    if (!c) return NULL;
    cx = c->x+c->w/2; cy = c->y+c->h/2;
    for (i = selmon->clients ; i ; i = i->next) {
        if (i == c) continue;
        if (ISVISIBLE(i)) {
            int dx = i->x+i->w/2 - cx, dy = i->y+i->h/2 - cy;
            if ((dir&EX_UP    && (dy < 0))||(dir&EX_RIGHT && (dx > 0))||
                (dir&EX_DOWN  && (dy > 0))||(dir&EX_LEFT  && (dx < 0))){
                if ((abs(dx) < snap) && (abs(dy) < snap)) continue;
                if (dir&(EX_UP|EX_DOWN)) {
                    int ndist = abs(dy), nadist = abs(dx);
                    if (((abs(dist - ndist) < snap) && (nadist < adist)) || (dist > ndist)) {
                        dist = ndist;
                        adist = nadist;
                        r = i;
                    }
                } else if (dir&(EX_RIGHT|EX_LEFT)) {
                    if (dist > abs(dx)) {
                        int ndist = abs(dx), nadist = abs(dy);
                        if (((abs(dist - ndist) < snap) && (nadist < adist)) || (dist > ndist)) {
                            dist = ndist;
                            adist = nadist;
                            r = i;
                        }
                    }
                }
            }
        }
    }
    return r;
}

void
exfocus(const Arg *arg) {
    Client *c;
    if (arg->i == 0) {
        Client *i = selmon->sel;
        int ix, iy;
        if (!i) return;
		detach(i);
		attachback(i);
        ix = i->x + i->w/2; iy = i->y + i->h/2;
		for (c = selmon->clients ; c && (!ISVISIBLE(c) || abs(c->x + c->w/2 - ix) > snap || abs(c->y + c->h/2 - iy) > snap) ; c = c->next); 
    } else {
        c = dirtoclient(arg->i);
    }
    if (c) {
        focus(c);
        restack(selmon);
    }
}

void
exresize(const Arg *arg) {
	Client *c;
	int x, y, nx, ny, nw, nh;
	c = selmon->sel;

	if (!c || !arg) return;
	if (selmon->lt[selmon->sellt]->arrange && !c->isfloating)
		togglefloating(NULL);
	if (c->expandmask != 0)
		expand(UNEXPAND);

	x = ((int *)arg->v)[0];
	y = ((int *)arg->v)[1];

	nw = MIN(selmon->ww - c->bw*2, c->w + x);
	nh = MIN(selmon->wh - c->bw*2, c->h + y);
	nx = c->x - x/2;
	ny = c->y - y/2;

	if (!((abs(c->x + c->w/2 - (selmon->wx + selmon->ww/2)) < snap))) {
		if ((nw == selmon->ww) || 
				(nx < selmon->wx) ||
				(abs(selmon->wx - c->x) < snap))
			nx = selmon->wx;
		else if ((nx+nw > (selmon->wx + selmon->ww)) || 
				(abs((selmon->wx + selmon->ww) - (c->x + c->w)) < snap))
			nx = (selmon->wx + selmon->ww) - nw - c->bw*2;
    } else 
		nx = selmon->wx + selmon->ww/2 - nw/2;

    if (!((abs(c->y + c->h/2 - (selmon->wy + selmon->wh/2)) < snap))) {

		if ((nh == selmon->wh) || 
				(ny < selmon->wy) ||
				(abs(selmon->wy - c->y) < snap))
			ny = selmon->wy;
		else if ((ny+nh > (selmon->wy + selmon->wh)) ||
				(abs((selmon->wy + selmon->wh) - (c->y + c->h)) < snap))
			ny = (selmon->wy + selmon->wh) - nh - c->bw*2;
	} else 
        ny = selmon->wy + selmon->wh/2 - nh/2;


	resizeclient(c, nx, ny, MAX(nw, 32), MAX(nh, 32));
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
}

void
explace(const Arg *arg) {
	Client *c, *i;
	int nx, ny;

	c = selmon->sel;
	if (!c || (arg->ui >= 9)) return;
	if (selmon->lt[selmon->sellt]->arrange && !c->isfloating)
		togglefloating(NULL);

	nx = (arg->ui % 3) - 1;
	ny = (arg->ui / 3) - 1;

	if (nx < 0) nx = selmon->wx;
	else if (nx > 0) nx = selmon->wx + selmon->ww - c->w - c->bw*2;
	else nx = selmon->wx + selmon->ww/2 - c->w/2;

	if (ny < 0) ny = selmon->wy;
	else if (ny > 0) ny = selmon->wy + selmon->wh - c->h - c->bw*2;
	else ny = selmon->wy + selmon->wh/2 - c->h/2;

    for (i = selmon->clients ; i ; i = i->next) {
        if (i == c) continue;
        if (ISVISIBLE(i) && ((i->x < nx + c->w) && (nx < i->x + i->w) &&  (i->y < ny + c->h) &&  (ny < i->y + i->h))) {
            if (abs(ny - (i->y + i->h)) < snap*2) 
                ny = MIN(i->y + i->h, selmon->wy + selmon->wh - c->h - c->bw*2);
            if (abs(ny + c->h - i->y) < snap*2) 
                ny = MAX(i->y - c->h, selmon->wy);
            if (abs(nx - (i->x + i->w)) < snap*2) 
                nx = MIN(i->x + i->w, selmon->wx + selmon->ww - c->w - c->bw*2);
            if (abs(nx + c->w - i->x) < snap*2) 
                nx = MAX(i->x - c->w, selmon->wx);
        }
    }

	resize(c, nx, ny, c->w, c->h, True);
	XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w/2, c->h/2);
}

int
calculate_expand(unsigned char mask, unsigned char curmask,
		unsigned char *newmask, unsigned char key,
		int *reset_value, int new_reset_value,
		int max_value, int old_value) {
	if (IS_SET(key, mask) ||
			(IS_SET(key, curmask) && (!IS_SET(key, mask) && IS_FORCED(key, mask))) ||
			(!IS_SET(key, curmask) && (IS_SET(key, mask) && IS_FORCED(key, mask)))) {

		if (IS_SET(key, mask) && (!IS_SET(key,curmask) || IS_FORCED(key,mask)))
		{
			if (!IS_SET(key, curmask))
				*reset_value = new_reset_value;
			*newmask |= key;
			return max_value;
		} else if ((IS_SET(key,curmask) && IS_SET(key, mask)) ||
				(!IS_SET(key, mask) && IS_FORCED(key, mask))) {
			*newmask &= ~key;
			return *reset_value;
		} else {
			*newmask &= ~key;
			return old_value;
		}
	} else
		return new_reset_value;
}

void
expand(unsigned char mask) {
	XEvent ev;
	int nx1, ny1, nx2, ny2;
	unsigned char curmask;
	unsigned char newmask;

	if(!selmon->sel || selmon->sel->isfixed)
		return;
	XRaiseWindow(dpy, selmon->sel->win);
	newmask = curmask = selmon->sel->expandmask;

	if (curmask == 0) {
		if(!selmon->lt[selmon->sellt]->arrange || selmon->sel->isfloating)
			selmon->sel->wasfloating = True;
		else {
			togglefloating(NULL);
			selmon->sel->wasfloating = False;
		}
	}

	nx1 = calculate_expand(mask, curmask, &newmask,
			EXPAND_LEFT, &selmon->sel->expandx1,
			selmon->sel->x,
			selmon->wx,
			selmon->sel->oldx);
	nx2 = calculate_expand(mask, curmask, &newmask,
			EXPAND_RIGHT, &selmon->sel->expandx2,
			selmon->sel->x + selmon->sel->w,
			selmon->wx + selmon->ww - 2*borderpx,
			selmon->sel->oldw + selmon->sel->x);
	ny1 = calculate_expand(mask, curmask, &newmask,
			EXPAND_UP, &selmon->sel->expandy1,
			selmon->sel->y,
			selmon->wy,
			selmon->sel->oldy);
	ny2 = calculate_expand(mask, curmask, &newmask,
			EXPAND_DOWN, &selmon->sel->expandy2,
			selmon->sel->y + selmon->sel->h,
			selmon->wy + selmon->wh - 2*borderpx,
			selmon->sel->oldh + selmon->sel->y);


	resizeclient(selmon->sel, nx1, ny1, MAX(nx2-nx1, 32), MAX(ny2-ny1, 32));

	if ((newmask == 0) && (!selmon->sel->wasfloating))
		togglefloating(NULL);
	selmon->sel->expandmask = newmask;
	drawbar(selmon);
	while(XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

void
togglemaximize(const Arg *arg) {
	if (arg->i > 0) expand(FORCE_EXPANDALL);
	else if (arg->i < 0) expand(UNEXPAND);
	else expand(EXPANDALL);
}

void
toggleverticalexpand(const Arg *arg) {
	if (arg->i < 0) expand(EXPAND_DOWN);
	else if (arg->i > 0) expand(EXPAND_UP);
	else expand(EXPAND_DOWN | EXPAND_UP);
}

void
togglehorizontalexpand(const Arg *arg) {
	if (arg->i < 0) expand(EXPAND_LEFT);
	else if (arg->i > 0) expand(EXPAND_RIGHT);
	else expand(EXPAND_LEFT | EXPAND_RIGHT);
}

