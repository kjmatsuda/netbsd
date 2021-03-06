/*	$NetBSD: null_subr.c,v 1.9 1997/09/10 13:44:22 christos Exp $	*/

/*
 * Copyright (c) 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software donated to Berkeley by
 * Jan-Simon Pendry.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	from: Id: lofs_subr.c,v 1.11 1992/05/30 10:05:43 jsp Exp
 *	@(#)null_subr.c	8.4 (Berkeley) 1/21/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/namei.h>
#include <sys/malloc.h>
#include <miscfs/specfs/specdev.h>
#include <miscfs/nullfs/null.h>

#define LOG2_SIZEVNODE 7		/* log2(sizeof struct vnode) */
#define	NNULLNODECACHE 16

/*
 * Null layer cache:
 * Each cache entry holds a reference to the lower vnode
 * along with a pointer to the alias vnode.  When an
 * entry is added the lower vnode is VREF'd.  When the
 * alias is removed the lower vnode is vrele'd.
 */

#define	NULL_NHASH(vp) \
	(&null_node_hashtbl[(((u_long)vp)>>LOG2_SIZEVNODE) & null_node_hash])
LIST_HEAD(null_node_hashhead, null_node) *null_node_hashtbl;
u_long null_node_hash;

void	nullfs_init __P((void));
static struct vnode *
	null_node_find __P((struct mount *, struct vnode *));
static int
	null_node_alloc __P((struct mount *, struct vnode *, struct vnode **));
/*
 * Initialise cache headers
 */
void
nullfs_init()
{

#ifdef NULLFS_DIAGNOSTIC
	printf("nullfs_init\n");		/* printed during system boot */
#endif
	null_node_hashtbl = hashinit(NNULLNODECACHE, M_CACHE, &null_node_hash);
}

/*
 * Return a VREF'ed alias for lower vnode if already exists, else 0.
 */
static struct vnode *
null_node_find(mp, lowervp)
	struct mount *mp;
	struct vnode *lowervp;
{
	struct null_node_hashhead *hd;
	struct null_node *a;
	struct vnode *vp;

	/*
	 * Find hash base, and then search the (two-way) linked
	 * list looking for a null_node structure which is referencing
	 * the lower vnode.  If found, the increment the null_node
	 * reference count (but NOT the lower vnode's VREF counter).
	 */
	hd = NULL_NHASH(lowervp);
loop:
	for (a = hd->lh_first; a != 0; a = a->null_hash.le_next) {
		if (a->null_lowervp == lowervp && NULLTOV(a)->v_mount == mp) {
			vp = NULLTOV(a);
			/*
			 * We need vget for the VXLOCK
			 * stuff, but we don't want to lock
			 * the lower node.
			 */
			if (vget(vp, 0)) {
				printf ("null_node_find: vget failed.\n");
				goto loop;
			};
			return (vp);
		}
	}

	return NULL;
}


/*
 * Make a new null_node node.
 * Vp is the alias vnode, lowervp is the lower vnode.
 * Maintain a reference to lowervp.
 */
static int
null_node_alloc(mp, lowervp, vpp)
	struct mount *mp;
	struct vnode *lowervp;
	struct vnode **vpp;
{
	struct null_node_hashhead *hd;
	struct null_node *xp;
	struct vnode *vp, *nvp;
	int error;
	extern int (**dead_vnodeop_p) __P((void *));

	if ((error = getnewvnode(VT_NULL, mp, null_vnodeop_p, &vp)) != 0)
		return (error);
	vp->v_type = lowervp->v_type;

	MALLOC(xp, struct null_node *, sizeof(struct null_node), M_TEMP,
	    M_WAITOK);
	if (vp->v_type == VBLK || vp->v_type == VCHR) {
		MALLOC(vp->v_specinfo, struct specinfo *,
		    sizeof(struct specinfo), M_VNODE, M_WAITOK);
		vp->v_rdev = lowervp->v_rdev;
	}

	vp->v_data = xp;
	xp->null_vnode = vp;
	xp->null_lowervp = lowervp;
	xp->null_flags = 0;
#ifdef DIAGNOSTIC
	xp->null_pid = -1;
	xp->null_lockpc = xp->null_lockpc2 = 0;
#endif
	/*
	 * Before we insert our new node onto the hash chains,
	 * check to see if someone else has beaten us to it.
	 * (We could have slept in MALLOC.)
	 */
	if ((nvp = null_node_find(mp, lowervp)) != NULL) {
		*vpp = nvp;

		/* free the substructures we've allocated. */
		FREE(xp, M_TEMP);
		if (vp->v_type == VBLK || vp->v_type == VCHR)
			FREE(vp->v_specinfo, M_VNODE);

		vp->v_type = VBAD;		/* node is discarded */
		vp->v_op = dead_vnodeop_p;	/* so ops will still work */
		vrele(vp);			/* get rid of it. */
		return (0);
	}

