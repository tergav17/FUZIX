/* UNIX V7 source code: see /COPYRIGHT or www.tuhs.org for details. */
/* Changes: Copyright (c) 1999 Robert Nordier.  All rights reserved.  */

#
/*
 * UNIX shell
 *
 * S. R. Bourne
 * Bell Telephone Laboratories
 *
 */

#include	"defs.h"
#include	"sym.h"

static CHAR	quote;	/* used locally */
static CHAR	quoted;	/* used locally */

static getch();
static comsubst();
static flush();


static STRING	copyto(endch)
	REG CHAR	endch;
{
	REG CHAR	c;

	WHILE (c=getch(endch))!=endch && c
	DO pushstak(c|quote) OD
	zerostak();
	IF c!=endch ) { error(badsub) ;}
}

static skipto(endch)
	REG CHAR	endch;
{
	/* skip chars up to } */
	REG CHAR	c;
	WHILE (c=readc()) && c!=endch
	DO	switch(c) {

		case SQUOTE:	skipto(SQUOTE); break;

		case DQUOTE:	skipto(DQUOTE); break;

		case DOLLAR:	IF readc()==BRACE
				) {	skipto('}');
				;}
		}
	OD
	IF c!=endch ) { error(badsub) ;}
}

static getch(endch)
	CHAR		endch;
{
	REG CHAR	d;

retry:
	d=readc();
	IF !subchar(d)
	) {	return(d);
	;}
	IF d==DOLLAR
	) {	REG INT	c;
		IF (c=readc(), dolchar(c))
		) {	NAMPTR		n=(NAMPTR)NIL;
			INT		dolg=0;
			BOOL		bra;
			REG STRING	argp, v;
			CHAR		idb[2];
			STRING		id=idb;

			IF bra=(c==BRACE) ) { c=readc() ;}
			IF letter(c)
			) {	argp=(STRING)relstak();
				WHILE alphanum(c) DO pushstak(c); c=readc() OD
				zerostak();
				n=lookup(absstak(argp)); setstak(argp);
				v = n->namval; id = n->namid;
				peekc = c|MARK;;
			} else if ( digchar(c)
			) {	*id=c; idb[1]=0;
				IF astchar(c)
				) {	dolg=1; c='1';
				;}
				c -= '0';
				v=((c==0) ? cmdadr : (c<=dolc) ? dolv[c] : (STRING)(dolg=0));
			} else if ( c=='$'
			) {	v=pidadr;
			} else if ( c=='!'
			) {	v=pcsadr;
			} else if ( c=='#'
			) {	v=dolladr;
			} else if ( c=='?'
			) {	v=exitadr;
			} else if ( c=='-'
			) {	v=flagadr;
			} else if ( bra ) { error(badsub);
			} else {	goto retry;
			;}
			c = readc();
			IF !defchar(c) && bra
			) {	error(badsub);
			;}
			argp=0;
			IF bra
			) {	IF c!='}'
				) {	argp=(STRING)relstak();
					IF (v==0)^(setchar(c))
					) {	copyto('}');
					} else {	skipto('}');
					;}
					argp=absstak(argp);
				;}
			} else {	peekc = c|MARK; c = 0;
			;}
			IF v
			) {	IF c!='+'
				) {	for (;;) {
				            WHILE c = *v++
					     DO pushstak(c|quote); OD
					     IF dolg==0 || (++dolg>dolc)
					     ) { break;
					     } else { v=dolv[dolg]; pushstak(SP|(*id=='*' ? quote : 0));
					     ;}
					}
				;}
			} else if ( argp
			) {	IF c=='?'
				) {	failed(id,*argp?argp:badparam);
				} else if ( c=='='
				) {	IF n
					) {	assign(n,argp);
					} else {	error(badsub);
					;}
				;}
			} else if ( flags&setflg
			) {	failed(id,badparam);
			;}
			goto retry;
		} else {	peekc=c|MARK;
		;}
	} else if ( d==endch
	) {	return(d);
	} else if ( d==SQUOTE
	) {	comsubst(); goto retry;
	} else if ( d==DQUOTE
	) {	quoted++; quote^=QUOTE; goto retry;
	;}
	return(d);
}

STRING	macro(as)
	STRING		as;
{
	/* Strip "" and do $ substitution
	 * Leaves result on top of stack
	 */
	REG BOOL	savqu =quoted;
	REG CHAR	savq = quote;
	FILEHDR		fb;

	push(&fb); estabf(as);
	usestak();
	quote=0; quoted=0;
	copyto(0);
	pop();
	IF quoted && (stakbot==staktop) ) { pushstak(QUOTE) ;}
	quote=savq; quoted=savqu;
	return(fixstak());
}

static comsubst()
{
	/* command substn */
	FILEBLK		cb;
	REG CHAR	d;
	REG STKPTR	savptr = fixstak();

	usestak();
	WHILE (d=readc())!=SQUOTE && d
	DO pushstak(d) OD

	{
	   REG STRING	argc;
	   trim(argc=fixstak());
	   push(&cb); estabf(argc);
	}
	{
	   REG TREPTR	t = makefork(FPOU,cmd(EOFSYM,MTFLG|NLFLG));
	   INT		pv[2];

	   /* this is done like this so that the pipe
	    * is open only when needed
	    */
	   chkpipe(pv);
	   initf(pv[INPIPE]);
	   execute(t, 0, 0, pv);
	   close(pv[OTPIPE]);
	}
	tdystak(savptr); staktop=movstr(savptr,stakbot);
	WHILE d=readc() DO pushstak(d|quote) OD
	await(0);
	WHILE stakbot!=staktop
	DO	IF (*--staktop&STRIP)!=NL
		) {	++staktop; break;
		;}
	OD
	pop();
}

#define CPYSIZ	512

subst(in,ot)
	INT		in, ot;
{
	REG CHAR	c;
	FILEBLK		fb;
	REG INT		count=CPYSIZ;

	push(&fb); initf(in);
	/* DQUOTE used to stop it from quoting */
	WHILE c=(getch(DQUOTE)&STRIP)
	DO pushstak(c);
	   IF --count == 0
	   ) {	flush(ot); count=CPYSIZ;
	   ;}
	OD
	flush(ot);
	pop();
}

static flush(ot)
{
	write(ot,stakbot,staktop-stakbot);
	IF flags&execpr ) { write(output,stakbot,staktop-stakbot) ;}
	staktop=stakbot;
}
