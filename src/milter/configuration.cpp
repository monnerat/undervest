//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Configuration support methods.         *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#include <confuse.h>

#include <cstring>
#include <cstdarg>

#include <string>
#include <unordered_map>
#include <memory>
#include <cerrno>
#include <stdexcept>
#include <system_error>

#include "source.h"
#include "domain.h"
#include "configuration.h"
#include "globals.h"
#include "stringlib.h"
#include "support11.h"



static int	get_log_level(cfg_t * cfg,
			cfg_opt_t * opt, const char * value, void * result);


typedef struct {
	const char *	token;		// Symbolic name.
	unsigned int	value;		// Value.
}		literal_t;


static const literal_t	log_levels[] = {
	{	"emergency",		LOG_EMERG	},
	{	"alert",		LOG_ALERT	},
	{	"critical",		LOG_CRIT	},
	{	"error",		LOG_ERR		},
	{	"warning",		LOG_WARNING	},
	{	"notice",		LOG_NOTICE	},
	{	"information",		LOG_INFO	},
	{	"debug",		LOG_DEBUG	},
	{	(const char *) NULL,	0		}
};


static cfg_opt_t	source_opts[] = {
	CFG_STR((char *) "uri", (char *) "", CFGF_NONE),
	CFG_STR((char *) "loader", (char *) "sendmail", CFGF_NONE),
	CFG_BOOL((char *) "enabled", cfg_true, CFGF_NONE),
	CFG_END()
};


static cfg_opt_t	thresholds_opts[] = {
	CFG_FLOAT((char *) "n-gram", 0.8, CFGF_NONE),
	CFG_FLOAT((char *) "word", 0.5, CFGF_NONE),
	CFG_FUNC((char *) "include", cfg_include),
	CFG_END()
};


static cfg_opt_t	domain_opts[] = {
	CFG_BOOL((char *) "enabled", cfg_true, CFGF_NONE),
	CFG_BOOL((char *) "autofix", cfg_false, CFGF_NONE),
	CFG_INT((char *) "refresh", 0, CFGF_NONE),
	CFG_INT((char *) "n-gram", 3, CFGF_NONE),
	CFG_SEC((char *) "thresholds", thresholds_opts, CFGF_NONE),
	CFG_SEC((char *) "source", source_opts, CFGF_MULTI),
	CFG_FUNC((char *) "include", cfg_include),
	CFG_END()
};


static cfg_opt_t	smtp_opts[] = {
	CFG_INT((char *) "status", 550, CFGF_NONE),
	CFG_STR((char *) "extended-status", (char *) "5.1.1", CFGF_NONE),
	CFG_STR((char *) "message", (char *) "No such recipient", CFGF_NONE),
	CFG_END()
};


static cfg_opt_t	log_opts[] = {
	CFG_INT_CB((char *) "minlevel", LOG_DEBUG, CFGF_NOCASE, get_log_level),
	CFG_INT_CB((char *) "maxlevel", LOG_EMERG, CFGF_NOCASE, get_log_level),
	CFG_END()
};


static cfg_opt_t	main_opts[] = {
	CFG_STR_LIST((char *) "loader-path", (char *) HANDLERDIR, CFGF_NONE),
	CFG_SEC((char *) "log", log_opts, CFGF_NONE),
	CFG_SEC((char *) "smtp", smtp_opts, CFGF_NONE),
	CFG_SEC((char *) "domain", domain_opts,
	    CFGF_TITLE | CFGF_MULTI | CFGF_NO_TITLE_DUPES),
	CFG_FUNC((char *) "include", cfg_include),
	CFG_END()
};



void
Configuration::config_error(cfg_t * cfg, const char * fmt, va_list args)

