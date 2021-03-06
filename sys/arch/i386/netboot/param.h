/*	$NetBSD: param.h,v 1.3 1994/10/27 04:21:23 cgd Exp $	*/

/*
 * Tunable parameters
 */

/*
 * RARP as well as TFTP uses a timeout mechanism which starts with four
 * seconds and eventually go up as high as 4 << NRETRIES seconds. That
 * is, the number of retry iterations.
 */
#define	NRETRIES	6

#ifdef MONITOR
/*
 * BIOS break interrupt. This interrupt is generated by the
 * keyboard driver every time CTRL BREAK is pressed. We pick
 * it up to jump into the monitor.
 */
#define	BRK_INTR	0x1B

/*
 * Number of arguments in the users' command line
 */
#define	ARGVECSIZE	5
#endif /* MONITOR */
