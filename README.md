# Solaris procutils

These programs are based in concept on two similar programs available with
most Linux distributions, but no source code from any other program was
consulted before, during, or after development.

This is likely of little use to anyone, but I wanted to get this code somewhere
other than a tarball on a web server that I may or may not continue to maintain
at any point in time.

## Build/Install
Edit `Makefile` to suit your likes, then type `make`.

## Notes

These programs are only known to work under Solaris 2.5.1, 2.6, [2.]7, and
[2.]8, but I would imagine that they also work under Solaris 2.5 and probably
even 2.4.  The 2.5.1 settings will compile and run under 2.6, and binaries
compiled under 2.5.1 also appear to work under 2.6 and later, but this still
requires a setuid-root installation for full functionality. This is, of
course, not recommended, as the code for 2.6 does not need to be run as root
due to the "new" procfs.

### Solaris 2.5.1 note
The programs should be installed setuid-root, but if this is not possible,
both programs will still function somewhat usefully. The limits are as
follows:

* `pidof` will not detect programs owned by other users. This
  also includes any setuid program whose real userid
  corresponds to the owner of the pidof process.

* `killcmd` will not be permitted to signal any setuid process
  whose real userid corresponds to the owner of the `killcmd`
  process, and the error reporting will be slightly different
  when one attempts to signal a process one does not own. Use
  of the `-q` option or a source modification is recommended for
  sanity's sake.
