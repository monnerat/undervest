//                                   \|||/
//                                  { o o }
//               *------------.oOO-----U-----OOo.------------*
//               *       D A T A S P H E R E   S . A .       *
//               *-------------------------------------------*
//               *                                           *
//               *                 UNDERVEST                 *
//               *                                           *
//               *  - LDAP recipient data importer.          *
//               *                                           *
//               *-------------------------------------------*
//               *                                           *
//               * Copyright (c) 2014 Datasphere S.A.        *
//               *                                           *
//               *   This software is licensed as described  *
//               * in the file LICENSE, which you should     *
//               * have received as part of this             *
//               * distribution.                             *
//               *   You may opt to use, copy, modify,       *
//               * merge, publish, distribute and/or sell    *
//               * copies of this software, and permit       *
//               * persons to whom this software is          *
//               * furnished to do so, under the terms of    *
//               * the LICENSE file.                         *
//               *   This software is distributed on an      *
//               * "AS IS" basis, WITHOUT WARRANTY OF ANY    *
//               * KIND, either express or implied.          *
//               *                                           *
//               *-------------------------------------------*
//               * CREATION                                  *
//               *   P. MONNERAT                  26/02/2014 *
//               *--------------.oooO-----Oooo.--------------*
//
//******************************************************************************

#include <sys/time.h>

#include <cstdlib>
#include <cstring>

#include <ldap.h>
#include <lber.h>

#include <string>
#include <exception>
#include <stdexcept>

#include "source.h"
#include "uri.h"



class ldap_error: public std::exception {
	int		code;

public:
	ldap_error(int error_code) { code = error_code; };
	virtual const char *	what() const throw()
		{ return ldap_err2string(code); };
};


class ldap_handle {

public:
	Source *	source;		// Our source.
	std::string	binddn;		// The bind distinguished name.
	std::string	password;	// The password.
	LDAP *		ldap;		// openldap handle.
	LDAPURLDesc *	uri;		// Parsed LDAP URI.
	LDAPMessage *	msg;		// Current LDAP response message.
	BerElement *	ber;		// Current BER element pointer.
	struct berval *	values;		// Result attribute values.
	struct berval *	value;		// Current result attribute value.

	ldap_handle();
	virtual ~ldap_handle();

	void		release_message();
};


