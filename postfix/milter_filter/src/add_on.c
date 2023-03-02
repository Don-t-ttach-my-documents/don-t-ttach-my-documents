#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <libmilter/mfapi.h>
#include <curl/curl.h>

#include "parsing.h"

#ifndef bool
# define bool	int
# define TRUE	1
# define FALSE	0
#endif /* ! bool */


struct mlfiPriv
{
	char	*mlfi_fname;
	char	*mlfi_connectfrom;
	char	*mlfi_helofrom;
	FILE	*mlfi_fp; // Log in a file to keep a trace
	unsigned char *body;
	size_t	 bodyLen;
};

#define MLFIPRIV	((struct mlfiPriv *) smfi_getpriv(ctx))

extern sfsistat		mlfi_cleanup(SMFICTX *, bool);

sfsistat
mlfi_connect(ctx, hostname, hostaddr)
	 SMFICTX *ctx;
	 char *hostname;
	 _SOCK_ADDR *hostaddr;
{

	struct mlfiPriv *priv;
	char *ident;

	/* alloue mémoire privéz */
	priv = malloc(sizeof *priv);
	if (priv == NULL)
	{
		/* refus temporaire du message */
		return SMFIS_TEMPFAIL;
	}
	memset(priv, '\0', sizeof *priv);

	/* sauvegarde données privées */
	priv->bodyLen =0;
	smfi_setpriv(ctx, priv);
	ident = smfi_getsymval(ctx, "_");
	if (ident == NULL)
		ident = "???";
	if ((priv->mlfi_connectfrom = strdup(ident)) == NULL)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_helo(ctx, helohost)
	 SMFICTX *ctx;
	 char *helohost;
{

	size_t len;
	char *tls;
	char *buf;
	struct mlfiPriv *priv = MLFIPRIV;

	tls = smfi_getsymval(ctx, "{tls_version}");
	if (tls == NULL)
		tls = "No TLS";
	if (helohost == NULL)
		helohost = "???";
	len = strlen(tls) + strlen(helohost) + 3;
	if ((buf = (char*) malloc(len)) == NULL)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}
	snprintf(buf, len, "%s, %s", helohost, tls);
	if (priv->mlfi_helofrom != NULL)
		free(priv->mlfi_helofrom);
	priv->mlfi_helofrom = buf;

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_envfrom(ctx, argv)
	 SMFICTX *ctx;
	 char **argv;
{

	int fd = -1;
	int argc = 0;
	struct mlfiPriv *priv = MLFIPRIV;
	char *mailaddr = smfi_getsymval(ctx, "{mail_addr}");

	/* ouvre un fichier pour garder le message */
	if ((priv->mlfi_fname = strdup("/tmp/msg.XXXXXX")) == NULL)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	if ((fd = mkstemp(priv->mlfi_fname)) == -1)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	if ((priv->mlfi_fp = fdopen(fd, "w+")) == NULL)
	{
		(void) close(fd);
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* compte les arguments */
	while (*argv++ != NULL)
		++argc;

	/* log les informations de connexions stockées */
	if (fprintf(priv->mlfi_fp, "Connect from %s (%s)\n\n",
		    priv->mlfi_helofrom, priv->mlfi_connectfrom) == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}
	/* log l'expéditeur */
	if (fprintf(priv->mlfi_fp, "FROM %s (%d argument%s)\n",
		    mailaddr ? mailaddr : "???", argc,
		    (argc == 1) ? "" : "s") == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_envrcpt(ctx, argv)
	 SMFICTX *ctx;
	 char **argv;
{

	struct mlfiPriv *priv = MLFIPRIV;
	char *rcptaddr = smfi_getsymval(ctx, "{rcpt_addr}");
	int argc = 0;

	/* compte les arguments */
	while (*argv++ != NULL)
		++argc;

	if (fprintf(priv->mlfi_fp, "RCPT %s (%d argument%s)\n",
		    rcptaddr ? rcptaddr : "???", argc,
		    (argc == 1) ? "" : "s") == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_header(ctx, headerf, headerv)
	 SMFICTX *ctx;
	 char *headerf;
	 unsigned char *headerv;
{
	/* écrit les headers dans le log */
	if (fprintf(MLFIPRIV->mlfi_fp, "%s: %s\n", headerf, headerv) == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_eoh(ctx)
	 SMFICTX *ctx;
{
	/* Retour à la ligne avant la fin de fichier */
	if (fprintf(MLFIPRIV->mlfi_fp, "\n") == EOF)
	{
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_body(ctx, bodyp, bodylen)
	 SMFICTX *ctx;
	 unsigned char *bodyp;
	 size_t bodylen;
{
        struct mlfiPriv *priv = MLFIPRIV;
	
	/* log le corps du mail */
	if (fwrite(bodyp, bodylen, 1, priv->mlfi_fp) != 1)
	{
		/* échec du log */
		fprintf(stderr, "Couldn't write file %s: %s\n",
			priv->mlfi_fname, strerror(errno));
		(void) mlfi_cleanup(ctx, FALSE);
		return SMFIS_TEMPFAIL;
	}

	/* Stocke le mail dans une variable pour l'envoyer au parser */
	if (priv->body==NULL) {
		priv->body = malloc(bodylen);
		memcpy(priv->body, bodyp, bodylen);
		priv->bodyLen = bodylen;
	} else {
		priv->body = realloc(priv->body, priv->bodyLen + bodylen);
		memcpy(priv->body+priv->bodyLen, bodyp, bodylen);
		priv->bodyLen += bodylen;
	}

	/* continue processing */
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_eom(ctx)
	 SMFICTX *ctx;
{
	bool ok = TRUE;
	struct mlfiPriv *priv = MLFIPRIV;
	
	struct MemoryStruct *parsed = malloc(sizeof(struct MemoryStruct));
	parsed->memory = malloc(1);
	parsed->size = 0;
	char *mailaddr = smfi_getsymval(ctx, "{mail_addr}");
	// Nouveau corps du mail
	if(sendBodyToParsing(priv->body, priv->bodyLen, parsed, mailaddr)==PARSING_OK){
		if (smfi_replacebody(ctx, parsed->memory, parsed->size)==MI_FAILURE){
			fprintf(stderr, "Replace body failed");
			ok = FALSE;
		}
	} else {
		fprintf(stderr, "Erreur parsing\n");
	}

	free(parsed->memory);
	free(parsed);

	return mlfi_cleanup(ctx, ok);
}

sfsistat
mlfi_abort(ctx)
	 SMFICTX *ctx;
{
	return mlfi_cleanup(ctx, FALSE);
}

sfsistat
mlfi_cleanup(ctx, ok)
	 SMFICTX *ctx;
	 bool ok;
{
	sfsistat rstat = SMFIS_CONTINUE;
	struct mlfiPriv *priv = MLFIPRIV;
	char *p;
	char host[512];
	char hbuf[1024];

	if (priv == NULL)
		return rstat;

	/* Fermeture du fichier de log */
	if (priv->mlfi_fp != NULL && fclose(priv->mlfi_fp) == EOF)
	{
		/* failed; we have to wait until later */
		fprintf(stderr, "Couldn't close archive file %s: %s\n",
			priv->mlfi_fname, strerror(errno));
		rstat = SMFIS_TEMPFAIL;
		(void) unlink(priv->mlfi_fname);
	}
	else if (ok)
	{
		/* ajout d'un header pour annoncer notre presence */
		if (gethostname(host, sizeof host) < 0)
			snprintf(host, sizeof host, "localhost");
		p = strrchr(priv->mlfi_fname, '/');
		if (p == NULL)
			p = priv->mlfi_fname;
		else
			p++;
		snprintf(hbuf, sizeof hbuf, "%s@%s", p, host);
	}
	else
	{
		/* message annulé -- suppression du fichier de log */
		fprintf(stderr, "Message aborted.  Removing %s\n",
			priv->mlfi_fname);
		rstat = SMFIS_TEMPFAIL;
		(void) unlink(priv->mlfi_fname);
	}

	/* Libération de la mémoire allouée */
	if (priv->mlfi_fname != NULL)
		free(priv->mlfi_fname);
	/* return status */
	return rstat;
}

sfsistat
mlfi_close(ctx)
	 SMFICTX *ctx;
{
	struct mlfiPriv *priv = MLFIPRIV;

	if (priv == NULL)
		return SMFIS_CONTINUE;
	if (priv->mlfi_connectfrom != NULL)
		free(priv->mlfi_connectfrom);
	if (priv->mlfi_helofrom != NULL)
		free(priv->mlfi_helofrom);
	if (priv->body!=NULL)
		free(priv->body);

	free(priv);
	smfi_setpriv(ctx, NULL);
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_unknown(ctx, cmd)
	SMFICTX *ctx;
	char *cmd;
{
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_data(ctx)
	SMFICTX *ctx;
{
	return SMFIS_CONTINUE;
}

sfsistat
mlfi_negotiate(ctx, f0, f1, f2, f3, pf0, pf1, pf2, pf3)
	SMFICTX *ctx;
	unsigned long f0;
	unsigned long f1;
	unsigned long f2;
	unsigned long f3;
	unsigned long *pf0;
	unsigned long *pf1;
	unsigned long *pf2;
	unsigned long *pf3;
{
	return SMFIS_ALL_OPTS;
}

struct smfiDesc smfilter =
{
	"AttachmentServerFilter",	/* nom du filtre */
	SMFI_VERSION,	/* version code -- ne pas changer */
	SMFIF_CHGBODY|SMFIF_ADDHDRS,
			/* flags */
	mlfi_connect,	/* connextion au filtre */
	mlfi_helo,	/* SMTP HELO commande filtre */
	mlfi_envfrom,	/* envelope expéditeur filtre */
	mlfi_envrcpt,	/* envelope destinataire filtre */
	mlfi_header,	/* header filtre */
	mlfi_eoh,	/* fin des headers */
	mlfi_body,	/* corps du mail filtre */
	mlfi_eom,	/* fin du message */
	mlfi_abort,	/* message annulé */
	mlfi_close,	/* connexion nettoyage */
	mlfi_unknown,	/* SMTP commandes inconnues */
	mlfi_data,	/* DATA command */
	mlfi_negotiate	/* Une seule fois, au début de chaque connexion smtp */
};

static void
usage(prog)
	char *prog;
{
	fprintf(stderr,
		"Usage: %s -p socket-addr [-t timeout]\n",
		prog);
}

int
main(argc, argv)
	 int argc;
	 char **argv;
{
	bool setconn = FALSE;
	int c;
	const char *args = "p:t:h";
	extern char *optarg;

	/* Processus command line options */

	while ((c = getopt(argc, argv, args)) != -1)
	{

		switch (c)
		{
		  case 'p':
			if (optarg == NULL || *optarg == '\0')
			{
				(void) fprintf(stderr, "Illegal conn: %s\n",
					       optarg);
				exit(EX_USAGE);
			}
			if (smfi_setconn(optarg) == MI_FAILURE)
			{
				(void) fprintf(stderr,
					       "smfi_setconn failed\n");
				exit(EX_SOFTWARE);
			}

			/*
			**  Si on utilise une socket local, vérifiez
			**  qu'elle n'existe pas. Ne jamais lancer ce
			**  code en tant que root !!
			*/

			if (strncasecmp(optarg, "unix:", 5) == 0)
				unlink(optarg + 5);
			else if (strncasecmp(optarg, "local:", 6) == 0)
				unlink(optarg + 6);
			setconn = TRUE;
			break;

		  case 't':
			if (optarg == NULL || *optarg == '\0')
			{
				(void) fprintf(stderr, "Illegal timeout: %s\n",
					       optarg);
				exit(EX_USAGE);
			}
			if (smfi_settimeout(atoi(optarg)) == MI_FAILURE)
			{
				(void) fprintf(stderr,
					       "smfi_settimeout failed\n");
				exit(EX_SOFTWARE);
			}
			break;

		  case 'h':
		  default:
			usage(argv[0]);
			exit(EX_USAGE);
		}
	}

	if (!setconn)
	{
		fprintf(stderr, "%s: Missing required -p argument\n", argv[0]);
		usage(argv[0]);
		exit(EX_USAGE);
	}
	if (smfi_register(smfilter) == MI_FAILURE)
	{
		fprintf(stderr, "smfi_register failed\n");
		exit(EX_UNAVAILABLE);
	}

	if(initLibcurl()==1)
		return -1;

	fprintf(stderr, "\n----------Demarrage du filtre--------\n");
	return smfi_main();
}
