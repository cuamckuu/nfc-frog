KentMsc
=======

Msc projet at University of Kent.
By Alexis Guillard, Maxime Marches and Thomas Brunner

Inspired from readnfccc by R. Lifchitz then rewritten to read more information and be more generic.

Reads basic information from NFC-enabled credit cards (owner, PAN, expiry date, last payments).

Tested with french Visa and MasterCard

All information retrieved are stored in plaintext (nothing is 'hacked', decrypted or anything else)
Yes this is a HUGE PRIVACY ISSUE.

Information about protocols and data formats are available on emvco.com (http://www.emvco.com/specifications.aspx?id=223).
Great tools are supplied by emvlab.org, especially the TLV decoder (http://www.emvlab.org/tlvutils/).

Beware of the GPO command that inserts entries into the paylog, even though it doesnt invalidate the card (we tested it)
Beware, the output doesnt mask the PAN number.

To save records, redirect the standard output to a file.
Use at your own risk.

Feel free to contribute.

Have fun!