{
	char * s;
	std::string msg;
	char delim;

	if (vasprintf(&s, fmt, args) < 0)
		throw std::system_error(errno, std::system_category());

	try {
		msg = s;
		}
	catch (...) {
		free(s);
		throw;
		}

	free(s);
	delim = ':';

	if (cfg) {
		if (cfg->line) {
			msg = std::string("line ") + std::to_string(cfg->line) +
			    delim + " " + msg;
			delim = ',';
			}

		if (cfg->filename)
			msg = std::string("file ") + cfg->filename +
			    delim + " " + msg;
		}

	throw std::runtime_error(msg.c_str());
}


static const literal_t *
get_literal(const literal_t * table, const char * value, int ignorecase)

{
	int (* f)(const char *, const char *);

	f = ignorecase? strcasecmp: strcmp;

	for (; table->token; table++)
		if (!(*f)(value, table->token))
			return table;

	return (const literal_t *) NULL;
}


static int
get_log_level(cfg_t * cfg, cfg_opt_t * opt, const char * value, void * result)

{
	const literal_t * p;

	p = get_literal(log_levels, value, opt->flags & CFGF_NOCASE);

	if (!p) {
		cfg_error(cfg, "Invalid value `%s' for option `%s'",
		    value, opt->name);
		return -1;
		}

	*(int *) result = p->value;
	return 0;
}


Configuration::Configuration(Globals * gp): Logger(gp)

{
	glob = gp;
	domains.max_load_factor(0.8);
	smtp_status = 0;
	minloglevel = LOG_DEBUG;
	maxloglevel = LOG_EMERG;
	luserWarned = false;
}


void
Configuration::get(const std::string& configfile)

{
	cfg_t * main_cfg(NULL);
	cfg_t * smtp_cfg;
	cfg_t * log_cfg;
	cfg_t * domain_cfg;
	cfg_t * threshold_cfg;
	cfg_t * source_cfg;
	std::shared_ptr<Domain> dp;
	int n;
	int m;
	int i;
	int j;

	try {
		//	Parse the configuration file.

		main_cfg = cfg_init(main_opts, CFGF_NONE);

		if (!main_cfg)
			throw std::system_error(errno, std::system_category());

		cfg_set_error_function(main_cfg, config_error);

		switch (cfg_parse(main_cfg, configfile.c_str())) {

		case CFG_PARSE_ERROR:
			throw std::runtime_error(std::string("Error while "
			    "parsing configuration file ") + configfile);

		case CFG_FILE_ERROR:
			cfg_parse_buf(main_cfg, "");
			break;
			}

		//	Get global configuration.

		if (!cfg_size(main_cfg, "log"))
			cfg_parse_buf(main_cfg, "log {}");

		log_cfg = cfg_getsec(main_cfg, "log");
		minloglevel = cfg_getint(log_cfg, "minlevel");
		maxloglevel = cfg_getint(log_cfg, "maxlevel");

		if (!cfg_size(main_cfg, "smtp"))
			cfg_parse_buf(main_cfg, "smtp {}");

		smtp_cfg = cfg_getsec(main_cfg, "smtp");
		smtp_status = cfg_getint(smtp_cfg, "status");
		smtp_extended = cfg_getstr(smtp_cfg, "extended-status");
		smtp_message = cfg_getstr(smtp_cfg, "message");

		for (i = cfg_size(main_cfg, "loader-path"); i--;)
			loader_path.push_front(cfg_getnstr(main_cfg,
			    "loader-path", i));

		//	Get mail domains.

		n = cfg_size(main_cfg, "domain");

		if (!n) {
			cfg_parse_buf(main_cfg, "domain localhost {}");
			n = 1;
			}

		for (i = 0; i < n; i++) {
			domain_cfg = cfg_getnsec(main_cfg, "domain", i);

			if (!cfg_getbool(domain_cfg, "enabled"))
				continue;

			dp = std::make_shared<Domain>(this,
			    cfg_title(domain_cfg));
			dp->ngram_size = cfg_getint(domain_cfg, "n-gram");
			dp->refresh_sec = cfg_getint(domain_cfg, "refresh");
			dp->fixRecipient = cfg_getbool(domain_cfg, "autofix");
			threshold_cfg = cfg_getsec(domain_cfg, "thresholds");
			dp->ngramThreshold =
			    cfg_getfloat(threshold_cfg, "n-gram");
			dp->wordThreshold =
			    cfg_getfloat(threshold_cfg, "word");

			//	Get data sources for that domain.

			m = cfg_size(domain_cfg, "source");

			if (!m) {
				cfg_parse_buf(domain_cfg,
				    "source { uri = /etc/passwd }");
				cfg_parse_buf(domain_cfg,
				    "source { uri = /etc/aliases }");
				m = 2;
				}

			for (j = 0; j < m; j++) {
				source_cfg = cfg_getnsec(domain_cfg,
				    "source", j);

				if (!cfg_getbool(source_cfg, "enabled"))
					continue;

				Source source(&(*dp));
				source.uri = cfg_getstr(source_cfg, "uri");
				source.loader = cfg_getstr(source_cfg,
				    "loader");
				dp->sources.push_back(source);
				}

			//	If no data source, ignore domain.

			if (dp->sources.empty())
				dp->log("no data source: domain ignored");
			else if (!um_emplace(domains, dp->name, dp).second)
				dp->log("Duplicate domain definition");

			dp = std::shared_ptr<Domain>();	// Release if orphan.
			}

		cfg_free(main_cfg);
		main_cfg = NULL;

		//	Open data source loaders and get initial data.

		for (auto it = domains.begin(); it != domains.end(); it++)
			(*it).second->load_loaders();

		if (!domains.size())
			log_throw("no domain configured", LOG_INFO);
		}
	catch (AlreadyLogged& e) {
		if (main_cfg)
			cfg_free(main_cfg);

		throw e;
		}
	catch (std::exception& e) {
		log(e.what());

		if (main_cfg)
			cfg_free(main_cfg);

		throw AlreadyLogged();
		}
	catch (...) {
		log("An unknown exception occurred");

		if (main_cfg)
			cfg_free(main_cfg);

		throw AlreadyLogged();
		}
}