	/*
	 * XXX if it's a device node, it needs to be checkalias()ed.
	 * however, for locking reasons, that's just not possible.
	 * so we have to do most of the dirty work inline.  Note that
	 * this is a limited case; we know that there's going to be
	 * an alias, and we know that that alias will be a "real"
	 * device node, i.e. not tagged VT_NON.
	 */
	if (vp->v_type == VBLK || vp->v_type == VCHR) {
		struct vnode *cvp, **cvpp;

		cvpp = &speclisth[SPECHASH(vp->v_rdev)];
loop:
		for (cvp = *cvpp; cvp; cvp = cvp->v_specnext) {
			if (vp->v_rdev != cvp->v_rdev ||
			    vp->v_type != cvp->v_type)
				continue;

			/*
			 * Alias, but not in use, so flush it out.
			 */
			if (cvp->v_usecount == 0) {
				vgone(cvp);
				goto loop;
			}
			if (vget(cvp, 0))	/* can't lock; will die! */
				goto loop;
			break;
		}

		vp->v_hashchain = cvpp;
		vp->v_specnext = *cvpp;
		vp->v_specflags = 0;
		*cvpp = vp;
#ifdef DIAGNOSTIC
		if (cvp == NULLVP)
			panic("null_node_alloc: no alias for device");
#endif
		vp->v_flag |= VALIASED;
		cvp->v_flag |= VALIASED;
		vrele(cvp);
	}
	/* XXX end of transmogrified checkalias() */

	*vpp = vp;
	VREF(lowervp);	/* Extra VREF will be vrele'd in null_node_create */
	hd = NULL_NHASH(lowervp);
	LIST_INSERT_HEAD(hd, xp, null_hash);
	return (0);
}


/*
 * Try to find an existing null_node vnode refering
 * to it, otherwise make a new null_node vnode which
 * contains a reference to the lower vnode.
 *
 * >>> we assume that the lower node is already locked upon entry, so we mark
 * the upper node as locked too (if caller requests it). <<<
 */
int
null_node_create(mp, lowervp, newvpp, takelock)
	struct mount *mp;
	struct vnode *lowervp;
	struct vnode **newvpp;
	int takelock;
{
	struct vnode *aliasvp;

	if ((aliasvp = null_node_find(mp, lowervp)) != NULL) {
		/*
		 * null_node_find has taken another reference
		 * to the alias vnode.
		 */
#ifdef NULLFS_DIAGNOSTIC
		vprint("null_node_create: exists", aliasvp);
#endif
		/* VREF(aliasvp); --- done in null_node_find */
	} else {
		int error;

		/*
		 * Get new vnode.
		 */
#ifdef NULLFS_DIAGNOSTIC
		printf("null_node_create: create new alias vnode\n");
#endif

		/*
		 * Make new vnode reference the null_node.
		 */
		if ((error = null_node_alloc(mp, lowervp, &aliasvp)) != 0)
			return error;

		/*
		 * aliasvp is already VREF'd by getnewvnode()
		 */
	}

	vrele(lowervp);

#ifdef DIAGNOSTIC
	if (lowervp->v_usecount < 1) {
		/* Should never happen... */
		vprint("null_node_create: alias", aliasvp);
		panic("null_node_create: lower has 0 usecount.");
	};
#endif

#ifdef NULLFS_DIAGNOSTIC
	vprint("null_node_create: alias", aliasvp);
#endif
	/* lower node was locked: mark it as locked and take
	   upper layer lock */
	VTONULL(aliasvp)->null_flags |= NULL_LLOCK;
	if (takelock)
		VOP_LOCK(aliasvp);

	*newvpp = aliasvp;
	return (0);
}

#ifdef NULLFS_DIAGNOSTIC
struct vnode *
null_checkvp(vp, fil, lno)
	struct vnode *vp;
	char *fil;
	int lno;
{
	struct null_node *a = VTONULL(vp);
#ifdef notyet
	/*
	 * Can't do this check because vop_reclaim runs
	 * with a funny vop vector.
	 */
	if (vp->v_op != null_vnodeop_p) {
		printf ("null_checkvp: on non-null-node\n");
#ifdef notyet
		while (null_checkvp_barrier) /*WAIT*/ ;
#endif
		panic("null_checkvp");
	};
#endif
	if (a->null_lowervp == NULL) {
		/* Should never happen */
		int i; u_long *p;
		printf("vp = %p, ZERO ptr\n", vp);
		for (p = (u_long *) a, i = 0; i < 8; i++)
			printf(" %lx", p[i]);
		printf("\n");
		/* wait for debugger */
#ifdef notyet
		while (null_checkvp_barrier) /*WAIT*/ ;
#endif
		panic("null_checkvp");
	}
	if (a->null_lowervp->v_usecount < 1) {
		int i; u_long *p;
		printf("vp = %p, unref'ed lowervp\n", vp);
		for (p = (u_long *) a, i = 0; i < 8; i++)
			printf(" %lx", p[i]);
		printf("\n");
		/* wait for debugger */
#ifdef notyet
		while (null_checkvp_barrier) /*WAIT*/ ;
#endif
		panic ("null with unref'ed lowervp");
	};
#ifdef notyet
	printf("null %p/%d -> %p/%d [%s, %d]\n",
	        NULLTOV(a), NULLTOV(a)->v_usecount,
		a->null_lowervp, a->null_lowervp->v_usecount,
		fil, lno);
#endif
	return a->null_lowervp;
}
#endif
