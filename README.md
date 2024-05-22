				UNDERVEST

This is an e-mail recipient address filter to be used in an MTA and able to:

- accept bad e-mail recipient addresses if they're close enough to a single
existing address,
- reject bad addresses in a relaying MTA front-end, providing the domain user
address list can be retrieved from the destination server via LDAP.
 
In the relaying case, this effectively replaces bounce e-mails by immediate
SMTP errors.

The fuzzy matching algorithms are tunable and may be used to implement a
limited range of mistyping error tolerance.

This filter is particularly suited for small to middle domains with a uniform
user e-mail address naming strategy. It is less appropriate for large domains
and/or random address naming.

The algorithms have been designed to only add a minimum overload to the MTA
server.


Note: To satisfy your own curiosity, the "undervest" name has been chosen after
the guy who did this job manually, named Marcel...


How to build an distribution tarball from a git clone.

1. $ git clone https://github.com/monnerat/undervest.git
2. $ cd undervest
3. $ autoreconf -fi
4. $ ./configure --prefix=/usr
5. $ make dist

Then find the tarball undervest-*.tar.gz in the current directory.


How to install from a distribution tarball:

1. Download the tarball, gunzip and untar
2. $ cd undervest-*
3. $ ./configure --prefix=/usr/
4. $ make
5. $ sudo make install

Then read undervest documentation and your MTA manual for configuration.


How to build an rpm package from a distribution tarball.

1. rpmbuild -ta undervest-*.tar.gz
