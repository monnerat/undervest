//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - Sendmail milter and context classes.   *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#ifndef __INCLUDE_MILTER_H__
#define __INCLUDE_MILTER_H__

#include <libmilter/mfapi.h>

#include <cstring>
#include <cstdlib>
#include <string>
#include <mutex>

#include "filepath.h"
#include "uri.h"
#include "stringlib.h"



class milter {
	std::string		_name;		// Filter name.
	std::string		_socket;	// Interface socket name.
	void *			_arg;
	int			_timeout;
	int			_debug;
	bool			_running;

	static std::mutex	_exclude;

protected:
	static milter *		_registered;


public:
	typedef struct {
		unsigned int	major;
		unsigned int	minor;
		unsigned int	patch;
	}		version;

	static constexpr unsigned long	allFlags =
	    SMFIF_ADDHDRS | SMFIF_CHGBODY | SMFIF_ADDRCPT | SMFIF_ADDRCPT_PAR |
	    SMFIF_DELRCPT | SMFIF_CHGHDRS | SMFIF_QUARANTINE | SMFIF_CHGFROM |
	    SMFIF_SETSYMLIST;


	class context {
		friend class milter;

		SMFICTX *	_context;

	public:
		context() {};

		static const unsigned long	callbackFlags = 0;
		static const unsigned long	flags = milter::allFlags;


		//	Default event handlers.

		virtual sfsistat
		connect(const char * hostname, const _SOCK_ADDR * hostaddr,
			void * arg)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		negotiate(unsigned long f0, unsigned long f1,
				unsigned long f2, unsigned long f3,
				unsigned long& rf0, unsigned long& rf1,
				unsigned long& rf2, unsigned long& rf3,
				void * arg)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		hello(const char * hellohost)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		envelopeFrom(const char * address, const char * const * argv)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		envelopeRecipient(const char * address,
						const char * const * argv)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		data()
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		header(const char * name, const char * value)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		endOfHeaders()
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		body(const char * bdy, size_t len)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		endOfMessage()
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		unknown(const char * command)
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		abort()
		{
			return SMFIS_CONTINUE;
		}

		virtual sfsistat
		close()
		{
			return SMFIS_CONTINUE;
		}


		//	Get the context object from the sendmail milter context.

		static context *
		milterContext(SMFICTX * ctx)
		{
			context * p((context *) ::smfi_getpriv(ctx));

			if (p)
				p->_context = ctx;

			return p;
		}


		//	Service methods.

		const char *
		symbolValue(const char * symname) const
		{
			return ::smfi_getsymval(_context, (char *) symname);
		}

		const char * symbolValue(const std::string& symname) const
		{
			return symbolValue(symname.c_str());
		}


		int
		reply(const char * rcode, const char * xcode,
						const char * message) const
		{
			return ::smfi_setreply(_context, (char *) rcode,
			    (char *) xcode, (char *) message);
		}

		int
		reply(const std::string& rcode, const std::string& xcode,
					const std::string& message) const
		{
			return reply(rcode.c_str(), xcode.c_str(),
			    message.c_str());
		}

		int
		reply(unsigned int rcode, const std::string& xcode,
					const std::string& message) const
		{
			return reply(std::to_string(rcode), xcode, message);
		}

		template<typename... Args>
		int
		reply(const char * rcode, const char * xcode,
				const char * message, Args&&... args) const
		{
			return ::smfi_setmlreply(_context, rcode, xcode,
			    message, args...);
		}


		int
		addHeader(const char * name, const char * value) const
		{
			return ::smfi_addheader(_context,
			    (char *) name, (char *) value);
		}

		int
		addHeader(const std::string& name,
						const std::string& value) const
		{
			return addHeader(name.c_str(), value.c_str());
		}


		int
		changeHeader(const char * name, unsigned int index,
						const char * value) const
		{
			return ::smfi_chgheader(_context,
			    (char *) name, (int) index + 1, (char *) value);
		}

		int
		changeHeader(const std::string& name, unsigned int index,
						const std::string& value) const
		{
			return changeHeader(name.c_str(), index, value.c_str());
		}


		int
		insertHeader(unsigned int index,
				const char * name, const char * value) const
		{
			return ::smfi_insheader(_context,
			    (int) index, (char *) name, (char *) value);
		}

		int
		insertHeader(unsigned int index,
			const std::string& name, const std::string& value) const
		{
			return insertHeader(index, name.c_str(), value.c_str());
		}


		int
		changeFrom(const char * mail, const char * args) const
		{
			return ::smfi_chgfrom(_context,
			    (char *) mail, (char *) args);
		}

		int
		changeFrom(const std::string& mail,
						const std::string& args) const
		{
			return changeFrom(mail.c_str(), args.c_str());
		}


		int
		addRecipient(const char * recipient,
						const char * args = NULL) const
		{
			if (!args || !args[0])
				return ::smfi_addrcpt(_context,
				    (char *) recipient);

			return ::smfi_addrcpt_par(_context, (char *) recipient,
			    (char *) args);
		}

