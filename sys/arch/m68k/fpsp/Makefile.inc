#	$NetBSD: Makefile.inc,v 1.6 1997/09/21 17:33:36 is Exp $
#
#	Configuration variables (default values are below):
#
#	S	must be set to the top of the 'sys' tree.
#	FPSPDST	may be set to the location of the directory where library
#		objects are to be built.  Defaults to ${.OBJDIR}/lib/kern.
#	FPSP_AS	isn't there. We always build an object file.
#	FPSPMISCCPPFLAGS
#		Miscellaneous cpp flags to be passed to the library's Makefile
#		when building.
#	FPSPMISCMAKEFLAGS
#		Miscellaneous flags to be passed to the library's Makefile when
#		building.  See library's Makefile for more details about
#		supported flags and their default values.

# Default values:
FPSPDST?=	${.OBJDIR}/lib/fpsp

FPSPDIR=	$S/arch/m68k/fpsp
FPSPOBJ=	${FPSPDST}/fpsp.o
FPSP!=	printf "\#ifdef FPSP\n${FPSPOBJ}\n\#endif\n" | ${CPP} -P -undef ${CPPFLAGS:M-DFPSP}


#	    CC=${CC:Q} CFLAGS=${CFLAGS:Q} \
#	    AS=${AS:Q} AFLAGS=${AFLAGS:Q} \
#	    LD=${LD:Q} STRIP=${STRIP:Q} \

FPSPMAKE= \
	cd ${FPSPDIR} && MAKEOBJDIR=${FPSPDST} ${MAKE} .OBJDIR=${FPSPDST} \
	    MACHINE=${MACHINE} MACHINE_ARCH=${MACHINE_ARCH:Q} \
	    FPSPCPPFLAGS=${CPPFLAGS:S@^-I.@-I../../.@g:Q} \
	    FPSPMISCCPPFLAGS=${FPSPMISCCPPFLAGS:Q} \
	    FPSPDIR=${FPSPDIR:Q} \
	    ${FPSPMISCMAKEFLAGS}

${FPSP}:		.NOTMAIN __always_make_fpsp
	@echo making sure the 68040 FPSP is up to date...
	@${FPSPMAKE} fpsp.o

${FPSPLIB_PROF}:	.NOTMAIN __always_make_fpsp
	@echo making sure the profiled 68040 FPSP is up to date...
	@${FPSPMAKE} fpsp.po

clean:			.NOTMAIN cleanfpsp
cleanfpsp:		.NOTMAIN __always_make_fpsp
	@echo cleaning the 68040 FPSP objects
	@${FPSPMAKE} clean

#cleandir:		.NOTMAIN cleandirfpsp
#cleandirfpsp:	.NOTMAIN __always_make_fpsp
#	@echo cleandiring the 68040 FPSP objects
#	@${FPSPMAKE} cleandir

#depend:			.NOTMAIN dependfpsp
#dependfpsp:		.NOTMAIN __always_make_fpsp
#	@echo depending the 68040 FPSP objects
#	@${FPSPMAKE} depend
#

__always_make_fpsp:	.NOTMAIN
	@(mkdir -p ${FPSPDST})
