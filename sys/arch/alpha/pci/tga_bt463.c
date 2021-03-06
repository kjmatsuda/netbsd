/* $NetBSD: tga_bt463.c,v 1.5 1997/09/02 13:19:56 thorpej Exp $ */

/*
 * Copyright (c) 1995, 1996 Carnegie-Mellon University.
 * All rights reserved.
 *
 * Author: Chris G. Demetriou
 *
 * Permission to use, copy, modify and distribute this software and
 * its documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND
 * FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */

#include <sys/cdefs.h>			/* RCS ID & Copyright macro defns */

__KERNEL_RCSID(0, "$NetBSD: tga_bt463.c,v 1.5 1997/09/02 13:19:56 thorpej Exp $");

#include <sys/param.h>
#include <sys/device.h>

#include <dev/pci/pcivar.h>
#include <machine/tgareg.h>
#include <alpha/pci/tgavar.h>
#include <alpha/pci/bt485reg.h>

#include <machine/fbio.h>

const struct tga_ramdac_conf tga_ramdac_bt463 = {
	"Bt463",
#if 0
	NULL,				/* XXX SET CMAP */
	NULL,				/* XXX GET CMAP */
	tga_builtin_set_cursor,
	tga_builtin_get_cursor,
	tga_builtin_set_curpos,
	tga_builtin_get_curpos,
	tga_builtin_get_curmax,
#endif
};