		int
		addRecipient(const std::string& recipient,
					const std::string& args = "") const
		{
			return addRecipient(recipient.c_str(), args.c_str());
		}


		int
		deleteRecipient(const char * recipient) const
		{
			return ::smfi_delrcpt(_context, (char *) recipient);
		}

		int
		deleteRecipient(const std::string& recipient) const
		{
			return deleteRecipient(recipient.c_str());
		}


		int
		progress() const
		{
			return ::smfi_progress(_context);
		}


		int
		replaceBody(const unsigned char * bdy,
					std::string::size_type len) const
		{
			return ::smfi_replacebody(_context,
			    (unsigned char *) bdy, (int) len);
		}

		int
		replaceBody(const char * bdy, std::string::size_type len) const
		{
			return ::smfi_replacebody(_context,
			    (unsigned char *) bdy, (int) len);
		}

		int
		replaceBody(const std::string& recipient,
			std::string::size_type len = std::string::npos) const
		{
			if (len > recipient.length())
				len = recipient.length();

			return replaceBody(recipient.c_str(), len);
		}


		int
		quarantine(const char * reason) const
		{
			return ::smfi_quarantine(_context, (char *) reason);
		}

		int
		quarantine(const std::string& reason) const
		{
			return quarantine(reason.c_str());
		}

		int
		symbolList(int stage, const char * macros) const
		{
			return ::smfi_setsymlist(_context,
			    stage, (char *) macros);
		}

		int
		symbolList(int stage, const std::string& macros) const
		{
			return symbolList(stage, macros.c_str());
		}

		template<typename iteratorT>
		int
		symbolList(int stage,
			const iteratorT& first, const iteratorT& last) const
		{
			std::string macros;

			for (auto i = first; i != last; i++)
				macros += " " + *i;

			return symbolList(stage, macros.substr(1));
		}


		//	Exception logger.

		static void
		uncatched_exception(const std::string& msg, void * arg)
		{
			//	Default is a no-op.
		}


		//	Wrappers to milter class methods.

		version
		getVersion() const
		{
			return _registered->getVersion();
		}

		int
		stop()
		{
			return _registered->stop();
		}
	};


public:
	milter(): _timeout(-1), _debug(0), _running(false) {};

	milter(const std::string& filtername):
		_name(filtername), _timeout(-1), _debug(0), _running(false) {};

	~milter()
	{
		URI u(_socket);
		std::string t(strtolower(u.scheme()));

		if (_running)
			stop();

		if (t.empty() || t == "local" || t == "unix")
			::unlink(u.path().get().c_str());
	}


	const std::string& name() const { return _name; };

	std::string
	name(const std::string& filtername)
	{
		std::string old(_name);

		_name = filtername;
		return old;
	}


	const std::string& socket() const { return _socket; };

	std::string
	socket(const std::string& socketname)
	{
		std::string old(_socket);

		_socket = socketname;
		return old;
	}


	int timeout() const { return _timeout; };

	int
	timeout(int seconds)
	{
		int old(_timeout);

		_timeout = seconds;
		return old;
	}


	int debug() const { return _debug; };
	int debug(int level) { int old(_debug); _debug = level; return old; };


	version
	getVersion() const
	{
		version v;

		::smfi_version(&v.major, &v.minor, & v.patch);
		return v;
	}


	template<class _CTX>
	int
	initialize(bool delPrevSock = true)
	{
		int i;
		struct ::smfiDesc d;

		if (!_name.length())
			return MI_FAILURE;

		if (!_socket.length())
			_socket = (FilePath("/var/run") +
			    FilePath(_name).filename()).get() + ".sock";

		memset(&d, 0, sizeof d);
		d.xxfi_name = (char *) name().c_str();
		d.xxfi_version = SMFI_VERSION;
		d.xxfi_flags = _CTX::flags;
		d.xxfi_connect = _connect<_CTX>;
		d.xxfi_helo = _CTX::callbackFlags & SMFIP_NOHELO?
		    NULL: _helo<_CTX>;
		d.xxfi_envfrom = _CTX::callbackFlags & SMFIP_NOMAIL?
		    NULL: _envfrom<_CTX>;
		d.xxfi_envrcpt = _CTX::callbackFlags & SMFIP_NORCPT?
		    NULL: _envrcpt<_CTX>;
		d.xxfi_data = _CTX::callbackFlags & SMFIP_NODATA?
		    NULL: _data<_CTX>;
		d.xxfi_header = _CTX::callbackFlags & SMFIP_NOHDRS?
		    NULL: _header<_CTX>;
		d.xxfi_eoh = _CTX::callbackFlags & SMFIP_NOEOH? NULL:
		    _eoh<_CTX>;
		d.xxfi_body = _CTX::callbackFlags & SMFIP_NOBODY? NULL:
		    _body<_CTX>;
		d.xxfi_eom = _eom<_CTX>;
		d.xxfi_abort = _abort<_CTX>;
		d.xxfi_close = _close<_CTX>;
		d.xxfi_unknown = _CTX::callbackFlags & SMFIP_NOUNKNOWN?
		    NULL: _unknown<_CTX>;
		d.xxfi_negotiate = _negotiate<_CTX>;

		_exclude.lock();

		try {
			i = ::smfi_setconn((char *) _socket.c_str());

			if (i != MI_SUCCESS)
				throw(i);

			if (_timeout >= 0) {
				i = ::smfi_settimeout(_timeout);

				if (i != MI_SUCCESS)
					throw i;
				}

			i = ::smfi_register(d);

			if (i != MI_SUCCESS)
				throw i;
			}
		catch (int err) {
			_exclude.unlock();
			return err;
			}

		_registered = this;
		_exclude.unlock();
		return ::smfi_opensocket(delPrevSock);
	}

