#	$NetBSD: Makefile,v 1.1 1997/11/17 01:36:00 cjs Exp $
# Build a smaller sh (e.g. for boot media)

# This can't be used, since crunchgen does not run this makefile
# from this directory.
#.CURDIR :=	${.CURDIR}/../../../bin/sh

# XXX Yes, this is really ugly.
.if !defined(BSDSRCDIR) || empty(BSDSRCDIR)
FOO!="***** error: BSDSRCDIR must be defined."
breakthingsnow
.endif
.CURDIR :=	${BSDSRCDIR}/bin/sh

NOMAN=1

CPPFLAGS+=	-I${.CURDIR}
CFLAGS+=	-DSMALL

.PATH: ${.CURDIR} ${.CURDIR}/bltin ${.CURDIR}/../../usr.bin/printf

.include "${.CURDIR}/Makefile"
