# FuzzyFox modifications

You probably want the Fuzzyfox-rebase branch instead, although this
one was more extensively tested.

Are you a researcher using fuzzyfox to test attacks/clocks/timing/etc?
Awesome! Please contact me and read
https://www.usenix.org/system/files/conference/usenixsecurity16/sec16_paper_kohlbrenner.pdf
.

Fuzzyfox is a prototype of the Fermata model described in the USENIX
2016 paper and protects only against a subset of the possible clocks
in Firefox. It is also slightly unstable (ex: does not shut down
cleanly) and should not be used as anything but a research tool.


# Original README follows

An explanation of the Mozilla Source Code Directory Structure and links to
project pages with documentation can be found at:

    https://developer.mozilla.org/en/Mozilla_Source_Code_Directory_Structure

For information on how to build Mozilla from the source code, see:

    http://developer.mozilla.org/en/docs/Build_Documentation

To have your bug fix / feature added to Mozilla, you should create a patch and
submit it to Bugzilla (https://bugzilla.mozilla.org). Instructions are at:

    http://developer.mozilla.org/en/docs/Creating_a_patch
    http://developer.mozilla.org/en/docs/Getting_your_patch_in_the_tree

If you have a question about developing Mozilla, and can't find the solution
on http://developer.mozilla.org, you can try asking your question in a
mozilla.* Usenet group, or on IRC at irc.mozilla.org. [The Mozilla news groups
are accessible on Google Groups, or news.mozilla.org with a NNTP reader.]

You can download nightly development builds from the Mozilla FTP server.
Keep in mind that nightly builds, which are used by Mozilla developers for
testing, may be buggy. Firefox nightlies, for example, can be found at:

    https://archive.mozilla.org/pub/firefox/nightly/latest-mozilla-central/
            - or -
    http://nightly.mozilla.org/
