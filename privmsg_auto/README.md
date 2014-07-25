privmsg_auto
===========

privmsg\_auto is aimed to solve the problem encountered by those who have at least one client that cannot make use of the [privmsg](http://wiki.znc.in/Privmsg) module and who are therefore stuck using the [privmsg\_prefix](http://wiki.znc.in/Privmsg_prefix) module.

What privmsg\_auto does is use the CTCP VERSION reply from connecting clients to attempt to determine if the client can make use of the non-prefixed format, and if so send those.  If there is no reply, or the reply is not known to be from a client that can handle the non-prefixed format, messages are sent with a prefix.

Additionally, privmsg\_auto provides the following commands:

* Status - View if the current client is to receive prefixed, or non-prefixed privmsg
* Toggle - Force privmsg\_auto to change what it will send the current client

If you have a client that can accept non-prefixed privmsg but isn't being recognized, create an issue with a sample of your client's CTCP VERSION reply, or make a pull request to add the needed regex for it.  Adding `/msg *privmsg_prefix toggle` to any available auto-command feature in your client should work for a temporary (if annoying) workaround in the meanwhile.

Compiling
=

privmsg\_auto requires that you have Boost, because the default regex implementation in g++ (what I use) is horrendous and I couldn't get it to work. (Sorry!)

The way I've done it is to add `-std=c++11` to `CXXFLAGS` and `-lboost_regex` to `LIBS` before using `./configure`, and afterwards ensuring the correct `libboost_regex.so` file can be found somewhere from `LD_LIBRARY_PATH`
