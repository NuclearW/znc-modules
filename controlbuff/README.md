# controlbuff

controlbuff is for making cleaning up query buffers that pile up easier.  It allows you to pin the buffers you care about in a quick manner, and a command to clean up all unpinned buffers.

## Commands

* `List` - View a list of all query buffers
* `Pin <buffer> (<buffer> ...)` - Pin all buffers named, space separated
* `Unpin <buffer> (<buffer> ...)` - Unpin all buffers named, space separated
* `Clean` - Delete all unpinned buffers
* `Help` - View a list of commands

## Example

```
<nw`> list
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | Name      | Size | Max Size | Pinned |
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | somebloke | 105  | 500      |        |
<*controlbuff> | Buddy     | 100  | 500      |        |
<*controlbuff> | adude     | 47   | 100      |        |
<*controlbuff> | otherguy  | 66   | 100      |        |
<*controlbuff> +-----------+------+----------+--------+
<nw`> pin somebloke Buddy invalid otherguy
<*controlbuff> Pinned somebloke
<*controlbuff> Pinned Buddy
<*controlbuff> Could not find buffer: invalid
<*controlbuff> Pinned otherguy
<nw`> list
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | Name      | Size | Max Size | Pinned |
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | somebloke | 105  | 500      | *      |
<*controlbuff> | Buddy     | 100  | 500      | *      |
<*controlbuff> | adude     | 47   | 100      |        |
<*controlbuff> | otherguy  | 66   | 100      | *      |
<*controlbuff> +-----------+------+----------+--------+
<nw`> unpin otherguy
<*controlbuff> Unpinned otherguy
<nw`> list
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | Name      | Size | Max Size | Pinned |
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | somebloke | 105  | 500      | *      |
<*controlbuff> | Buddy     | 100  | 500      | *      |
<*controlbuff> | adude     | 47   | 100      |        |
<*controlbuff> | otherguy  | 66   | 100      |        |
<*controlbuff> +-----------+------+----------+--------+
<nw`> clean
<*controlbuff> Cleaned buffers.
<nw`> list
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | Name      | Size | Max Size | Pinned |
<*controlbuff> +-----------+------+----------+--------+
<*controlbuff> | somebloke | 105  | 500      | *      |
<*controlbuff> | Buddy     | 100  | 500      | *      |
<*controlbuff> +-----------+------+----------+--------+
```

## Notes

* Pinning of buffers does not persist, you will need to re-pin when znc restarts, or the module is reloaded for any reason.
* Pinning does not yet prevent a buffer from being removed automatically if znc runs into `MaxQueryBuffers` and needs to make room.