static const char	mailchars[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-+_.{}@";



void
ldap_handle::release_message()

{
	if (msg)
		ldap_msgfree(msg);

	if (ber)
		ber_free(ber, 0);

	if (values)
		ber_memfree(values);

	msg = NULL;
	ber = NULL;
	values = NULL;
	value = NULL;
}


ldap_handle::~ldap_handle()

{
	release_message();

	if (ldap)
		ldap_unbind_ext(ldap, NULL, NULL);

	if (uri)
		ldap_free_urldesc(uri);
}


ldap_handle::ldap_handle()

{
	source = NULL;
	ldap = NULL;
	uri = NULL;
	msg = NULL;
	ber = NULL;
	values = NULL;
	value = NULL;
}


extern "C" void *
openSource(Source * source)

{
	URI u;
	ldap_handle * h;
	int version;
	int rc;
	int i;
	char * s;
	int msgid;
	std::string locfilter;
	struct berval passwd;
	struct timeval tv;

	if (!source->uri.length())
		throw std::runtime_error("LDAP loader requires an LDAP URI");

	//	Create the handle.

	h = new ldap_handle();
	h->source = source;

	//	Extract the binddn:password from the URI and rebuild
	//		an authentication-less URI.

	try {
		if (!u.parse(source->uri))
			throw std::runtime_error("Cannot parse URI");

		if (u.user().empty() && u.password().empty())
			h->binddn = u.unescape(u.userinfo());
		else {
			h->binddn = u.user();
			h->password = u.password();
			}

		u.userinfo("");

		//	Parse the LDAP URI.

		if ((rc = ldap_url_parse(std::string(u).c_str(), &h->uri)) !=
		    LDAP_SUCCESS)
			throw ldap_error(rc);

		if (!h->uri->lud_attrs[0])
			throw std::runtime_error(
			    "LDAP url should select at least one attribute");

		//	Connect to LDAP server.

		rc = ldap_initialize(&h->ldap, ((std::string(h->uri->lud_scheme?
		    h->uri->lud_scheme: "ldap")) + "://" +
		    (h->uri->lud_host? h->uri->lud_host: "") +
		    (h->uri->lud_port? ":" +
		    std::to_string(h->uri->lud_port): "")).c_str());

		if (rc != LDAP_SUCCESS)
			throw ldap_error(rc);

		//	Set a very low timeout: connection should be quick.

		tv.tv_sec = 5;
		tv.tv_usec = 0;
		ldap_set_option(h->ldap, LDAP_OPT_NETWORK_TIMEOUT, &tv);

		//	Bind if needed.

		if (ldap_get_option(h->ldap,
		    LDAP_OPT_PROTOCOL_VERSION, &version) != LDAP_SUCCESS)
			version = LDAP_VERSION2;	// Bind is mandatory.

		rc = LDAP_SUCCESS;

		if (!h->binddn.length()) {
			if (version >= LDAP_VERSION2) {
				//	Anonymous bind is mandatory for
				//		version 2.

				rc = ldap_sasl_bind_s(h->ldap, "",
				    LDAP_SASL_SIMPLE, NULL, NULL, NULL, NULL);
				}
			}
		else {
			passwd.bv_val = (char *) h->password.c_str();
			passwd.bv_len = h->password.length();
			rc = ldap_sasl_bind_s(h->ldap, h->binddn.c_str(),
			    LDAP_SASL_SIMPLE, &passwd, NULL, NULL, NULL);
			}

		if (rc != LDAP_SUCCESS)
			throw ldap_error(rc);

		//	Update the given filter to not select entries not
		//		providing mail address attributes.

		locfilter = h->uri->lud_filter;

		if (locfilter.length()) {
			if (locfilter[0] != '(')
				locfilter = std::string("(") + locfilter + ")";

			locfilter = std::string("(&") + locfilter;
			}

		if (h->uri->lud_attrs[1])
			locfilter += "(|";

		for (i = 0; (s = h->uri->lud_attrs[i]); i++)
			locfilter = locfilter + "(" + s + "=*)";

		if (h->uri->lud_attrs[1])
			locfilter += ')';

		if (h->uri->lud_filter)
			locfilter += ')';

		rc = ldap_search_ext(h->ldap, h->uri->lud_dn, h->uri->lud_scope,
		    locfilter.c_str(), h->uri->lud_attrs, 0,
		    NULL, NULL, NULL, 0x7FFFFFFF, &msgid);

		if (rc != LDAP_SUCCESS)
			throw ldap_error(rc);
		}
	catch (...) {
		delete h;
		throw;
		}

	return h;
}


extern "C" void
closeSource(void * vh, Source * source)

{
	delete (ldap_handle *) vh;
}


extern "C" std::string
getRecipient(void * vh, Source * source)

{
	ldap_handle * h = (ldap_handle *) vh;
	int msgtype;
	int code;
	int rc;
	struct berval bv;
	std::string result;

	if (!h)
		return "";

	for (;;) {
		if (h->value && h->value->bv_val) {
			result = std::string(h->value->bv_val,
			    h->value->bv_len);
			h->value++;

			if (result.length() > 5 &&
			    !strncasecmp(result.c_str(), "smtp:", 5))
				result = result.substr(5);

			if (result.find_first_not_of(mailchars) !=
			    std::string::npos)
				continue;

			return result;
			}

		if (h->ber) {
			rc = ldap_get_attribute_ber(h->ldap, h->msg, h->ber,
			    &bv, &h->values);
			h->value = h->values;

			if (rc == LDAP_SUCCESS && bv.bv_val)
				continue;
			}

		h->release_message();

		rc = ldap_result(h->ldap,
		    LDAP_RES_ANY, LDAP_MSG_ONE, NULL, &h->msg);

		if (!h->msg)
			throw ldap_error(rc);

		msgtype = ldap_msgtype(h->msg);

		switch (msgtype) {

		case LDAP_RES_SEARCH_RESULT:
			rc = ldap_parse_result(h->ldap, h->msg, &code,
			    NULL, NULL, NULL, NULL, 0);

			if (rc != LDAP_SUCCESS)
				throw ldap_error(rc);

			if (code != LDAP_SUCCESS)
				throw ldap_error(code);

			return "";

		case LDAP_RES_SEARCH_ENTRY:
			break;

		default:
			continue;
			}

		rc = ldap_get_dn_ber(h->ldap, h->msg, &h->ber, &bv);

		if (rc != LDAP_SUCCESS)
			throw ldap_error(rc);
		}

	return "";
}
