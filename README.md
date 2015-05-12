# LC RemoteHelp

LC RemoteHelp is a free solution for remote assistance.  It is based off of
the [TightVNC](http://tightvnc.org) version 2 source code, under the GPL v2+
license.  It consists of a modifed RFB server (LC RemoteHelp) which only does
outgoing client connections, a Python socket server called the connector, and
a modified viewer (LC RemoteHelp Viewer).  The connector Python source is
licensed under the BSD/ISC License.

The connector listens on port 5499 for LC RemoteHelp connections and on 5899
for LC RemoteHelp Viewer connections.  The connector is responsible for
issuing a Session ID and pairing up connections.

## Note:

To properly check out this repository, you will need to have a utf16 filter
in your `~/.gitconfig` file.

An example filter:

    [filter "utf16"]
        clean = iconv -f utf-16le -t utf-8
        smudge = iconv -f utf-8 -t utf-16le
        required

This project also requires at least Visual Studio 2013 to build.
