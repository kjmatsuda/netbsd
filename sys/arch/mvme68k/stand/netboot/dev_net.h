/*	$NetBSD: dev_net.h,v 1.2 1996/05/17 21:08:50 chuck Exp $	*/

int	net_open __P((struct open_file *, ...));
int	net_close __P((struct open_file *));
int	net_ioctl();
int	net_strategy();