void
Configuration::log(const std::string& msg, int level)

{
	//	Level higher severity is lower integer...

	if (level < maxloglevel)
		level = maxloglevel;

	if (level > minloglevel)
		level = minloglevel;

	up->log(msg, level);
}


std::string
Configuration::recipient(const std::string& emailaddr,
						bool * fixit, bool inError)

{
	std::string addr(trim(emailaddr));
	size_t atpos;
	std::shared_ptr<Domain> dom;
	const std::string * rcpt;
	std::string domain;
	std::string origrcpt;
	bool hasltgt;

	hasltgt = addr.length() &&
	    addr[0] == '<' && addr[addr.length() - 1] == '>';

	if (hasltgt)
		addr = trim(addr.substr(1, addr.length() - 2));

	atpos = addr.find('@');

	if (!atpos || atpos >= addr.length() - 1)
		return emailaddr;

	origrcpt = addr.substr(0, atpos);
	domain = addr.substr(atpos + 1);
	auto domit = domains.find(domain);

	if (domit == domains.end())
		return emailaddr;

	dom = (*domit).second;

	if (inError && !luserWarned) {
		dom->log("LUSER_RELAY should be configured in mailer to "
		    "properly process a local domain. Will do what I can",
		    LOG_WARNING);
		luserWarned = true;
		}

	try {
		rcpt = dom->recipient(origrcpt);
		}
	catch (AlreadyLogged& e) {
		return emailaddr;
		}
	catch (std::exception& e) {
		log(e.what());
		return emailaddr;
		}
	catch (...) {
		log("An unknown exception occurred");
		return emailaddr;
		}

	if (!rcpt)
		return "";

	if (fixit)
		*fixit = dom->fixRecipient;

	addr = *rcpt + "@" + domain;

	if (hasltgt)
		addr = "<" + addr + ">";

	return addr;
}
