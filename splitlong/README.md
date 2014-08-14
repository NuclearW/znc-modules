splitlong
===========

splitlong is a module designed to break up messages that would be too long for the IRCd to send out to clients into smaller messages so that the entire message is sent.

Most clients these days are smart enough to break up the messages on their own, or can be made to do so with a script.  However, some fail spectacularly for CTCP ACTION, or just in general.  This module will perform the correct splitting before the message is sent to IRC.

Caveats
=
* If your hostmask is changed after you connect and ZNC has not updated it in memory, and your client is smart enough to cut messages on its own, **your messages may not be cut up properly!**
* If your client is not smart enough to cut up, but truncates to 512 anyway (irssi I'm looking at you) this module cannot help you.