	int
	start(void * arg = NULL)
	{
		int i;

		_exclude.lock();

		if (_running || _registered != this) {
			_exclude.unlock();
			return MI_FAILURE;
			}

		_running = true;
		_arg = arg;
		_exclude.unlock();
		i = ::smfi_main();
		_running = false;
		return i;
	}

	int
	stop()
	{
		if (_running || _registered != this)
			return MI_FAILURE;

		return ::smfi_stop();
	}


	//	Static interfaces to event handlers.

	template<class _CTX>
	static void
	_uncatched_exception(const std::string funcname,
						const std::exception * e = NULL)
	{
		if (e)
			_CTX::uncatched_exception("An uncatched exception "
			    "occured in milter::" + funcname + "(): " +
			    e->what(), _registered->_arg);
		else
			_CTX::uncatched_exception("An uncatched unknown "
			    "exception occured in milter::" + funcname + "()",
			    _registered->_arg);
	}

	template<class _CTX>
	static sfsistat
	_connect(SMFICTX * ctx, char * hostname, _SOCK_ADDR * hostaddr)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

			try {
				p = (_CTX *) context::milterContext(ctx);

				if (p)
					i = p->connect(hostname, hostaddr,
					    _registered->_arg);
			else {
				p = new _CTX();
				p->_context = ctx;

				if (::smfi_setpriv(ctx, (void *) p) !=
				    MI_FAILURE)
					i = p->connect(hostname, hostaddr,
					    _registered->_arg);
				}
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("connect", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("connect");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_negotiate(SMFICTX * ctx, unsigned long f0, unsigned long f1,
	    unsigned long f2, unsigned long f3,
	    unsigned long * pf0, unsigned long * pf1,
	    unsigned long * pf2, unsigned long * pf3)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			*pf0 = _CTX::flags & f0;
			*pf1 = _CTX::callbackFlags & f1;
			*pf2 = *pf3 = 0;

			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->negotiate(f0, f1, f2, f3,
				    *pf0, *pf1, *pf2, *pf3, _registered->_arg);
			else {
				p = new _CTX();
				p->_context = ctx;

				if (::smfi_setpriv(ctx, (void *) p) !=
				    MI_FAILURE)
					i = p->negotiate(f0, f1, f2, f3,
					    *pf0, *pf1, *pf2, *pf3,
					    _registered->_arg);
				}
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("negotiate", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("negotiate");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_helo(SMFICTX * ctx, char * hellohost)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i =  p->hello(hellohost);
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("hello", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("hello");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_envfrom(SMFICTX * ctx, char * * argv)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->envelopeFrom(argv[0], argv + 1);
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("envelopeFrom", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("envelopeFrom");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_envrcpt(SMFICTX * ctx, char * * argv)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->envelopeRecipient(argv[0], argv + 1);
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("envelopeRecipient", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("envelopeRecipient");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_data(SMFICTX * ctx)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->data();
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("data", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("data");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_header(SMFICTX * ctx, char * name, char * value)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->header(name, value);
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("header", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("header");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_eoh(SMFICTX * ctx)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->endOfHeaders();
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("endOfHeaders", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("endOfHeaders");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_body(SMFICTX * ctx, unsigned char * bdy, size_t len)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->body((char *) bdy, len);
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("endOfHeaders", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("endOfHeaders");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_eom(SMFICTX * ctx)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->endOfMessage();
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("endOfMessage", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("endOfMessage");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_unknown(SMFICTX * ctx, const char * command)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->unknown(command);
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("unknown", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("unknown");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_abort(SMFICTX * ctx)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p)
				i = p->abort();
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("abort", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("abort");
			}

		return i;
	}

	template<class _CTX>
	static sfsistat
	_close(SMFICTX * ctx)
	{
		_CTX * p;
		int i(SMFIS_TEMPFAIL);

		try {
			p = (_CTX *) context::milterContext(ctx);

			if (p) {
				::smfi_setpriv(ctx, NULL);
				i = p->close();
				delete p;
				}
			}
		catch (std::exception& e) {
			_uncatched_exception<_CTX>("close", &e);
			}
		catch (...) {
			_uncatched_exception<_CTX>("close");
			}

		return i;
	}
};

#endif
